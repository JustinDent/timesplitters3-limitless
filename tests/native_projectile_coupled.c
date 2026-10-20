/* Coupled timing audit of the complete second projectile updater, kind-35
 * initializer, target sampler and steering. Scene queries miss a controlled
 * target; effect allocation/position calls are spies, not rendered effects. */
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
enum { SHOT=0x1000,TARGET=0x2000,FX=0x5000,STACK=0x30000 };
static unsigned sweeps,scans,rng,fx_updates,allocated,retired,opened,seed;
#undef RECOMP_ITAIL
#define RECOMP_ITAIL(target) assert(!"Unexpected coupled projectile tail")
#undef RECOMP_ICALL_SAFE_AT
#define RECOMP_ICALL_SAFE_AT(target,stack,site) assert(!"Unexpected coupled projectile callback")
#undef RECOMP_LOAD_GLOBAL_U32
static uint32_t gate(uint32_t site,uint32_t address,uint32_t value) {
    if(opened && site==0xf08d0 && g_tsfp_native_timing_active)return MEMF(0x4e794c)>0;
    return pc_native_global_load(site,address,value);
}
#define RECOMP_LOAD_GLOBAL_U32(site,address,value) gate(site,address,value)
void sub_00101660(void) {
    const unsigned ids[]={83,83,84,85};
    assert(allocated<4 && MEM32(g_esp+8)==ids[allocated]);
    assert(MEM32(g_esp+4)==SHOT+0xac+4*allocated);
    assert(MEM32(g_esp+12)==0 && MEM32(g_esp+16)==SHOT+0x1c && MEM32(g_esp+20)==0xffff);
    g_eax=FX+256*allocated++;g_esp+=4;
}
void sub_000FF420(void) {
    const float sizes[]={45,40,37,35};
    uint32_t fx=MEM32(g_esp+4);
    assert(fx>=FX && fx<FX+1024 && (fx-FX)%256==0);
    assert(MEMF(g_esp+8)==sizes[(fx-FX)/256]);g_esp+=4;
}
void sub_000FF490(void) {
    uint32_t fx=MEM32(g_esp+4),point=MEM32(g_esp+8);
    assert(fx>=FX && fx<FX+1024 && (fx-FX)%256==0);
    for(unsigned i=0;i<3;i++) {
        assert(isfinite(MEMF(point+i*4)));MEMF(fx+0x20+i*4)=MEMF(point+i*4);
    }
    fx_updates++;g_esp+=4;
}
void sub_00153B40(void) { rng++;seed=seed*1664525u+1013904223u;g_eax=seed;g_esp+=4; }
void sub_000DCB80(void) {
    assert(g_esi==SHOT);uint32_t delta=MEM32(g_esp+4);
    for(unsigned i=0;i<3;i++)assert(isfinite(MEMF(delta+i*4)));
    sweeps++;g_eax=0;g_esp+=4;
}
void sub_00088C60(void) { scans++;g_eax=TARGET;g_esp+=4; }
void sub_000EDBE0(void) { g_esp+=4; }
void sub_000DC550(void) { retired++;g_esp+=4; }
void sub_00045390(void) { assert(!"Unexpected call sub_00045390"); }
void sub_00045820(void) { assert(!"Unexpected call sub_00045820"); }
void sub_000B91F0(void) { assert(!"Unexpected call sub_000B91F0"); }
void sub_000B9220(void) { assert(!"Unexpected call sub_000B9220"); }
void sub_000BD0F0(void) { assert(!"Unexpected call sub_000BD0F0"); }
void sub_000C3C50(void) { assert(!"Unexpected call sub_000C3C50"); }
void sub_000DC2B0(void) { assert(!"Unexpected call sub_000DC2B0"); }
void sub_000DC4A0(void) { assert(!"Unexpected call sub_000DC4A0"); }
void sub_000DC5D0(void) { assert(!"Unexpected call sub_000DC5D0"); }
void sub_000DCEF0(void) { assert(!"Unexpected call sub_000DCEF0"); }
void sub_000DD170(void) { assert(!"Unexpected call sub_000DD170"); }
void sub_000E2520(void) { assert(!"Unexpected call sub_000E2520"); }
void sub_000E94E0(void) { assert(!"Unexpected call sub_000E94E0"); }
void sub_000E9FE0(void) { assert(!"Unexpected call sub_000E9FE0"); }
void sub_000EDD40(void) { assert(!"Unexpected call sub_000EDD40"); }
void sub_000F85C0(void) { assert(!"Unexpected call sub_000F85C0"); }
void sub_000FF3D0(void) { assert(!"Unexpected call sub_000FF3D0"); }
void sub_00100260(void) { assert(!"Unexpected call sub_00100260"); }
void sub_00100AA0(void) { assert(!"Unexpected call sub_00100AA0"); }
void sub_00105910(void) { assert(!"Unexpected call sub_00105910"); }
void sub_00114AF0(void) { assert(!"Unexpected call sub_00114AF0"); }
void sub_00153BD0(void) { assert(!"Unexpected call sub_00153BD0"); }
void sub_001CDAE0(void) { assert(!"Unexpected call sub_001CDAE0"); }
void sub_00238510(void) { assert(!"Unexpected call sub_00238510"); }
void sub_000CFE40(void) { assert(!"Unexpected call sub_000CFE40"); }
/* GENERATED_ROUTINES */
static void call(void (*fn)(void)) {
    g_esp=STACK;MEM32(STACK)=0x12345678;MEM32(STACK+4)=SHOT;
    g_esi=0xdead;g_edi=0xfeed;g_ebx=0xbeef;g_ebp=0xabcd;
    fn();
    assert(g_esp==STACK+4 && g_fp_top==0);
    assert(g_esi==0xdead && g_edi==0xfeed && g_ebx==0xbeef && g_ebp==0xabcd);
}
static void run(unsigned fps,int native,float slow,unsigned homing,unsigned open) {
    memset((void *)((uintptr_t)SHOT+g_xbox_mem_offset),0,0x22c);
    memset((void *)((uintptr_t)FX+g_xbox_mem_offset),0,1024);
    MEM32(SHOT)=35;MEMF(SHOT+0x64)=1;MEMF(SHOT+0x88)=240;
    MEM32(SHOT+0xc)=homing?0x2000:0;
    MEM32(SHOT+0x19c)=homing?TARGET:0;
    MEMF(TARGET+0x64)=1000;MEMF(TARGET+0x68)=500;
    sweeps=scans=rng=fx_updates=allocated=retired=0;seed=1;
    g_tsfp_native_timing_active=native;opened=open;
    call(sub_000E78F0);assert(allocated==4);
    unsigned frames=(unsigned)(fps/slow),fresh=0;
    double ticks=0;unsigned previous=0;float delta=60.f/fps*slow;
    for(unsigned i=0;i<frames;i++) {
        ticks+=60.0/fps*slow;
        unsigned whole=(unsigned)(ticks+1e-6)-previous;previous+=whole;
        MEMF(SHOT+0x98)+=delta/60;
        MEM32(0x4e7944)=whole;MEMF(0x4e794c)=delta;
        float before=MEMF(SHOT+0x1c);
        call(sub_000F08D0);
        if(MEMF(SHOT+0x1c)!=before)fresh++;
        assert(!retired);
        assert(MEM32(0x4e7944)==whole && MEMF(0x4e794c)==delta);
        unsigned char saved[0x22c],effects[1024];
        memcpy(saved,(void *)((uintptr_t)SHOT+g_xbox_mem_offset),sizeof saved);
        memcpy(effects,(void *)((uintptr_t)FX+g_xbox_mem_offset),sizeof effects);
        unsigned prior_rng=rng,prior_scans=scans,prior_sweeps=sweeps,prior_fx=fx_updates;
        MEM32(0x4e7944)=0;MEMF(0x4e794c)=0;call(sub_000F08D0);
        assert(!memcmp(saved,(void *)((uintptr_t)SHOT+g_xbox_mem_offset),sizeof saved));
        assert(!memcmp(effects,(void *)((uintptr_t)FX+g_xbox_mem_offset),sizeof effects));
        assert(rng==prior_rng && scans==prior_scans && sweeps==prior_sweeps && fx_updates==prior_fx);
    }
    assert(rng==9*sweeps && fx_updates==4*sweeps);
    if(homing)assert(scans>0);else assert(scans==0);
    printf("coupled_projectile fps=%u native=%d slow=%.2f homing=%u opened=%u frames=%u fresh=%u x=%.6f y=%.6f speed=%.6f scans=%u rng=%u fx=%u\n",
        fps,native,slow,homing,opened,frames,fresh,MEMF(SHOT+0x1c),MEMF(SHOT+0x20),
        MEMF(SHOT+0x88),scans,rng,fx_updates);
}
int main(void) {
    unsigned char *memory=calloc(1,0x800000);assert(memory);
    g_xbox_mem_offset=(ptrdiff_t)(uintptr_t)memory;
    /* ORIGINAL_CONSTANTS */
    MEM32(0x4e8cec)=60;
    setvbuf(stdout,NULL,_IONBF,0);
    for(unsigned homing=0;homing<2;homing++) {
        run(60,0,1,homing,0);
        unsigned char original[0x22c];
        memcpy(original,(void *)((uintptr_t)SHOT+g_xbox_mem_offset),sizeof original);
        const unsigned rates[]={60,120,144};
        for(unsigned i=0;i<3;i++)for(unsigned slow=0;slow<2;slow++) {
            run(rates[i],1,slow?.25f:1,homing,0);
            if(i==0 && !slow)assert(!memcmp(original,
                (void *)((uintptr_t)SHOT+g_xbox_mem_offset),sizeof original));
        }
        run(144,1,1,homing,1);
    }
    puts("PASS: coupled projectile audit executes original initialization, motion, steering and effect positioning");
    free(memory);return 0;
}
