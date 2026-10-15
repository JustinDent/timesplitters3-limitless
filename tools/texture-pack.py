"""Compile an authored PNG into a compressed, fully mipmapped texture override.

This performs encoding/resampling, not artwork generation. Originals are never
overwritten. Runtime supports powers of two up to 4096 pixels on each axis.
"""
import argparse
import hashlib
import io
import json
import re
import struct
from pathlib import Path
from PIL import Image


def encode(image, pixel_format='DXT5'):
    w,h=image.size
    if not (w and h and w<=4096 and h<=4096 and not w&(w-1) and not h&(h-1)):
        raise ValueError('Dimensions must be powers of two, at most 4096.')
    pieces=[]
    while True:
        buf=io.BytesIO();image.save(buf,format='DDS',pixel_format=pixel_format)
        data=buf.getvalue()
        if not pieces: header=bytearray(data[:128])
        pieces.append(data[128:])
        if image.size==(1,1): break
        image=image.resize((max(1,image.width//2),max(1,image.height//2)),Image.Resampling.LANCZOS)
    struct.pack_into('<I',header,8,0xA1007)
    struct.pack_into('<I',header,20,len(pieces[0]))
    struct.pack_into('<I',header,28,len(pieces))
    struct.pack_into('<I',header,108,0x401008)
    return bytes(header)+b''.join(pieces),len(pieces)


def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('image',type=Path)
    ap.add_argument('--key',required=True)
    ap.add_argument('--output',type=Path,default=Path('private/texture-remaster/pack'))
    ap.add_argument('--width',type=int,required=True)
    ap.add_argument('--height',type=int,required=True)
    ap.add_argument('--format',choices=['DXT1','DXT5'],default='DXT5')
    ap.add_argument('--budget-mib',type=int,default=6144,help='Maximum total active pack size (default 6 GiB)')
    a=ap.parse_args()
    if not re.fullmatch('[0-9a-f]{16}',a.key):ap.error('key must be 16 lowercase hexadecimal characters')
    if a.budget_mib<=0:ap.error('budget must be positive')
    if (a.output/f'{a.key}.dds').exists():ap.error('This key already exists; preserve or explicitly retire the prior version first')
    w,h=a.width,a.height
    if not(w>0 and h>0 and w<=4096 and h<=4096 and not w&(w-1) and not h&(h-1)):
        ap.error('Dimensions must be powers of two up to 4096')
    expected=128
    while True:
        expected+=((w+3)//4)*((h+3)//4)*(8 if a.format=='DXT1' else 16)
        if w==h==1:break
        w=max(1,w//2);h=max(1,h//2)
    used=sum(p.stat().st_size for p in a.output.rglob('*') if p.is_file()) if a.output.exists() else 0
    if used+expected>a.budget_mib*1024*1024:
        ap.error(f'Pack budget exceeded: existing {used} + new {expected} bytes; reduce asset resolution or retire unused assets')
    image=Image.open(a.image).convert('RGBA');native=image.size
    if native[0]*a.height!=native[1]*a.width:ap.error('Resizing would change the aspect ratio')
    if native!=(a.width,a.height):image=image.resize((a.width,a.height),Image.Resampling.LANCZOS)
    data,mips=encode(image,a.format)
    meta=dict(key=a.key,source=str(a.image.resolve()),source_size=native,
        output_size=[a.width,a.height],resampled=native!=(a.width,a.height),
        format=a.format,mips=mips,bytes=len(data),sha256=hashlib.sha256(data).hexdigest())
    metadata=(json.dumps(meta,indent=2)+'\n').encode('utf-8')
    # The user's cap includes provenance files, not only GPU payloads.
    if used+len(data)+len(metadata)>a.budget_mib*1024*1024:
        ap.error('Pack budget exceeded including texture metadata')
    path=a.output/f'{a.key}.dds'
    if path.with_suffix('.json').exists():
        ap.error('Metadata for this key already exists; inspect it before replacing')
    a.output.mkdir(parents=True,exist_ok=True)
    with path.open('xb') as f:f.write(data)
    with path.with_suffix('.json').open('xb') as f:f.write(metadata)
    print(json.dumps(meta,indent=2))


if __name__=='__main__':main()
