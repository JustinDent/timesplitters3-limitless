"""Exercise the production menu reconstruction shader on a real D3D11 device."""
import subprocess
import sys
from pathlib import Path
import pytest
from native_compiler import native_compiler
ROOT=Path(__file__).resolve().parents[1]

@pytest.mark.skipif(sys.platform!='win32',reason='Windows D3D11 renderer')
def test_menu_sampling_gpu(tmp_path):
    compiler,env=native_compiler(tmp_path)
    exe=tmp_path/'menu-sampling.exe'
    command=[compiler,'/nologo','/std:c11','/O2','/I'+str(ROOT/'src'),
        str(ROOT/'tests/menu_sampling_gpu.c'),'/Fe:'+str(exe),
        '/link','d3d11.lib','d3dcompiler.lib','dxgi.lib']
    result=subprocess.run(command,cwd=tmp_path,env=env,capture_output=True,text=True,timeout=60)
    assert result.returncode==0,result.stdout+result.stderr
    result=subprocess.run([str(exe)],capture_output=True,text=True,timeout=30)
    assert result.returncode==0,result.stdout+result.stderr
    assert 'PASS: GPU menu magnification' in result.stdout
