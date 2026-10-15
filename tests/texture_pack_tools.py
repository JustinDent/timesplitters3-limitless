"""Offline texture tool checks; tiny synthetic images, no game-data copies."""
import importlib.util
import io
from pathlib import Path
import struct
import subprocess
import sys
import pytest
from PIL import Image

ROOT=Path(__file__).resolve().parents[1]
def module(name):
    spec=importlib.util.spec_from_file_location(name,ROOT/'tools'/f'{name}.py')
    m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
pack=module('texture-pack');catalog=module('texture-catalog')
worklist=module('texture-worklist')

def test_mip_chain_and_compressed_roundtrip():
    b,n=pack.encode(Image.new('RGBA',(8,4),(120,80,40,255)),'DXT1')
    assert n==4 and len(b)==128+16+8+8+8
    assert struct.unpack_from('<I',b,28)[0]==4
    color=Image.open(io.BytesIO(b)).convert('RGB').getpixel((0,0))
    assert all(abs(a-b)<9 for a,b in zip(color,(120,80,40)))

def test_alpha_and_bad_dimensions():
    b,n=pack.encode(Image.new('RGBA',(4,4),(80,80,80,0)),'DXT5')
    assert n==3 and Image.open(io.BytesIO(b)).getpixel((0,0))[3]==0
    with pytest.raises(ValueError):pack.encode(Image.new('RGB',(7,4)))

def test_content_identity():
    assert catalog.texture_key(12,4,4,bytes(range(8)))=='1fc92492b76218f1'
    assert catalog.texture_key(14,4,4,bytes(range(8)))!='1fc92492b76218f1'

def test_archive_index_bounds(tmp_path):
    p=tmp_path/'bad.pak';p.write_bytes(struct.pack('<4sII',b'P5CK',500,16))
    with pytest.raises(ValueError):list(catalog.entries(p))

def test_budget_rejection_writes_no_asset(tmp_path):
    result=subprocess.run([sys.executable,str(ROOT/'tools/texture-pack.py'),'nonexistent.png',
        '--key','0000000000000001','--width','4096','--height','4096','--budget-mib','1',
        '--output',str(tmp_path/'pack')],capture_output=True,text=True)
    assert result.returncode!=0 and 'budget exceeded' in result.stderr
    assert not (tmp_path/'pack').exists()


def test_worklist_deduplicates_keys_but_keeps_unsupported(tmp_path):
    import json
    entries=[]
    for digest, key in [('a', '1'*16), ('b', '1'*16), ('c', None)]:
        entry=dict(sha256=digest, width=4, height=4, archive_format=0 if key else 5,
                   sources=[dict(archive=f'pak/{digest}.pak')])
        if key: entry.update(key=key, xbox_format=12)
        entries.append(entry)
    (tmp_path/'catalog.json').write_text(json.dumps(dict(scope='test',textures=entries)))
    report=worklist.build(tmp_path)
    assert report['counts']==dict(needs_art=1,unsupported_format=1)
    assert not report['complete']
    row=next(t for t in report['textures'] if t['id']=='1'*16)
    assert row['catalog_payloads']==2 and row['occurrences']==2
    assert row['archives']==['pak/a.pak','pak/b.pak']


def test_worklist_rejects_stale_metadata_and_alpha_loss(tmp_path):
    import hashlib
    data,mips=pack.encode(Image.new('RGBA',(8,8),(80,90,100,255)), 'DXT1')
    path=tmp_path/('1'*16+'.dds');path.write_bytes(data)
    meta=dict(key=path.stem,sha256=hashlib.sha256(data).hexdigest(),output_size=[8,8],bytes=len(data))
    original=dict(width=4,height=4,xbox_format=12)
    assert worklist.check_asset(path,meta,original)==len(data)
    with pytest.raises(ValueError,match='Alpha format lost'):
        worklist.check_asset(path,meta,dict(original,xbox_format=15))
    with pytest.raises(ValueError,match='Metadata mismatch'):
        worklist.check_asset(path,dict(meta,sha256='0'*64),original)
    path.write_bytes(data[:-8])
    with pytest.raises(ValueError,match='Incomplete mip chain'):
        worklist.check_asset(path,meta,original)


def test_budget_counts_metadata_before_writing(tmp_path):
    image=tmp_path/'source.png';Image.new('RGB',(4,4),(90,90,90)).save(image)
    output=tmp_path/'pack';output.mkdir()
    encoded,_=pack.encode(Image.open(image).convert('RGBA'),'DXT1')
    # The DDS alone fits exactly; its required metadata must trigger rejection.
    (output/'existing.bin').write_bytes(bytes(1024*1024-len(encoded)))
    result=subprocess.run([sys.executable,str(ROOT/'tools/texture-pack.py'),str(image),
        '--key','0000000000000001','--width','4','--height','4','--format','DXT1',
        '--budget-mib','1','--output',str(output)],capture_output=True,text=True)
    assert result.returncode!=0 and 'including texture metadata' in result.stderr
    assert len(list(output.iterdir()))==1
