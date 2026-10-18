#define TSFP_MENU_MOUSE_TEST
#include "pc_menu_mouse.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#define CHECK(x) do { if(!(x)) { fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#x);exit(1); } } while(0)
static uint32_t memory[0x800000/4],g_esp,g_esi,g_ebp,g_edi;
static intptr_t g_xbox_mem_offset;
#define WORD_AT(a) (*(uint32_t *)((uintptr_t)(a)+g_xbox_mem_offset))
typedef struct GuestRegisters { uint32_t sp; } GuestRegisters;
static void save_registers(GuestRegisters *r) { r->sp=g_esp; }
static void restore_registers(GuestRegisters *r) { g_esp=r->sp; }
static int guest_writable(uint32_t p,unsigned n) { return p>=0x1000 && p<sizeof memory && n<=sizeof memory-p; }
static PcMenuPointer cursor;
static int active=1,mode,original_calls,selections;
static uint32_t navigation;
static int closed_pause,in_dispatch,changed_tab;
static unsigned long long tick=1000;
static unsigned long long GetTickCount64(void) {return tick;}
static int footer_enabled=1,measure_profile;
static uint32_t early_action,early_held;
static unsigned shape_w=16,shape_h=9;
static unsigned music_previews,sound_previews;
void sub_00045C60(void) {mode=13;}
void sub_00021780(void) {mode=14;}
static PcMenuPointer pc_controls_menu_pointer(void) { PcMenuPointer p=cursor;cursor.wheel=0;return p; }
static PcMenuPointer pc_controls_menu_hover_pointer(void) { return cursor; }
static void pc_controls_menu_mode(int on) { mode=on; }
void d3d8_display_output_shape(unsigned w,unsigned h,unsigned *sw,unsigned *sh)
{ (void)w;(void)h;*sw=shape_w;*sh=shape_h; }
void sub_00074760(void) { mode=5; }
void sub_00076EF0(void) { mode=6; }
void sub_00076510(void) {mode=7;}
void sub_0003FCF0(void) {mode=8;}
void sub_002E6D40(void) {mode=9;}
void sub_00079860(void) {mode=10;}
void sub_002E49F0(void) {mode=11;}
void sub_002E4A30(void) {mode=12;}
static uint32_t guest_call(void (*fn)(void),unsigned n,...)
{
    va_list ap; uint32_t menu,item;
    if(fn==sub_00045C60) {CHECK(n==0);++music_previews;return 0;}
    if(fn==sub_00021780) {CHECK(n==1);++sound_previews;return 0;}
    if(fn==sub_002E49F0) {CHECK(n==0);return 0;}
    if(fn==sub_002E4A30) {CHECK(n==0);return 3;}
    va_start(ap,n);menu=va_arg(ap,uint32_t);
    if(fn==sub_00079860) {
        CHECK(!in_dispatch && n==3 && menu==WORD_AT(0x761bbc));
        uint32_t target=va_arg(ap,uint32_t);
        CHECK(va_arg(ap,uint32_t)==UINT32_MAX);
        WORD_AT(WORD_AT(menu+0x170)+4)=target;++changed_tab;
        va_end(ap);return 0;
    }
    if(fn==sub_002E6D40) {
        CHECK(!in_dispatch && n==1 && menu==0);
        WORD_AT(WORD_AT(0x761c48))&=~0x101u;++closed_pause;
        va_end(ap);return 0;
    }
    if(fn==sub_00074760) { va_end(ap); return active; }
    if(fn==sub_00076510) {va_end(ap);return footer_enabled;}
    if(fn==sub_0003FCF0) {
        uint32_t height=va_arg(ap,uint32_t);
        float width=98.f,h=18.f;
        uint32_t text=va_arg(ap,uint32_t);
        if(measure_profile) width=(float)strlen((const char *)&WORD_AT(text))*5.f;
        memcpy(&WORD_AT(menu),&width,4);memcpy(&WORD_AT(height),&h,4);
        va_end(ap);return 0;
    }
    CHECK(fn==sub_00076EF0 && n==2);
    item=va_arg(ap,uint32_t);va_end(ap);
    WORD_AT(menu+0x198)=item; ++selections; return 0;
}
#include "pc_menu_mouse_guest.inc"
static void original(void) { ++original_calls;WORD_AT(WORD_AT(g_esp+4)+0xc4)|=navigation;g_esp+=4; }
static void dispatch(void)
{
    uint32_t menu=WORD_AT(g_esp+4);
    uint32_t frame=g_esp;
    ++in_dispatch;
    pc_display_menu_mouse_input(original);
    early_action=WORD_AT(menu+0xc4)&0xf0u;early_held=WORD_AT(menu+0xc8);
    g_esp=frame;
    pc_display_menu_mouse(original);
    /* The profile callback scans raw pad edges to choose a controller.
       Menu flags without this edge would select mapping[-1]. */
    if(WORD_AT(menu+0xc4)&0x10) CHECK(WORD_AT(0x6b93e0)&0x80);
    --in_dispatch;
}
static void run(uint32_t menu)
{
    uint32_t pressed=WORD_AT(0x6b93e0);
    WORD_AT(menu+0xc4)=WORD_AT(menu+0xc8)=0;
    g_esp=0x1000;WORD_AT(g_esp+4)=menu;
    pc_display_menu_mouse_dispatch(dispatch);CHECK(g_esp==0x1004);
    CHECK(WORD_AT(0x6b93e0)==pressed); /* source edge never escapes dispatch */
}
static void parent_dispatch(void)
{
    uint32_t frame=g_esp,root=WORD_AT(g_esp+4);
    ++in_dispatch;
    g_esp=0x1600;WORD_AT(g_esp+4)=WORD_AT(root+0x170);
    pc_display_menu_mouse_dispatch(dispatch);
    CHECK(WORD_AT(WORD_AT(0x761c48))&1); /* parent still uses this menu */
    --in_dispatch;g_esp=frame+4;
}
static unsigned hovered;
static void footer_draw(void)
{
    uint32_t frame=g_esp;
    for(g_esi=0;g_esi<4;g_esi++) {
        g_esp=0x1800;WORD_AT(g_esp)=0x767e4;
        FLOAT_AT(g_esp+4)=80.f+110.f*(float)g_esi;
        FLOAT_AT(g_esp+8)=430.f;WORD_AT(g_esp+12)=0x6000;
        if(menu_mouse_footer_text()) hovered|=1u<<g_esi;
    }
    g_esp=frame+4;
}
static void capture_footer(uint32_t menu)
{
    g_esp=0x1000;WORD_AT(g_esp+4)=menu;
    hovered=0;pc_display_menu_mouse_footer(footer_draw);
}
static unsigned profile_draws;
static uint32_t profile_colors[4];
static float profile_x[4];
static char profile_labels[4][100];
static void profile_draw(void)
{
    CHECK(profile_draws<4);
    profile_colors[profile_draws]=WORD_AT(g_esp+16);
    profile_x[profile_draws]=FLOAT_AT(g_esp+4);
    snprintf(profile_labels[profile_draws],100,"%s",(char *)&WORD_AT(WORD_AT(g_esp+12)));
    ++profile_draws;g_esp+=4;
}
static void audio_draw_original(void) {g_esp+=4;}
static void audio_tracks(uint32_t m,uint32_t music,uint32_t sound,int compact)
{
    for(unsigned i=0;i<2;i++) {
        uint32_t row=i?sound:music;
        WORD_AT(row)=4;WORD_AT(row+0x88)=m;WORD_AT(row+0x14)=i?3:1;
        FLOAT_AT(0x7000)=180;FLOAT_AT(0x7004)=100.f+(float)i*100;
        FLOAT_AT(0x7008)=460;FLOAT_AT(0x700c)=160.f+(float)i*100;
        g_esp=0x1800;WORD_AT(g_esp+4)=m;WORD_AT(g_esp+8)=row;WORD_AT(g_esp+12)=0x7000;
        pc_display_menu_audio_draw(audio_draw_original,compact);CHECK(g_esp==0x1804);
    }
}
static void audio_tests(void)
{
    const uint32_t m=0x2000,music=0x3000,sound=0x3400;
    memset(memory,0,sizeof memory);memset(&menu_mouse,0,sizeof menu_mouse);
    memset(menu_mouse_footer,0,sizeof menu_mouse_footer);menu_audio_drag=0;
    active=1;navigation=0;WORD_AT(0x79094c)=101;
    WORD_AT(0x4e8cf0)=640;WORD_AT(0x4e8cf4)=480;
    WORD_AT(m+0x1b0)=3;WORD_AT(m+0x178)=music;WORD_AT(music+4)=sound;
    FLOAT_AT(m+0xe4)=100;FLOAT_AT(m+0xe8)=70;FLOAT_AT(m+0xec)=540;FLOAT_AT(m+0xf0)=400;
    WORD_AT(0x7ba16c)=1;WORD_AT(0x7ba178)=0x6000;WORD_AT(0x6008)=42;
    for(unsigned layout=0;layout<2;layout++) for(unsigned ratio=0;ratio<4;ratio++) {
        const unsigned widths[]={640,1920,3440,1280},heights[]={480,1080,1440,1024};
        shape_w=widths[ratio];shape_h=heights[ratio];
        float scale=fminf((float)shape_w/640,(float)shape_h/480);
        float ox=((float)shape_w-640*scale)*.5f,oy=((float)shape_h-480*scale)*.5f;
        cursor=(PcMenuPointer){0,0,(float)shape_w,(float)shape_h,1,0,0,0};run(m);
        audio_tracks(m,music,sound,(int)layout);
        MenuAudioTrack *t=&menu_audio_tracks[0];
        CHECK(t->x==225 && t->w==215 && t->h==(layout?10:20));
        WORD_AT(0x5236bc)=16;WORD_AT(0x5236c0)=16;
        cursor.x=ox+(t->x+t->w*.25f)*scale;cursor.y=oy+(t->y+2)*scale;
        run(m);cursor.left=1;run(m);
        CHECK(WORD_AT(0x5236bc)==4 && WORD_AT(m+0x198)==music);
        CHECK(!(WORD_AT(m+0xc4)&0x10) && menu_audio_drag==music);
        unsigned previews=music_previews;run(m);CHECK(music_previews==previews);
        cursor.x=ox+(t->x+t->w*.75f)*scale;cursor.y=oy+(t->y+105)*scale;cursor.wheel=-1;
        run(m);CHECK(WORD_AT(0x5236bc)==12 && WORD_AT(0x5236c0)==16);
        CHECK(WORD_AT(m+0x198)==music && !(WORD_AT(m+0xc4)&0x001F001F));
        cursor.x=ox+(t->x-30)*scale;run(m);CHECK(WORD_AT(0x5236bc)==0);
        cursor.x=ox+(t->x+t->w+30)*scale;run(m);CHECK(WORD_AT(0x5236bc)==16);
        cursor.left=0;run(m);CHECK(!menu_audio_drag);
        t=&menu_audio_tracks[1];cursor.x=ox+(t->x+t->w*.5f)*scale;cursor.y=oy+(t->y+2)*scale;
        run(m);cursor.left=1;run(m);CHECK(WORD_AT(0x5236c0)==8);
        CHECK(WORD_AT(m+0x198)==sound && WORD_AT(0x5236bc)==16);
        cursor.valid=0;run(m);cursor.valid=1;cursor.x+=20;run(m);
        CHECK(!menu_audio_drag && WORD_AT(0x5236c0)==8); /* focus loss cancels */
        cursor.left=0;run(m);WORD_AT(sound+0x28)=1;cursor.left=1;run(m);
        CHECK(!menu_audio_drag && WORD_AT(0x5236c0)==8); /* disabled */
        WORD_AT(sound+0x28)=0;cursor.left=0;run(m);tick+=101;cursor.left=1;run(m);
        CHECK(!menu_audio_drag && WORD_AT(0x5236c0)==8); /* stale draw */
        audio_tracks(m,music,sound,(int)layout);run(m);
        CHECK(!menu_audio_drag); /* held button cannot resume after cancellation */
        cursor.left=0;run(m);cursor.left=1;run(m);CHECK(menu_audio_drag==sound);
        ++shape_w;run(m);CHECK(!menu_audio_drag); /* resize invalidates drag */
        --shape_w;cursor.left=0;run(m);cursor.left=1;run(m);
        WORD_AT(m+4)=1;run(m);CHECK(!menu_audio_drag); /* replaced menu */
        WORD_AT(m+4)=0;cursor.left=0;run(m);
    }
    CHECK(music_previews && sound_previews);
    CHECK(pc_menu_slider_value(-1,0,100,16)==0 && pc_menu_slider_value(101,0,100,16)==16);
    CHECK(pc_menu_slider_value(NAN,0,100,16)==0 && pc_menu_slider_value(1,0,0,16)==0);
}
int main(void)
{
    const uint32_t m=0x2000,item=0x3000,v=0x4000;
    float x,y;
    PcMenuPointer p={960,540,1920,1080,1,0,0,0};
    CHECK(pc_menu_mouse_position(p,16.f/9,640,480,&x,&y) && x==320 && y==240);
    p.x=480;CHECK(pc_menu_mouse_position(p,16.f/9,640,480,&x,&y) && fabsf(x-106.6667f)<.001f);
    p=(PcMenuPointer){1720,720,3440,1440,1,0,0,0};
    CHECK(pc_menu_mouse_position(p,3440.f/1440,640,480,&x,&y) && x==320 && y==240);
    p.x=100;CHECK(pc_menu_mouse_position(p,3440.f/1440,640,480,&x,&y) && x<0);
    p=(PcMenuPointer){0,500,1920,1080,1,0,0,0};
    CHECK(!pc_menu_mouse_position(p,4.f/3,640,480,&x,&y)); /* pillarbox */
    p=(PcMenuPointer){400,10,800,800,1,0,0,0};
    CHECK(!pc_menu_mouse_position(p,16.f/9,640,480,&x,&y)); /* letterbox */
    p=(PcMenuPointer){540,540,1080,1080,1,0,0,0};
    CHECK(pc_menu_mouse_position(p,1,640,480,&x,&y) && x==320 && y==240);
    p.x=NAN;CHECK(!pc_menu_mouse_position(p,1,640,480,&x,&y));
    /* Render resolution need not equal the window's client size. */
    p=(PcMenuPointer){320,315,640,480,1,0,0,0};
    CHECK(pc_menu_mouse_position(p,16.f/9,640,480,&x,&y) && x==320 && y==340);

    g_xbox_mem_offset=(intptr_t)memory;
    WORD_AT(0x79094c)=101;WORD_AT(0x7e71c8)=v;
    WORD_AT(0x4e8cf0)=640;WORD_AT(0x4e8cf4)=480;
    WORD_AT(v+4)=640;WORD_AT(v+0x18)=480;
    WORD_AT(m+0x1b0)=3;WORD_AT(m+0x178)=item;WORD_AT(item)=4;
    FLOAT_AT(m+0xe4)=100;FLOAT_AT(m+0xe8)=70;FLOAT_AT(m+0xec)=540;FLOAT_AT(m+0xf0)=400;
    FLOAT_AT(item+0x78)=200;FLOAT_AT(item+0x7c)=100;FLOAT_AT(item+0x80)=440;FLOAT_AT(item+0x84)=160;
    cursor=(PcMenuPointer){640,180,1280,720,1,0,0,0};
    run(m);CHECK(!selections && mode);
    cursor.y=181;run(m);CHECK(selections==1 && WORD_AT(m+0x198)==item);
    WORD_AT(m+0x198)=0;run(m);CHECK(!WORD_AT(m+0x198)); /* keyboard keeps selection */
    cursor.left=1;run(m);CHECK(WORD_AT(m+0xc4)==0x100010 && selections==2);
    run(m);CHECK(!WORD_AT(m+0xc4)); /* held click does not repeat */
    cursor.left=0;run(m);
    WORD_AT(0x6b93e0)=0x180;cursor.left=1;run(m); /* preserve real pad edges */
    CHECK(WORD_AT(0x6b93e0)==0x180);
    WORD_AT(0x6b93e0)=0;cursor.left=0;run(m);
    WORD_AT(item+0x28)=4;cursor.left=1;run(m);CHECK(!WORD_AT(m+0xc4));
    cursor.left=0;run(m);WORD_AT(item+0x28)=0;
    cursor.x=0;cursor.left=1;run(m);CHECK(!WORD_AT(m+0xc4));
    cursor.left=0;cursor.right=1;run(m);CHECK(WORD_AT(m+0xc4)==0x20);
    run(m);CHECK(!WORD_AT(m+0xc4));
    /* A changed menu cannot consume a button held from the previous one. */
    memcpy((char *)memory+0x5000,(char *)memory+m,0x23c);
    cursor.x=640;cursor.left=1;run(0x5000);CHECK(!WORD_AT(0x5000+0xc4));
    cursor.left=cursor.right=0;run(0x5000);cursor.left=1;run(0x5000);
    CHECK(WORD_AT(0x5000+0xc4)==0x100010);
    cursor.left=0;run(m);cursor.valid=0;cursor.left=1;run(m);
    cursor.valid=1;run(m);CHECK(!WORD_AT(m+0xc4)); /* focus/drag into window */
    cursor.left=0;run(m);active=0;cursor.left=1;run(m);CHECK(!WORD_AT(m+0xc4));
    active=1;cursor.left=0;run(m);
    /* A held controller/key repeat must not move off the item being hovered.
       Once the pointer stops, keyboard/controller navigation owns selection. */
    navigation=0x20002;cursor.x+=2;run(m);CHECK(!WORD_AT(m+0xc4));
    navigation=0x20000;run(m);CHECK(!WORD_AT(m+0xc4)); /* between mouse packets */
    navigation=0x20002;run(m);CHECK(WORD_AT(m+0xc4)==navigation); /* fresh direction */
    /* Clicking into a submenu must retain mouse ownership even if an older
       direction is still held and no further mouse motion arrives. */
    navigation=0x20000;cursor.left=1;run(m);
    CHECK(WORD_AT(m+0xc4)==0x100010);
    run(0x5000);CHECK(!WORD_AT(0x5000+0xc4)); /* no repeat or click-through */
    cursor.left=0;
    for(int n=0;n<30;n++) { run(0x5000);CHECK(!WORD_AT(0x5000+0xc4)); }
    navigation=0x20002;run(0x5000);CHECK(WORD_AT(0x5000+0xc4)==navigation);
    /* Back also claims the pointer, even outside the selectable rows. */
    navigation=0x20000;cursor.x=0;cursor.right=1;run(0x5000);
    CHECK(WORD_AT(0x5000+0xc4)==0x20);
    run(m);CHECK(!WORD_AT(m+0xc4));
    cursor.right=0;run(m);CHECK(!WORD_AT(m+0xc4));
    cursor.valid=0;run(m);CHECK(WORD_AT(m+0xc4)==navigation);
    cursor.valid=1;run(m);CHECK(WORD_AT(m+0xc4)==navigation);
    navigation=0;
    /* Wheel steps navigate without confirming or snapping to the hovered row. */
    cursor.x=640;cursor.wheel=-3;run(m);
    CHECK(WORD_AT(m+0xc4)==0x20002);
    CHECK(!(WORD_AT(m+0xc8)&0x10));
    run(m);CHECK(WORD_AT(m+0xc4)==0x20002);
    run(m);CHECK(WORD_AT(m+0xc4)==0x20002);
    run(m);CHECK(!WORD_AT(m+0xc4));
    cursor.wheel=2;run(m);CHECK(WORD_AT(m+0xc4)==0x10001);
    cursor.wheel=-1;run(m);CHECK(WORD_AT(m+0xc4)==0x20002);
    run(m);CHECK(!WORD_AT(m+0xc4)); /* reversal cancels queued Up */
    cursor.wheel=5;run(m);cursor.valid=0;run(m);
    cursor.valid=1;run(m);CHECK(!WORD_AT(m+0xc4)); /* focus loss drains queue */
    cursor.wheel=5;run(m);run(0x5000);CHECK(!WORD_AT(0x5000+0xc4));
    run(m);cursor.wheel=-5;run(m);navigation=0x10001;run(m);
    CHECK(WORD_AT(m+0xc4)==0x10001);navigation=0;run(m);CHECK(!WORD_AT(m+0xc4));
    cursor.wheel=-5;run(m);cursor.left=1;run(m);
    CHECK(WORD_AT(m+0xc4)==0x100010);cursor.left=0;run(m);CHECK(!WORD_AT(m+0xc4));
    cursor.x+=1;cursor.wheel=-1;int selected_before=selections;run(m);
    CHECK(selections==selected_before && WORD_AT(m+0xc4)==0x20002);
    /* Real wheel notches arrive between frames of continuous pointer motion.
       Hover must not pull the selection back between those notches, or when
       list animation moves a different row underneath the moving cursor. */
    cursor.left=cursor.right=0;cursor.x=640;cursor.y=181;cursor.wheel=0;
    selected_before=selections;WORD_AT(m+0x198)=0;
    for(int frame=0;frame<90;frame++) {
        tick+=16;cursor.y=181.f+(float)(frame%12)*2.f;
        cursor.wheel=(frame<40 && frame%8==0)?-1:0;
        run(m);
        CHECK(selections==selected_before); /* no hover during scroll gesture */
        CHECK(WORD_AT(m+0xc4)==((frame<40 && frame%8==0)?0x20002u:0u));
    }
    tick+=300;run(m);CHECK(selections==selected_before);
    cursor.y+=1;tick+=16;run(m);CHECK(selections==selected_before+1);
    /* A deliberate click remains immediate during the scrolling gesture. */
    cursor.wheel=-1;run(m);cursor.left=1;tick+=16;run(m);
    CHECK(WORD_AT(m+0xc4)==0x100010);cursor.left=0;run(m);
    active=1;WORD_AT(0x79094c)=1;run(m);CHECK(!WORD_AT(m+0xc4));
    /* Pause and quick Objectives accept the same input, but gameplay does not. */
    WORD_AT(0x761c48)=0x7000;WORD_AT(0x7000)=0x101;
    cursor.left=0;cursor.wheel=0;run(m);CHECK(mode);
    cursor.left=1;run(m);CHECK(WORD_AT(m+0xc4)==0x100010);
    /* Paused update still has a cropped world viewport; the menu is drawn
       on the full UI canvas. Its top row must hit the actual visual position. */
    WORD_AT(v+4)=639;WORD_AT(v+0x18)=430;
    cursor.y=154.5f;cursor.left=0;run(m);cursor.left=1;run(m);
    CHECK(WORD_AT(m+0xc4)==0x100010); /* pause pointer matches rendered row */
    WORD_AT(v+4)=640;WORD_AT(v+0x18)=480;
    WORD_AT(0x7000)=1;cursor.left=0;run(m);cursor.right=1;run(m);
    CHECK(WORD_AT(m+0xc4)==0x20);
    WORD_AT(0x761bbc)=0x7500;WORD_AT(0x7500+0x170)=m;WORD_AT(0x761bb8)=UINT32_MAX;
    WORD_AT(0x7000)=0x101;cursor.right=0;run(m);cursor.right=1;run(m);
    CHECK(closed_pause==1 && !(WORD_AT(0x7000)&1) && !mode);
    run(m);CHECK(closed_pause==1); /* held Back cannot toggle it open */
    WORD_AT(0x7000)=0x21;WORD_AT(0x761bb8)=0;cursor.right=0;run(m);cursor.right=1;run(m);
    CHECK(closed_pause==2 && !mode); /* full Pause resumes too */
    WORD_AT(0x7000)=0x21;cursor.right=0;run(m);cursor.right=1;
    g_esp=0x1000;WORD_AT(g_esp+4)=0x7500;
    pc_display_menu_mouse_dispatch(parent_dispatch);
    CHECK(closed_pause==3 && !mode && g_esp==0x1004);
    WORD_AT(0x7000)=0x101;WORD_AT(m+0x1c0)=0x1234;
    cursor.right=0;run(m);cursor.right=1;run(m);
    CHECK(closed_pause==3 && WORD_AT(m+0xc4)==0x20); /* callback owns Back */
    WORD_AT(m+0x1c0)=0;WORD_AT(0x7000)=5;
    cursor.right=0;run(m);cursor.right=1;run(m);CHECK(closed_pause==3);
    WORD_AT(0x7000)=1;WORD_AT(0x761bb8)=1;
    cursor.right=0;run(m);cursor.right=1;run(m);CHECK(closed_pause==3);
    WORD_AT(0x761bb8)=UINT32_MAX;WORD_AT(0x761bbc)=0x5000;
    cursor.right=0;run(m);cursor.right=1;run(m);CHECK(closed_pause==3);
    WORD_AT(0x7000)=0;run(m);CHECK(!WORD_AT(m+0xc4) && !mode);
    WORD_AT(0x7000)=1;cursor.right=0;cursor.left=0;run(m);
    for(unsigned i=0;i<4;i++) {
        WORD_AT(m+0x128+i*16)=i;
        WORD_AT(m+0x12c+i*16)=0x6000+i;
        WORD_AT(m+0x130+i*16)=0;
    }
    WORD_AT(m+0x40)=8;cursor.wheel=-1;run(m);CHECK((early_held&0xf)==2);
    run(m);CHECK(!(early_held&0xf));WORD_AT(m+0x40)=0;
    cursor=(PcMenuPointer){295,652.5f,1280,720,1,0,0,2};
    capture_footer(m);CHECK(hovered==1 && cursor.wheel==2); /* draw never eats wheel */
    cursor.valid=0;capture_footer(m);CHECK(!hovered);cursor.valid=1;
    active=0;capture_footer(m);CHECK(!hovered);active=1;
    footer_enabled=0;capture_footer(m);CHECK(!hovered);footer_enabled=1;
    WORD_AT(m+0x130)=1;capture_footer(m);CHECK(!hovered);WORD_AT(m+0x130)=0;
    WORD_AT(m+0x40)=0x4000000;capture_footer(m);CHECK(!hovered);WORD_AT(m+0x40)=0;
    cursor.x=0;capture_footer(m);CHECK(!hovered);
    cursor.wheel=0;
    for(unsigned i=0;i<4;i++) {
        cursor.left=0;cursor.x=640.f+(90.f+110.f*(float)i-320.f)*1.5f;
        cursor.y=435.f*1.5f;run(m);
        uint32_t selected=WORD_AT(m+0x198);
        cursor.left=1;run(m);
        CHECK(early_action==(0x10u<<i)); /* before tick callbacks, not just rows */
        CHECK(WORD_AT(m+0x198)==selected);
        run(m);CHECK(!early_action); /* held click never repeats */
    }
    cursor.left=0;run(m);footer_enabled=0;cursor.left=1;run(m);CHECK(!early_action);
    footer_enabled=1;cursor.left=0;run(m);WORD_AT(m+0x160)=2;
    cursor.left=1;run(m);CHECK(!early_action); /* now-hidden prompt */
    WORD_AT(m+0x160)=0;cursor.left=0;run(m);tick+=101;
    cursor.left=1;run(m);CHECK(!early_action); /* stale rendering */
    capture_footer(m);cursor.left=0;run(m);shape_w=4;shape_h=3;
    cursor.left=1;run(m);CHECK(!early_action); /* wait for redraw after resize */
    shape_w=16;shape_h=9;cursor.left=0;run(m);WORD_AT(m+0x40)=0x4000000;
    cursor.left=1;run(m);CHECK(!early_action); /* hidden footer */
    /* Themed renderers pack visible slots: hidden Select must not shift
     * Back/Player Progress onto the wrong action. */
    WORD_AT(m+0x40)=0;WORD_AT(m+0x130)=2;WORD_AT(m+0x168)=0x2d91f0;
    menu_mouse_footer_drawing=m;g_esi=0;g_esp=0x1800;
    WORD_AT(g_esp)=0x2d91b9;FLOAT_AT(g_esp+4)=300;FLOAT_AT(g_esp+8)=440;
    WORD_AT(g_esp+12)=0x6000;menu_mouse_footer_text();menu_mouse_footer_drawing=0;
    CHECK(menu_mouse_footer[1].x==300 && menu_mouse_footer[1].descriptor[0]==1);
    cursor.left=0;cursor.x=640;cursor.y=444*1.5f;run(m);
    cursor.left=1;run(m);CHECK(early_action==0x20);
    WORD_AT(m+0x40)=0;cursor.left=0;run(m);run(0x5000);cursor.left=1;
    run(0x5000);CHECK(!early_action); /* previous screen's footer is inert */
    /* Pause tabs belong to the parent, not the selectable row list. */
    WORD_AT(0x79094c)=1;WORD_AT(0x7000)=0x21;
    WORD_AT(0x761bbc)=0x7500;WORD_AT(0x7500+0x170)=m;
    WORD_AT(m+4)=0x524700;WORD_AT(0x5247c4)=0x524700;
    WORD_AT(0x5247c4+3*12)=0x524730;
    cursor=(PcMenuPointer){760,75,1280,720,1,0,0,0};run(m);
    g_ebp=0x1700;g_esp=0x1800;WORD_AT(g_esp)=0x2e51f9;
    WORD_AT(g_esp+0x2c)=4; /* generated EBP is local; shared EBP is unrelated */
    FLOAT_AT(g_esp+4)=390;FLOAT_AT(g_esp+8)=40;WORD_AT(g_esp+12)=0x6000;
    CHECK(menu_mouse_footer_text() && g_ebp==0x1700);
    cursor.left=1;run(m);
    CHECK(changed_tab==1 && WORD_AT(m+4)==0x524730 && WORD_AT(0x7000+0x264)==2);
    CHECK(FLOAT_AT(0x7000+0x260)==1.f && !WORD_AT(m+0xc4));
    run(m);CHECK(changed_tab==1); /* held click cannot change another tab */
    cursor.left=0;run(m);cursor.left=1;run(m);CHECK(changed_tab==1); /* stale source */
    WORD_AT(m+4)=0x524700;cursor.left=0;run(m);tick+=101;
    cursor.left=1;run(m);CHECK(changed_tab==1); /* stale draw */
    g_esp=0x1800;WORD_AT(g_esp)=0x2e51f9;menu_mouse_footer_text();
    cursor.left=0;run(m);WORD_AT(0x789640)=1;cursor.left=1;run(m);
    CHECK(changed_tab==1);WORD_AT(0x789640)=0;
    cursor.valid=0;g_esp=0x1800;CHECK(!menu_mouse_footer_text());cursor.valid=1;
    /* Nested dispatch must finish before its child is replaced. */
    cursor.left=0;run(m);cursor.left=1;
    g_esp=0x1000;WORD_AT(g_esp+4)=0x7500;
    pc_display_menu_mouse_dispatch(parent_dispatch);
    CHECK(changed_tab==2 && WORD_AT(m+4)==0x524730 && g_esp==0x1004);
    /* Profile footer: two independently hovered button tokens in one
       centered row, with remapped bindings measured by the game font. */
    WORD_AT(0x79094c)=101;WORD_AT(m+4)=0x4e8d54;
    WORD_AT(item+0x14)=(uint32_t)-4;WORD_AT(item+0x88)=m;
    WORD_AT(item+0x20)=0x6000;WORD_AT(item+0x28)=0x40e0004;
    strcpy((char *)&WORD_AT(0x6000),"\v2 Select  \v1 Back");
    measure_profile=1;WORD_AT(0x7e7260)=1;
    cursor=(PcMenuPointer){610,502.5f,1280,720,1,0,0,0};run(m);
    g_esp=0x1800;g_edi=m;g_esi=item;WORD_AT(g_esp)=0x763c3;
    FLOAT_AT(g_esp+4)=320;FLOAT_AT(g_esp+8)=330;
    WORD_AT(g_esp+12)=0x6000;WORD_AT(g_esp+16)=0x7f7f7f55;
    CHECK(menu_mouse_profile_text(profile_draw));
    CHECK(profile_draws==2 && profile_colors[0]==0x405a7f55 && profile_colors[1]==0x7f7f7f55);
    CHECK(!strcmp(profile_labels[0],"\v2 Select") && !strcmp(profile_labels[1],"\v1 Back"));
    CHECK(profile_x[1]>profile_x[0]+40 && WORD_AT(0x7e7260)==1 && g_esp==0x1804);
    cursor.left=1;run(m);CHECK(early_action==0x10);
    cursor.left=0;cursor.x=670;run(m);cursor.left=1;run(m);CHECK(early_action==0x20);
    cursor.left=0;run(m);WORD_AT(item+0x28)|=1;cursor.left=1;run(m);CHECK(!early_action);
    g_esp=0x1800;CHECK(!menu_mouse_profile_text(profile_draw)); /* disabled label */
    WORD_AT(item+0x28)&=~1u;cursor.left=0;run(m);tick+=101;
    cursor.left=1;run(m);CHECK(!early_action); /* stale profile footer */
    cursor.left=0;run(m);WORD_AT(m+4)=0;cursor.left=1;run(m);CHECK(!early_action);
    CHECK(original_calls>10);
    audio_tests();
    puts("PASS: menu pointer mapping, selection, click lifetime and gameplay isolation");
    return 0;
}
