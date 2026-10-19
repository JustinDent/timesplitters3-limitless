/* SDL owns OS events and physical window dimensions; renderer policy is shared. */
#include "renderer_host_sdl.h"
#include "input_sdl.h"
#include "rhi.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

static struct {
    SDL_Window *window;
    void (*on_quit)(void);
    uint32_t width, height;
} host;
static pthread_mutex_t size_lock = PTHREAD_MUTEX_INITIALIZER;

static void refresh_size(void)
{
    int width = 0, height = 0;
    if (!SDL_IsMainThread() || !host.window) return;
    if (!SDL_GetWindowSizeInPixels(host.window, &width, &height) || width <= 0 || height <= 0)
        return;
    pthread_mutex_lock(&size_lock);
    host.width = (uint32_t)width;
    host.height = (uint32_t)height;
    pthread_mutex_unlock(&size_lock);
}

static int drawable_size(void *context, void *window, uint32_t *width, uint32_t *height)
{
    (void)context;
    if (!host.window || window != host.window) return 0;
    refresh_size();
    /* Worker rendering uses the last main-thread snapshot. SDL window queries
     * and event processing must stay on the Cocoa main thread. */
    pthread_mutex_lock(&size_lock);
    *width = host.width;
    *height = host.height;
    pthread_mutex_unlock(&size_lock);
    return *width != 0 && *height != 0;
}

static void pump_events(void *context)
{
    SDL_Event event;
    (void)context;
    if (!SDL_IsMainThread() || !host.window) return;
    tsfp_native_input_frame(host.window);
    while (SDL_PollEvent(&event)) {
        tsfp_native_input_event(&event, host.window);
        if (event.type == SDL_EVENT_QUIT ||
            (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED &&
             event.window.windowID == SDL_GetWindowID(host.window))) {
            fprintf(stderr, "[native-input] window close requested\n");
            if (host.on_quit) host.on_quit();
            exit(0);
        }
    }
    refresh_size();
}

void tsfp_renderer_host_bind(SDL_Window *window, void (*on_quit)(void))
{
    /* Binding/unbinding is outside the render-thread lifetime. */
    host.window = window;
    host.on_quit = on_quit;
    host.width = host.height = 0;
    if (window) {
        RhiHost callbacks = {NULL, pump_events, drawable_size};
        refresh_size();
        rhi_set_host(&callbacks);
    } else {
        rhi_set_host(NULL);
    }
}
