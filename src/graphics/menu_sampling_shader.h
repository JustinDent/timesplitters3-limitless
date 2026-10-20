#ifndef TSFP_MENU_SAMPLING_SHADER_H
#define TSFP_MENU_SAMPLING_SHADER_H
/* One algorithm for both backends. Dialects below only map resource access,
 * vector types and intrinsic names. Keep reconstruction/coverage rules here;
 * no host headers or graphics API are required to consume this source. */
#define TSFP_MENU_SHADER_BODY \
"TSFP_MENU_VEC4 menu_weights(float f) {\n" \
"    return TSFP_MENU_VEC4(f*(-0.5+f*(1.0-0.5*f)), 1.0+f*f*(-2.5+1.5*f),\n" \
"                  f*(0.5+f*(2.0-1.5*f)), f*f*(-0.5+0.5*f));\n" \
"}\n" \
"TSFP_MENU_VEC4 menu_sample(TSFP_MENU_SAMPLERS, TSFP_MENU_VEC2 uv, float enabled) {\n" \
"    TSFP_MENU_VEC4 original = TSFP_MENU_SAMPLE(uv);\n" \
"    float coverage_width = max(fwidth(original.a), 0.5);\n" \
"    if (enabled < 0.5) return original;\n" \
"    TSFP_MENU_VEC2 size = TSFP_MENU_SIZE;\n" \
"    float footprint = max(length(TSFP_MENU_DX(uv)*size), length(TSFP_MENU_DY(uv)*size));\n" \
"    float blend = TSFP_MENU_SAT((1.0-footprint)*2.0);\n" \
"    if (blend <= 0.0) return original;\n" \
"    TSFP_MENU_VEC2 pixel = uv*size-0.5, base = floor(pixel), f = TSFP_MENU_FRAC(pixel);\n" \
"    TSFP_MENU_VEC4 wx = menu_weights(f.x), wy = menu_weights(f.y);\n" \
"    TSFP_MENU_VEC4 result = TSFP_MENU_VEC4(0.0,0.0,0.0,0.0), lo = TSFP_MENU_VEC4(1.0,1.0,1.0,1.0), hi = TSFP_MENU_VEC4(0.0,0.0,0.0,0.0);\n" \
"    TSFP_MENU_UNROLL for (int y=0; y<4; ++y) {\n" \
"        TSFP_MENU_UNROLL for (int x=0; x<4; ++x) {\n" \
"            TSFP_MENU_VEC4 v = TSFP_MENU_SAMPLE_LOD((base+TSFP_MENU_VEC2(x-1,y-1)+0.5)/size);\n" \
"            result += v*wx[x]*wy[y];\n" \
"            if (x>=1 && x<=2 && y>=1 && y<=2) { lo=min(lo,v); hi=max(hi,v); }\n" \
"        }\n" \
"    }\n" \
"    result = TSFP_MENU_MIX(original, clamp(result,lo,hi), blend);\n" \
"    if (enabled > 1.5) result.a = TSFP_MENU_MIX(result.a,\n" \
"        TSFP_MENU_SAT((result.a-0.5)/coverage_width+0.5), blend);\n" \
"    return result;\n" \
"}\n"

#define TSFP_MENU_SHADER_UNDEF \
"#undef TSFP_MENU_VEC2\n" \
"#undef TSFP_MENU_VEC4\n" \
"#undef TSFP_MENU_SAMPLERS\n" \
"#undef TSFP_MENU_SAMPLE\n" \
"#undef TSFP_MENU_SIZE\n" \
"#undef TSFP_MENU_SAMPLE_LOD\n" \
"#undef TSFP_MENU_DX\n" \
"#undef TSFP_MENU_DY\n" \
"#undef TSFP_MENU_SAT\n" \
"#undef TSFP_MENU_FRAC\n" \
"#undef TSFP_MENU_MIX\n" \
"#undef TSFP_MENU_UNROLL\n"

#define TSFP_MENU_HLSL_ADAPTER \
"float2 tsfp_menu_size(Texture2D t) { uint w,h; t.GetDimensions(w,h); return float2(w,h); }\n" \
"#define TSFP_MENU_VEC2 float2\n" \
"#define TSFP_MENU_VEC4 float4\n" \
"#define TSFP_MENU_SAMPLERS Texture2D t, SamplerState s\n" \
"#define TSFP_MENU_SAMPLE(uv) t.Sample(s,uv)\n" \
"#define TSFP_MENU_SIZE tsfp_menu_size(t)\n" \
"#define TSFP_MENU_SAMPLE_LOD(uv) t.SampleLevel(s,uv,0)\n" \
"#define TSFP_MENU_DX ddx\n" \
"#define TSFP_MENU_DY ddy\n" \
"#define TSFP_MENU_SAT saturate\n" \
"#define TSFP_MENU_FRAC frac\n" \
"#define TSFP_MENU_MIX lerp\n" \
"#define TSFP_MENU_UNROLL [unroll]\n"

#define TSFP_MENU_GLSL_ADAPTER \
"#define TSFP_MENU_VEC2 vec2\n" \
"#define TSFP_MENU_VEC4 vec4\n" \
"#define TSFP_MENU_SAMPLERS sampler2D t\n" \
"#define TSFP_MENU_SAMPLE(uv) texture(t,uv)\n" \
"#define TSFP_MENU_SIZE vec2(textureSize(t,0))\n" \
"#define TSFP_MENU_SAMPLE_LOD(uv) textureLod(t,uv,0.0)\n" \
"#define TSFP_MENU_DX dFdx\n" \
"#define TSFP_MENU_DY dFdy\n" \
"#define TSFP_MENU_SAT(x) clamp(x,0.0,1.0)\n" \
"#define TSFP_MENU_FRAC fract\n" \
"#define TSFP_MENU_MIX mix\n" \
"#define TSFP_MENU_UNROLL\n"

#define TSFP_MENU_HLSL TSFP_MENU_HLSL_ADAPTER TSFP_MENU_SHADER_BODY TSFP_MENU_SHADER_UNDEF
#define TSFP_MENU_GLSL TSFP_MENU_GLSL_ADAPTER TSFP_MENU_SHADER_BODY TSFP_MENU_SHADER_UNDEF
#endif
