"""Execute actual audio adapters against a deterministic fake output device."""
import os
from pathlib import Path
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[1]
RUNTIME = ROOT / 'external/ts3FPrecomp'


def compile_audio(kind, tmp_path_factory):
    directory = tmp_path_factory.mktemp('audio-' + kind)
    environment = os.environ.copy()
    compiler = shutil.which(os.environ.get('CC', 'cl' if os.name == 'nt' else 'cc'))
    if not compiler and os.name == 'nt':
        locator = Path(os.environ.get('ProgramFiles(x86)', r'C:\Program Files (x86)')) / 'Microsoft Visual Studio/Installer/vswhere.exe'
        if locator.is_file():
            found = subprocess.run([str(locator), '-latest', '-requires',
                                    'Microsoft.VisualStudio.Component.VC.Tools.x86.x64',
                                    '-property', 'installationPath'], capture_output=True, text=True, check=True)
            setup = Path(found.stdout.strip()) / 'VC/Auxiliary/Build/vcvars64.bat'
            if found.stdout.strip() and setup.is_file():
                script = directory / 'msvc-env.cmd'
                script.write_text(f'@echo off\ncall "{setup}" >nul\nif errorlevel 1 exit /b 1\nset\n', encoding='utf-8')
                output = subprocess.check_output(f'cmd.exe /d /u /s /c ""{script}""',
                                                 encoding='utf-16-le')
                environment.update((key.upper(), value) for key, value in
                                   (line.split('=', 1) for line in output.splitlines()
                                    if '=' in line and not line.startswith('=')))
                compiler = shutil.which('cl', path=environment.get('Path', environment.get('PATH')))
    assert compiler, 'Audio regressions require a C compiler (MSVC x64 on Windows)'
    exe = directory / (kind + ('.exe' if os.name == 'nt' else ''))
    source = ROOT / f'tests/audio_{kind}_timing.c'
    if Path(compiler).stem.lower() == 'cl':
        command = [compiler, '/nologo', '/std:c11', '/O2', '/I' + str(RUNTIME / 'src'),
                   str(source), '/Fe:' + str(exe), '/Fo:' + str(directory / 'audio.obj')]
    else:
        command = [compiler, '-std=c11', '-O2', '-I', str(RUNTIME / 'src'), str(source),
                   str(RUNTIME / 'src/platform/win32_compat.c'), '-lm', '-lpthread', '-o', str(exe)]
    built = subprocess.run(command, cwd=directory, env=environment, capture_output=True, text=True)
    assert built.returncode == 0, built.stdout + built.stderr
    return exe


@pytest.fixture(scope='module')
def stream_test(tmp_path_factory):
    return compile_audio('stream', tmp_path_factory)


@pytest.fixture(scope='module')
def buffer_test(tmp_path_factory):
    return compile_audio('buffer', tmp_path_factory)


@pytest.mark.parametrize('scenario', ['underrun', 'process', 'flush', 'fallback',
                                     'snapshot', 'pause', 'completion', 'lost-device', 'bad-data',
                                     'diagnostics', 'adpcm', 'release-reuse', 'flush-ex', 'flush-ex-async'])
def test_stream_timing(stream_test, scenario):
    result = subprocess.run([str(stream_test), scenario], capture_output=True, text=True, timeout=10)
    assert result.returncode == 0, result.stdout + result.stderr
    assert f'PASS: {scenario}' in result.stdout


@pytest.mark.parametrize('looping', [False, True])
def test_buffer_backpressure(buffer_test, looping):
    result = subprocess.run([str(buffer_test)] + (['loop'] if looping else []),
                            capture_output=True, text=True, timeout=10)
    assert result.returncode == 0, result.stdout + result.stderr
    assert 'PASS:' in result.stdout


def test_stream_headroom(buffer_test):
    result = subprocess.run([str(buffer_test), 'headroom'], capture_output=True, text=True, timeout=10)
    assert result.returncode == 0, result.stdout + result.stderr
    assert 'PASS:' in result.stdout
