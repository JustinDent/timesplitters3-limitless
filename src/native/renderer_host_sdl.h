#ifndef TSFP_RENDERER_HOST_SDL_H
#define TSFP_RENDERER_HOST_SDL_H
#include <SDL3/SDL.h>
/* Call on the main thread before rendering starts. Unbind with NULL after
 * rendering stops and before destroying the window. */
void tsfp_renderer_host_bind(SDL_Window *window, void (*on_quit)(void));
#endif
