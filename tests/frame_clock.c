#include "../src/pc_frame_clock.h"
#include <assert.h>
#include <stdio.h>

int main(void)
{
    const int rates[] = {120, 144};
    int r;
    for (r = 0; r < 2; r++) {
        HleFrameClock clock;
        const int64_t frequency = 10000000, epoch = 123456789;
        int64_t previous = epoch, i;
        hle_frame_clock_reset(&clock, epoch, frequency, rates[r]);
        /* Ten simulated minutes, including fractional 144/60 boundaries.
         * Every deadline is within one timer tick of the ideal lattice. */
        for (i = 1; i <= rates[r] * 600; i++) {
            int64_t due = hle_frame_clock_due(&clock);
            assert(due > previous);
            assert(due - previous >= frequency / rates[r]);
            assert(due - previous <= frequency / rates[r] + 1);
            assert((due - epoch) * rates[r] <= i * frequency);
            assert(i * frequency - (due - epoch) * rates[r] < rates[r]);
            previous = due;
            clock.slot++;
        }
        assert(previous == epoch + frequency * 600);
        /* A stall drops expired slots. No catch-up burst or phase drift. */
        hle_frame_clock_reset(&clock, epoch, frequency, rates[r]);
        hle_frame_clock_catch_up(&clock, epoch + frequency * 2 + 12345);
        assert(hle_frame_clock_due(&clock) >= epoch + frequency * 2 + 12345);
        assert(hle_frame_clock_due(&clock) - (epoch + frequency * 2 + 12345) < frequency / rates[r] + 1);
        previous = clock.slot;
        hle_frame_clock_catch_up(&clock, epoch);
        assert(clock.slot == previous);
    }
    puts("PASS: 120/144 Hz rational cadence, long-run phase and stall recovery");
    return 0;
}
