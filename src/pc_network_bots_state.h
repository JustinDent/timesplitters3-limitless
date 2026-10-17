#ifndef TSFP_PC_NETWORK_BOTS_STATE_H
#define TSFP_PC_NETWORK_BOTS_STATE_H
#include <stdint.h>
#include <math.h>
/* Wire values only: no guest pointers, controller records or animation handles. */
typedef struct PcNetworkBotState {
    uint32_t slot,tick,entity_flags,flags[3],life[4],animation;
    float position[3],health,armor;
    uint32_t last_damage_tick;
    float max_health,health_scale[2];
    float angles[3];
    int32_t clip;
    float clip_channels[12];
    int16_t scores[42];
} PcNetworkBotState;
static int pc_network_bot_state_valid(const PcNetworkBotState *state,unsigned humans,unsigned bots)
{
    unsigned i;
    if(!humans || !bots || bots>10 || humans+bots>16 ||
       state->slot<humans || state->slot>=humans+bots) return 0;
    for(i=0;i<3;i++) if(!isfinite(state->position[i]) || fabsf(state->position[i])>1000000.f) return 0;
    for(i=0;i<2;i++) if(!isfinite(state->health_scale[i]) || fabsf(state->health_scale[i])>1000.f) return 0;
    if(state->clip<0 || state->clip>4095) return 0;
    for(i=0;i<3;i++) if(!isfinite(state->angles[i]) || fabsf(state->angles[i])>100000.f) return 0;
    for(i=0;i<12;i++) if(!isfinite(state->clip_channels[i]) || fabsf(state->clip_channels[i])>1000000.f) return 0;
    return isfinite(state->health) && state->health>=-1000.f && state->health<=1000.f &&
           isfinite(state->armor) && state->armor>=0.f && state->armor<=1000.f &&
           isfinite(state->max_health) && state->max_health>=0.f && state->max_health<=1000.f;
}
#endif
