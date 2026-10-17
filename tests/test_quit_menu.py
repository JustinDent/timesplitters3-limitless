"""Compile the production Quit to Desktop guest adapter on either host."""
import os
from pathlib import Path
import subprocess

from native_compiler import native_compiler

ROOT = Path(__file__).resolve().parents[1]


def test_quit_menu(tmp_path):
    compiler, env = native_compiler(tmp_path)
    source = ROOT / 'tests/quit_menu.c'
    exe = tmp_path / ('quit.exe' if os.name == 'nt' else 'quit')
    if os.name == 'nt':
        args = [compiler, '/nologo', '/std:c11', '/O2', '/W4',
                '/I' + str(tmp_path), '/I' + str(ROOT / 'src'), str(source), '/Fe:' + str(exe)]
    else:
        args = [compiler, '-std=c11', '-O2', '-Wall', '-Wextra', '-Werror',
                '-I' + str(tmp_path), '-I' + str(ROOT / 'src'), str(source), '-lm', '-o', str(exe)]
    result = subprocess.run(args, cwd=tmp_path, env=env, capture_output=True, text=True, timeout=60)
    assert result.returncode == 0, result.stdout + result.stderr
    result = subprocess.run([str(exe)], capture_output=True, text=True, timeout=10)
    assert result.returncode == 0, result.stdout + result.stderr
    assert 'PASS:' in result.stdout
