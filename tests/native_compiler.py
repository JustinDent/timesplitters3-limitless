"""Find a native C compiler, including standalone MSVC Build Tools on Windows."""
import os
from pathlib import Path
import shutil
import subprocess


def native_compiler(directory):
    if os.name != 'nt':
        environment = dict(os.environ)
        compiler = shutil.which(environment.get('CC', 'cc'))
        assert compiler, 'Native regressions require a C compiler'
        return compiler, environment
    # Windows environment names are case-insensitive, Python dict keys are not.
    environment = {key.upper(): value for key, value in os.environ.items()}
    compiler = shutil.which('cl', path=environment.get('PATH'))
    if compiler and environment.get('VSCMD_ARG_TGT_ARCH') == 'x64' and environment.get('INCLUDE') and environment.get('LIB'):
        return compiler, environment
    locator = Path(environment.get('PROGRAMFILES(X86)', r'C:\Program Files (x86)')) / 'Microsoft Visual Studio/Installer/vswhere.exe'
    assert locator.is_file(), 'Install Visual Studio or Build Tools with the MSVC x64 C++ toolchain'
    installation = subprocess.check_output([
        str(locator), '-latest', '-products', '*', '-requires',
        'Microsoft.VisualStudio.Component.VC.Tools.x86.x64', '-property', 'installationPath',
    ], text=True).strip()
    setup = Path(installation) / 'VC/Auxiliary/Build/vcvars64.bat'
    assert installation and setup.is_file(), 'MSVC x64 is required (Visual Studio or standalone Build Tools)'
    script = Path(directory) / 'native-compiler-env.cmd'
    script.write_text(f'@echo off\ncall "{setup}" >nul\nif errorlevel 1 exit /b 1\nset\n', encoding='utf-8')
    output = subprocess.check_output(
        f'cmd.exe /d /u /s /c ""{script}""', encoding='utf-16-le', env=environment,
    )
    environment.update((key.upper(), value) for key, value in
                       (line.split('=', 1) for line in output.splitlines()
                        if '=' in line and not line.startswith('=')))
    compiler = shutil.which('cl', path=environment.get('PATH'))
    assert compiler and environment.get('VSCMD_ARG_TGT_ARCH') == 'x64', 'Could not initialize the MSVC x64 environment'
    return compiler, environment
