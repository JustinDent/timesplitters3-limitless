#ifndef TSFP_PC_NATIVE_NUMERIC_H
#define TSFP_PC_NATIVE_NUMERIC_H
#include <stddef.h>
#include <stdint.h>
#include <math.h>
extern ptrdiff_t g_xbox_mem_offset;
extern int g_tsfp_native_timing_active;
/* These original integer-to-float reads are continuous motion consumers of
 * the engine delta (animation, particles, camera easing). Native updates use
 * the adjacent fractional delta. Integer arithmetic and counters keep using
 * whole ticks, and the original path retains its exact numeric conversion.
 * Use the live float field: guest slow-motion/temporary pause scopes may
 * change it after the main timing routine has returned. */
static inline double pc_native_numeric(uint32_t address, int32_t value)
{
    if ((address == 0x4E7940u || address == 0x4E7944u) && g_tsfp_native_timing_active)
        return *(const float *)((uintptr_t)(address + 8u) + g_xbox_mem_offset);
    return (double)value;
}
#define RECOMP_I32_TO_F32(address, value) ((float)pc_native_numeric((uint32_t)(address), (int32_t)(value)))
#define RECOMP_I32_TO_F64(address, value) pc_native_numeric((uint32_t)(address), (int32_t)(value))
/* US script dispatcher sub_00228870 uses this load only as a pause gate.
 * At native rates a zero whole tick still contains fractional game time.
 * Skipping the dispatcher loses animation events whose previous/current
 * bounds are replaced by the next animation update. Let it consume every
 * advancing interval without changing the whole-tick field used by timers.
 * NPC dispatcher sub_0015EAE0 likewise tests its load against one, then
 * overwrites the register before use. Skipping it discards fractional root
 * motion and slows its elapsed-time counters by 60 / native FPS.
 * Player dispatcher sub_000D5EB0 also overwrites its pause-gate register
 * immediately after the test; its movement consumers use fractional time.
 * All other loads, including other reads of this same address, stay exact. */
static inline uint32_t pc_native_global_load(uint32_t site, uint32_t address,
                                           uint32_t value)
{
    if ((site == 0x228880u || site == 0x15EB0Cu || site == 0xD5EBFu) &&
        address == 0x4E7944u &&
        g_tsfp_native_timing_active)
        return *(const float *)((uintptr_t)0x4E794Cu + g_xbox_mem_offset) > 0;
    return value;
}
#define RECOMP_LOAD_GLOBAL_U32(site, address, value) \
    pc_native_global_load((uint32_t)(site), (uint32_t)(address), (uint32_t)(value))
/* Player+0x1008 is halved by the US update at 0xD5F60. Once the update
 * runs every fractional frame, retain the same half-life in game time.
 * Do not rewrite the shared 0.5 constant or unrelated multiplications. */
static inline float pc_native_multiply(uint32_t site, float left, float right)
{
    if (site == 0xD5F60u && g_tsfp_native_timing_active) {
        float delta=*(const float *)((uintptr_t)0x4E794Cu + g_xbox_mem_offset);
        right=powf(right,delta);
    }
    return left*right;
}
#define RECOMP_F32_MUL_AT(site, left, right) \
    pc_native_multiply((uint32_t)(site), (float)(left), (float)(right))
#endif
