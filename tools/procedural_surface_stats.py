"""Validate bounded procedural GPU captures and measure actual image freshness.

The synchronous readback changes workload. No wall-clock FPS/scanout claims.
"""
import argparse
import csv
import json
import math
from pathlib import Path

IMAGE_BYTES=128*128*4
LIMIT=360


def image_hash(data):
    value=14695981039346656037
    for byte in data:
        value=((value ^ byte)*1099511628211)&0xffffffffffffffff
    return f'{value:016x}'


def measure(rows, pixels):
    if not 2<=len(rows)<=LIMIT or len(pixels)!=len(rows)*IMAGE_BYTES:
        raise ValueError('Incomplete or oversized image capture')
    images=[];typed=[]
    for i,row in enumerate(rows):
        value={key:int(row[key]) for key in ('sample','frame','scene','level_ticks','whole_ticks','native','resource','executed','format')}
        value['delta']=float(row['delta'])
        if value['sample']!=i or value['scene']!=4 or value['executed'] not in (0,1) or value['native'] not in (0,1):
            raise ValueError('Unexpected sample/scene/flags')
        if not math.isfinite(value['delta']) or not 0<=value['delta']<=15:
            raise ValueError('Invalid game delta')
        if i and (value['frame']!=(typed[-1]['frame']+1)&0xffffffff or any(value[k]!=typed[0][k] for k in ('resource','format','native'))):
            raise ValueError('Frame gap or changed resource/format/path')
        image=memoryview(pixels)[i*IMAGE_BYTES:(i+1)*IMAGE_BYTES]
        if image_hash(image)!=row['hash']:raise ValueError('Image hash mismatch')
        images.append(image);typed.append(value)
    changed=unchanged=executed_changed=executed_unchanged=skipped_changed=skipped_unchanged=0
    longest=streak=0
    for i in range(1,len(rows)):
        fresh=images[i]!=images[i-1]
        if fresh:changed+=1;streak=0
        else:unchanged+=1;streak+=1;longest=max(longest,streak)
        if typed[i]['executed']:
            if fresh:executed_changed+=1
            else:executed_unchanged+=1
        elif fresh:skipped_changed+=1
        else:skipped_unchanged+=1
    return dict(samples=len(rows),comparisons=len(rows)-1,changed_images=changed,unchanged_images=unchanged,
                executed_changed=executed_changed,executed_unchanged=executed_unchanged,
                skipped_changed=skipped_changed,skipped_unchanged=skipped_unchanged,
                longest_unchanged_updates=longest,game_ticks=sum(r['delta'] for r in typed[1:]),
                native=typed[0]['native'],scope='GPU readback image changes; not FPS, scanout or mechanics parity')


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('csv',type=Path)
    args=parser.parse_args()
    if args.csv.stat().st_size>128*1024:raise ValueError('CSV exceeds bound')
    path=Path(str(args.csv)+'.rgba')
    if path.stat().st_size>LIMIT*IMAGE_BYTES:raise ValueError('Pixels exceed bound')
    with args.csv.open(newline='') as source:rows=list(csv.DictReader(source))
    print(json.dumps(measure(rows,path.read_bytes()),indent=2))


if __name__=='__main__':main()
