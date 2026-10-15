"""Validate the complete patch stack before updating the runtime checkout.

Unwind recognized patches in reverse order in a small temporary tree, then
reapply the stack in order. Later patches may extend earlier patches' context;
checking each patch independently incorrectly reports those as conflicts.
Keep existing patches stable and put new changes in a separate patch.
"""
from pathlib import Path
import argparse
import shutil
import subprocess
import tempfile


def git(tree, *args):
    return subprocess.run(
        # Upstream stores some sources as CRLF. Match context whitespace on
        # every host while retaining it in the output; non-whitespace edits
        # still conflict. Do not depend on a developer's global Git settings.
        ['git', '-C', str(tree), '-c', 'apply.ignoreWhitespace=change', *map(str, args)],
        capture_output=True, encoding='utf-8', errors='replace',
    )


def apply_patches(runtime, patch_dir):
    runtime, patch_dir = Path(runtime).resolve(), Path(patch_dir).resolve()
    patches = sorted(patch_dir.glob('*.patch'))
    names = set()
    for patch in patches:
        for line in patch.read_text(encoding='utf-8').splitlines():
            if line.startswith('+++ b/'):
                name = line[6:]
                target = runtime / name
                if not target.resolve().is_relative_to(runtime) or not target.is_file():
                    raise RuntimeError(f'{patch.name}: invalid or missing target {name}')
                names.add(name)
    originals = {name: (runtime / name).read_bytes() for name in names}
    with tempfile.TemporaryDirectory(prefix='tsfp-patches-') as directory:
        stage = Path(directory)
        initialized = git(stage, 'init', '-q')
        if initialized.returncode:
            raise RuntimeError(initialized.stderr)
        for name, content in originals.items():
            target = stage / name
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_bytes(content)
        for patch in reversed(patches):
            if git(stage, 'apply', '--reverse', '--check', patch).returncode == 0:
                result = git(stage, 'apply', '--reverse', patch)
                if result.returncode:
                    raise RuntimeError(f'{patch.name}: {result.stderr}')
        for patch in patches:
            result = git(stage, 'apply', '--check', patch)
            if result.returncode:
                raise RuntimeError(
                    f'{patch.name} conflicts with the runtime checkout. '
                    f'No runtime files were changed.\n{result.stderr}'
                )
            result = git(stage, 'apply', patch)
            if result.returncode:
                raise RuntimeError(f'{patch.name}: {result.stderr}')
        changed = [name for name in sorted(names)
                   if (stage / name).read_bytes() != originals[name]]
        # Do not overwrite an edit made while the stack was being validated.
        for name in names:
            if (runtime / name).read_bytes() != originals[name]:
                raise RuntimeError(f'{name} changed during patch validation; retry.')
        for name in changed:
            shutil.copyfile(stage / name, runtime / name)
    return changed


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('runtime', type=Path)
    parser.add_argument('patches', type=Path)
    args = parser.parse_args()
    try:
        changed = apply_patches(args.runtime, args.patches)
    except RuntimeError as error:
        parser.exit(1, f'{error}\n')
    print(f'Local patches verified; {len(changed)} runtime file(s) updated.')
