"""Compile the production pointer policy and guest adapter on either host."""
import os
from pathlib import Path
import subprocess
import pytest

from native_compiler import native_compiler

ROOT = Path(__file__).resolve().parents[1]


@pytest.mark.parametrize('mutation', ['fixed', 'missing-source', 'hover-conflict', 'missing-slider-drag'])
def test_menu_mouse(tmp_path, mutation):
    compiler, env = native_compiler(tmp_path)
    source = ROOT / 'tests/menu_mouse.c'
    exe = tmp_path / ('menu.exe' if os.name == 'nt' else 'menu')
    if mutation == 'missing-slider-drag':
        adapter = (ROOT / 'src/pc_menu_mouse_guest.inc').read_text(encoding='utf-8')
        audio = (ROOT / 'src/pc_menu_audio.inc').read_text(encoding='utf-8')
        hook = 'if(WORD_AT(volume)!=value)'
        assert hook in audio
        (tmp_path / 'pc_menu_mouse_guest.inc').write_text(adapter, encoding='utf-8')
        (tmp_path / 'pc_menu_audio.inc').write_text(audio.replace(hook, 'if(0 && WORD_AT(volume)!=value)'), encoding='utf-8')
    if mutation == 'missing-source':
        adapter = (ROOT / 'src/pc_menu_mouse_guest.inc').read_text(encoding='utf-8')
        injection = 'WORD_AT(0x006B93E0u)|=0x80u;'
        assert injection in adapter
        (tmp_path / 'pc_menu_mouse_guest.inc').write_text(adapter.replace(injection, '(void)0;'), encoding='utf-8')
    if mutation == 'hover-conflict':
        policy = (ROOT / 'src/pc_menu_mouse.h').read_text(encoding='utf-8')
        guard = 'if(moved && !s->scrolling)'
        assert guard in policy
        (tmp_path / 'pc_menu_mouse.h').write_text(policy.replace(guard, 'if(moved)'), encoding='utf-8')
        # Quoted includes resolve alongside the adapter before -I directories.
        (tmp_path / 'pc_menu_mouse_guest.inc').write_text(
            (ROOT / 'src/pc_menu_mouse_guest.inc').read_text(encoding='utf-8'), encoding='utf-8')
    if os.name == 'nt':
        args = [compiler, '/nologo', '/std:c11', '/O2', '/W4',
                '/I' + str(tmp_path), '/I' + str(ROOT / 'src'), str(source), '/Fe:' + str(exe)]
    else:
        args = [compiler, '-std=c11', '-O2', '-Wall', '-Wextra', '-Werror',
                '-I' + str(tmp_path), '-I' + str(ROOT / 'src'), str(source), '-lm', '-o', str(exe)]
    result = subprocess.run(args, cwd=tmp_path, env=env, capture_output=True, text=True, timeout=60)
    assert result.returncode == 0, result.stdout + result.stderr
    result = subprocess.run([str(exe)], capture_output=True, text=True, timeout=10)
    if mutation == 'missing-source':
        assert result.returncode != 0
        assert 'WORD_AT(0x6b93e0)&0x80' in result.stderr
        return
    if mutation == 'hover-conflict':
        assert result.returncode != 0
        assert 'selections==selected_before' in result.stderr
        return
    if mutation == 'missing-slider-drag':
        assert result.returncode != 0
        assert 'WORD_AT(0x5236bc)==4' in result.stderr
        return
    assert result.returncode == 0, result.stdout + result.stderr
    assert 'PASS:' in result.stdout
