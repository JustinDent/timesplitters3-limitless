#include <stdio.h>
#include "../src/pc_network_bots.h"
#include "../src/pc_network_bots_state.h"
#define CHECK(x) do { if (!(x)) { fprintf(stderr,"FAIL line %u: %s\n",(unsigned)__LINE__,#x);return 1; } } while (0)
int main(void)
{
    uint8_t config[PC_NETWORK_MATCH_BYTES],before[PC_NETWORK_MATCH_BYTES];
    uint32_t active[16]={1,1};unsigned i,mode;
    PcNetworkBot roster[10];
    for (i=0;i<10;i++) { PcNetworkBot bot={42,0,2,i<3,-1,0,-1};roster[i]=bot; }
    memset(config,0xa5,sizeof config);config[0]=16;config[1]=1;config[2]=0;
    config[0x14]=42;config[0x58]=0;
    memcpy(before,config,sizeof config);
    CHECK(!pc_network_bots_configure(config,sizeof config,active,roster,0,1,1,11));
    CHECK(!memcmp(config,before,sizeof config));
    CHECK(!pc_network_bots_configure(config,sizeof config,active,roster,1,1,0,11));
    CHECK(!memcmp(config,before,sizeof config));
    CHECK(!pc_network_bots_configure(config,sizeof config,active,roster,1,0,1,11));
    CHECK(!memcmp(config,before,sizeof config));
    for (mode=0;mode<22;mode++) if (mode<11 || mode>13) {
        CHECK(!pc_network_bots_configure(config,sizeof config,active,roster,1,1,1,mode));
        CHECK(!memcmp(config,before,sizeof config));
    }
    CHECK(!pc_network_bots_configure(config,sizeof config-1,active,roster,1,1,1,11));
    CHECK(!memcmp(config,before,sizeof config));
    CHECK(pc_network_bots_configure(config,sizeof config,active,roster,1,1,1,11)==3);
    CHECK(config[0]==2 && config[1]==1 && config[2]==3);
    for (i=2;i<5;i++) CHECK(config[0x14+i]==42 && config[0x24+i]==0 && config[0x34+i]==2 && config[0x44+i]==255);
    /* Existing humans and unrelated settings remain byte-for-byte intact. */
    for (i=3;i<sizeof config;i++) {
        if ((i>=0x16 && i<0x19) || (i>=0x26 && i<0x29) ||
            (i>=0x36 && i<0x39) || (i>=0x46 && i<0x49)) continue;
        CHECK(config[i]==before[i]);
    }
    /* Sparse human IDs must not be reused; total slots never exceeds sixteen. */
    active[13]=1;config[0x58]=1;
    for (i=0;i<10;i++) { roster[i].enabled=1;roster[i].team=(int)i%2;roster[i].skill=4; }
    CHECK(pc_network_bots_configure(config,sizeof config,active,roster,1,1,1,12)==2);
    CHECK(config[0]==14 && config[2]==2);
    CHECK(config[0x24+14]==1 && config[0x24+15]==2);
    config[0x56]=4;roster[0].team=3;roster[1].team=2;
    CHECK(pc_network_bots_configure(config,sizeof config,active,roster,1,1,1,12)==2);
    CHECK(config[0x24+14]==4 && config[0x24+15]==3);
    memcpy(before,config,sizeof config);active[15]=1;
    CHECK(!pc_network_bots_configure(config,sizeof config,active,roster,1,1,1,12));
    CHECK(!memcmp(config,before,sizeof config));
    memcpy(config,before,sizeof config);active[15]=0;
    for (i=0;i<10;i++) roster[i].character=-1;
    CHECK(!pc_network_bots_configure(config,sizeof config,active,roster,1,1,1,11));
    CHECK(!memcmp(config,before,sizeof config));
    memset(active,0,sizeof active);
    CHECK(!pc_network_bots_configure(config,sizeof config,active,roster,1,1,1,11));
    CHECK(!memcmp(config,before,sizeof config));
    active[0]=1;config[1]=1;
    roster[0].character=42;roster[0].skill=5;
    CHECK(!pc_network_bots_configure(config,sizeof config,active,roster,1,1,1,11));
    roster[0].skill=2;roster[0].override_enabled=0;
    CHECK(!pc_network_bots_configure(config,sizeof config,active,roster,1,1,1,11));
    roster[0].override_enabled=-1;roster[0].variant=256;
    CHECK(!pc_network_bots_configure(config,sizeof config,active,roster,1,1,1,11));
    {
        PcNetworkBotState state={0};state.slot=2;state.health=1;
        CHECK(pc_network_bot_state_valid(&state,2,6));
        state.body_present=2;CHECK(!pc_network_bot_state_valid(&state,2,6));
        state.body_present=1;
        state.position[0]=NAN;CHECK(!pc_network_bot_state_valid(&state,2,6));
        state.position[0]=0;state.armor=INFINITY;
        CHECK(!pc_network_bot_state_valid(&state,2,6));
        state.armor=0;state.bounds_radius[1]=-1.f;
        CHECK(!pc_network_bot_state_valid(&state,2,6));
        state.bounds_radius[1]=0;state.bounds_center[0]=NAN;
        CHECK(!pc_network_bot_state_valid(&state,2,6));
        state.bounds_center[0]=0;state.clip_channels[0]=NAN;
        CHECK(!pc_network_bot_state_valid(&state,2,6));
        state.clip_channels[0]=0;state.clip=-1;
        CHECK(!pc_network_bot_state_valid(&state,2,6));
        state.clip=0;state.next_channels[17]=INFINITY;
        CHECK(!pc_network_bot_state_valid(&state,2,6));
        state.next_channels[17]=0;state.overlay_phase=3;
        CHECK(!pc_network_bot_state_valid(&state,2,6));
        state.overlay_phase=0;state.shot.serial=1;state.shot.weapon=7;state.shot.target_slot=1;
        CHECK(pc_network_bot_state_valid(&state,2,6));
        state.shot.hand=2;CHECK(!pc_network_bot_state_valid(&state,2,6));
        state.shot.hand=0;state.shot.weapon=66;CHECK(!pc_network_bot_state_valid(&state,2,6));
        state.shot.weapon=7;state.shot.target_slot=8;CHECK(!pc_network_bot_state_valid(&state,2,6));
        state.shot.target_slot=1;state.shot.transform[11]=NAN;CHECK(!pc_network_bot_state_valid(&state,2,6));
        state.shot.transform[11]=0;state.shot.aim[2]=INFINITY;CHECK(!pc_network_bot_state_valid(&state,2,6));
        state.shot.aim[2]=0;state.slot=1;CHECK(!pc_network_bot_state_valid(&state,2,6));
        state.slot=16;CHECK(!pc_network_bot_state_valid(&state,2,6));
    }
    puts("PASS: opt-in host config, preserved human IDs/settings, capacity and invalid-state rejection");
    return 0;
}
