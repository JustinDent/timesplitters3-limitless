"""Exercise production FOV persistence and the guest settings callbacks."""
import subprocess
import sys
from pathlib import Path
import pytest
from native_compiler import native_compiler
ROOT = Path(__file__).resolve().parents[1]

@pytest.mark.skipif(sys.platform != 'win32', reason='Windows settings persistence adapter')
def test_fov_settings(tmp_path):
    compiler, environment = native_compiler(tmp_path)
    exe = tmp_path / 'fov.exe'
    result = subprocess.run([compiler, '/nologo', '/std:c11', '/O2', '/W3',
        '/I' + str(ROOT / 'src'), '/I' + str(ROOT / 'external/ts3FPrecomp/templates/runtime'),
        str(ROOT / 'tests/fov_settings.c'), '/Fe:' + str(exe), '/link', 'user32.lib'],
        cwd=tmp_path, env=environment, capture_output=True, text=True, timeout=60)
    assert result.returncode == 0, result.stdout + result.stderr
    result = subprocess.run([str(exe), str(tmp_path / 'display.ini')],
        capture_output=True, text=True, timeout=15)
    assert result.returncode == 0, result.stdout + result.stderr
    assert 'PASS:' in result.stdout
