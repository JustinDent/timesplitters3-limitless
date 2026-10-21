import os
from pathlib import Path
import subprocess
import pytest
from native_compiler import native_compiler

ROOT = Path(__file__).resolve().parents[1]


@pytest.mark.parametrize('fixture', ['text_read_scope', 'prompt_guest_scope'])
def test_text_read_scope(tmp_path, fixture):
    compiler, env = native_compiler(tmp_path)
    exe = tmp_path / ('text-read-scope.exe' if os.name == 'nt' else 'text-read-scope')
    source = str(ROOT / 'tests' / (fixture+'.c'))
    args = ([compiler, '/nologo', '/std:c11', '/W4', '/I'+str(ROOT/'src'), source, '/Fe:'+str(exe)]
            if os.name == 'nt' else
            [compiler, '-std=c11', '-Wall', '-Wextra', '-Werror', '-I'+str(ROOT/'src'), source, '-o', str(exe)])
    subprocess.run(args, cwd=tmp_path, env=env, check=True, timeout=60)
    subprocess.run([str(exe)], check=True, timeout=10)
