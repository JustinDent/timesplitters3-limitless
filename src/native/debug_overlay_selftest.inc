static int native_test_debug_overlay(IDirect3DDevice8 *dev) {
    NATIVE_GL_SCOPE;
    PcDebugOptions previous,on={1,1,0,0,0},off={0};pc_debug_get(&previous);
    GLint read,draw,binding,after;glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING,&read);glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING,&draw);glGetIntegerv(GL_TEXTURE_BINDING_2D,&binding);
    GLuint texture,fbo;glGenTextures(1,&texture);glBindTexture(GL_TEXTURE_2D,texture);glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,1024,128,0,GL_RGBA,GL_UNSIGNED_BYTE,NULL);
    glGenFramebuffers(1,&fbo);glBindFramebuffer(GL_FRAMEBUFFER,fbo);glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,texture,0);
    int ok=glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE;
    /* Uploads must not interpret CPU pixels as a caller's PBO offset/stride. */
    GLuint pbo;glGenBuffers(1,&pbo);glBindBuffer(GL_PIXEL_UNPACK_BUFFER,pbo);
    glPixelStorei(GL_UNPACK_ALIGNMENT,8);glPixelStorei(GL_UNPACK_ROW_LENGTH,2048);
    glPixelStorei(GL_UNPACK_SKIP_ROWS,1);glPixelStorei(GL_UNPACK_SKIP_PIXELS,2);
    glDisable(GL_SCISSOR_TEST);glColorMask(1,1,1,1);glClearColor(0,0,1,1);glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_SCISSOR_TEST);glScissor(0,0,1,1);
    pc_debug_set(&on);pc_debug_sample(60,120);native_debug_draw(fbo,1024,128);
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING,&after);ok &= (GLuint)after==fbo;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING,&after);ok &= (GLuint)after==fbo && glIsEnabled(GL_SCISSOR_TEST);
    glGetIntegerv(GL_TEXTURE_BINDING_2D,&after);ok &= (GLuint)after==texture;
    glGetIntegerv(GL_PIXEL_UNPACK_BUFFER_BINDING,&after);ok &= (GLuint)after==pbo;
    glGetIntegerv(GL_UNPACK_ALIGNMENT,&after);ok &= after==8;
    glGetIntegerv(GL_UNPACK_ROW_LENGTH,&after);ok &= after==2048;
    glGetIntegerv(GL_UNPACK_SKIP_ROWS,&after);ok &= after==1;
    glGetIntegerv(GL_UNPACK_SKIP_PIXELS,&after);ok &= after==2;
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER,0);glDeleteBuffers(1,&pbo);
    glPixelStorei(GL_UNPACK_ALIGNMENT,4);glPixelStorei(GL_UNPACK_ROW_LENGTH,0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS,0);glPixelStorei(GL_UNPACK_SKIP_PIXELS,0);
    unsigned char corner[4],pixels[1004*48*4];
    glReadPixels(0,0,1,1,GL_RGBA,GL_UNSIGNED_BYTE,corner);ok &= corner[0]==0 && corner[2]==255;
    glReadPixels(10,70,1004,48,GL_RGBA,GL_UNSIGNED_BYTE,pixels);
    unsigned white=0,dark=0;
    for(unsigned i=0;i<sizeof pixels;i+=4){white+=pixels[i]>200 && pixels[i+1]>200 && pixels[i+2]>200;dark+=pixels[i]<15 && pixels[i+2]<15;}
    ok &= white>100 && dark>1000;
    /* Retain one small rendered fixture for orientation/readability review. */
    const char *root=getenv("RECOMP_SAVE_DIR");char path[4096];
    if(root && snprintf(path,sizeof path,"%s/debug-overlay.bmp",root)<(int)sizeof path) {
        unsigned char flipped[sizeof pixels];
        for(int y=0;y<48;y++)memcpy(flipped+y*1004*4,pixels+(47-y)*1004*4,1004*4);
        SDL_Surface *surface=SDL_CreateSurfaceFrom(1004,48,SDL_PIXELFORMAT_RGBA32,flipped,1004*4);
        ok &= surface && SDL_SaveBMP(surface,path);SDL_DestroySurface(surface);
    } else ok=0;
    pc_debug_set(&off);glDisable(GL_SCISSOR_TEST);glClear(GL_COLOR_BUFFER_BIT);native_debug_draw(fbo,1024,128);
    glReadPixels(20,80,1,1,GL_RGBA,GL_UNSIGNED_BYTE,corner);ok &= corner[0]==0 && corner[2]==255;
    glReadPixels(10,10,1004,48,GL_RGBA,GL_UNSIGNED_BYTE,pixels);
    white=0;for(unsigned i=0;i<sizeof pixels;i+=4)white+=pixels[i]>200 && pixels[i+1]>200 && pixels[i+2]>200;
    ok &= white>100 && !strcmp(native_debug_lines[1].text,pc_debug_watermark());
    if(root && snprintf(path,sizeof path,"%s/build-watermark.bmp",root)<(int)sizeof path) {
        unsigned char flipped[sizeof pixels];
        for(int y=0;y<48;y++)memcpy(flipped+y*1004*4,pixels+(47-y)*1004*4,1004*4);
        SDL_Surface *surface=SDL_CreateSurfaceFrom(1004,48,SDL_PIXELFORMAT_RGBA32,flipped,1004*4);
        ok &= surface && SDL_SaveBMP(surface,path);SDL_DestroySurface(surface);
    } else ok=0;
    /* A long build label fits a narrow drawable; outside its inset stays blue. */
    char long_label[300];memset(long_label,'W',sizeof long_label-1);long_label[sizeof long_label-1]=0;
    glClear(GL_COLOR_BUFFER_BIT);
    native_debug_line(&native_debug_lines[1],long_label,fbo,320,128,1);
    ok &= native_debug_lines[1].width>900 && native_debug_lines[1].width<=1024;
    glBindFramebuffer(GL_READ_FRAMEBUFFER,fbo);
    glReadPixels(319,20,1,1,GL_RGBA,GL_UNSIGNED_BYTE,corner);ok &= corner[0]==0 && corner[2]==255;
    native_debug_release();ok &= !native_debug_lines[0].texture && !native_debug_lines[1].fbo;
    native_debug_draw(fbo,1024,128);ok &= native_debug_lines[1].texture && !strcmp(native_debug_lines[1].text,pc_debug_watermark());
    glBindFramebuffer(GL_READ_FRAMEBUFFER,read);glBindFramebuffer(GL_DRAW_FRAMEBUFFER,draw);
    glBindTexture(GL_TEXTURE_2D,binding);glDeleteFramebuffers(1,&fbo);glDeleteTextures(1,&texture);
    pc_debug_set(&previous);
    /* Drawing the host overlay has not changed the guest scene. */
    ok &= native_test_pixel(dev,0,0,0,0,255);
    ok &= glGetError()==GL_NO_ERROR;
    fprintf(stderr,"[native-debug-overlay-test] FPS visibility, independent watermark, narrow/long labels, GL restoration, cache lifetime and guest-scene isolation: %s\n",ok?"PASS":"FAIL");return ok;
}
