"""Run the actual shared framebuffer cache against a fake screen-copy device."""
import os
from pathlib import Path
import shutil
import subprocess
import sys

import pytest

ROOT = Path(__file__).resolve().parents[1]


@pytest.mark.skipif(sys.platform not in ('win32', 'darwin'),
                    reason='HLE renderer is compiled on Windows and macOS')
@pytest.mark.parametrize('break_refresh', [False, True], ids=['fixed', 'detect-stale-copy'])
def test_framebuffer_sampling_passes(tmp_path, break_refresh):
    environment = os.environ.copy()
    compiler = shutil.which('cl' if os.name == 'nt' else 'cc')
    if not compiler and os.name == 'nt':
        locator = Path(os.environ['ProgramFiles(x86)']) / 'Microsoft Visual Studio/Installer/vswhere.exe'
        install = subprocess.check_output([str(locator), '-latest', '-requires',
                    'Microsoft.VisualStudio.Component.VC.Tools.x86.x64',
                    '-property', 'installationPath'], text=True).strip()
        setup = Path(install) / 'VC/Auxiliary/Build/vcvars64.bat'
        assert install and setup.is_file(), 'MSVC x64 is required'
        script = tmp_path / 'compiler-env.cmd'
        script.write_text(f'@echo off\ncall "{setup}" >nul\nif errorlevel 1 exit /b 1\nset\n')
        output = subprocess.check_output(f'cmd.exe /d /u /s /c ""{script}""', encoding='utf-16-le')
        environment.update((key.upper(), value) for key, value in
                           (line.split('=', 1) for line in output.splitlines()
                            if '=' in line and not line.startswith('=')))
        compiler = shutil.which('cl', path=environment.get('PATH'))
    assert compiler, 'A native C compiler is required'
    source = ROOT / 'external/ts3FPrecomp/src'
    includes = [source, *(source / name for name in ('d3d', 'hle', 'kernel', 'config', 'platform'))]
    if break_refresh:
        # Prove the behavioral test detects losing the production binding hook.
        original = (source / 'hle/hle_d3d8_texture.c').read_text()
        hook = '        framebuffer_binding_changed(stage, texture);'
        assert original.count(hook) == 1
        mutant = tmp_path / 'mutant/hle/hle_d3d8_texture.c'
        mutant.parent.mkdir(parents=True)
        mutant.write_text(original.replace(hook, '        /* mutation: stale copy */'))
        includes.insert(0, mutant.parent.parent)
    exe = tmp_path / ('framebuffer.exe' if os.name == 'nt' else 'framebuffer')
    fixture = ROOT / 'tests/framebuffer_sampling.c'
    if os.name == 'nt':
        command = [compiler, '/nologo', '/std:c11', '/O2', '/Gy', '/Gw']
        command += ['/I' + str(path) for path in includes]
        command += [str(fixture), '/Fe:' + str(exe), '/Fo:' + str(tmp_path / 'test.obj'),
                    '/link', '/OPT:REF']
    else:
        command = [compiler, '-std=gnu11', '-O2', '-Wl,-dead_strip']
        command += ['-I' + str(path) for path in includes]
        command += [str(fixture), '-o', str(exe)]
    result = subprocess.run(command, cwd=tmp_path, env=environment,
                            capture_output=True, text=True, timeout=60)
    assert result.returncode == 0, result.stdout + result.stderr
    result = subprocess.run([str(exe)], capture_output=True, text=True, timeout=10)
    if break_refresh:
        assert result.returncode != 0, 'Regression did not detect the stale framebuffer copy'
        assert 'copies == 2 && sampled == 20' in result.stderr
    else:
        assert result.returncode == 0, result.stdout + result.stderr
        assert 'PASS: framebuffer sampling passes' in result.stdout
