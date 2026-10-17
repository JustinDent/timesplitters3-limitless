/* Execute production placement scopes with a simulated deferred draw queue. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "pc_ui_layout.h"
#define RECOMP_TLS
static uint32_t stack[32],g_esp;
static uint32_t view_address=0x100,view[9]={0,639,320,640,0,0,479,240,480};
static uint32_t *word(uint32_t a) {
    if(a==0x7e71c8)return &view_address;
    if(a>=0x100 && a<0x124)return &view[(a-0x100)/4];
    return &stack[a/4];
}
#define WORD_AT(a) (*word(a))
static int guest_writable(uint32_t a,size_t n){return a==0x100 && n<=sizeof view;}
static int wide=1,players=1,placement,pending=-1,draw_count;
static int drawn[64],selected[64],selection_count;
static uint32_t tag;
#define CHECK(x) do { if(!(x)){fprintf(stderr,"FAIL: %s line %d\n",#x,__LINE__);exit(1);} } while(0)
float pc_display_hud_scale(void) { return .75f; }
int pc_display_wide_now(void) { return wide; }
void sub_00022FF0(void) { if(pending>=0){drawn[draw_count++]=placement;pending=-1;} }
void sub_001CDAE0(void) {}
static uint32_t guest_call(void (*fn)(void),unsigned count,...)
{ (void)count;if(fn==sub_001CDAE0)return players;fn();return 0; }
void host_SetTwoDPlacement(int value,uint32_t t)
{ CHECK(pending<0);placement=value;tag=t;selected[selection_count++]=value; }
#include "pc_display_ui.inc"
static void draw(void){pending=1;g_esp+=4;}
static void rank_widget(void)
{
    /* The label, rank number and frame can be separate deferred batches. */
    for(int part=0;part<3;part++){
        pending=1;
        if(part<2)sub_00022FF0();
    }
    g_esp+=4;
}
static void radar(void)
{
    CHECK(placement==0);pending=1; /* World targets precede the radar. */
    pc_display_radar_begin();CHECK(placement==4);pending=1;g_esp+=4;
}
static void health(void)
{
    float scale;memcpy(&scale,&stack[4],4);
    CHECK(scale<=.958334f);CHECK(placement==2);
    pending=1; /* The central hit marker precedes both side gauges. */
    for(int pass=0;pass<3;pass++)for(int side=0;side<2;side++){
        stack[13]=side;pc_display_health_side();
        CHECK(placement==(wide?(side?4:3):2));pending=1;
    }
    g_esp+=4;
}
int main(void)
{
    for(unsigned i=0;i<2;i++){
        g_esp=0;stack[5]=i;pending=1;int n=draw_count;
        pc_display_ui(draw,0x1334d0);
        CHECK(drawn[n]==0 && drawn[n+1]==(i?4:3));
        CHECK(placement==0 && tag==0 && g_esp==4);
    }
    g_esp=0;int n=draw_count;pc_display_ui(radar,0x133ab0);
    CHECK(drawn[n]==0 && drawn[n+1]==4 && !radar_group && placement==0);
    int count=selection_count;pc_display_radar_begin();CHECK(count==selection_count);
    const uint32_t left[]={0x12f670,0x14b930,0x1409c0,0x13fc50};
    for(unsigned i=0;i<4;i++){g_esp=0;n=draw_count;pc_display_ui(draw,left[i]);CHECK(drawn[n]==3);}
    g_esp=0;n=draw_count;pc_display_ui(draw,0x1358a0);CHECK(drawn[n]==5 && placement==0);
    g_esp=0;stack[0]=0x12ec8b;n=draw_count;pending=1;
    pc_display_ui(rank_widget,0x12e3f0);
    CHECK(drawn[n]==0); /* Earlier HUD batches retain their own anchor. */
    for(int part=1;part<=3;part++)CHECK(drawn[n+part]==3);
    CHECK(placement==0 && tag==0 && g_esp==4);
    draw();sub_00022FF0();CHECK(drawn[n+4]==0);
    /* The same renderer is used for timers and other counters. */
    g_esp=0;stack[0]=0x14f5cc;n=draw_count;int before_count=selection_count;
    pc_display_ui(rank_widget,0x12e3f0);sub_00022FF0();
    CHECK(selection_count==before_count);
    for(int part=0;part<3;part++)CHECK(drawn[n+part]==0);
    /* Scope decorations in the middle retain the central anchor on both hosts. */
    CHECK(pc_ui_side_anchor(20,120,640)==0);
    CHECK(pc_ui_side_anchor(520,620,640)==1);
    CHECK(pc_ui_side_anchor(300,340,640)==.5f);
    CHECK(pc_ui_side_anchor(0,640,640)==.5f);
    for(int mode=0;mode<2;mode++){
        wide=mode;g_esp=0;stack[2]=stack[3]=0;
        float scale=1.2f;memcpy(&stack[4],&scale,4);uint32_t saved=stack[4];
        n=draw_count;pc_display_health(health);
        CHECK(stack[4]==saved && g_esp==4 && placement==0 && !health_group);
        CHECK(drawn[n]==2); /* Hit direction never inherits a side anchor. */
        count=selection_count;pc_display_health_side();CHECK(count==selection_count);
    }
    for(int i=0;i<2;i++){
        wide=i;players=i?2:1;g_esp=0;count=selection_count;
        pc_display_ui(draw,0x1334d0);CHECK(count==selection_count && g_esp==4);
        sub_00022FF0();
        g_esp=0;stack[0]=0x12ec8b;count=selection_count;
        pc_display_ui(rank_widget,0x12e3f0);
        CHECK(count==selection_count && g_esp==4 && placement==0);
        sub_00022FF0();
    }
    players=2;g_esp=0;count=selection_count;stack[4]=0x3f99999a;
    pc_display_health(draw);
    CHECK(selection_count==count && stack[4]==0x3f99999a && g_esp==4);
    sub_00022FF0();
    /* Proportional pixels and original margins at every supported wide ratio. */
    const float sizes[][2]={{640,480},{1280,1024},{1920,1080},{1920,1200},{3440,1440},{5120,1440},{7680,4320}};
    for(unsigned i=0;i<7;i++){
        float w=sizes[i][0],h=sizes[i][1],scale=h/480.f,k=4*h/(3*w);
        CHECK(fabsf(pc_ui_scale_x(k)*w/640-pc_ui_scale_y(k)*h/480)<.0001f);
        float fit=pc_ui_health_scale(1.2f,0,0,640,480,320,240);
        CHECK(fit<1 && pc_ui_health_scale(.5f,0,0,640,480,320,240)==.5f);
        /* Full outer mesh stays inside its authored inset after anchoring. */
        CHECK(320-307.2f*fit>=25.59f && 320+307.2f*fit<=614.41f);
        if(k>1)continue;
        CHECK(fabsf((w-(640-20)*w/640*k-w*(1-k))-20*scale)<.001f);
    }
    puts("PASS: queued widget boundaries, radar world-marker separation, anchors, defaults and split-screen guard");
}
