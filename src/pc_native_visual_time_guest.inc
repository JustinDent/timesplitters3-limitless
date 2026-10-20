#include "pc_native_visual_time.h"
static PcNativeVisualTime pc_native_level_visual_time;
uint32_t pc_native_visual_store(uint32_t site,uint32_t value)
{
    /* The six ordinary stores belong to mutually exclusive level-update
     * branches. All other direct writes (init/demo/network) are rebases.
     * Reading the live delta here respects temporary guest time scopes. */
    int increment=site==0x688BBu || site==0x68F8Eu || site==0x691F3u ||
                  site==0x69239u || site==0x69265u || site==0x69325u;
    uint32_t previous=*(const uint32_t *)((uintptr_t)0x7DE318u+g_xbox_mem_offset);
    uint32_t whole=*(const uint32_t *)((uintptr_t)0x4E7944u+g_xbox_mem_offset);
    float delta=*(const float *)((uintptr_t)0x4E794Cu+g_xbox_mem_offset);
    pc_native_visual_time_store(&pc_native_level_visual_time,
        g_tsfp_native_timing_active,increment,previous,value,whole,delta);
    return value;
}
float pc_native_visual_sample(uint32_t value)
{
    return (float)pc_native_visual_time_sample(&pc_native_level_visual_time,value);
}
