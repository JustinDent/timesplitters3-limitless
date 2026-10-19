"""Compile actual shared interpolation math with dead-code removal on Windows."""
import os
from pathlib import Path
import subprocess
import pytest
from native_compiler import native_compiler

ROOT = Path(__file__).resolve().parents[1]


@pytest.mark.skipif(os.name != 'nt', reason='MSVC dead-code/linker fixture; Mac retains its GPU regressions')
def test_camera_interpolation(tmp_path):
    compiler, environment = native_compiler(tmp_path)
    source=ROOT / 'external/ts3FPrecomp/src'
    output=tmp_path / 'frame-interpolation.exe'
    command=[compiler, '/nologo', '/std:c11', '/O2', '/Gy', '/MD', '/DTSFP_PC_CONTROLS',
             '/I'+str(ROOT / 'src'), '/I'+str(source)]
    command += ['/I'+str(source / name) for name in ('d3d', 'rhi', 'config', 'hle', 'kernel', 'platform', 'nv2a')]
    command += [str(ROOT / 'tests/frame_interpolation.c'), '/Fe:'+str(output),
                '/Fo:'+str(tmp_path / 'interp.obj'), '/link', '/OPT:REF']
    subprocess.run(command, env=environment, cwd=tmp_path, check=True)
    subprocess.run([str(output)], check=True, timeout=10)
