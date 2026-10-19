/* Exercise the installed production SDL adapter, including its worker boundary. */
typedef struct { SDL_Window *window; uint32_t width, height; int ok; } NativeHostProbe;
static void *native_test_host_worker(void *opaque) {
    NativeHostProbe *probe=opaque;
    probe->ok=rhi_drawable_size(probe->window,&probe->width,&probe->height);
    rhi_pump_events(); /* must not consume main-thread events */
    return NULL;
}
static int native_test_renderer_host(SDL_Window *window, void (*on_quit)(void)) {
    int expected_w=0,expected_h=0;
    uint32_t width=0,height=0;
    int ok=SDL_GetWindowSizeInPixels(window,&expected_w,&expected_h);
    ok &= rhi_drawable_size(window,&width,&height);
    ok &= width==(uint32_t)expected_w && height==(uint32_t)expected_h;
    uint32_t unchanged_w=width,unchanged_h=height;
    ok &= !rhi_drawable_size(NULL,&width,&height);
    ok &= width==unchanged_w && height==unchanged_h;
    Uint32 event_type=SDL_RegisterEvents(1);
    if(!event_type)return 0;
    SDL_Event event={0};event.type=event_type;
    ok &= SDL_PushEvent(&event);
    NativeHostProbe probe={.window=window};pthread_t thread;
    if(pthread_create(&thread,NULL,native_test_host_worker,&probe)!=0)return 0;
    pthread_join(thread,NULL);
    ok &= probe.ok && probe.width==width && probe.height==height;
    ok &= SDL_PeepEvents(&event,1,SDL_PEEKEVENT,event_type,event_type)==1;
    rhi_pump_events();
    ok &= SDL_PeepEvents(&event,1,SDL_PEEKEVENT,event_type,event_type)==0;
    /* Explicitly unregister before destruction; no stale callback/window use. */
    tsfp_renderer_host_bind(NULL,NULL);
    ok &= !rhi_drawable_size(window,&width,&height);
    ok &= width==unchanged_w && height==unchanged_h;
    tsfp_renderer_host_bind(window,on_quit);
    ok &= rhi_drawable_size(window,&width,&height);
    fprintf(stderr,"[native-host-test] physical drawable, worker snapshot, main-thread events, unbind: %s\n",ok?"PASS":"FAIL");
    return ok;
}
