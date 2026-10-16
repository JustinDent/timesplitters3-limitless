#ifndef TSFP_PC_DISPLAY_H
#define TSFP_PC_DISPLAY_H
#ifdef _WIN32
#include <windows.h>
#else
typedef void *HWND;
#endif
#define PC_FOV_DEFAULT 55
#define PC_FOV_MIN 40
#define PC_FOV_MAX 100
int pc_display_fov(void);
int pc_display_set_fov(int degrees);
float pc_display_fov_scale(void);

typedef struct PcDisplayMode { unsigned width, height; } PcDisplayMode;
/* Called on the guest/render thread. Zero dimensions retain the legacy mode. */
PcDisplayMode pc_display_current(void);
int pc_display_valid(PcDisplayMode mode);
int pc_display_modes(HWND window, PcDisplayMode *out, int capacity);
int pc_display_apply(PcDisplayMode mode);
int pc_display_preview(PcDisplayMode mode);
int pc_display_save(void);
void pc_display_scene_size(unsigned guest_w, unsigned guest_h, unsigned *w, unsigned *h);
void pc_display_shape(unsigned *w, unsigned *h);
float pc_display_camera_factor(void);
float pc_display_hud_scale(void);
int pc_display_wide_now(void);
void pc_display_frame_done(void);
HWND xbox_D3D8Window(void);
int xbox_D3D8ChangeResolution(unsigned width, unsigned height);
#endif
