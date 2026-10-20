/* Rational host presentation clock; independent of the 60 Hz guest clock.
 * Never restart phase at guest boundaries: 144/60 is 12/5, not an integer.
 * Dropped deadlines advance to the next slot instead of creating a burst. */
#ifndef HLE_FRAME_CLOCK_H
#define HLE_FRAME_CLOCK_H
#include <stdint.h>
typedef struct {
    int64_t epoch, frequency, slot;
    int rate;
} HleFrameClock;
static int64_t hle_frame_clock_due(const HleFrameClock *c)
{
    return c->epoch + c->slot * c->frequency / c->rate;
}
static void hle_frame_clock_reset(HleFrameClock *c, int64_t now, int64_t frequency, int rate)
{
    c->epoch = now; c->frequency = frequency; c->rate = rate; c->slot = 1;
}
static void hle_frame_clock_catch_up(HleFrameClock *c, int64_t now)
{
    if (now > hle_frame_clock_due(c)) {
        int64_t slot = (now - c->epoch) * c->rate / c->frequency;
        if (c->epoch + slot * c->frequency / c->rate < now) slot++;
        if (slot > c->slot) c->slot = slot;
    }
}
#endif
