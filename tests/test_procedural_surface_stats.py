import sys
from pathlib import Path
import pytest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from procedural_surface_stats import IMAGE_BYTES,image_hash,measure


def capture():
    data=[bytes([v])*IMAGE_BYTES for v in (10,10,20,20)]
    rows=[dict(sample=i,frame=100+i,scene=4,level_ticks=300+i//2,whole_ticks=i%2,
               delta=.5,native=1,resource=0x80001000,executed=int(i in (0,2)),format=6,
               hash=image_hash(image)) for i,image in enumerate(data)]
    return rows,b''.join(data)


def test_freshness_counts_retained_images():
    rows,pixels=capture();result=measure(rows,pixels)
    assert result['changed_images']==1 and result['unchanged_images']==2
    assert result['executed_changed']==1 and result['skipped_unchanged']==2
    assert result['game_ticks']==1.5


def test_executed_but_frozen_is_not_fresh():
    rows,pixels=capture();rows[1]['executed']=1
    assert measure(rows,pixels)['executed_unchanged']==1


@pytest.mark.parametrize('mutation',['short','hash','gap','resource','nan','scene'])
def test_invalid_evidence(mutation):
    rows,pixels=capture()
    if mutation=='short':pixels=pixels[:-1]
    if mutation=='hash':rows[2]['hash']='0'*16
    if mutation=='gap':rows[2]['frame']+=1
    if mutation=='resource':rows[2]['resource']+=4
    if mutation=='nan':rows[2]['delta']=float('nan')
    if mutation=='scene':rows[2]['scene']=30
    with pytest.raises(ValueError):measure(rows,pixels)


def test_channel_statistics_are_exact_for_flat_and_varying_bytes():
    rows,pixels=capture()
    result=measure(rows,pixels)
    assert result['channel_statistics']==[
        dict(mean=15,std=5,minimum=10,maximum=20,distinct=2)]*4
    pixel=bytes((255,128,128,255))
    frame=pixel*(IMAGE_BYTES//4)
    for row in rows:
        row['hash']=image_hash(frame)
        row['executed']=1
    result=measure(rows,frame*len(rows))
    assert result['changed_images']==0 and result['executed_unchanged']==3
    assert [v['mean'] for v in result['channel_statistics']]==[255,128,128,255]
    assert all(v['std']==0 and v['distinct']==1 for v in result['channel_statistics'])


def test_compressed_capture_preserves_bytes_and_checks_expanded_bound(tmp_path):
    import csv,gzip
    from procedural_surface_stats import load_capture,LIMIT
    rows,pixels=capture();path=tmp_path/'surface.csv'
    with path.open('w',newline='') as out:
        writer=csv.DictWriter(out,fieldnames=list(rows[0]));writer.writeheader();writer.writerows(rows)
    compressed=Path(str(path)+'.rgba.gz')
    compressed.write_bytes(gzip.compress(pixels))
    loaded,data=load_capture(path)
    assert data==pixels and measure(loaded,data)==measure(rows,pixels)
    compressed.write_bytes(gzip.compress(bytes(LIMIT*IMAGE_BYTES+1)))
    with pytest.raises(ValueError,match='Expanded'):load_capture(path)
