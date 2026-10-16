"""Compile and exercise the Windows display model with isolated settings."""
from pathlib import Path
import subprocess
import sys

import pytest
from native_compiler import native_compiler

ROOT = Path(__file__).resolve().parents[1]


@pytest.mark.skipif(sys.platform != 'win32', reason='Windows display adapter test')
def test_primary_display_modes(tmp_path):
    compiler, environment = native_compiler(tmp_path)
    executable = tmp_path / 'display-modes.exe'
    subprocess.run([compiler, '/nologo', '/std:c11', '/O2', '/MD',
                    str(ROOT / 'tests/pc_display_modes.c'),
                    '/Fe:' + str(executable), '/Fo:' + str(tmp_path / 'modes.obj'),
                    '/link', 'user32.lib'], check=True, cwd=tmp_path, env=environment)
    subprocess.run([str(executable), str(tmp_path / 'display.ini')], check=True,
                   cwd=tmp_path, timeout=10)
