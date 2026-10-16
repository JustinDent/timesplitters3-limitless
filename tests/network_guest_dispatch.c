/* Shared ABI check: native fallback must not reference the Windows transport. */
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

/* Select only the guest adapter after including the host's standard headers. */
#ifdef TSFP_TEST_NATIVE_FALLBACK
#undef _WIN32
#elif !defined(_WIN32)
#define _WIN32 1
#endif

static uint32_t memory[0x800000 / 4],g_eax,g_esp;
static ptrdiff_t g_xbox_mem_offset;
static int originals,enabled;
#define CHECK(x) do { if(!(x)) { fprintf(stderr,"FAIL %d: %s\n",__LINE__,#x); return 1; } } while(0)

#ifdef _WIN32
int pc_network_enabled(void) { return enabled; }
int pc_network_latency(double *milliseconds,unsigned *count)
{ *milliseconds=-1;*count=0;return 1; }
void pc_debug_latency(int mode,double milliseconds,unsigned count)
{ (void)mode;(void)milliseconds;(void)count; }
unsigned pc_network_arguments(uint32_t address) { return address==0x1234u?2u:0u; }
uint32_t pc_network_call(uint32_t address,const uint32_t *args,ptrdiff_t offset)
{
    return address==0x1234u && args[0]==17 && args[1]==29 && offset==g_xbox_mem_offset
        ? 0x55667788u : 0;
}
#endif

/* Exercise the simulated Windows transport branch on any C host without
   importing native settings-window headers; telemetry is stubbed above. */
#define TSFP_PC_DEBUG_H
#include "../src/pc_network_guest.inc"

static void original(void) { originals++;g_eax=0x11223344u;g_esp+=4; }
static unsigned movement_sends;
static void original_pump(void)
{
    uint32_t epoch=memory[0x76B0F0/4];
    originals++;
    if ((int32_t)epoch>(int32_t)memory[0x76B0EC/4]) {
        movement_sends++;
        memory[0x76B0EC/4]=epoch;
    }
    g_eax=0x11223344u;g_esp+=4;
}

int main(void)
{
    g_xbox_mem_offset=(ptrdiff_t)(uintptr_t)memory;
    g_esp=16;memory[5]=17;memory[6]=29;
    pc_network_guest(0x1234u,original);
    CHECK(originals==1 && g_eax==0x11223344u && g_esp==20);
    enabled=1;g_esp=16;
    pc_network_guest(0x1234u,original);
#ifdef TSFP_TEST_NATIVE_FALLBACK
    CHECK(originals==2 && g_eax==0x11223344u && g_esp==20);
    puts("PASS: native fallback retains original call/register/stack behavior without host networking");
#else
    CHECK(originals==1 && g_eax==0x55667788u && g_esp==28);
    puts("PASS: Windows transport dispatch and disabled fallback retain the guest ABI");
#endif
    /* Same epoch, different guest frames: movement must be sampled immediately
       rather than waiting seven ticks. Duplicate pump calls must not resend. */
    memory[0x76B1A4/4]=15;memory[0x76B0F0/4]=20;memory[0x76B0EC/4]=20;
    for(unsigned frame=140;frame<147;frame++) {
        memory[0x7497A8/4]=frame;g_esp=16;
        pc_network_guest_tick(original_pump);
        CHECK(g_eax==0x11223344u && g_esp==20);
        CHECK(memory[0x76B0F0/4]==20 && memory[0x7497A8/4]==frame);
        pc_network_guest_tick(original_pump);
    }
#ifdef TSFP_TEST_NATIVE_FALLBACK
    CHECK(movement_sends==0);
#else
    CHECK(movement_sends==7);
#endif
    /* No override for offline play or any lobby/loading/disconnect state. */
    for(unsigned state=0;state<18;state++) {
        if(state==15)continue;
        memory[0x76B1A4/4]=state;memory[0x76B0EC/4]=20;
        memory[0x7497A8/4]++;pc_network_guest_tick(original_pump);
        CHECK(memory[0x76B0EC/4]==20);
    }
    enabled=0;memory[0x76B1A4/4]=15;memory[0x7497A8/4]++;
    pc_network_guest_tick(original_pump);CHECK(memory[0x76B0EC/4]==20);
    /* Held keys and mouse-only look must not leave full movement state stale.
       Three ticks bound drift, including clock resets and a sender stall. */
    enabled=1;memory[0x76B1A4/4]=15;memory[5]=0x1000;
    memory[0x1020/4]=4;
    for(unsigned i=0;i<4;i++) memory[(0x100C+i*4)/4]=0x2000+i*0x200;
    for(unsigned age=0;age<9;age++) {
        memory[0x7DE318/4]=100+age;
        for(unsigned i=0;i<4;i++) {
            memory[(0x2100+i*0x200)/4]=0;
            memory[(0x210C+i*0x200)/4]=100;
        }
        g_esp=16;pc_network_guest_sample(original);
        CHECK(g_eax==0x11223344u && g_esp==20);
        for(unsigned i=0;i<4;i++) {
#ifdef TSFP_TEST_NATIVE_FALLBACK
            CHECK(memory[(0x2100+i*0x200)/4]==0);
#else
            CHECK(memory[(0x2100+i*0x200)/4]==(age>=3));
#endif
            CHECK(memory[(0x210C+i*0x200)/4]==100);
        }
    }
    enabled=0;g_esp=16;memory[0x2100/4]=0;
    pc_network_guest_sample(original);CHECK(memory[0x2100/4]==0);
    enabled=1;memory[0x76B1A4/4]=14;g_esp=16;
    pc_network_guest_sample(original);CHECK(memory[0x2100/4]==0);
    memory[0x76B1A4/4]=15;memory[0x7DE318/4]=90;g_esp=16;
    pc_network_guest_sample(original);
#ifndef TSFP_TEST_NATIVE_FALLBACK
    CHECK(memory[0x2100/4]==1);
#endif
    puts("PASS: movement gate, duplicate calls, offline/native fallback and unchanged guest clocks");
    return 0;
}
