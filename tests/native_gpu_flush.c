/* Execute the complete original D3D submit routine. The legacy control models
 * an acknowledgement after eight polls; the host path must preserve all guest
 * outputs while acknowledging only the PFB flush, without that scheduler wait. */
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
static uint32_t ram[0x400000/4],flush,other;
static uint32_t *word(uint32_t address)
{
    if(address==0xfd100410u)return &flush;
    if(address==0xfd100414u)return &other;
    assert(address<sizeof ram && !(address&3));
    return &ram[address/4];
}
#undef MEM32
#define MEM32(address) (*word((uint32_t)(address)))
/* GPU_FLUSH_POLICY */
static uint32_t production_poll(uint32_t site,uint32_t address,uint32_t value)
{ return RECOMP_TEST_MEM_U32_AT(site,address,value); }
static unsigned polls,drains,fast;
static uint32_t observe_poll(uint32_t site,uint32_t address,uint32_t value)
{
    if(site!=0x3d66d0u)return production_poll(site,address,value);
    assert(++polls<12);
    if(!fast || address!=0xfd100410u) {
        if(polls==8)MEM32(address)&=~0x10000u;
        return value;
    }
    return production_poll(site,address,value);
}
#undef RECOMP_TEST_MEM_U32_AT
#define RECOMP_TEST_MEM_U32_AT(site,address,value) observe_poll(site,address,value)
enum { DEVICE=0x1000, CHANNEL=0x8000, OWNER=0x9000, OUTPUT=0xa000, STACK=0x30000 };
void sub_003D4220(void)
{
    assert(MEM32(CHANNEL+0x40)==0x123456u || MEM32(CHANNEL+0x40)==0x654321u);
    assert(!(flush&0x10000u));
    g_eax=++drains==1;g_esp+=4;
}
/* GENERATED_ROUTINES */
typedef struct { uint32_t a,c,d,sp,b,si,di,bp,flush,other;unsigned drains; } Result;
static Result run(unsigned flags,unsigned drain,unsigned accelerated,unsigned offset)
{
    memset(ram,0,sizeof ram);flush=other=0x80000042u;
    MEM32(DEVICE)=0x80123456u;MEM32(DEVICE+0x770)=0x80654321u;
    MEM32(DEVICE+8)=flags;MEM32(DEVICE+0x1c20)=CHANNEL;
    MEM32(DEVICE+0x2c)=13;MEM32(DEVICE+0x30)=OUTPUT;
    MEM32(DEVICE+0x2478)=0xabcdef01u;
    MEM32(0x3e3f58)=OWNER;MEM32(OWNER+0x1c28)=0xfd000000u+offset;
    MEM32(0x3e3ab0)=drain;
    g_esp=STACK;MEM32(STACK)=0xdeadbeefu;
    g_ecx=DEVICE;g_eax=0x1111;g_edx=0x2222;
    g_esi=0x3333;g_ebx=0x4444;g_edi=0x5555;g_ebp=0x6666;
    polls=drains=0;fast=accelerated;
    sub_003D6690();
    assert(g_esp==STACK+4 && g_esi==0x3333 && g_ebx==0x4444 && g_edi==0x5555 && g_ebp==0x6666);
    assert(polls==((flags&0x2000)?0:(accelerated && !offset?1:9)));
    assert(drains==(!(flags&0x2000) && drain?2:0));
    assert(flush==0x80000042u && other==0x80000042u);
    Result r={g_eax,g_ecx,g_edx,g_esp,g_ebx,g_esi,g_edi,g_ebp,flush,other,drains};
    return r;
}
int main(void)
{
    g_xbox_mem_offset=(ptrdiff_t)((uintptr_t)&flush-0xfd100410u);
    flush=0x10081;
    assert(production_poll(0,0xfd100410u,flush)==0x10081 && flush==0x10081);
    assert(pc_gpu_flush_poll(1,0x10000)==0x10000 && flush==0x10081);
    assert(pc_gpu_flush_poll(0xfd100410u,0x80)==0x80 && flush==0x10081);
    assert(pc_gpu_flush_poll(0xfd100410u,0x10000)==0x81 && flush==0x81);
    unsigned char *saved=malloc(sizeof ram);assert(saved);
    unsigned cases=0;
    for(unsigned native=0;native<2;native++)for(unsigned f=0;f<4;f++)
    for(unsigned drain=0;drain<2;drain++)for(unsigned offset=0;offset<=4;offset+=4) {
        g_tsfp_native_timing_active=native;
        unsigned flags=0x81u|((f&1)?4u:0u)|((f&2)?0x2000u:0u);
        Result original=run(flags,drain,0,offset);
        memcpy(saved,ram,sizeof ram);
        Result corrected=run(flags,drain,1,offset);
        assert(!memcmp(saved,ram,sizeof ram));
        assert(!memcmp(&original,&corrected,sizeof original));cases++;
    }
    free(saved);
    printf("PASS: GPU PFB poll, %u original submit comparisons; 9 polls become 1, unrelated registers and fences unchanged\n",cases);
    return 0;
}
