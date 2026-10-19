#ifndef TSFP_PC_MENU_SAMPLING_H
#define TSFP_PC_MENU_SAMPLING_H
/* Catmull-Rom reconstruction of the original texels. Only magnification uses
 * this path; mipmapped/minified samples and point-filtered stages stay native.
 * Clamp to the four central texels to suppress ringing at glyph/atlas edges.
 * SampleLevel at texel centres preserves the sampler's wrap/clamp modes. */
#define TSFP_MENU_HLSL \
"float4 menu_weights(float f) {\n" \
"    return float4(f*(-0.5+f*(1.0-0.5*f)), 1.0+f*f*(-2.5+1.5*f),\n" \
"                  f*(0.5+f*(2.0-1.5*f)), f*f*(-0.5+0.5*f));\n" \
"}\n" \
"float4 menu_sample(Texture2D t, SamplerState s, float2 uv, float enabled) {\n" \
"    float4 original = t.Sample(s, uv);\n" \
"    float coverage_width = max(fwidth(original.a), 0.5);\n" \
"    if (enabled < 0.5) return original;\n" \
"    uint width, height; t.GetDimensions(width, height);\n" \
"    float2 size = float2(width, height);\n" \
"    float footprint = max(length(ddx(uv)*size), length(ddy(uv)*size));\n" \
"    float blend = saturate((1.0-footprint)*2.0);\n" \
"    if (blend <= 0.0) return original;\n" \
"    float2 pixel = uv*size-0.5, base = floor(pixel), f = frac(pixel);\n" \
"    float4 wx = menu_weights(f.x), wy = menu_weights(f.y);\n" \
"    float4 result = 0.0, lo = 1.0, hi = 0.0;\n" \
"    [unroll] for (int y=0; y<4; ++y) {\n" \
"        [unroll] for (int x=0; x<4; ++x) {\n" \
"            float4 v = t.SampleLevel(s, (base+float2(x-1,y-1)+0.5)/size, 0);\n" \
"            result += v*wx[x]*wy[y];\n" \
"            if (x>=1 && x<=2 && y>=1 && y<=2) { lo=min(lo,v); hi=max(hi,v); }\n" \
"        }\n" \
"    }\n" \
"    result = lerp(original, clamp(result,lo,hi), blend);\n" \
"    if (enabled > 1.5) result.a = lerp(result.a,\n" \
"        saturate((result.a-0.5)/coverage_width+0.5), blend);\n" \
"    return result;\n" \
"}\n"

/* Called only by the Windows renderer, after the host has bound its textures.
 * tex_scale.z is spare per-stage constant space in both pixel shader paths. */
#ifdef TSFP_PC_DISPLAY
#include "pc_display.h"
static float pc_menu_filter_stage(unsigned stage)
{
    IDirect3DBaseTexture8 *base=d3d8_GetStageTexture(stage);
    const DWORD *tss=d3d8_GetTSS(stage);
    D3D8Texture *texture;
    if(!pc_display_menu_filter() || !base || !tss ||
       tss[D3DTSS_MAGFILTER]!=D3DTEXF_LINEAR ||
       IDirect3DBaseTexture8_GetType(base)!=D3DRTYPE_TEXTURE) return 0.f;
    texture=(D3D8Texture *)base;
    if(texture->usage || texture->screen_copy)return 0.f;
    /* Reconstruct glyph coverage only inside the verified main-menu row
       painter. Other alpha artwork keeps its gradients and opacity. */
    D3DFORMAT format=d3d8_base_format(base);
    return pc_display_menu_filter()==2 &&
        (format==D3DFMT_A8 || format==D3DFMT_LIN_A8) ? 2.f : 1.f;
}
#endif
#endif
