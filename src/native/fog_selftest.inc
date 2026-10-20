/* Exercise actual combiner output, including the title's fog-disabled custom
 * blend and its unwritten-fog sky/particle distinction. */
static int native_test_fog(IDirect3DDevice8 *dev) {
    NATIVE_GL_SCOPE;
    struct FogVertex {float pos[4];DWORD color;float fog[4];} v[4]={
        {{0,0,0,1},0xffff0000},{{64,0,0,1},0xffff0000},
        {{64,64,0,1},0xffff0000},{{0,64,0,1},0xffff0000}};
    const DWORD code[]={0,0x0020001B,0x0836106C,0x2070F801};
    DWORD shader=0;int ok=SUCCEEDED(d3d8_vsh_create_shader(code,1,&shader)),cases=0;
    if(!shader)return 0;
    D3D8VshInput decl[]={{0,RHI_FORMAT_R32G32B32A32_FLOAT,0},
        {1,RHI_FORMAT_B8G8R8A8_UNORM,16},{2,RHI_FORMAT_R32G32B32A32_FLOAT,20}};
    ok &= SUCCEEDED(d3d8_vsh_set_declaration(shader,decl,3));
    const float scale[]={32,-32,1,1},offset[]={32,32,0,0};
    d3d8_vsh_set_screenspace(scale,offset);
    DWORD old_fog=g.rs[D3DRS_FOGENABLE],old_color=g.rs[D3DRS_FOGCOLOR];
    dev->lpVtbl->SetRenderState(dev,D3DRS_FOGCOLOR,0x1200ff00);
    dev->lpVtbl->SetRenderState(dev,D3DRS_ALPHABLENDENABLE,FALSE);
    dev->lpVtbl->SetRenderState(dev,D3DRS_ZENABLE,FALSE);
    d3d8_combiners_set_pixel_shader(1);
    dev->lpVtbl->SetRenderState(dev,D3DRS_PSCOMBINERCOUNT,0);
    dev->lpVtbl->SetRenderState(dev,D3DRS_PSTEXTUREMODES,0);
    /* A=fog alpha, B=fog RGB, C=vertex RGB, D=zero; output alpha=one. */
    dev->lpVtbl->SetRenderState(dev,D3DRS_PSFINALCOMBINERINPUTSABCD,0x13030400);
    dev->lpVtbl->SetRenderState(dev,D3DRS_PSFINALCOMBINERINPUTSEFG,0x00002000);
    for(int write=0;write<4;write++) {
        native_gpu_delete(shader-0x10000);
        NV2AVshProgram *p=&native_programs[shader-0x10000];memset(p,0,sizeof *p);p->length=3;
        for(int i=0;i<3;i++) {
            NV2AVshInstruction *in=&p->insns[i];
            in->mac_op=NV2A_VSH_MAC_MOV;
            in->mac_src[0]=native_test_source(NV2A_VSH_REG_INPUT,i);
            in->mac_dst=(NV2AVshDstOperand){-1,i==0?NV2A_VSH_OUT_POS:i==1?NV2A_VSH_OUT_D0:NV2A_VSH_OUT_FOG,0,i==2?1:15};
            in->ilu_dst=(NV2AVshDstOperand){-1,NV2A_VSH_OUT_NONE,0,0};
            in->is_final=i==2;
        }
        if(write==0)p->insns[2].mac_op=NV2A_VSH_MAC_NOP;
        if(write==2) {
            p->insns[2].ilu_op=NV2A_VSH_ILU_MOV;
            p->insns[2].ilu_src=p->insns[2].mac_src[0];
            p->insns[2].ilu_dst=p->insns[2].mac_dst;
            p->insns[2].mac_op=NV2A_VSH_MAC_NOP;
        }
        /* An instruction after the end marker must not count as a fog write. */
        if(write==3)p->insns[1].is_final=1;
        for(int cpu=0;cpu<2;cpu++) {
            if(cpu)setenv("TSFP_NATIVE_CPU_VERTEX","1",1);else unsetenv("TSFP_NATIVE_CPU_VERTEX");
            dev->lpVtbl->SetVertexShader(dev,shader);
            for(int enabled=0;enabled<2;enabled++)for(int value=0;value<3;value++) {
                const float factors[]={-1,.25f,2};float factor=factors[value];
                for(int i=0;i<4;i++)for(int c=0;c<4;c++)v[i].fog[c]=factor;
                dev->lpVtbl->SetRenderState(dev,D3DRS_FOGENABLE,enabled);
                dev->lpVtbl->Clear(dev,0,NULL,D3DCLEAR_TARGET,0xff0000ff,1,0);
                const uint16_t indices[]={0,1,2,0,2,3};
                if(value==1)ok &= SUCCEEDED(dev->lpVtbl->DrawIndexedPrimitiveUP(dev,D3DPT_TRIANGLELIST,0,4,2,indices,D3DFMT_INDEX16,v,sizeof v[0]));
                else ok &= SUCCEEDED(dev->lpVtbl->DrawPrimitiveUP(dev,D3DPT_QUADLIST,1,v,sizeof v[0]));
                float expected=write==0||write==3?(float)enabled:fminf(1,fmaxf(0,factor));
                ok &= native_test_pixel(dev,32,32,(unsigned)lroundf(255*(1-expected)),(unsigned)lroundf(255*expected),0);
                cases++;
            }
        }
    }
    unsetenv("TSFP_NATIVE_CPU_VERTEX");d3d8_vsh_delete_shader(shader);
    dev->lpVtbl->SetVertexShader(dev,D3DFVF_XYZRHW|D3DFVF_DIFFUSE);
    dev->lpVtbl->SetRenderState(dev,D3DRS_FOGENABLE,FALSE);
    dev->lpVtbl->DrawPrimitiveUP(dev,D3DPT_QUADLIST,1,v,sizeof v[0]);
    ok &= native_test_pixel(dev,32,32,0,255,0); /* Fixed shader factor defaults to one. */
    dev->lpVtbl->SetRenderState(dev,D3DRS_FOGENABLE,old_fog);
    dev->lpVtbl->SetRenderState(dev,D3DRS_FOGCOLOR,old_color);
    d3d8_combiners_set_pixel_shader(0);
    dev->lpVtbl->SetVertexShader(dev,D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1);
    ok &= glGetError()==GL_NO_ERROR;
    fprintf(stderr,"[native-gl-test] programmed/unwritten fog, MAC/ILU masks, GPU/CPU, fixed default: %s (%d cases)\n",ok?"PASS":"FAIL",cases+1);
    return ok;
}
