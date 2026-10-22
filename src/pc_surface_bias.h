#ifndef PC_SURFACE_BIAS_H
#define PC_SURFACE_BIAS_H
#include <stdlib.h>
static inline float pc_surface_bias(float delta)
{ return .5f+(128.f/255.f-.5f)*delta; }
static inline int pc_surface_bias_requested(void)
{
    static int checked,enabled;
    if(!checked) {
        const char *bias=getenv("TSFP_SURFACE_BIAS_TEST");
        const char *fractional=getenv("TSFP_SURFACE_FRACTIONAL_TEST");
        const char *precision=getenv("TSFP_SURFACE_FLOAT_TEST");
        enabled=bias && bias[0]=='1' && fractional && fractional[0]=='1' && precision && precision[0]=='1';checked=1;
    }
    return enabled;
}
#endif
