/* Host-only overlay: drawn after scene presentation so guest captures, copied
 * textures, and interpolation history cannot contain the debug text. */
static GLuint native_debug_texture,native_debug_fbo;
static char native_debug_text[384];
static void native_debug_draw(GLuint destination,int width,int height) {
    char text[320],line[384];
    if(width<=20 || height<=60 || !pc_debug_overlay(text,sizeof text,NULL))return;
    PcDebugOptions options;pc_debug_get(&options);
    snprintf(line,sizeof line,"%s%s",options.show_fps?"Game/Shown: ":"",text);
    GLint read,draw,binding,unpack;GLboolean scissor=glIsEnabled(GL_SCISSOR_TEST);
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING,&read);glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING,&draw);
    glGetIntegerv(GL_TEXTURE_BINDING_2D,&binding);glGetIntegerv(GL_UNPACK_ALIGNMENT,&unpack);
    if(!native_debug_texture){glGenTextures(1,&native_debug_texture);glGenFramebuffers(1,&native_debug_fbo);}
    if(strcmp(line,native_debug_text)) {
        static unsigned char pixels[1024*48*4];
        if(!tsfp_native_debug_bitmap(line,pixels,1024,48))return;
        glBindTexture(GL_TEXTURE_2D,native_debug_texture);glPixelStorei(GL_UNPACK_ALIGNMENT,1);
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,1024,48,0,GL_RGBA,GL_UNSIGNED_BYTE,pixels);
        snprintf(native_debug_text,sizeof native_debug_text,"%s",line);
    }
    glBindFramebuffer(GL_READ_FRAMEBUFFER,native_debug_fbo);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,native_debug_texture,0);
    if(glCheckFramebufferStatus(GL_READ_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE) {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER,destination);glDisable(GL_SCISSOR_TEST);
        int shown=width-20<1024?width-20:1024;
        /* CoreGraphics bitmap rows begin at the top; OpenGL textures at the bottom. */
        glBlitFramebuffer(0,48,shown,0,10,height-58,10+shown,height-10,GL_COLOR_BUFFER_BIT,GL_NEAREST);
    }
    if(scissor)glEnable(GL_SCISSOR_TEST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER,read);glBindFramebuffer(GL_DRAW_FRAMEBUFFER,draw);
    glBindTexture(GL_TEXTURE_2D,binding);glPixelStorei(GL_UNPACK_ALIGNMENT,unpack);
}
static void native_debug_present(Uint64 now,int guest) {
    static Uint64 start;static unsigned games,shown;
    if(guest)pc_debug_frame();
    if(!start){start=now;return;}
    games+=guest;shown++;
    if(now-start>=500000000) {
        double seconds=(now-start)/1e9;pc_debug_sample(games/seconds,shown/seconds);
        start=now;games=shown=0;
    }
}
