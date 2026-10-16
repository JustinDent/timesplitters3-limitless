/* Each explicit wrapper lives in recomp_manual.c so the lifter can find it. */
#ifdef _WIN32
#include "pc_network.h"
#include "pc_debug.h"
static void pc_network_guest(uint32_t address, void (*original)(void))
{
    uint32_t args[12]; unsigned n,i,count; double latency; int mode;
    if(!pc_network_enabled()) { pc_debug_latency(0,-1,0); original(); return; }
    n=pc_network_arguments(address);
    for(i=0;i<n;i++) args[i]=*(uint32_t *)((uintptr_t)(g_esp+4+i*4)+g_xbox_mem_offset);
    g_eax=pc_network_call(address,args,g_xbox_mem_offset);
    mode=pc_network_latency(&latency,&count);
    pc_debug_latency(mode,latency,count);
    g_esp+=4+n*4;
}
#else
/* Native Mac has no PC XNet transport yet; keep the original guest path. */
static void pc_network_guest(uint32_t address, void (*original)(void))
{
    (void)address;
    original();
}
#endif
