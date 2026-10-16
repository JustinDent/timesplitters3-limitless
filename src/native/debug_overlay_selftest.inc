static int native_test_debug_overlay(IDirect3DDevice8 *dev) {
    NATIVE_GL_SCOPE;
    PcDebugOptions previous,on={1,1,0,0},off={0};pc_debug_get(&previous);
    GLint read,draw,binding,after;glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING,&read);glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING,&draw);glGetIntegerv(GL_TEXTURE_BINDING_2D,&binding);
    GLuint texture,fbo;glGenTextures(1,&texture);glBindTexture(GL_TEXTURE_2D,texture);glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,1024,128,0,GL_RGBA,GL_UNSIGNED_BYTE,NULL);
    glGenFramebuffers(1,&fbo);glBindFramebuffer(GL_FRAMEBUFFER,fbo);glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,texture,0);
    int ok=glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE;
    glDisable(GL_SCISSOR_TEST);glColorMask(1,1,1,1);glClearColor(0,0,1,1);glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_SCISSOR_TEST);glScissor(0,0,1,1);
    pc_debug_set(&on);pc_debug_sample(60,120);native_debug_draw(fbo,1024,128);
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING,&after);ok &= (GLuint)after==fbo;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING,&after);ok &= (GLuint)after==fbo && glIsEnabled(GL_SCISSOR_TEST);
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
    glBindFramebuffer(GL_READ_FRAMEBUFFER,read);glBindFramebuffer(GL_DRAW_FRAMEBUFFER,draw);
    glBindTexture(GL_TEXTURE_2D,binding);glDeleteFramebuffers(1,&fbo);glDeleteTextures(1,&texture);
    pc_debug_set(&previous);
    /* Drawing the host overlay has not changed the guest scene. */
    ok &= native_test_pixel(dev,0,0,0,0,255);
    ok &= glGetError()==GL_NO_ERROR;
    fprintf(stderr,"[native-debug-overlay-test] GPU text, hidden state, GL restoration, guest-scene isolation: %s\n",ok?"PASS":"FAIL");return ok;
}
