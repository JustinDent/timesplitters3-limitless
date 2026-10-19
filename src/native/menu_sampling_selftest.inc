/* Check production texture metadata reaches the shared shader through the
 * native binding path, independently of the asset-free shader pixel contract. */
static int native_test_menu_sampling(IDirect3DDevice8 *dev)
{
    NATIVE_GL_SCOPE;
    IDirect3DTexture8 *textures[4]={0};int ok=1;
    const D3DFORMAT formats[]={D3DFMT_LIN_A8R8G8B8,D3DFMT_A8,
        D3DFMT_LIN_A8R8G8B8,D3DFMT_LIN_A8R8G8B8};
    DWORD previous_mag[4];
    for(int stage=0;stage<4;stage++) {
        previous_mag[stage]=g.tss[stage][D3DTSS_MAGFILTER];
        ok &= SUCCEEDED(dev->lpVtbl->CreateTexture(dev,16,16,1,
            stage==2?D3DUSAGE_RENDERTARGET:0,formats[stage],D3DPOOL_DEFAULT,&textures[stage]));
        dev->lpVtbl->SetTexture(dev,stage,(IDirect3DBaseTexture8 *)textures[stage]);
        dev->lpVtbl->SetTextureStageState(dev,stage,D3DTSS_MAGFILTER,D3DTEXF_LINEAR);
    }
    if(textures[3])((GLTexture *)textures[3])->screen_copy=1;
    struct Vertex {float x,y,z,w;DWORD color;float u,v;} v[4]={
        {0,0,0,1,0xffffffff,0,0},{64,0,0,1,0xffffffff,1,0},
        {64,64,0,1,0xffffffff,1,1},{0,64,0,1,0xffffffff,0,1}};
    d3d8_combiners_set_pixel_shader(0);
    dev->lpVtbl->SetVertexShader(dev,D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1);
    for(int mode=0;mode<3;mode++)for(int point=0;point<2;point++) {
        pc_display_menu_filter_begin(mode!=0);pc_display_menu_font_filter(mode==2);
        dev->lpVtbl->SetTextureStageState(dev,0,D3DTSS_MAGFILTER,point?D3DTEXF_POINT:D3DTEXF_LINEAR);
        ok &= SUCCEEDED(dev->lpVtbl->DrawPrimitiveUP(dev,D3DPT_QUADLIST,1,v,sizeof v[0]));
        const float expected[]={mode && !point?1.f:0.f,mode==2?2.f:mode?1.f:0.f,0,0};
        for(int stage=0;stage<4;stage++) {
            char name[32];snprintf(name,sizeof name,"u_menu_filter[%d]",stage);
            GLint location=glGetUniformLocation(g.prog,name);float value=-1;
            if(location>=0)glGetUniformfv(g.prog,location,&value);
            if(value!=expected[stage]) {
                fprintf(stderr,"[native-menu-sampling-test] mode %d stage %d: %g expected %g\n",mode,stage,value,expected[stage]);ok=0;
            }
        }
    }
    /* A dependent read must not inherit frontend magnification policy. */
    DWORD previous_modes=g.rs[D3DRS_PSTEXTUREMODES];
    d3d8_combiners_set_pixel_shader(1);
    dev->lpVtbl->SetRenderState(dev,D3DRS_PSTEXTUREMODES,D3D8_PSTEXTUREMODES_XBOX|(15u<<5));
    ok &= SUCCEEDED(dev->lpVtbl->DrawPrimitiveUP(dev,D3DPT_QUADLIST,1,v,sizeof v[0]));
    float dependent=-1;glGetUniformfv(g.prog,glGetUniformLocation(g.prog,"u_menu_filter[1]"),&dependent);
    ok &= dependent==0.f;
    d3d8_combiners_set_pixel_shader(0);
    dev->lpVtbl->SetRenderState(dev,D3DRS_PSTEXTUREMODES,previous_modes);
    pc_display_menu_font_filter(0);pc_display_menu_filter_begin(0);
    for(int stage=0;stage<4;stage++) {
        dev->lpVtbl->SetTexture(dev,stage,NULL);
        dev->lpVtbl->SetTextureStageState(dev,stage,D3DTSS_MAGFILTER,previous_mag[stage]);
        if(textures[stage])textures[stage]->lpVtbl->Release(textures[stage]);
    }
    ok &= glGetError()==GL_NO_ERROR;
    fprintf(stderr,"[native-menu-sampling-test] shared eligibility, glyph scope, point/render-target/screen-copy exclusions: %s\n",ok?"PASS":"FAIL");
    return ok;
}
