/* Execute the original US player dispatcher on its inactive-player branch.
 * Its real elapsed-time and damping math run; world/AI services are mocked.
 * Full player movement remains a separate bounded gameplay comparison. */
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define RECOMP_GENERATED_CODE
/* NUMERIC_POLICY */
#include "recomp_types.h"
ptrdiff_t g_xbox_mem_offset;
int g_tsfp_native_timing_active;
RECOMP_TLS uint32_t g_eax,g_ecx,g_edx,g_esp,g_ebx,g_esi,g_edi,g_ebp,g_seh_ebp;
RECOMP_TLS RecompXmm g_xmm0,g_xmm1,g_xmm2,g_xmm3,g_xmm4,g_xmm5,g_xmm6,g_xmm7;
RECOMP_TLS double g_fp_stack[8];
RECOMP_TLS int g_fp_top;
#define STUB(name) void name(void) { g_eax=0; g_esp+=4; }
#define UNUSED(name) void name(void) { assert(!"unexpected player path"); g_esp+=4; }
UNUSED(sub_00042870)
UNUSED(sub_00058C70)
UNUSED(sub_00061CA0)
UNUSED(sub_00065FE0)
UNUSED(sub_00099280)
UNUSED(sub_000A5A60)
UNUSED(sub_000BA7D0)
UNUSED(sub_000BD090)
UNUSED(sub_000C45F0)
UNUSED(sub_000CB3B0)
UNUSED(sub_000CBD10)
UNUSED(sub_000D0CA0)
UNUSED(sub_000D19F0)
UNUSED(sub_000D1B40)
UNUSED(sub_000D1CB0)
UNUSED(sub_000D2190)
UNUSED(sub_000D2280)
UNUSED(sub_000D2330)
UNUSED(sub_000D2B80)
UNUSED(sub_000D3FE0)
UNUSED(sub_000D5C70)
STUB(sub_001213D0)
STUB(sub_001260C0)
UNUSED(sub_00238510)
UNUSED(sub_0023F1E0)
UNUSED(sub_00259E50)
UNUSED(sub_0025A080)
UNUSED(sub_0025A0C0)
UNUSED(sub_0025A3B0)
UNUSED(sub_0025A7F0)
UNUSED(sub_0025B720)
STUB(sub_0025C120)
UNUSED(sub_0025C650)
UNUSED(sub_0025CE80)
UNUSED(sub_0025D520)
UNUSED(sub_0025E650)
STUB(sub_0025F0C0)
UNUSED(sub_0033E530)
STUB(sub_003533F0)
UNUSED(sub_00356940)
/* GENERATED_ROUTINES */
static void dispatch(unsigned whole, float delta)
{
    MEM32(0x4e7944)=whole; MEMF(0x4e794c)=delta;
    g_esp=0x7000; MEM32(g_esp)=0x12345678;
    g_esi=0xdef;
    sub_000D5EB0();
    assert(g_esp==0x7004 && MEM32(0x7000)==0x12345678 && g_esi==0xdef);
    assert(MEM32(0x4e7944)==whole && MEMF(0x4e794c)==delta);
}
int main(void)
{
    unsigned char *memory=calloc(1,0x800000);
    const unsigned rates[]={60,120,144};
    assert(memory); g_xbox_mem_offset=(ptrdiff_t)(uintptr_t)memory;
    MEM32(0x7b0c7c)=0x1000; MEM32(0x1014)=0x3000; MEM32(0x307c)=0x4000;
    MEM32(0x1004)=0xffffffff; MEM32(0x4e8cec)=60;
    MEMF(0x475c78)=1; MEMF(0x475cd4)=.5f;
    for(unsigned r=0;r<3;r++) for(int native=0;native<2;native++) {
        unsigned fps=rates[r];
        MEMF(0x4b30)=0; MEMF(0x2008)=1;
        g_tsfp_native_timing_active=native;
        for(unsigned frame=0;frame<fps;frame++)
            dispatch((frame+1)*60/fps-frame*60/fps,60.0f/fps);
        float expected=native ? 1.0f : 60.0f/fps;
        assert(fabsf(MEMF(0x4b30)-expected)<.00001f);
        assert(fabsf(MEMF(0x2008)/ldexpf(1,-60)-1)<.00002f);
        float before=MEMF(0x4b30); MEMF(0x2008)=1;
        dispatch(0,0); assert(MEMF(0x4b30)==before && MEMF(0x2008)==1);
        if(native) {
            dispatch(0,.25f);
            assert(fabsf(MEMF(0x4b30)-before-.25f/60)<.00001f);
            assert(fabsf(MEMF(0x2008)-powf(.5f,.25f))<.000001f);
            dispatch(3,3);
            assert(fabsf(MEMF(0x4b30)-before-3.25f/60)<.00001f);
            assert(fabsf(MEMF(0x2008)-powf(.5f,3.25f))<.000001f);
        }
    }
    assert(RECOMP_LOAD_GLOBAL_U32(0xd5ec0,0x4e7944,7)==7);
    assert(RECOMP_LOAD_GLOBAL_U32(0xd6251,0x4e7944,0)==0);
    assert(RECOMP_F32_MUL_AT(0xd5f61,8,.5f)==4);
    g_tsfp_native_timing_active=0;
    assert(RECOMP_LOAD_GLOBAL_U32(0xd5ebf,0x4e7944,3)==3);
    assert(RECOMP_F32_MUL_AT(0xd5f60,8,.5f)==4);
    free(memory);
    puts("PASS: original player elapsed time and damping half-life at 60/120/144");
    return 0;
}
