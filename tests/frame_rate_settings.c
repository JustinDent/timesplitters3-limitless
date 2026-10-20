#include "recomp_config.h"
#include "pc_frame_rate_settings.h"
#include "pc_native_rate.h"
#include <assert.h>
static void env(const char *key,const char *value) {
#ifdef _WIN32
    assert(!_putenv_s(key,value));
#else
    assert(!setenv(key,value,1));
#endif
}
int main(int argc,char **argv)
{
    assert(argc==4);
    env("TSFP_DISPLAY_CONFIG",argv[1]);env("TSFP_NATIVE_FPS","144");
    env("RECOMP_DISPLAY_CONFIG",argv[1]);
    int before=atoi(argv[2]),next=atoi(argv[3]);
    assert(pc_native_rate()==before);
    assert(!pc_frame_rate_save(30) && !pc_frame_rate_save(240));
    assert(pc_frame_rate_save(next));assert(pc_frame_rate_read()==next);
    /* Saving updates the next launch, never an already running clock. */
    assert(pc_native_rate()==before);
    assert(!strcmp(recomp_config_lookup("RECOMP_FRAME_CAP","frame_cap"),"60"));
    puts("PASS: persisted rate, startup snapshot, original clock and invalid values");
    return 0;
}
