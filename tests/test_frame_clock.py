"""Exercise the fixed 60 FPS configuration policy on both hosts."""
import os
from pathlib import Path
import subprocess
from native_compiler import native_compiler

ROOT = Path(__file__).resolve().parents[1]


def test_late_frame_does_not_wait_twice(tmp_path):
    source=(ROOT/'external/ts3FPrecomp/src/kernel/xbox_memory_layout.c').read_text(encoding='utf-8')
    start=source.index('void xbox_Nv2aFlipGateArm(void)')
    end=source.index('\nvoid xbox_Nv2aFlipGateRelease(void)',start)
    (tmp_path/'arm.inc').write_text(source[start:end],encoding='utf-8')
    compiler, environment=native_compiler(tmp_path)
    output=tmp_path/('gate-deadline.exe' if os.name=='nt' else 'gate-deadline')
    fixture=str(ROOT/'tests/flip_gate_deadline.c')
    command=([compiler,'/nologo','/std:c11','/I'+str(tmp_path),fixture,'/Fe:'+str(output)]
             if os.name=='nt' else [compiler,'-std=c11','-I'+str(tmp_path),fixture,'-o',str(output)])
    subprocess.run(command,cwd=tmp_path,env=environment,check=True,timeout=60)
    subprocess.run([str(output)],check=True,timeout=10)


def test_target_settings_round_trip(tmp_path):
    compiler, environment = native_compiler(tmp_path)
    config=ROOT / 'external/ts3FPrecomp/src/config'
    output=tmp_path / ('target-settings.exe' if os.name == 'nt' else 'target-settings')
    sources=[str(ROOT / 'tests/frame_target_settings.c'), str(config / 'recomp_config.c')]
    command=([compiler, '/nologo', '/std:c11', '/I'+str(config), *sources,
              '/Fe:'+str(output), '/Fo:'+str(tmp_path)+os.sep]
             if os.name == 'nt' else [compiler, '-std=c11', '-I'+str(config), *sources, '-o', str(output)])
    subprocess.run(command, cwd=tmp_path, env=environment, check=True)
    subprocess.run([str(output), str(tmp_path / 'runtime.ini')], check=True, timeout=10)
