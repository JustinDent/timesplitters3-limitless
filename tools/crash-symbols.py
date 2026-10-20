"""Retain one matching Windows executable/PDB bundle, separate from player packages."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import zipfile

ROOT = Path(__file__).resolve().parents[1]


def archive(output, root=ROOT, build=None):
    build = Path(build) if build is not None else root / 'build/windows/Release'
    files = [build / 'futureperfect_recomp.exe', build / 'futureperfect_recomp.pdb']
    metadata = dict(schema=1, source_commit=subprocess.check_output(
        ['git', 'rev-parse', 'HEAD'], cwd=root, text=True).strip(),
        source_changes=subprocess.check_output(['git', 'status', '--porcelain'], cwd=root, text=True).splitlines(),
        files={p.name: hashlib.sha256(p.read_bytes()).hexdigest() for p in files})
    output = Path(output)
    output.parent.mkdir(parents=True, exist_ok=True)
    temporary = output.with_suffix('.tmp')
    with zipfile.ZipFile(temporary, 'w', zipfile.ZIP_DEFLATED, compresslevel=1) as bundle:
        for path in files:
            bundle.write(path, path.name)
        bundle.writestr('BUILD.json', json.dumps(metadata, indent=2) + '\n')
    temporary.replace(output)
    print('Matching crash symbols:', output)
    return metadata


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, default=ROOT / 'build/windows/crash-symbols.zip')
    parser.add_argument('--build-dir', type=Path, default=ROOT / 'build/windows/Release')
    args = parser.parse_args()
    archive(args.output, build=args.build_dir)
