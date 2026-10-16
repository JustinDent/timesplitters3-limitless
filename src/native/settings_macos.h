#ifndef TSFP_SETTINGS_MACOS_H
#define TSFP_SETTINGS_MACOS_H
#include <SDL3/SDL.h>
void tsfp_native_settings_attach(SDL_Window *window);
void tsfp_native_settings_show(SDL_Window *window);
void tsfp_native_settings_release_input(SDL_Window *window);
int tsfp_native_settings_test(void);
int tsfp_native_capture_test(void);
int tsfp_native_debug_bitmap(const char *text,unsigned char *pixels,int width,int height);
void pc_controls_get_mouse(double *sensitivity,int *enabled,int *invert_y);
void pc_controls_set_mouse(double sensitivity,int enabled,int invert_y);
void tsfp_native_key_name(int key,char *out,size_t bytes);
#endif
