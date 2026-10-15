/* Real GPU regressions: topology, A8 swizzle/sampling, compositing, readback,
 * and moving the GL context to a guest-style worker and back. */
static int native_test_pixel(IDirect3DDevice8 *dev, UINT x, UINT y,
                             unsigned r, unsigned green, unsigned b) {
    IDirect3DSurface8 *surface=NULL;
    D3DLOCKED_RECT lock;
    if(FAILED(dev->lpVtbl->GetBackBuffer(dev,0,0,&surface)) || !surface)return 0;
    int ok=0;
    if(SUCCEEDED(surface->lpVtbl->LockRect(surface,&lock,NULL,D3DLOCK_READONLY))) {
        const BYTE *p=(const BYTE*)lock.pBits+(size_t)y*lock.Pitch+x*4;
        ok=abs((int)p[0]-(int)r)<3 && abs((int)p[1]-(int)green)<3 && abs((int)p[2]-(int)b)<3;
        if(!ok)fprintf(stderr,"[native-gl-test] pixel %u,%u: %u,%u,%u expected %u,%u,%u\n",x,y,p[0],p[1],p[2],r,green,b);
        surface->lpVtbl->UnlockRect(surface);
    }
    surface->lpVtbl->Release(surface);
    return ok;
}
static void *native_test_worker(void *arg) {
    IDirect3DDevice8 *dev=arg;
    dev->lpVtbl->Clear(dev,0,NULL,D3DCLEAR_TARGET,0xFF00FF00,1,0);
    return NULL;
}
int tsfp_native_renderer_test(void) {
    IDirect3D8 *api=xbox_Direct3DCreate8(0);
    IDirect3DDevice8 *dev=NULL;
    D3DPRESENT_PARAMETERS pp={0};
    pp.BackBufferWidth=pp.BackBufferHeight=64;pp.Windowed=TRUE;
    xbox_D3D8SetWindowTitle("Native renderer regression test");
    if(FAILED(api->lpVtbl->CreateDevice(api,0,1,NULL,0,&pp,&dev)) || !dev)return 1;
    dev->lpVtbl->SetRenderState(dev,D3DRS_ZENABLE,FALSE);
    dev->lpVtbl->SetRenderState(dev,D3DRS_CULLMODE,D3DCULL_NONE);
    dev->lpVtbl->SetRenderState(dev,D3DRS_ALPHATESTENABLE,FALSE);
    dev->lpVtbl->Clear(dev,0,NULL,D3DCLEAR_TARGET,0xFF0000FF,1,0);
    struct TestVertex {float x,y,z,w;DWORD color;float u,v;} vertices[4]={
        {8,8,0,1,0xFFFF0000,0,0}, {56,8,0,1,0xFFFF0000,1,0},
        {56,56,0,1,0xFFFF0000,1,1}, {8,56,0,1,0xFFFF0000,0,1}
    };
    dev->lpVtbl->SetVertexShader(dev,D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1);
    int ok=SUCCEEDED(dev->lpVtbl->DrawPrimitiveUP(dev,D3DPT_QUADLIST,1,vertices,sizeof vertices[0]));
    ok &= native_test_pixel(dev,16,16,255,0,0);
    ok &= native_test_pixel(dev,48,48,255,0,0);
    ok &= native_test_pixel(dev,4,4,0,0,255);
    /* Queue many different uploads before any readback synchronizes the GPU.
     * Earlier draws must retain their geometry/color as the stream is reused,
     * including changing sizes and both 16/32-bit index uploads. */
    dev->lpVtbl->Clear(dev,0,NULL,D3DCLEAR_TARGET,0xFF000000,1,0);
    for(int pass=0;pass<16;pass++)for(int tile=0;tile<8;tile++) {
        DWORD color=tile&1?0xFF00FF00:0xFFFF0000;
        struct TestVertex quad[4]={
            {tile*8,8,0,1,color,0,0},{tile*8+8,8,0,1,color,1,0},
            {tile*8+8,56,0,1,color,1,1},{tile*8,56,0,1,color,0,1}
        };
        if(tile%3==0) {
            struct TestVertex triangles[6]={quad[0],quad[1],quad[2],quad[0],quad[2],quad[3]};
            ok &= SUCCEEDED(dev->lpVtbl->DrawPrimitiveUP(dev,D3DPT_TRIANGLELIST,2,triangles,sizeof quad[0]));
        } else {
            const uint16_t short_indices[6]={0,1,2,0,2,3};
            const uint32_t long_indices[6]={0,2,3,0,1,2};
            ok &= SUCCEEDED(dev->lpVtbl->DrawIndexedPrimitiveUP(dev,D3DPT_TRIANGLELIST,0,4,2,
                tile%3==1?(const void*)short_indices:(const void*)long_indices,
                tile%3==1?D3DFMT_INDEX16:D3DFMT_INDEX32,quad,sizeof quad[0]));
        }
    }
    for(int tile=0;tile<8;tile++)ok &= native_test_pixel(dev,tile*8+4,32,tile&1?0:255,tile&1?255:0,0);
    IDirect3DTexture8 *texture=NULL;
    ok &= SUCCEEDED(dev->lpVtbl->CreateTexture(dev,8,8,1,0,D3DFMT_A8,D3DPOOL_MANAGED,&texture));
    if(texture) {
        D3DLOCKED_RECT lock;
        if(SUCCEEDED(texture->lpVtbl->LockRect(texture,0,&lock,NULL,0))) {
            for(UINT y=0;y<8;y++)for(UINT x=0;x<8;x++)
                ((BYTE*)lock.pBits)[swizzle_offset(x,y,8,8)]=x<4?255:0;
            ok &= SUCCEEDED(texture->lpVtbl->UnlockRect(texture,0));
        } else ok=0;
        dev->lpVtbl->Clear(dev,0,NULL,D3DCLEAR_TARGET,0xFF0000FF,1,0);
        dev->lpVtbl->SetTexture(dev,0,(IDirect3DBaseTexture8*)texture);
        dev->lpVtbl->SetRenderState(dev,D3DRS_ALPHABLENDENABLE,TRUE);
        dev->lpVtbl->DrawPrimitiveUP(dev,D3DPT_QUADLIST,1,vertices,sizeof vertices[0]);
        ok &= native_test_pixel(dev,18,32,255,0,0);
        ok &= native_test_pixel(dev,46,32,0,0,255);
        dev->lpVtbl->SetTexture(dev,0,NULL);
        texture->lpVtbl->Release(texture);
    }
    /* Synthetic MOV oPos,v0, plus two register-combiner stages. Unique C0
     * must select stage 1's green, not stage 0's red. No game assets used. */
    const DWORD program[4]={0,0x0020001B,0x0836106C,0x2070F801};
    DWORD shader=0;
    ok &= SUCCEEDED(d3d8_vsh_create_shader(program,1,&shader));
    D3D8VshInput decl={0,RHI_FORMAT_R32G32B32A32_FLOAT,0};
    ok &= SUCCEEDED(d3d8_vsh_set_declaration(shader,&decl,1));
    const float scale[4]={32,-32,1,1},offset[4]={32,32,0,0};
    d3d8_vsh_set_screenspace(scale,offset);
    dev->lpVtbl->SetVertexShader(dev,shader);
    dev->lpVtbl->SetRenderState(dev,D3DRS_ALPHABLENDENABLE,FALSE);
    d3d8_combiners_set_pixel_shader(1);
    dev->lpVtbl->SetRenderState(dev,D3DRS_PSCOMBINERCOUNT,0x1002);
    dev->lpVtbl->SetRenderState(dev,D3DRS_PSCONSTANT0_0,0xFFFF0000);
    dev->lpVtbl->SetRenderState(dev,D3DRS_PSCONSTANT0_1,0xFF00FF00);
    for(int stage=0;stage<2;stage++) {
        dev->lpVtbl->SetRenderState(dev,(D3DRENDERSTATETYPE)(210+stage),0x01200000);
        dev->lpVtbl->SetRenderState(dev,(D3DRENDERSTATETYPE)(218+stage),0x000000C0);
        dev->lpVtbl->SetRenderState(dev,(D3DRENDERSTATETYPE)(200+stage),0x20200000);
        dev->lpVtbl->SetRenderState(dev,(D3DRENDERSTATETYPE)(226+stage),0x000000C0);
    }
    dev->lpVtbl->SetRenderState(dev,D3DRS_PSFINALCOMBINERINPUTSABCD,0x0000000C);
    dev->lpVtbl->SetRenderState(dev,D3DRS_PSFINALCOMBINERINPUTSEFG,0x00001C80);
    ok &= SUCCEEDED(dev->lpVtbl->DrawPrimitiveUP(dev,D3DPT_QUADLIST,1,vertices,sizeof vertices[0]));
    ok &= native_test_pixel(dev,16,16,0,255,0);
    ok &= native_test_pixel(dev,48,48,0,255,0);
    d3d8_vsh_delete_shader(shader);
    /* A later offscreen clear must never erase the already drawn world.
     * Sampling the target must preserve its top/bottom D3D orientation. */
    d3d8_combiners_set_pixel_shader(0);
    dev->lpVtbl->SetVertexShader(dev,D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1);
    for(int i=0;i<4;i++)vertices[i].color=0xFFFFFFFF;
    dev->lpVtbl->Clear(dev,0,NULL,D3DCLEAR_TARGET,0xFFFF0000,1,0);
    IDirect3DTexture8 *target=NULL;IDirect3DSurface8 *surface=NULL,*depth=NULL,*original_depth=NULL;
    ok &= SUCCEEDED(dev->lpVtbl->GetDepthStencilSurface(dev,&original_depth));
    ok &= SUCCEEDED(dev->lpVtbl->CreateTexture(dev,16,16,1,D3DUSAGE_RENDERTARGET,D3DFMT_LIN_A8R8G8B8,D3DPOOL_DEFAULT,&target));
    ok &= target && SUCCEEDED(target->lpVtbl->GetSurfaceLevel(target,0,&surface)) && surface;
    ok &= SUCCEEDED(dev->lpVtbl->CreateDepthStencilSurface(dev,16,16,D3DFMT_D24S8,0,&depth));
    if(surface && depth) {
        ok &= SUCCEEDED(dev->lpVtbl->SetRenderTarget(dev,surface,depth));
        /* Reject an incomplete target without moving subsequent clears there. */
        IDirect3DTexture8 *incomplete=NULL;IDirect3DSurface8 *bad_surface=NULL;
        dev->lpVtbl->CreateTexture(dev,16,16,1,0,D3DFMT_LIN_A8R8G8B8,D3DPOOL_DEFAULT,&incomplete);
        if(incomplete)incomplete->lpVtbl->GetSurfaceLevel(incomplete,0,&bad_surface);
        ok &= bad_surface && FAILED(dev->lpVtbl->SetRenderTarget(dev,bad_surface,depth));
        if(bad_surface)bad_surface->lpVtbl->Release(bad_surface);
        if(incomplete)incomplete->lpVtbl->Release(incomplete);
        dev->lpVtbl->Clear(dev,0,NULL,D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,0xFF0000FF,1,0);
        D3DRECT top={0,0,16,8};
        dev->lpVtbl->Clear(dev,1,&top,D3DCLEAR_TARGET,0xFF00FF00,1,0);
        ok &= native_test_pixel(dev,32,32,255,0,0); /* Back buffer read while RT bound. */
        ok &= SUCCEEDED(dev->lpVtbl->SetRenderTarget(dev,NULL,original_depth));
        dev->lpVtbl->SetTexture(dev,0,(IDirect3DBaseTexture8*)target);
        dev->lpVtbl->DrawPrimitiveUP(dev,D3DPT_QUADLIST,1,vertices,sizeof vertices[0]);
        ok &= native_test_pixel(dev,18,18,0,255,0);
        ok &= native_test_pixel(dev,18,46,0,0,255);
        ok &= native_test_pixel(dev,4,4,255,0,0);
        dev->lpVtbl->SetTexture(dev,0,NULL);
    }
    if(surface)surface->lpVtbl->Release(surface);if(depth)depth->lpVtbl->Release(depth);
    if(target)target->lpVtbl->Release(target);if(original_depth)original_depth->lpVtbl->Release(original_depth);
    pthread_t worker;
    if(!pthread_create(&worker,NULL,native_test_worker,dev)) {
        pthread_join(worker,NULL);ok &= native_test_pixel(dev,32,32,0,255,0);
    } else ok=0;
    {NATIVE_GL_SCOPE;GLenum e=glGetError();if(e){fprintf(stderr,"[native-gl-test] GL error 0x%x\n",e);ok=0;}}
    SDL_GL_DestroyContext(g.glctx);g.glctx=NULL;
    SDL_DestroyWindow(g.window);g.window=NULL;
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
    fprintf(stderr,"[native-gl-test] quad topology, queued stream uploads, A8 swizzle, alpha blend, NV2A vertex/combiner programs, readback, offscreen clear isolation/orientation, worker handoff: %s\n",ok?"PASS":"FAIL");
    return ok?0:1;
}
