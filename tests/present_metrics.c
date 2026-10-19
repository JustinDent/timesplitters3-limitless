/* Deterministic clock samples through the production recorder. */
#include "graphics/present_metrics.h"
#include <assert.h>
int main(void) {
    TsfpPresentMetrics m={0};
    tsfp_present_record(&m,"test",1,1,0,1000000); /* skipped */
    tsfp_present_record(&m,"test",2,-1,1000000,2000000); /* failed */
    assert(!m.total && m.skipped==1 && m.failed==1 && !m.checked);
    tsfp_present_record(&m,"test",1,0,2000000,3000000);
    tsfp_present_record(&m,"test",2,1,4000000,5000000);
    tsfp_present_record(&m,"test",2,-1,6000000,7000000);
    tsfp_present_record(&m,"test",2,0,8000000,10000000);
    assert(m.total==2 && m.guest==1 && m.interpolated==1 && !m.other);
    assert(m.skipped==2 && m.failed==2 && m.last_ns==10000000 && m.rows==1);
    assert(m.output);fflush(m.output);
    /* Exercise cap/close without producing 20,000 log lines. */
    m.rows=TSFP_PRESENT_PROFILE_LIMIT-1;
    tsfp_present_record(&m,"test",0,0,11000000,12000000);
    assert(!m.output && m.rows==TSFP_PRESENT_PROFILE_LIMIT && m.other==1);
    tsfp_present_record(&m,"test",1,0,13000000,14000000);
    assert(!m.output && m.rows==TSFP_PRESENT_PROFILE_LIMIT && m.total==4);
    puts("PASS: successful presentation kinds, skipped/failed gaps and bounded profile");
    return 0;
}
