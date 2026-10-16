"""Shared UI placement rules; no game assets required."""
from pathlib import Path
import os
import subprocess
import pytest
from native_compiler import native_compiler
ROOT=Path(__file__).resolve().parents[1]
@pytest.mark.parametrize("fixture", ["ui_layout", "tutorial_layout"])
def test_ui_layout(tmp_path,fixture):
    compiler,environment=native_compiler(tmp_path)
    exe=tmp_path/('ui.exe' if os.name=='nt' else 'ui')
    source=str(ROOT/'tests'/f'{fixture}.c')
    if os.name=='nt':
        command=[compiler,'/nologo','/std:c11','/O2','/I'+str(ROOT/'src'),source,'/Fe:'+str(exe),'/Fo:'+str(tmp_path/'ui.obj')]
    else:
        command=[compiler,'-std=c11','-O2','-I'+str(ROOT/'src'),source,'-lm','-o',str(exe)]
    subprocess.run(command,cwd=tmp_path,env=environment,check=True,capture_output=True,text=True)
    result=subprocess.run([str(exe)],capture_output=True,text=True,timeout=10)
    assert result.returncode==0,result.stdout+result.stderr


def test_guest_placement_hooks():
    manual=(ROOT/'external/ts3FPrecomp/titles/futureperfect/src/recomp_manual.c').read_text()
    for address in (0x1334d0,0x133ab0,0x1358a0,0x12f670,0x14b930,0x1409c0,0x13fc50):
        assert f'pc_display_ui(sub_{address:08X}_gen,0x{address:08X}u)' in manual
    assert 'if(FP_MEM32(g_esp)==0x001341B7u)pc_display_radar_begin();' in manual
    for address,name in ((0x1476f0,'tutorial'),(0x40f00,'tutorial_wrap'),(0x41dc0,'tutorial_text')):
        assert f'pc_display_{name}(sub_{address:08X}_gen);' in manual
    assert 'pc_display_health(sub_0013AB60_gen);' in manual
    assert 'if(FP_MEM32(g_esp)==0x0013BA9Cu)pc_display_health_side();' in manual


@pytest.mark.parametrize("fixture_name", ["ui_viewport", "ui_screen_effects"])
def test_production_viewport_and_scissor(tmp_path, fixture_name):
    source=(ROOT/'external/ts3FPrecomp/src/d3d/d3d8_device.c').read_text()
    def body(signature):
        start=source.index(signature+'\n{');end=source.index('{',start);depth=1;end+=1
        while depth:
            depth+=(source[end]=='{')-(source[end]=='}');end+=1
        return source[start:end]
    signatures=(
        'static BOOL two_d_squeeze(float *k_out, float *cx_out)',
        'static float two_d_vertical_scale(void)',
        'BOOL d3d8_GetScissor(RhiRect *out)',
        'static void apply_host_viewport(void)')
    if fixture_name=='ui_screen_effects':
        signatures=(
            'static BOOL draw_extent(const void *vertices, UINT stride, UINT count,\n                        float *lo_out, float *hi_out)',
            'BOOL d3d8_draw_escapes_squeeze(const void *vertices, UINT stride, UINT count)')
    implementations='\n'.join(body(name) for name in signatures)
    fixture=(ROOT/'tests'/f'{fixture_name}.c').read_text().replace('/* PRODUCTION */',implementations)
    path=tmp_path/'viewport.c';path.write_text(fixture)
    compiler,environment=native_compiler(tmp_path);exe=tmp_path/('viewport.exe' if os.name=='nt' else 'viewport')
    if os.name=='nt':
        command=[compiler,'/nologo','/std:c11','/O2','/I'+str(ROOT/'src'),str(path),'/Fe:'+str(exe),'/Fo:'+str(tmp_path/'viewport.obj')]
    else:command=[compiler,'-std=c11','-O2','-I'+str(ROOT/'src'),str(path),'-lm','-o',str(exe)]
    subprocess.run(command,cwd=tmp_path,env=environment,check=True,capture_output=True,text=True)
    result=subprocess.run([str(exe)],capture_output=True,text=True,timeout=10)
    assert result.returncode==0,result.stdout+result.stderr
