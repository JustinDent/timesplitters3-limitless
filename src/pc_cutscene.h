#ifndef TSFP_PC_CUTSCENE_H
#define TSFP_PC_CUTSCENE_H

/* The US game's camera/viewport coordinates have a 4:3 reference shape.
 * Preserve the original fully-open picture's aspect, then scale the current
 * animated matte by the resulting height. Work in guest pixels: each renderer
 * already maps those coordinates to the selected output resolution.
 * At wider-than-cinematic aspects, the matching side matte preserves framing.
 * A zero output mode retains the original/legacy presentation. */
static inline int pc_cutscene_bar_height(unsigned width, unsigned height,
                                         int viewport_height, int full_bar,
                                         int animated_bar)
{
    double factor, full, adjusted;
    if (!width || !height || viewport_height <= 0 || full_bar <= 0 ||
        full_bar >= viewport_height / 2 || animated_bar < 0 || animated_bar > full_bar)
        return animated_bar;
    factor = (double)width * 3.0 / ((double)height * 4.0);
    full = (viewport_height - (viewport_height - 2.0 * full_bar) * factor) / 2.0;
    if (full <= 0) return 0;
    adjusted = animated_bar * full / full_bar;
    return (int)(adjusted + .5);
}

static inline int pc_cutscene_bar_width(unsigned width, unsigned height,
                                        int viewport_width, int viewport_height,
                                        int full_bar, int animated_bar)
{
    double factor, full;
    if (!width || !height || viewport_width <= 0 || viewport_height <= 0 ||
        full_bar <= 0 || full_bar >= viewport_height / 2 ||
        animated_bar < 0 || animated_bar > full_bar)
        return 0;
    factor = (double)width * 3.0 / ((double)height * 4.0);
    full = viewport_width * (1.0 - viewport_height /
           ((viewport_height - 2.0 * full_bar) * factor)) / 2.0;
    return full > 0 ? (int)(animated_bar * full / full_bar + .5) : 0;
}

static inline float pc_cutscene_camera_zoom(unsigned width, unsigned height,
                                             int viewport_height, int full_bar,
                                             int animated_bar)
{
    int adjusted;
    if (viewport_height <= 0 || animated_bar <= 0 ||
        animated_bar >= viewport_height / 2 || full_bar <= 0 ||
        animated_bar > full_bar || full_bar >= viewport_height / 2)
        return 1.f;
    adjusted = pc_cutscene_bar_height(width,height,viewport_height,full_bar,animated_bar);
    return (float)(viewport_height - 2 * adjusted) /
           (float)(viewport_height - 2 * animated_bar);
}
#endif
