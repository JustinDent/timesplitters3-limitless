/* GPU coverage for the destination-alpha masks used by lobby text. Read the
 * actual attachment, including alpha/depth/stencil, rather than RGB captures. */
static int native_clear_pixel(unsigned x, unsigned y, DWORD rgba,
                              float depth, unsigned stencil) {
    double sx=native_scene_active?(double)native_scene_w/g.backbuf_w:1;
    double sy=native_scene_active?(double)native_scene_h/g.backbuf_h:1;
    GLint read,draw;glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING,&read);
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING,&draw);
    glBindFramebuffer(GL_READ_FRAMEBUFFER,draw);
    BYTE pixel[4],st=0;float z=0;
    int px=(x+.5)*sx,py=(native_target_height-y-.5)*sy;
    glReadPixels(px,py,1,1,GL_RGBA,GL_UNSIGNED_BYTE,pixel);
    glReadPixels(px,py,1,1,GL_DEPTH_COMPONENT,GL_FLOAT,&z);
    glReadPixels(px,py,1,1,GL_STENCIL_INDEX,GL_UNSIGNED_BYTE,&st);
    glBindFramebuffer(GL_READ_FRAMEBUFFER,read);
    unsigned expected[4]={(rgba>>16)&255,(rgba>>8)&255,rgba&255,rgba>>24};
    int ok=fabsf(z-depth)<.00001f && st==stencil;
    for(int i=0;i<4;i++)ok &= abs((int)pixel[i]-(int)expected[i])<=1;
    if(!ok)fprintf(stderr,"[native-gl-test] clear pixel %u,%u: RGBA %u,%u,%u,%u Z %.4f S %u; expected %08x Z %.4f S %u\n",
        x,y,pixel[0],pixel[1],pixel[2],pixel[3],z,st,rgba,depth,stencil);
    return ok;
}

static int native_test_channel_clears(IDirect3DDevice8 *dev) {
    NATIVE_GL_SCOPE;
    int ok=1,cases=0;
    IDirect3DSurface8 *saved_depth=NULL,*surface=NULL,*depth=NULL;
    IDirect3DTexture8 *target=NULL;
    dev->lpVtbl->GetDepthStencilSurface(dev,&saved_depth);
    ok &= SUCCEEDED(dev->lpVtbl->CreateTexture(dev,16,16,1,D3DUSAGE_RENDERTARGET,
        D3DFMT_LIN_A8R8G8B8,D3DPOOL_DEFAULT,&target));
    if(target)ok &= SUCCEEDED(target->lpVtbl->GetSurfaceLevel(target,0,&surface));
    ok &= SUCCEEDED(dev->lpVtbl->CreateDepthStencilSurface(dev,16,16,D3DFMT_D24S8,0,&depth));
    if(!surface || !depth)ok=0;
    DWORD saved_color=g.rs[D3DRS_COLORWRITEENABLE],saved_z=g.rs[D3DRS_ZWRITEENABLE];
    GLint saved_stencil;glGetIntegerv(GL_STENCIL_WRITEMASK,&saved_stencil);
    for(int mode=0;mode<3 && surface && depth;mode++) {
        ok &= pc_display_apply(mode?(PcDisplayMode){1920,1080}:(PcDisplayMode){0,0});
        ok &= SUCCEEDED(dev->lpVtbl->SetRenderTarget(dev,mode==2?surface:NULL,mode==2?depth:saved_depth));
        unsigned size=mode==2?16:64;
        D3DVIEWPORT8 full={0,0,size,size,0,1};
        D3DRECT empty={0,0,0,0};
        xbox_D3D8SetScissors(1,FALSE,&empty);
        dev->lpVtbl->SetRenderState(dev,D3DRS_COLORWRITEENABLE,5);
        dev->lpVtbl->SetRenderState(dev,D3DRS_ZWRITEENABLE,FALSE);
        glStencilMask(0x12);
        for(unsigned mask=0;mask<16;mask++)for(unsigned ds=0;ds<4;ds++) {
            dev->lpVtbl->SetViewport(dev,&full);
            ok &= SUCCEEDED(dev->lpVtbl->Clear(dev,0,NULL,
                D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL,0x44332211,.25f,0x3c));
            DWORD flags=(mask<<4)|(ds&1?D3DCLEAR_ZBUFFER:0)|(ds&2?D3DCLEAR_STENCIL:0);
            ok &= SUCCEEDED(dev->lpVtbl->Clear(dev,0,NULL,flags,0xbbaaccdd,.75f,0xa5));
            DWORD expected=0x44332211;
            const unsigned shifts[4]={16,8,0,24};
            for(int c=0;c<4;c++)if(mask&(1u<<c))
                expected=(expected&~(255u<<shifts[c]))|(0xbbaaccdd&(255u<<shifts[c]));
            ok &= native_clear_pixel(size/2,size/2,expected,ds&1?.75f:.25f,ds&2?0xa5:0x3c);
            GLboolean color[4],z;GLint stencil;
            glGetBooleanv(GL_COLOR_WRITEMASK,color);glGetBooleanv(GL_DEPTH_WRITEMASK,&z);
            glGetIntegerv(GL_STENCIL_WRITEMASK,&stencil);
            ok &= color[0] && !color[1] && color[2] && !color[3] && !z && stencil==0x12;
            ok &= glIsEnabled(GL_SCISSOR_TEST);
            cases++;
        }
        /* Clear rectangles intersect the viewport, ignore drawing scissors,
         * and leave all other pixels and attachments intact. */
        dev->lpVtbl->Clear(dev,0,NULL,D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL,0x44332211,.25f,0x3c);
        D3DVIEWPORT8 inset={size/4,size/4,size/2,size/2,0,1};
        dev->lpVtbl->SetViewport(dev,&inset);
        D3DRECT rectangles[2]={{0,0,size/2,size},{size,size,0,0}};
        dev->lpVtbl->Clear(dev,2,rectangles,D3DCLEAR_CHANNEL_ALPHA|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL,0xbbabcdef,.75f,0xa5);
        ok &= native_clear_pixel(size/4,size/2,0xbb332211,.75f,0xa5);
        ok &= native_clear_pixel(size/2,size/2,0x44332211,.25f,0x3c);
        ok &= native_clear_pixel(0,size/2,0x44332211,.25f,0x3c);
        ok &= native_clear_pixel(size/4,0,0x44332211,.25f,0x3c);
        ok &= FAILED(dev->lpVtbl->Clear(dev,1,NULL,D3DCLEAR_TARGET,0,1,0));
        if(mode==2) {
            /* This target was cleared independently of the enlarged scene. */
            ok &= SUCCEEDED(dev->lpVtbl->SetRenderTarget(dev,NULL,saved_depth));
            ok &= native_clear_pixel(16,32,0xbb332211,.75f,0xa5);
            ok &= native_clear_pixel(32,32,0x44332211,.25f,0x3c);
        }
        fprintf(stderr,"[native-gl-test] channel clears target %d: %s\n",mode,ok?"PASS":"FAIL");
    }
    dev->lpVtbl->SetRenderTarget(dev,NULL,saved_depth);
    if(surface)surface->lpVtbl->Release(surface);if(target)target->lpVtbl->Release(target);
    if(depth)depth->lpVtbl->Release(depth);if(saved_depth)saved_depth->lpVtbl->Release(saved_depth);
    dev->lpVtbl->SetRenderState(dev,D3DRS_COLORWRITEENABLE,saved_color);
    dev->lpVtbl->SetRenderState(dev,D3DRS_ZWRITEENABLE,saved_z);
    glStencilMask(saved_stencil);xbox_D3D8SetScissors(0,FALSE,NULL);
    ok &= pc_display_apply((PcDisplayMode){0,0});
    ok &= glGetError()==GL_NO_ERROR;
    fprintf(stderr,"[native-gl-test] channel clears: %s (%d mask/depth/stencil combinations)\n",ok?"PASS":"FAIL",cases);
    return ok;
}
