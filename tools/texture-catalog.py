"""Read-only P5CK texture inventory and DXT source export for the Xbox US build.

Archive layout reference: https://github.com/OpenRadical/tspak (tspak.h).
The 128-byte texture header detection is verified against this local game dump;
unrecognized assets remain explicitly unclassified, never silently converted.
"""
import argparse
import collections
import hashlib
import io
import json
from pathlib import Path
import struct


def texture_key(fmt, width, height, data):
    h = 0xcbf29ce484222325
    for b in struct.pack('<III', fmt, width, height) + data:
        h = ((h ^ b) * 0x100000001b3) & 0xffffffffffffffff
    return f'{h:016x}'


def dds_header(w, h, fourcc=b'DXT1', levels=1):
    words = [124, 0xA1007, h, w, ((w+3)//4)*((h+3)//4)*(8 if fourcc == b'DXT1' else 16), 0, levels]
    words += [0]*11 + [32, 4, int.from_bytes(fourcc, 'little'), 0, 0, 0, 0, 0]
    words += [0x401008 if levels > 1 else 0x1000, 0, 0, 0, 0]
    return b'DDS ' + struct.pack('<31I', *words)


def entries(path):
    size = path.stat().st_size
    with path.open('rb') as f:
        header = f.read(12)
        if len(header) != 12 or header[:4] != b'P5CK':
            raise ValueError(f'Not P5CK: {path}')
        offset, length = struct.unpack_from('<II', header, 4)
        if length % 16 or offset + length > size:
            raise ValueError(f'Invalid archive index: {path}')
        f.seek(offset)
        table = f.read(length)
        for crc, pos, length, compressed in struct.iter_unpack('<4I', table):
            if pos + (compressed or length) > size:
                raise ValueError(f'Out-of-bounds entry: {path}, {crc:x}')
            yield f, crc, pos, length, compressed


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--game', type=Path, default=Path('private/game'))
    ap.add_argument('--output', type=Path, default=Path('private/texture-remaster'))
    ap.add_argument('--export-archive', help='Export DXT textures from archive paths containing this string')
    args = ap.parse_args()
    args.output.mkdir(parents=True, exist_ok=True)
    catalog = {}
    counts = collections.Counter()
    from PIL import Image
    for path in sorted((args.game/'pak').rglob('*.pak')):
        rel = path.relative_to(args.game).as_posix()
        counts['archives'] += 1
        for f, crc, pos, length, compressed in entries(path):
            counts['entries'] += 1
            if compressed:
                counts['compressed_unclassified'] += 1
                continue
            f.seek(pos)
            header = f.read(min(length, 128))
            if len(header) != 128:
                continue
            w, h, w2, h2, unknown, kind, sentinel = struct.unpack_from('<7I', header)
            if not (w and h and w <= 4096 and h <= 4096 and w & (w-1) == 0 and h & (h-1) == 0
                    and w == w2 and h == h2 and sentinel == 0xffffffff):
                continue
            counts['texture_occurrences'] += 1
            raw = f.read(length-128)
            digest = hashlib.sha256(header+raw).hexdigest()
            source = {'archive': rel, 'crc': f'{crc:08x}', 'offset': pos, 'length': length}
            if digest not in catalog:
                entry = catalog[digest] = dict(width=w, height=h, archive_format=kind,
                    bytes=len(raw), sha256=digest, sources=[], status='unclassified_format')
                # Verified against 104 exact runtime texture payloads:
                # archive 0 -> DXT1, 1 -> DXT3, 4 -> DXT5.
                formats = {0:(12,b'DXT1',8), 1:(14,b'DXT3',16), 4:(15,b'DXT5',16)}
                if kind in formats:
                    gpu,fourcc,block = formats[kind]
                    n = ((w+3)//4)*((h+3)//4)*block
                    if len(raw) >= n:
                        entry.update(status='source_only', xbox_format=gpu,
                                     key=texture_key(gpu, w, h, raw[:n]))
            entry = catalog[digest]
            entry['sources'].append(source)
            if args.export_archive and args.export_archive in rel and 'key' in entry:
                dest = args.output/'originals'/f"{entry['key']}.png"
                dest.parent.mkdir(exist_ok=True)
                if not dest.exists():
                    fourcc = {12:b'DXT1',14:b'DXT3',15:b'DXT5'}[entry['xbox_format']]
                    n = ((w+3)//4)*((h+3)//4)*(8 if fourcc==b'DXT1' else 16)
                    Image.open(io.BytesIO(dds_header(w,h,fourcc)+raw[:n])).save(dest)
                entry['original_png'] = dest.relative_to(args.output).as_posix()
            if 'key' in entry and (args.output/'originals'/f"{entry['key']}.png").is_file():
                entry['original_png'] = f"originals/{entry['key']}.png"
    counts['unique_texture_payloads'] = len(catalog)
    counts['unique_archive_ids'] = len({s['crc'] for e in catalog.values() for s in e['sources']})
    result = dict(schema=1, scope='P5CK entries with recognized 128-byte Xbox texture header; embedded textures may remain',
                  counts=dict(counts), formats=dict(collections.Counter(e['archive_format'] for e in catalog.values())),
                  textures=list(catalog.values()))
    (args.output/'catalog.json').write_text(json.dumps(result, indent=2)+'\n', encoding='utf-8')
    print(json.dumps({k:v for k,v in result.items() if k != 'textures'}, indent=2))


if __name__ == '__main__':
    main()
