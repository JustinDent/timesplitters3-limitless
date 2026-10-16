"""Track which generator sources produced the local, ignored game C files."""
import argparse
import hashlib
import json
from pathlib import Path


def fingerprint(runtime):
    paths = [runtime / 'scripts/recompile.py',
             runtime / 'templates/runtime/recomp_types.h',
             runtime / 'titles/futureperfect/src/recomp_manual.c']
    paths.extend(p for p in (runtime / 'tools/recomp').rglob('*')
                 if p.is_file() and p.suffix in ('.py', '.json', '.txt')
                 and '__pycache__' not in p.parts and not p.name.startswith('test_'))
    return {p.relative_to(runtime).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest()
            for p in sorted(paths)}


def marker(runtime):
    return runtime / 'titles/futureperfect/src/recomp/gen/tsfp-generator.json'


def current(runtime):
    try:
        return json.loads(marker(runtime).read_text(encoding='utf-8')) == fingerprint(runtime)
    except (OSError, ValueError):
        return False


def record(runtime):
    path = marker(runtime)
    if not (path.parent / 'recomp_funcs.h').is_file():
        raise RuntimeError('Generate the game sources before recording their generator.')
    temporary = path.with_suffix('.tmp')
    temporary.write_text(json.dumps(fingerprint(runtime), indent=2) + '\n', encoding='utf-8')
    temporary.replace(path)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('runtime', type=Path)
    parser.add_argument('--record', action='store_true')
    args = parser.parse_args()
    runtime = args.runtime.resolve()
    if args.record:
        record(runtime)
    else:
        raise SystemExit(0 if current(runtime) else 1)
