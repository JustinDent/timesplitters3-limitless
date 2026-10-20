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
 * Camera translation sub_000C73C0 discards its gate register before use;
 * direction easing sub_000C7610 only tests it, then overwrites it on the
 * direct-copy path. Their motion and easing must consume each live interval.
 * Camera effects have an outer gate at 0xCEA8D and a whole-tick decay loop
 * bounded by 0xC7D73 / 0xC7E0C. In native mode run that loop once per live
 * interval, with its two decay factors raised to elapsed ticks below.
 * Camera tracks gate evaluation at 0xCD70F, after advancing their float clock.
 * Their keyframe span at 0xCD851 needs an adjacent sample even when the
 * integer delta is zero; the existing fractional weight then selects the
 * pose at that clock time. Keep multi-tick spans and cut detection intact.
 * All other loads, including other reads of this same address, stay exact. */
static inline uint32_t pc_native_global_load(uint32_t site, uint32_t address,
                                           uint32_t value)
{
    if (site == 0xCD851u && address == 0x4E7944u && value == 0 &&
        g_tsfp_native_timing_active &&
        *(const float *)((uintptr_t)0x4E794Cu + g_xbox_mem_offset) > 0)
        return 1;
    if ((site == 0x228880u || site == 0x15EB0Cu || site == 0xD5EBFu ||
         site == 0xC73C0u || site == 0xC7610u || site == 0xCEA8Du ||
         site == 0xC7D73u || site == 0xC7E0Cu || site == 0xCD70Fu) &&
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
    /* Here the left operand is the decay factor, unlike player damping.
     * The original camera-kick setter supplies 0.8 for both channels. */
    if ((site == 0xC7DA8u || site == 0xC7DC5u) && g_tsfp_native_timing_active) {
        float delta=*(const float *)((uintptr_t)0x4E794Cu + g_xbox_mem_offset);
        if (delta != 1.0f) left=powf(left,delta);
    }
    return left*right;
}
#define RECOMP_F32_MUL_AT(site, left, right) \
    pc_native_multiply((uint32_t)(site), (float)(left), (float)(right))
/* Scripted camera direction closes 1 / remaining_ticks of the angular gap
 * per original update. Normalize that decay to the live game interval,
 * preserving the original coefficient for one tick. Scaling the pause gate
 * alone would make this easing run native_fps / 60 times too quickly.
 * The original caller supplies a positive integer divisor here; its zero
 * and negative cases copy the target directly without reaching this divide. */
static inline float pc_native_divide(uint32_t site, float left, float right)
{
    /* Track blending uses an integer keyframe span. The original float delta
     * matched it; fractional catch-up no longer does. Normalize by the span
     * actually selected at 0xCD851, retaining the original multi-tick path. */
    if (site == 0xCD88Eu && g_tsfp_native_timing_active) {
        uint32_t span=*(const uint32_t *)((uintptr_t)0x4E7944u + g_xbox_mem_offset);
        right=span ? (float)span : 1.0f;
    }
    float value=left/right;
    if (site == 0xC7651u && g_tsfp_native_timing_active) {
        float delta=*(const float *)((uintptr_t)0x4E794Cu + g_xbox_mem_offset);
        if (delta == 1.0f) return value;
        if (delta <= 0.0f) return 0.0f;
        if (value >= 1.0f) return 1.0f;
        if (value > 0.0f)
            return -expm1f(delta*log1pf(-value));
    }
    return value;
}
#define RECOMP_F32_DIV_AT(site, left, right) \
    pc_native_divide((uint32_t)(site), (float)(left), (float)(right))
#endif
