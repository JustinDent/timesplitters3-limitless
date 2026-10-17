/* Shared ABI check: native fallback must not reference the Windows transport. */
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Select only the guest adapter after including the host's standard headers. */
#ifdef TSFP_TEST_NATIVE_FALLBACK
#undef _WIN32
#elif !defined(_WIN32)
#define _WIN32 1
#endif

static uint32_t memory[0x800000 / 4],g_eax,g_ecx,g_esp;
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
#ifdef _WIN32
static int bot_test_enabled;
static const char *bot_test_getenv(const char *name)
{ return bot_test_enabled && !strcmp(name,"TSFP_NETWORK_BOTS")?"1":NULL; }
#define getenv bot_test_getenv
#endif
#ifdef _WIN32
uint32_t pc_network_bots_revive(unsigned slot);
void pc_network_bots_entity_refresh(uint32_t entity);
void pc_network_bots_present(uint32_t entity,int32_t clip,const float *channels);
#endif
#include "../src/pc_network_bots_guest.inc"
#ifdef _WIN32
#undef getenv
#endif

#ifdef _WIN32
static unsigned bot_sends,bot_refreshes;
static uint32_t bot_host_ip;
static PcNetworkBotState last_bot;
static int bot_incoming;
void pc_network_bots_context(unsigned role,uint32_t ip,uint32_t match,unsigned humans,unsigned bots)
{ bot_host_ip=ip;(void)role;(void)match;(void)humans;(void)bots; }
void pc_network_bots_send(const PcNetworkBotState *state) {last_bot=*state;bot_sends++;}
void pc_network_bots_targets(const uint32_t *addresses,unsigned count) {(void)addresses;(void)count;}
int pc_network_bots_receive(unsigned slot,PcNetworkBotState *state)
{ if(bot_incoming && slot==last_bot.slot) {*state=last_bot;bot_incoming=0;return 1;}return 0; }
uint32_t pc_network_bots_revive(unsigned slot) {memory[(0x1000+slot*0xea0)/4]=0x7000;return 0x7000;}
void pc_network_bots_entity_refresh(uint32_t entity) {(void)entity;bot_refreshes++;}
void pc_network_bots_present(uint32_t entity,int32_t clip,const float *channels)
{(void)entity;(void)clip;(void)channels;}
#endif
static void original(void) { originals++;g_eax=0x11223344u;g_esp+=4; }
#ifdef _WIN32
static uint8_t broadcast_config[PC_NETWORK_MATCH_BYTES];
static void original_broadcast(void)
{
    memcpy(broadcast_config,(uint8_t *)memory+memory[(g_esp+4)/4],sizeof broadcast_config);
    original();
}
#endif
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
    {
        int before=originals;
        g_esp=16;pc_network_bots_guest_config(original);
        CHECK(originals==before+1 && g_eax==0x11223344u && g_esp==20);
    }
#ifdef _WIN32
    {
        uint8_t *config=(uint8_t *)memory+0x78ad58;
        PcNetworkBot *bot=(PcNetworkBot *)((uint8_t *)memory+0x75fd90);
        PcNetworkBot chosen={42,0,2,1,-1,0,-1};
        bot[0]=chosen;config[0]=16;config[1]=1;config[2]=0;
        memory[0x77728c/4]=1;memory[0x76b108/4]=1;
        memory[0x76b140/4]=1;memory[0x78ad2c/4]=13;bot_test_enabled=1;
        memory[5]=0x78ad58;g_esp=16;pc_network_bots_guest_config(original);
        CHECK(config[0]==1 && config[2]==1 && g_eax==0x11223344u && g_esp==20);
        config[0]=16;config[2]=0;memory[5]=0x774044;
        g_esp=16;pc_network_bots_guest_config(original);
        CHECK(config[0]==16 && config[2]==0 && g_eax==0x11223344u && g_esp==20);
        memory[5]=0x78ad58;memory[0x76b140/4]=2;
        g_esp=16;pc_network_bots_guest_config(original);
        CHECK(config[0]==16 && config[2]==0 && g_eax==0x11223344u && g_esp==20);
        /* Reproduce the real caller: its post-builder zero must be repaired
           BEFORE serialization, and again before the local pending-load copy.
           The receiver uses the host's bytes without rebuilding its own roster. */
        memory[0x76b140/4]=1;memory[5]=0x78ad58;
        config[0]=16;config[2]=0;
        g_esp=16;pc_network_bots_guest_config(original_broadcast);
        CHECK(broadcast_config[0]==1 && broadcast_config[2]==1 && broadcast_config[0x15]==42);
        CHECK(g_eax==0x11223344u && g_esp==20);
        config[2]=0;g_esp=16;
        pc_network_bots_guest_config(original_broadcast);
        CHECK(broadcast_config[2]==1);
        bot_test_enabled=0;
    }
#endif
#ifdef _WIN32
    {
        uint8_t *live=(uint8_t *)memory+0x7de400;
        live[0]=2;live[2]=6;memory[0x7356d8/4]=0x1000;
        memory[0x76b108/4]=1;
        for(unsigned slot=0;slot<17;slot++) {
            int before=originals;g_esp=16;memory[4]=0x13e0bb;memory[5]=slot;
            pc_network_bots_guest_active(original);
            if(slot>=2 && slot<8) CHECK(originals==before && g_eax==1 && g_esp==20);
            else CHECK(originals==before+1 && g_eax==0x11223344u && g_esp==20);
        }
        g_esp=16;memory[4]=0x133bc9;memory[5]=2;
        pc_network_bots_guest_active(original);CHECK(g_eax==1 && g_esp==20);
        /* In particular, do not turn AI into local controller records. */
        g_esp=16;memory[4]=0x15c5a3;memory[5]=2;
        pc_network_bots_guest_active(original);CHECK(g_eax==0x11223344u && g_esp==20);
        g_esp=16;memory[4]=0x13e0bb;enabled=0;
        pc_network_bots_guest_active(original);CHECK(g_eax==0x11223344u && g_esp==20);
        enabled=1;memory[0x7356d8/4]=0;g_esp=16;
        pc_network_bots_guest_active(original);CHECK(g_eax==0x11223344u && g_esp==20);
    }
#else
    g_esp=16;pc_network_bots_guest_active(original);
    CHECK(g_eax==0x11223344u && g_esp==20);
#endif
#ifdef _WIN32
    {
        uint8_t *live=(uint8_t *)memory+0x7de400;
        live[0]=2;live[2]=6;memory[0x7356d8/4]=0x1000;
        memory[(0x1000+2*0xea0)/4]=0x7000;
        memory[0x76b108/4]=1;
        g_esp=16;memory[5]=0x7000;pc_network_bots_guest_serializable(original);
        CHECK(g_eax==1 && g_esp==20);
        g_esp=16;memory[5]=0x7100;pc_network_bots_guest_serializable(original);
        CHECK(g_eax==0x11223344u && g_esp==20);
        g_esp=16;g_eax=0x200;g_ecx=0x7000;memory[0x200/4]=0x300;
        pc_network_bots_guest_encode(original);
        CHECK(memory[0x200/4]==0x302 && ((uint8_t *)memory)[0x300]==4 && ((uint8_t *)memory)[0x301]==2 && g_esp==20);
        g_esp=16;g_ecx=0x200;memory[0x200/4]=0x300;
        pc_network_bots_guest_decode(original);
        CHECK(g_eax==0x7000 && g_esp==20 && memory[0x200/4]==0x302);
        ((uint8_t *)memory)[0x301]=16;memory[0x200/4]=0x300;g_esp=16;
        pc_network_bots_guest_decode(original);
        CHECK(g_eax==0 && g_esp==20 && memory[0x200/4]==0x302);
        ((uint8_t *)memory)[0x301]=1;memory[0x200/4]=0x300;g_esp=16;
        pc_network_bots_guest_decode(original);CHECK(g_eax==0 && g_esp==20);
        ((uint8_t *)memory)[0x300]=1;memory[0x200/4]=0x300;g_esp=16;
        pc_network_bots_guest_decode(original);CHECK(g_eax==0x11223344u && g_esp==20);
    }
#else
    g_esp=16;pc_network_bots_guest_serializable(original);CHECK(g_eax==0x11223344u && g_esp==20);
    g_esp=16;pc_network_bots_guest_encode(original);CHECK(g_eax==0x11223344u && g_esp==20);
    g_esp=16;pc_network_bots_guest_decode(original);CHECK(g_eax==0x11223344u && g_esp==20);
#endif
#ifdef _WIN32
    {
        uint8_t *live=(uint8_t *)memory+0x7de400;
        uint8_t *actor=(uint8_t *)memory+0x1000+2*0xea0;
        float health=1.f;uint32_t life=8;
        live[0]=2;live[2]=1;memory[0x76b140/4]=1;memory[0x76b1a4/4]=15;
        memory[0x7356d8/4]=0x1000;memory[(0x1000+2*0xea0)/4]=0x7000;
        memcpy((uint8_t *)memory+0x71cc,&health,4);memcpy(actor+0x18,&life,4);
        memory[(0x7000+0x28)/4]=0;g_esp=16;pc_network_bots_guest_frame(original);
        CHECK(g_eax==0x11223344u && g_esp==20 && bot_sends==1 && bot_refreshes==1);
        CHECK((memory[(0x7000+0x28)/4]&0x61u)==0x61u && last_bot.slot==2);
        memory[(0x7000+0x28)/4]=0;life=2;memcpy(actor+0x18,&life,4);
        memory[0x7de318/4]+=3;g_esp=16;pc_network_bots_guest_frame(original);
        CHECK(!(memory[(0x7000+0x28)/4]&1));
        life=8;memcpy(actor+0x18,&life,4);memory[(0x1000+2*0xea0+0x2c)/4]=4;
        g_esp=16;pc_network_bots_guest_frame(original);CHECK(!(memory[(0x7000+0x28)/4]&1));
        memory[0x76b140/4]=2;last_bot.slot=2;last_bot.position[0]=25.f;
        last_bot.health=.75f;last_bot.max_health=1.1f;last_bot.entity_flags=0x61u;last_bot.flags[0]=4;
        memory[(0x7000+0x150)/4]=0x1234;bot_incoming=1;
        memory[0x774040/4]=3;
        memory[(0x66648c+3*16)/4]=0x0200007fu;
        memory[(0x774184+3*0x30c)/4]=99; /* connection handle is not an IP */
        g_esp=16;pc_network_bots_guest_frame(original);
        CHECK(bot_host_ip==0x0200007fu);
        CHECK(g_eax==0x11223344u && g_esp==20 && bot_refreshes==2);
        CHECK(!memcmp((uint8_t *)memory+0x7064,last_bot.position,12));
        CHECK(!memcmp((uint8_t *)memory+0x71cc,&last_bot.health,4) && !memcmp((uint8_t *)memory+0x71d8,&last_bot.max_health,4));
        CHECK(memory[(0x7000+0x150)/4]==0x1234 && memory[(0x1000+2*0xea0)/4]==0x7000);
        CHECK((memory[(0x7000+0x28)/4]&0x61u)==0x61u && memory[(0x1000+2*0xea0+0x24)/4]==4);
        memory[(0x1000+2*0xea0)/4]=0;bot_incoming=1;
        g_esp=16;pc_network_bots_guest_frame(original);
        CHECK(memory[(0x1000+2*0xea0)/4]==0x7000 && bot_refreshes==3);
        last_bot.position[0]=NAN;bot_incoming=1;
        g_esp=16;pc_network_bots_guest_frame(original);CHECK(bot_refreshes==3);

    }
#else
    g_esp=16;pc_network_bots_guest_frame(original);CHECK(g_eax==0x11223344u && g_esp==20);
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
