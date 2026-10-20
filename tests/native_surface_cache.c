/* Original surface-motion/cache caller and complete procedural-texture
 * dispatcher. Graphics/pipeline passes are spies, not a GPU simulation.
 * Two material entries share one procedural resource and one active view. */
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
enum { MATERIALS=0x1000,PARAMS=0x2000,TEXTURE=0x3000,RESOURCE=0x4000,
       PLAYER=0x5000,STACK=0x10000 };
static unsigned passes,updates,state_calls;
static int unpatched;
static uint32_t load(uint32_t site,uint32_t address,uint32_t value) {
    if(unpatched && (((site==0x12764c || site==0x127694) && address==0x7497a8) ||
                     (site==0x1956a0 && address==0x4e7944)))
        return value;
    return pc_native_global_load(site,address,value);
}
#undef RECOMP_LOAD_GLOBAL_U32
#define RECOMP_LOAD_GLOBAL_U32(site,address,value) load(site,address,value)
#undef RECOMP_ITAIL
#define RECOMP_ITAIL(target) assert(!"Unexpected surface tail")
#undef RECOMP_ICALL_SAFE_AT
#define RECOMP_ICALL_SAFE_AT(target,stack,site) assert(!"Unexpected surface callback")
static void state(void) { state_calls++;g_eax=7;g_esp+=4; }
void sub_00019650(void) { state(); }
void sub_0001ED60(void) { state(); }
void sub_00023270(void) { state(); }
void sub_00021450(void) { state(); }
void sub_000214C0(void) { state(); }
void sub_0001EDE0(void) { state(); }
static void pass(unsigned index) {
    assert(passes%5==index && g_esi==RESOURCE && g_edi==RESOURCE);
    assert(MEM32(RESOURCE+0x24)==(index<3?0x2222u:0x1111u));
    assert(MEM32(RESOURCE+0x28)==(index<3?0x1111u:0x2222u));
    passes++;if(index==4)updates++;g_esp+=4;
}
void sub_001953B0(void) { pass(0); }
void sub_001952D0(void) { pass(1); }
void sub_001950B0(void) { pass(2); }
void sub_00195110(void) { pass(3); }
void sub_00195280(void) { pass(4); }
/* GENERATED_ROUTINES */
static void setup(unsigned view) {
    memset((void *)(g_xbox_mem_offset+MATERIALS),0,0x6000);
    MEM32(0x74227c)=2;MEM32(0x7ac6e4)=MATERIALS;MEM32(0x7b0cc0)=PLAYER;
    MEM32(PLAYER+0x214)=view;MEM32(0x4e8cec)=60;
    MEMF(TEXTURE+0x10)=MEMF(TEXTURE+0x14)=64;
    MEM32(RESOURCE+0x24)=0x1111;MEM32(RESOURCE+0x28)=0x2222;
    for(unsigned i=0;i<4;i++)MEM32(RESOURCE+4+i*4)=0xffffffffu;
    /* Constant velocity and zero forcing/drag isolate the cache defect.
     * The U/V and phase integrations still run through original code. */
    MEMF(PARAMS+0x10)=.1f;
    for(unsigned i=0;i<2;i++) {
        uint32_t p=MATERIALS+i*0xc0;
        MEM32(p)=PARAMS;MEM32(p+4)=TEXTURE;MEM32(p+0x24)=0x10;
        MEMF(p+0xa8)=2;MEMF(p+0xac)=-1;
        MEMF(p+0xb0)=.1f;MEMF(p+0xb4)=.4f;MEM32(p+0xbc)=RESOURCE;
    }
    passes=updates=state_calls=0;
}
static void step(unsigned frame,unsigned vblank,unsigned tick,unsigned whole,float delta) {
    MEM32(0x7497b8)=frame;MEM32(0x7497a8)=vblank;MEM32(0x7de318)=tick;
    MEM32(0x4e7944)=whole;MEMF(0x4e794c)=delta;
    g_esp=STACK;MEM32(STACK)=0x12345678;
    g_esi=0xdead;g_edi=0xbeef;g_ebx=0xfeed;g_ebp=0xabcd;
    sub_00127500();
    assert(g_esp==STACK+4 && !g_fp_top);
    assert(g_esi==0xdead && g_edi==0xbeef && g_ebx==0xfeed && g_ebp==0xabcd);
    assert(MEM32(0x7497b8)==frame && MEM32(0x7497a8)==vblank);
    assert(MEM32(0x4e7944)==whole && MEMF(0x4e794c)==delta && MEM32(0x7de318)==tick);
    assert(passes==updates*5 && state_calls==updates*13);
    assert(MEM32(RESOURCE+0x24)==0x1111 && MEM32(RESOURCE+0x28)==0x2222);
}
int main(void) {
    void *memory=calloc(1,0x800000);assert(memory);g_xbox_mem_offset=(ptrdiff_t)memory;
    /* ORIGINAL_CONSTANTS */
    setvbuf(stdout,NULL,_IONBF,0);
    unsigned losses=0;const unsigned rates[]={60,120,144};
    for(unpatched=1;unpatched>=0;unpatched--)for(unsigned slow=1;slow<=4;slow*=4)
    for(unsigned r=0;r<3;r++)
    for(unsigned phase=0;phase<4;phase++)for(unsigned view=0;view<4;view+=3) {
        setup(view);g_tsfp_native_timing_active=1;
        unsigned previous=0;
        for(unsigned f=1;f<=rates[r]*slow*8;f++) {
            unsigned tick=f*60/(rates[r]*slow),whole=tick-previous;
            unsigned vblank=100+(f*60*4+phase*rates[r])/(rates[r]*4);
            unsigned before=updates;
            step(f,vblank,tick,whole,60.f/(rates[r]*slow));
            assert(updates-before<=1 && (whole || updates==before));
            if(!unpatched)assert(updates-before==(whole?1u:0u));
            previous=tick;
        }
        printf("surface_cache original=%d fps=%u slow=%u vblank_phase=%u view=%u updates=%u expected=480\n",
               unpatched,rates[r],slow,phase,view,updates);
        if(unpatched && updates<480)losses++;
        if(!unpatched || rates[r]==60)assert(updates==480);
        for(unsigned i=0;i<2;i++) {
            uint32_t p=MATERIALS+i*0xc0;
            assert(fabsf(MEMF(p+0xb0)-.35f)<.0001f);
            assert(fabsf(MEMF(p+0xb4)-.275f)<.0001f);
            assert(fabsf(MEMF(p+0xb8)-.8f)<.0001f);
        }
    }
    assert(losses>0);unpatched=0;
    /* Original inactive path still uses its vblank key, even when the native
     * frame id changes. Native pause/resume must not consume a real update. */
    for(unsigned native=0;native<2;native++) {
        setup(0);g_tsfp_native_timing_active=native;
        step(1,100,1,1,1);assert(updates==1);
        step(2,100,1,0,0);assert(updates==1);
        step(3,100,2,1,1);assert(updates==(native?2u:1u));
        step(4,101,3,1,1);assert(updates==(native?3u:2u));
        assert(pc_native_global_load(0x12764d,0x7497a8,999)==999);
    }
    unpatched=1;setup(0);g_tsfp_native_timing_active=1;
    step(1,100,1,1,0);assert(updates==1); /* original paused-image counterexample */
    unpatched=0;setup(0);
    step(1,100,1,1,1);assert(updates==1);
    /* A temporary zero-float pause must not evolve the procedural image
     * from a stale positive whole count, even on a new display tick. */
    step(2,101,1,1,0);assert(updates==1);
    step(3,101,2,1,1);assert(updates==2);
    step(4,102,5,3,2.75f);assert(updates==3); /* original one-dispatch catch-up */
    /* Each view owns its stamp; aliases within a view must still deduplicate. */
    MEM32(PLAYER+0x214)=1;
    step(4,102,5,3,2.75f);assert(updates==4);
    /* Inactive-native retains the original stale-whole behavior as well. */
    setup(0);g_tsfp_native_timing_active=0;
    step(1,100,1,1,0);assert(updates==1);
    puts("PASS: procedural texture updates survive native/vblank phase differences; original UV motion and resource swaps retained");
    free(memory);return 0;
}
