#ifndef PC_SURFACE_PRECISION_H
#define PC_SURFACE_PRECISION_H
#include <stdint.h>
#include <string.h>
/* Bounded host-only precision hints. Guest texture headers/data stay intact. */
typedef struct { uint32_t targets[15]; } PcSurfacePrecision;
static inline void pc_surface_precision_reset(PcSurfacePrecision *state)
{ memset(state,0,sizeof *state); }
static inline int pc_surface_precision_set(PcSurfacePrecision *state,unsigned slot,uint32_t va)
{
    if(slot>=15 || va<0x10000u || va>0x83ffffccu)return 0;
    state->targets[slot]=va;return 1;
}
static inline uint32_t pc_surface_precision_format(const PcSurfacePrecision *state,uint32_t va,uint32_t format)
{
    if(format!=0x06u && format!=0x12u)return format;
    for(unsigned i=0;i<15;i++)if(va && state->targets[i]==va)return 0x34u;
    return format;
}
#endif
