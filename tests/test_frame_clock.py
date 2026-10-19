"""Exercise host presentation deadlines independently from the guest clock."""
import os
from pathlib import Path
import subprocess
from native_compiler import native_compiler

ROOT = Path(__file__).resolve().parents[1]


def test_rational_presentation_clock(tmp_path):
    compiler, environment = native_compiler(tmp_path)
    output = tmp_path / ('frame-clock.exe' if os.name == 'nt' else 'frame-clock')
    source = ROOT / 'tests/frame_clock.c'
    command = ([compiler, '/nologo', '/std:c11', '/W4', str(source),
                '/Fe:' + str(output), '/Fo:' + str(tmp_path / 'clock.obj')]
               if os.name == 'nt' else [compiler, '-std=c11', '-Wall', '-Wextra', str(source), '-o', str(output)])
    subprocess.run(command, cwd=tmp_path, env=environment, check=True)
    subprocess.run([str(output)], check=True, timeout=10)


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
