#include "rhi_backend.h"
#include <assert.h>
#include <stdio.h>
#ifdef _WIN32
/* Host contract does not create a graphics device. */
const RhiBackend rhi_d3d11_backend = {0};
#endif
static int pumps, size_calls, accept = 1;
static uint32_t requested_width = 1920, requested_height = 1080;
static void pump(void *context) { assert(context == &pumps); ++pumps; }
static int size(void *context, void *window, uint32_t *width, uint32_t *height)
{
    assert(context == &pumps && window == &size_calls);
    ++size_calls;
    *width = requested_width; *height = requested_height;
    return accept;
}
int main(void)
{
    uint32_t width = 3, height = 7;
    RhiHost host = {&pumps, pump, size};
    rhi_set_host(&host);
    host.pump_events = NULL; /* registration owns a copy */
    rhi_pump_events(); assert(pumps == 1);
    assert(rhi_drawable_size(&size_calls, &width, &height));
    assert(width == 1920 && height == 1080 && size_calls == 1);
    accept = 0;
    assert(!rhi_drawable_size(&size_calls, &width, &height));
    assert(width == 1920 && height == 1080);
    accept = 1; requested_height = 0;
    assert(!rhi_drawable_size(&size_calls, &width, &height));
    assert(width == 1920 && height == 1080);
    assert(!rhi_drawable_size(&size_calls, NULL, &height));
    assert(size_calls == 3);
    rhi_set_host(NULL);
    assert(!rhi_drawable_size(NULL, &width, &height));
    assert(width == 1920 && height == 1080);
    puts("PASS: host callbacks, registration copy, drawable failure preservation");
    return 0;
}
