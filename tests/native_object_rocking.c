/* Original object rocking/bounce updater and matrix builders. Free-flight
 * characterization only: bounce, impact audio and collision response are
 * deliberately not approximated. Unexpected services fail the fixture. */
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
RECOMP_TLS int g_fp_top,g_fp_cmp,g_df;
enum { ENTITY=0x1000, MODEL=0x3000, MATRICES=0x4000, STACK=0x10000 };
#undef RECOMP_ICALL_SAFE_AT
#define RECOMP_ICALL_SAFE_AT(target,stack,site) assert(!"Unexpected object callback")
#define UNUSED(name) void name(void) { assert(!"Unexpected " #name); }
UNUSED(sub_003C9DF8)
UNUSED(sub_00021910)
UNUSED(sub_00045390)
UNUSED(sub_00045820)
/* GENERATED_ROUTINES */
static void prepare(void)
{
    memset((void *)((uintptr_t)ENTITY+g_xbox_mem_offset),0,0x300);
    MEM32(ENTITY+4)=MODEL;MEM32(MODEL+0x10)=MATRICES;
    MEM32(ENTITY+0x16c)=0xa84f0;
    MEMF(ENTITY+0x220)=10;MEMF(ENTITY+0x224)=5;
}
static float call(unsigned whole,float delta)
{
    MEM32(0x4e7944)=whole;MEMF(0x4e794c)=delta;
    /* The caller supplies the current base transform each update. */
    for(unsigned i=0;i<16;i++) MEMF(MATRICES+i*4)=i%5==0?1.f:0.f;
    g_esp=STACK;MEM32(STACK)=0x12345678;MEM32(STACK+4)=ENTITY;
    g_esi=0xdead;g_ebp=0xbeef;g_ebx=0xfeed;g_edi=0xabcd;
    sub_000A84F0();
    assert(g_esp==STACK+4 && MEM32(STACK)==0x12345678 && g_fp_top==0);
    /* Framed matrix helpers publish their frame in g_ebp; the translator
     * keeps the caller's actual EBP local rather than restoring this TLS. */
    assert(g_esi==0xdead && g_ebx==0xfeed && g_edi==0xabcd);
    assert(MEM32(0x4e7944)==whole && MEMF(0x4e794c)==delta);
    assert(MEMF(ENTITY+0x220)>.3f); /* Stay in the audited free-flight path. */
    for(unsigned i=0;i<16;i++) assert(isfinite(MEMF(MATRICES+i*4)));
    assert(fabsf(MEMF(MATRICES+0x34)-MEMF(ENTITY+0x220))<.00001f);
    return MEMF(ENTITY+0x220);
}
int main(void)
{
    unsigned char *memory=calloc(1,0x800000);
    const unsigned rates[]={60,120,144};
    assert(memory);g_xbox_mem_offset=(ptrdiff_t)(uintptr_t)memory;
    /* ORIGINAL_CONSTANTS */
    MEM32(0x4e8cec)=60;
    for(unsigned i=0;i<16;i++) MEMF(0x4e7900+i*4)=i%5==0?1.f:0.f;
    for(unsigned slow=1;slow<=4;slow*=4) {
        float reference_h=0,reference_v=0;
        for(unsigned rate=0;rate<3;rate++) {
            unsigned fps=rates[rate],changed=0;
            prepare();g_tsfp_native_timing_active=rate!=0;
            float previous=MEMF(ENTITY+0x220);
            for(unsigned frame=0;frame<fps;frame++) {
                unsigned whole=(frame+1)*60/(fps*slow)-frame*60/(fps*slow);
                float height=call(whole,rate?60.f/(fps*slow):(float)whole);
                changed+=height!=previous;previous=height;
            }
            if(!rate) { reference_h=previous;reference_v=MEMF(ENTITY+0x224); }
            assert(previous==reference_h && MEMF(ENTITY+0x224)==reference_v);
            printf("rocking_audit fps=%u slow=%u changed=%u frozen=%u height=%.9f velocity=%.9f\n",
                   fps,slow,changed,fps-changed,previous,MEMF(ENTITY+0x224));
            float paused=call(0,0);assert(paused==previous);
        }
    }
    /* Multi-tick catch-up equals the same sequence of original steps. */
    prepare();g_tsfp_native_timing_active=0;
    call(1,1);call(1,1);float reference=call(1,1);
    prepare();g_tsfp_native_timing_active=1;
    assert(call(3,3)==reference);
    free(memory);
    puts("PASS: original object rocking endpoint/ABI checks; fractional progression evaluated separately");
    return 0;
}
