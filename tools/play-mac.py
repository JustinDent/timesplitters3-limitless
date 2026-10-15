"""Launch the existing Windows build through CrossOver without copying game data."""
import argparse
from datetime import datetime
import hashlib
import json
import math
import ntpath
import os
from pathlib import Path
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
EXPECTED_XBE = '3cfd001a84fc3e08175d6c4b2e42ebf49577a089b5d0bd87ac724f61a41816bc'
DEFAULT_BOTTLE = 'TimeSplitters-Future-Perfect'
RUNTIME_FILES = ('futureperfect_recomp.exe', 'avcodec-62.dll', 'avutil-60.dll',
                 'swresample-6.dll', 'vcruntime140.dll')


def crossover_bin():
    for app in (Path('/Applications/CrossOver.app'), Path.home() / 'Applications/CrossOver.app'):
        directory = app / 'Contents/SharedSupport/CrossOver/bin'
        if (directory / 'wine').is_file():
            return directory
    raise ValueError('Install CrossOver from https://www.codeweavers.com/crossover first.')


def check_game(root):
    game = root / 'private/game'
    if not (game / 'default.xbe').is_file():
        raise ValueError(f'Missing game data. Copy the other computer\'s private/game folder to {game}.')
    if hashlib.sha256((game / 'default.xbe').read_bytes()).hexdigest() != EXPECTED_XBE:
        raise ValueError('default.xbe does not match the supported US Xbox release.')
    for name in ('pak', 'xmv'):
        if not (game / name).is_dir() or not any((game / name).iterdir()):
            raise ValueError(f'Missing or empty game asset directory: {game / name}')


def read_controls(path):
    controls = json.loads(path.read_text(encoding='utf-8-sig'))
    mouse = controls.get('mouse', {})
    sensitivity = mouse.get('sensitivity', 0.10)
    if isinstance(sensitivity, bool) or not isinstance(sensitivity, (int, float)) or not math.isfinite(sensitivity) or not 0.001 <= sensitivity <= 10:
        raise ValueError('Mouse sensitivity must be a finite number from 0.001 to 10.')
    return sensitivity, bool(mouse.get('enabled', True)), bool(mouse.get('invert_y', False))


def game_environment(base, windows_root, tag, mouse):
    # Do not inherit smoke-test replay, capture, save, or texture overrides.
    env = {k: v for k, v in base.items() if not k.startswith(('RECOMP_', 'TSFP_'))}
    join = lambda p: ntpath.join(windows_root, *p.split('/'))
    sensitivity, enabled, invert = mouse
    env.update({
        'RECOMP_GAME_DIR': join('private/game'),
        'RECOMP_SAVE_DIR': join('private/saves'),
        'RECOMP_INPUT_CONFIG': join('config/pc-controls.json'),
        'TSFP_DISPLAY_CONFIG': join('config/pc-display.ini'),
        'RECOMP_FFMPEG_DIR': join('build/windows/Release'),
        'RECOMP_LOG_FILE': join(f'logs/{tag}.err'),
        'RECOMP_TRACE_BUDGET': '0',
        'RECOMP_PC_MOUSE': str(int(enabled)),
        'RECOMP_MOUSE_INVERT_Y': str(int(invert)),
        'RECOMP_MOUSE_SENSITIVITY': str(sensitivity),
        'WINEDEBUG': '-all',
    })
    return env


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--check', action='store_true', help='Read-only prerequisite check; do not launch or create settings.')
    parser.add_argument('--bottle', default=DEFAULT_BOTTLE)
    args = parser.parse_args()
    if sys.platform != 'darwin':
        raise ValueError('This launcher is for macOS. Use Play.cmd on Windows.')
    cx = crossover_bin()
    executable = ROOT / 'build/windows/Release/futureperfect_recomp.exe'
    errors = []
    for name in RUNTIME_FILES:
        if not (executable.parent / name).is_file():
            errors.append(f'Missing Windows runtime file: {executable.parent / name}')
    bottle = subprocess.run([str(cx / 'cxbottle'), '--bottle', args.bottle, '--status'],
                            capture_output=True, text=True, timeout=30)
    if bottle.returncode:
        errors.append(f'Create the {args.bottle!r} Windows 10 64-bit bottle in CrossOver, then select D3DMetal graphics.')
    try:
        check_game(ROOT)
    except ValueError as exc:
        errors.append(str(exc))
    config = ROOT / 'config/pc-controls.json'
    default = ROOT / 'config/pc-controls.default.json'
    mouse = read_controls(config if config.exists() else default)
    print(f'CrossOver: {cx.parents[3]}')
    print(f'Bottle: {args.bottle}')
    print(f'Build: {executable}')
    print(f'Free disk space: {shutil.disk_usage(ROOT).free / 1024**3:.1f} GiB')
    if errors:
        for error in errors:
            print(error, file=sys.stderr)
        return 1
    if args.check:
        print('Files and bottle are present. Gameplay compatibility is still unverified.')
        return 0
    # Translate through the selected bottle rather than assuming a Z: mapping.
    result = subprocess.run([str(cx / 'wine'), '--bottle', args.bottle, 'winepath', '-w', str(ROOT)],
                            capture_output=True, text=True, timeout=60, check=True)
    windows_root = result.stdout.strip()
    if not ntpath.isabs(windows_root) or '\n' in windows_root or '\r' in windows_root:
        raise ValueError(f'CrossOver returned an invalid Windows path: {windows_root!r}')
    if not config.exists():
        with config.open('xb') as destination:
            destination.write(default.read_bytes())
    (ROOT / 'private/saves').mkdir(parents=True, exist_ok=True)
    logs = ROOT / 'logs'
    logs.mkdir(exist_ok=True)
    tag = datetime.now().strftime('mac-play-%Y%m%d-%H%M%S-%f')
    environment = game_environment(os.environ, windows_root, tag, mouse)
    print(f'Launching through CrossOver. Logs: {logs / tag}', flush=True)
    with (logs / f'{tag}.out').open('wb') as output, (logs / f'{tag}.err').open('wb') as errors:
        result = subprocess.run([str(cx / 'wine'), '--bottle', args.bottle, '--workdir',
                                 str(executable.parent), str(executable)],
                                cwd=executable.parent, env=environment, stdout=output, stderr=errors)
    if result.returncode:
        print(f'Game exited with code {result.returncode}; inspect {logs / (tag + ".err")}', file=sys.stderr)
    return result.returncode


if __name__ == '__main__':
    try:
        sys.exit(main())
    except (OSError, ValueError, subprocess.SubprocessError) as exc:
        print(f'Mac launch failed: {exc}', file=sys.stderr)
        sys.exit(1)
