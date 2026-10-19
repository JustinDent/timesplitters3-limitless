#ifndef TSFP_POINT_SIZE_SHADER_H
#define TSFP_POINT_SIZE_SHADER_H
/* D3D11 rasterizes point lists as one-pixel points. Vulkan requires an explicit
 * PointSize output even for that default. Keep this API mapping shared by the
 * fixed-function and NV2A program generators. Larger guest point sprites still
 * require geometry expansion; this does not add that feature. */
#define TSFP_POINT_SIZE_FIELD \
    "#ifdef __spirv__\n" \
    "    [[vk::builtin(\"PointSize\")]] float rhiPointSize : PSIZE1;\n" \
    "#endif\n"
#define TSFP_POINT_SIZE_INIT \
    "#ifdef __spirv__\n" \
    "    o.rhiPointSize = 1.0;\n" \
    "#endif\n"
#endif
