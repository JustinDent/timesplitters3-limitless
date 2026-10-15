/* Content keys and strict legacy BC1/BC2/BC3 DDS validation. */
#ifndef TSFP_TEXTURE_PACK_FORMAT_H
#define TSFP_TEXTURE_PACK_FORMAT_H
#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint32_t width, height, levels, format, bytes;
    uint32_t offset[13], pitch[13], size[13];
} TsfpDDS;

static uint32_t tsfp_u32(const unsigned char *p) {
    return (uint32_t)p[0] | (uint32_t)p[1]<<8 | (uint32_t)p[2]<<16 | (uint32_t)p[3]<<24;
}
static uint64_t tsfp_texture_key(uint32_t format, uint32_t w, uint32_t h,
                                 const unsigned char *data, size_t size) {
    uint64_t key = UINT64_C(0xcbf29ce484222325);
    uint32_t words[3]; unsigned i, j;
    words[0]=format; words[1]=w; words[2]=h;
    for (i=0;i<3;i++) for(j=0;j<4;j++)
        key=(key ^ ((words[i]>>(8*j))&255))*UINT64_C(0x100000001b3);
    for (i=0;i<size;i++) key=(key ^ data[i])*UINT64_C(0x100000001b3);
    return key;
}
static int tsfp_dds_parse(const unsigned char *p, size_t length, TsfpDDS *out) {
    uint32_t fourcc, w, h, n, block, i, at=128, required=1, m;
    if(length<128 || tsfp_u32(p)!=0x20534444 || tsfp_u32(p+4)!=124 ||
       tsfp_u32(p+76)!=32 || tsfp_u32(p+80)!=4 || tsfp_u32(p+112)!=0 ||
       tsfp_u32(p+24)!=0) return 0;
    w=tsfp_u32(p+16);h=tsfp_u32(p+12);n=tsfp_u32(p+28);
    if(!w || !h || w>4096 || h>4096 || (w&(w-1)) || (h&(h-1))) return 0;
    for(m=w>h?w:h;m>1;m>>=1) required++;
    /* Replacements require the full mip chain, preventing distant shimmer. */
    if(n!=required || n>13) return 0;
    fourcc=tsfp_u32(p+84);
    if(fourcc==0x31545844) {out->format=12;block=8;}
    else if(fourcc==0x33545844) {out->format=14;block=16;}
    else if(fourcc==0x35545844) {out->format=15;block=16;}
    else return 0;
    out->width=w;out->height=h;out->levels=n;
    for(i=0;i<n;i++) {
        uint32_t pitch=((w+3)/4)*block, size=pitch*((h+3)/4);
        out->offset[i]=at;out->pitch[i]=pitch;out->size[i]=size;
        if(at>length || size>length-at) return 0;
        at+=size;w=w>1?w>>1:1;h=h>1?h>>1:1;
    }
    out->bytes=at-128;
    return at==length;
}
#endif
