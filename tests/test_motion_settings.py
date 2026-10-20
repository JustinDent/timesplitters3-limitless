"""Compile production in-game FPS persistence against disposable configs."""
import sys
import subprocess
from pathlib import Path
import pytest
from native_compiler import native_compiler
ROOT = Path(__file__).resolve().parents[1]

@pytest.mark.skipif(sys.platform != 'win32', reason='Windows persistence adapter')
def test_motion_settings(tmp_path):
    compiler, env = native_compiler(tmp_path)
    exe=tmp_path/'motion.exe'
    result=subprocess.run([compiler,'/nologo','/std:c11','/O2',str(ROOT/'tests/motion_settings.c'),'/Fe:'+str(exe)],
                          cwd=tmp_path,env=env,capture_output=True,text=True,timeout=60)
    assert result.returncode==0,result.stdout+result.stderr
    result=subprocess.run([str(exe),str(tmp_path/'runtime.ini')],capture_output=True,text=True,timeout=10)
    assert result.returncode==0,result.stdout+result.stderr
