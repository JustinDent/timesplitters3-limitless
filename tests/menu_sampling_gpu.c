/* Exercise the production reconstruction on D3D11 WARP, including alpha and
 * minification. No original artwork, desktop capture or personal data. */
#define COBJMACROS
#include <d3d11.h>
#include <d3dcompiler.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "pc_menu_sampling.h"
#define CHECK(x) do { if(!(x)) { fprintf(stderr,"FAIL %d: %s\n",__LINE__,#x);exit(1); } } while(0)
#define HR(x) CHECK(SUCCEEDED(x))
static ID3D11Device *device;
static ID3D11DeviceContext *context;
static ID3D11Texture2D *target,*readback;
static ID3D11Buffer *parameters;
static ID3DBlob *compile(const char *source,const char *profile)
{
    ID3DBlob *code=NULL,*errors=NULL;
    HRESULT hr=D3DCompile(source,strlen(source),NULL,NULL,NULL,"main",profile,
        D3DCOMPILE_ENABLE_STRICTNESS|D3DCOMPILE_OPTIMIZATION_LEVEL3,0,&code,&errors);
    if(FAILED(hr) && errors) fprintf(stderr,"%s\n",(char *)ID3D10Blob_GetBufferPointer(errors));
    if(errors) ID3D10Blob_Release(errors);
    HR(hr);return code;
}
static void render(float enabled,float scale,float *pixels)
{
    float data[4]={enabled,scale,0,0};D3D11_MAPPED_SUBRESOURCE map;
    ID3D11DeviceContext_UpdateSubresource(context,(ID3D11Resource *)parameters,0,NULL,data,0,0);
    ID3D11DeviceContext_Draw(context,3,0);
    ID3D11DeviceContext_CopyResource(context,(ID3D11Resource *)readback,(ID3D11Resource *)target);
    HR(ID3D11DeviceContext_Map(context,(ID3D11Resource *)readback,0,D3D11_MAP_READ,0,&map));
    for(int y=0;y<64;y++) memcpy(pixels+y*64*4,(char *)map.pData+y*map.RowPitch,64*4*sizeof(float));
    ID3D11DeviceContext_Unmap(context,(ID3D11Resource *)readback,0);
}
int main(void)
{
    ID3D11VertexShader *vs;ID3D11PixelShader *ps;
    ID3D11Texture2D *texture;ID3D11ShaderResourceView *srv;
    ID3D11RenderTargetView *rtv;ID3D11SamplerState *sampler;
    D3D11_TEXTURE2D_DESC td={0};D3D11_BUFFER_DESC bd={0};
    D3D11_SAMPLER_DESC sd={0};D3D11_VIEWPORT viewport={0,0,64,64,0,1};
    D3D_FEATURE_LEVEL feature;
    HR(D3D11CreateDevice(NULL,D3D_DRIVER_TYPE_WARP,NULL,0,NULL,0,D3D11_SDK_VERSION,&device,&feature,&context));
    const char *vsource="float4 main(uint id:SV_VertexID):SV_POSITION {"
        "return float4(id==2?3.0:-1.0,id==1?3.0:-1.0,0,1);}";
    const char *psource=TSFP_MENU_HLSL
        "Texture2D tex:register(t0); SamplerState samp:register(s0);"
        "cbuffer Params:register(b0) {float enabled;float scale;float2 pad;}"
        "float4 main(float4 p:SV_POSITION):SV_TARGET {"
        "return menu_sample(tex,samp,p.xy/64.0*scale,enabled);}";
    ID3DBlob *code=compile(vsource,"vs_5_0");
    HR(ID3D11Device_CreateVertexShader(device,ID3D10Blob_GetBufferPointer(code),ID3D10Blob_GetBufferSize(code),NULL,&vs));
    ID3D10Blob_Release(code);code=compile(psource,"ps_5_0");
    HR(ID3D11Device_CreatePixelShader(device,ID3D10Blob_GetBufferPointer(code),ID3D10Blob_GetBufferSize(code),NULL,&ps));
    ID3D10Blob_Release(code);
    td.Width=td.Height=64;td.MipLevels=td.ArraySize=1;td.Format=DXGI_FORMAT_R32G32B32A32_FLOAT;
    td.SampleDesc.Count=1;td.BindFlags=D3D11_BIND_RENDER_TARGET;
    HR(ID3D11Device_CreateTexture2D(device,&td,NULL,&target));
    HR(ID3D11Device_CreateRenderTargetView(device,(ID3D11Resource *)target,NULL,&rtv));
    td.BindFlags=0;td.Usage=D3D11_USAGE_STAGING;td.CPUAccessFlags=D3D11_CPU_ACCESS_READ;
    HR(ID3D11Device_CreateTexture2D(device,&td,NULL,&readback));
    float texels[16*16*4];
    for(int y=0;y<16;y++)for(int x=0;x<16;x++) {
        int i=(y*16+x)*4;
        texels[i]=x<8?.1f:.9f;texels[i+1]=.4f;texels[i+2]=y<8?.2f:.7f;
        texels[i+3]=x<8?0.f:1.f;
    }
    td.Width=td.Height=16;td.Usage=D3D11_USAGE_IMMUTABLE;td.CPUAccessFlags=0;td.BindFlags=D3D11_BIND_SHADER_RESOURCE;
    D3D11_SUBRESOURCE_DATA initial={texels,16*4*sizeof(float),0};
    HR(ID3D11Device_CreateTexture2D(device,&td,&initial,&texture));
    HR(ID3D11Device_CreateShaderResourceView(device,(ID3D11Resource *)texture,NULL,&srv));
    bd.ByteWidth=16;bd.BindFlags=D3D11_BIND_CONSTANT_BUFFER;
    HR(ID3D11Device_CreateBuffer(device,&bd,NULL,&parameters));
    sd.Filter=D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sd.AddressU=sd.AddressV=sd.AddressW=D3D11_TEXTURE_ADDRESS_CLAMP;sd.MaxLOD=D3D11_FLOAT32_MAX;
    HR(ID3D11Device_CreateSamplerState(device,&sd,&sampler));
    ID3D11DeviceContext_OMSetRenderTargets(context,1,&rtv,NULL);
    ID3D11DeviceContext_RSSetViewports(context,1,&viewport);
    ID3D11DeviceContext_IASetPrimitiveTopology(context,D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ID3D11DeviceContext_VSSetShader(context,vs,NULL,0);
    ID3D11DeviceContext_PSSetShader(context,ps,NULL,0);
    ID3D11DeviceContext_PSSetShaderResources(context,0,1,&srv);
    ID3D11DeviceContext_PSSetSamplers(context,0,1,&sampler);
    ID3D11DeviceContext_PSSetConstantBuffers(context,0,1,&parameters);
    float linear[64*64*4],cubic[64*64*4];int changed=0;
    render(0,1,linear);render(1,1,cubic);
    for(int i=0;i<64*64*4;i++) {
        CHECK(isfinite(cubic[i]));CHECK(cubic[i]>=-.00001f && cubic[i]<=1.00001f);
        if(fabsf(cubic[i]-linear[i])>.005f)changed++;
        if(i%4==1)CHECK(fabsf(cubic[i]-.4f)<.00001f);
    }
    CHECK(changed>100);
    /* The dark side of a glyph edge darkens and the bright side brightens,
       while opaque/transparent plateaus retain exactly their original alpha. */
    CHECK(cubic[(20*64+30)*4+3]<linear[(20*64+30)*4+3]);
    CHECK(cubic[(20*64+33)*4+3]>linear[(20*64+33)*4+3]);
    CHECK(cubic[(20*64+10)*4+3]==0.f && cubic[(20*64+50)*4+3]==1.f);
    render(2,1,cubic);
    CHECK(cubic[(20*64+30)*4+3]<linear[(20*64+30)*4+3]);
    CHECK(cubic[(20*64+33)*4+3]>linear[(20*64+33)*4+3]);
    for(int i=0;i<64*64*4;i++)CHECK(cubic[i]>=-.00001f && cubic[i]<=1.00001f);
    for(int scale=4;scale<=8;scale+=4) {
        render(0,(float)scale,linear);render(1,(float)scale,cubic);
        CHECK(!memcmp(linear,cubic,sizeof(linear)));
        render(2,(float)scale,cubic);CHECK(!memcmp(linear,cubic,sizeof(linear)));
    }
    ID3D11DeviceContext_ClearState(context);
    ID3D11Buffer_Release(parameters);ID3D11SamplerState_Release(sampler);
    ID3D11ShaderResourceView_Release(srv);ID3D11Texture2D_Release(texture);
    ID3D11RenderTargetView_Release(rtv);ID3D11Texture2D_Release(target);ID3D11Texture2D_Release(readback);
    ID3D11PixelShader_Release(ps);ID3D11VertexShader_Release(vs);
    ID3D11DeviceContext_Release(context);ID3D11Device_Release(device);
    puts("PASS: GPU menu magnification, alpha, flat colours, bounded edges and unchanged native/minified sampling");
}
