#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pc_ui_layout.h"
#define TSFP_PC_DISPLAY
#define TRUE 1
#define FALSE 0
typedef int BOOL;
typedef float FLOAT;
typedef struct {float X,Y,Width,Height,MinZ,MaxZ;} D3DVIEWPORT8;
typedef struct {float x,y,width,height,min_depth,max_depth;} RhiViewport;
typedef struct {int32_t left,top,right,bottom;} RhiRect;
enum {XBOX_D3D8_2D_AUTO,XBOX_D3D8_2D_STRETCH,XBOX_D3D8_2D_CENTRE,XBOX_D3D8_2D_LEFT,XBOX_D3D8_2D_RIGHT};
static struct {unsigned width,height;D3DVIEWPORT8 viewport;} g_device_state;
static int g_2d_squeeze=1,g_cur_rt,g_2d_anchor=XBOX_D3D8_2D_CENTRE,g_scissor_enabled=1;
static RhiRect g_scissor={20,20,620,460};
static RhiViewport actual;
typedef struct {unsigned width,height;} Mode;
static Mode pc_display_effective(void){Mode m={g_device_state.width,g_device_state.height};return m;}
static float pc_display_hud_scale(void){return 4.f*g_device_state.height/(3.f*g_device_state.width);}
static float rt_scale_x(void){return g_device_state.width/640.f;}
static float rt_scale_y(void){return g_device_state.height/480.f;}
static int rhi_device_ready(void){return 1;}
static void rhi_set_viewports(int n,const RhiViewport *v){actual=*v;}
/* PRODUCTION */
#define CHECK(x) do {if(!(x)){fprintf(stderr,"FAIL: line %d %s\n",__LINE__,#x);return 1;}}while(0)
int main(void){
 const unsigned sizes[][2]={{640,480},{1280,1024},{1920,1080},{1920,1200},{3440,1440},{5120,1440},{7680,4320}};
 for(unsigned i=0;i<7;i++)for(int anchor=2;anchor<=4;anchor++){
  float w=sizes[i][0],h=sizes[i][1],scale=fminf(w/640,h/480);
  g_device_state.width=sizes[i][0];g_device_state.height=sizes[i][1];
  g_device_state.viewport=(D3DVIEWPORT8){0,0,640,480,0,1};g_2d_anchor=anchor;
  g_2d_squeeze=1;apply_host_viewport();RhiRect clip;CHECK(d3d8_GetScissor(&clip));
  CHECK(fabsf(actual.width/640-actual.height/480)<.001f);
  CHECK(fabsf(actual.width/640-scale)<.001f);
  CHECK(fabsf(clip.left-actual.x-20*scale)<1.1f);
  CHECK(fabsf(clip.right-actual.x-620*scale)<1.1f);
  CHECK(fabsf(clip.top-actual.y-20*scale)<1.1f);
  CHECK(fabsf(clip.bottom-actual.y-460*scale)<1.1f);
  if(anchor==3)CHECK(actual.x==0);
  if(anchor==4)CHECK(fabsf(actual.x+actual.width-w)<.001f);
  g_2d_squeeze=0;apply_host_viewport();CHECK(actual.x==0 && actual.y==0 && actual.width==w && actual.height==h);
 }
 puts("PASS: production viewport/scissor agreement at seven ratios and every anchor");return 0;
}
