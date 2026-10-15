#!/usr/bin/env python3
"""Launch the experimental native Mac build for interactive play."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import platform
import subprocess
import sys
import threading

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('mac_common_launch', ROOT / 'tools/play-mac.py')
common = importlib.util.module_from_spec(spec)
spec.loader.exec_module(common)


def game_environment(base, root):
    env = {k:v for k,v in base.items() if not k.startswith(('RECOMP_', 'TSFP_'))}
    settings = root / 'private/native-settings'
    env.update(RECOMP_GAME_DIR=str(root / 'private/game'),
               RECOMP_SAVE_DIR=str(root / 'private/saves'),
               RECOMP_INPUT_CONFIG=str(root / 'config/pc-controls.json'),
               RECOMP_DISPLAY_CONFIG=str(settings / 'runtime.conf'),
               TSFP_DISPLAY_CONFIG=str(root / 'config/pc-display.ini'),
               XDG_CONFIG_HOME=str(settings / 'xdg-config'),
               XDG_DATA_HOME=str(settings / 'xdg-data'), RECOMP_TRACE_BUDGET='0')
    return env


def prepare_settings(root):
    config = root / 'config/pc-controls.json'
    config.parent.mkdir(parents=True, exist_ok=True)
    if not config.exists():
        with config.open('xb') as output:
            output.write((root / 'config/pc-controls.default.json').read_bytes())
    (root / 'private/saves').mkdir(parents=True, exist_ok=True)
    settings = root / 'private/native-settings'
    settings.mkdir(parents=True, exist_ok=True)
    runtime = settings / 'runtime.conf'
    if not runtime.exists():
        with runtime.open('x') as output:
            output.write('# Native runtime settings; PC settings remain in config/pc-display.ini\n')


def drain_log(stream, path, budget=4*1024*1024):
    # Drain after the cap too, so verbose guest logging cannot stall gameplay.
    with path.open('wb') as output:
        while True:
            chunk = stream.read(65536)
            if not chunk:
                break
            if budget:
                kept = chunk[:budget]
                output.write(kept)
                budget -= len(kept)
    stream.close()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--check', action='store_true', help='Read-only prerequisite check')
    args = parser.parse_args()
    if platform.system() != 'Darwin' or platform.machine() != 'arm64':
        raise ValueError('This native build requires an Apple Silicon Mac.')
    common.check_game(ROOT)
    executable = ROOT / 'build/macos/futureperfect_recomp'
    kind = subprocess.check_output(['file', str(executable)], text=True)
    if 'Mach-O 64-bit executable arm64' not in kind:
        raise ValueError('Build the native runtime with tools/build-native-mac.py first.')
    manifest = json.loads((executable.parent / 'BUILD.json').read_text())
    if hashlib.sha256(executable.read_bytes()).hexdigest() != manifest.get('executable_sha256'):
        raise ValueError('Native executable differs from BUILD.json; rebuild before launching.')
    controls = ROOT / 'config/pc-controls.json'
    common.read_controls(controls if controls.exists() else ROOT / 'config/pc-controls.default.json')
    print(f'Native ARM64 build ready: {executable}')
    if args.check:
        return 0
    prepare_settings(ROOT)
    logs = ROOT / 'logs'
    logs.mkdir(exist_ok=True)
    print('Arrows/Enter navigate. Escape pauses. F1 captures/releases mouse. Close the game window to quit.', flush=True)
    print('Experimental: first-level rendering and basic input tested; campaign completion is unverified.', flush=True)
    proc = subprocess.Popen([str(executable)], cwd=ROOT, env=game_environment(os.environ, ROOT),
                            stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    readers = [threading.Thread(target=drain_log, args=(stream, logs / name))
               for stream, name in ((proc.stdout, 'native-play.out'), (proc.stderr, 'native-play.err'))]
    for reader in readers:
        reader.start()
    try:
        result = proc.wait()
    except KeyboardInterrupt:
        proc.terminate()
        try:
            result = proc.wait(timeout=3)
        except subprocess.TimeoutExpired:
            proc.kill()
            result = proc.wait()
    finally:
        for reader in readers:
            reader.join()
    return result


if __name__ == '__main__':
    try:
        sys.exit(main())
    except (OSError, ValueError, subprocess.SubprocessError) as exc:
        print(f'Native Mac launch failed: {exc}', file=sys.stderr)
        sys.exit(1)
