"""Execute the production guest wrapper with isolated camera and stack memory."""
import re

import pytest

from test_cutscene_letterbox import ROOT, RUNTIME, compile_and_run


@pytest.mark.parametrize('camera_available', [True, False])
@pytest.mark.parametrize('platform_define', ['TSFP_PC_CONTROLS', 'TSFP_NATIVE_DISPLAY'])
def test_decal_projection(tmp_path, camera_available, platform_define):
    manual = (RUNTIME / 'titles/futureperfect/src/recomp_manual.c').read_text()
    wrapper = re.search(r'void sub_00151CF0\(void\)\n\{.*?\n\}', manual, re.S)
    assert wrapper, 'Apply the complete patch stack before testing'
    camera = (ROOT / 'src/pc_display_menu.c').read_text()
    function = camera[camera.index('void pc_display_decal_projection'):camera.index('void pc_display_menu_button')]
    source = r'''
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define PLATFORM_DEFINE
static unsigned char memory[0x800000];
static uintptr_t g_xbox_mem_offset;
static uint32_t g_esp, g_eax;
#define WORD_AT(a) (*(uint32_t *)(memory+(uint32_t)(a)))
static int guest_writable(uint32_t a,size_t n)
{ return a && a<sizeof memory && n<=sizeof memory-a; }
#define CHECK(x) do { if(!(x)) { fprintf(stderr,"FAIL: %s\n",#x); exit(1); } } while(0)
static float original[16];
void sub_00151CF0_gen(void)
{
    memcpy(memory+WORD_AT(g_esp+4),original,sizeof original);
    g_eax=0x12345678;g_esp+=4;
}
/* FUNCTION */
/* WRAPPER */
int main(void)
{
    const uint32_t callers[]={0x112F7F,0x115B0C,0xC7832,0x1B7482,0x2C46EF};
    const float apertures[][2]={{1.44f,1.92f},{1.08f,1.92f},{.8f,1.92f},{1.44f,2.56f}};
    size_t i,j,k;
    g_xbox_mem_offset=(uintptr_t)memory;
    WORD_AT(0x7B0CC0)=CAMERA_AVAILABLE ? 0x4000 : 0;
    WORD_AT(0x4204)=0x5000;
    for(k=0;k<16;k++)original[k]=(float)k;
    original[0]=1.44f;original[5]=1.92f;original[10]=-1.00005555f;original[14]=-.10050558f;
    for(i=0;i<sizeof apertures/sizeof apertures[0];i++) {
        float *active=(float *)(memory+0x5000);
        active[0]=apertures[i][0];active[5]=apertures[i][1];
        for(j=0;j<sizeof callers/sizeof callers[0];j++) {
            float *decal=(float *)(memory+0x6000);
            g_esp=0x10000;WORD_AT(g_esp)=callers[j];WORD_AT(g_esp+4)=0x6000;
            sub_00151CF0();
            CHECK(g_esp==0x10004 && g_eax==0x12345678);
            for(k=0;k<16;k++) {
                float expected=CAMERA_AVAILABLE && j<2 && (k==0 || k==5) ? active[k] : original[k];
                CHECK(decal[k]==expected);
            }
        }
    }
    puts("PASS: both decal passes match camera aperture; depth, other callers and guest ABI preserved");
    return 0;
}
'''
    source = source.replace('/* FUNCTION */', function).replace('/* WRAPPER */', wrapper.group())
    source = source.replace('CAMERA_AVAILABLE', str(int(camera_available)))
    source = source.replace('PLATFORM_DEFINE', platform_define)
    compile_and_run(tmp_path, source)
