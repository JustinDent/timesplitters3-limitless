/* US 45410066 UI groups. Placement values are the renderer's AUTO=0,
 * CENTRE=2, LEFT=3, RIGHT=4, SIDE=5 ABI. Keep complete widgets together;
 * the renderer applies the same anchor to their viewport and scissors. */
extern void sub_00022FF0(void), sub_001CDAE0(void);
extern void host_SetTwoDPlacement(int placement,uint32_t tag);
#include "pc_ui_layout.h"
static RECOMP_TLS int ui_placement, radar_group, health_group;
static RECOMP_TLS uint32_t ui_tag;
static void ui_place(int placement,uint32_t tag)
{
    /* Flush guest sprite batches before changing their recorded placement.
       Host state alone would move the preceding batch with the next widget. */
    guest_call(sub_00022FF0,0);
    host_SetTwoDPlacement(placement,tag);
    ui_placement=placement;ui_tag=tag;
}
void pc_display_radar_begin(void)
{
    /* The radar routine first draws world target markers. Only its later
       HUD disc and all following blips belong at the top-right corner. */
    if(radar_group)ui_place(4,0x00133AB0u);
}
void pc_display_health_side(void)
{
    /* sub_0013AB60 emits three passes over each side. At this primitive
       allocation call, its side index is above four arguments + return PC.
       Change placement only between halves, never inside an open batch. */
    if(health_group) {
        int side=WORD_AT(g_esp+0x34u)==0?3:4;
        if(side!=ui_placement)ui_place(side,0x0013AB60u);
    }
}
void pc_display_health(void (*original)(void))
{
    uint32_t view=WORD_AT(0x007E71C8u),frame=g_esp,saved;
    int old=ui_placement,old_health=health_group;
    uint32_t old_tag=ui_tag;
    float scale,x,y,left,top,right,bottom,adjusted;
    if(guest_call(sub_001CDAE0,0)!=1 || !guest_writable(view,0x24)) {
        original();return;
    }
    saved=WORD_AT(frame+16u);memcpy(&scale,&saved,4);
    uint32_t xb=WORD_AT(frame+8u),yb=WORD_AT(frame+12u);
    memcpy(&x,&xb,4);memcpy(&y,&yb,4);
    left=(float)(int32_t)WORD_AT(view);top=(float)(int32_t)WORD_AT(view+0x14u);
    right=(float)(int32_t)WORD_AT(view+4u)+1;bottom=(float)(int32_t)WORD_AT(view+0x18u)+1;
    x+=(float)(int32_t)WORD_AT(view+8u)-left;
    y+=(float)(int32_t)WORD_AT(view+0x1Cu)-top;
    adjusted=pc_ui_health_scale(scale,left,top,right,bottom,x,y);
    memcpy(&WORD_AT(frame+16u),&adjusted,4);
    health_group=pc_display_wide_now() && pc_display_hud_scale()<1.f;
    ui_place(2,0x0013AB60u); /* Directional hit marker stays centred. */
    original();
    ui_place(old,old_tag);
    health_group=old_health;WORD_AT(frame+16u)=saved;
}
void pc_display_ui(void (*original)(void),uint32_t tag)
{
    int old=ui_placement,old_radar=radar_group,placement=0;
    uint32_t old_tag=ui_tag;
    if(!pc_display_wide_now() || pc_display_hud_scale()>=1.f || guest_call(sub_001CDAE0,0)!=1) {
        original();return; /* Preserve the existing split-screen layout. */
    }
    switch(tag) {
    case 0x001334D0u: /* Ammo: fifth argument identifies the right-hand gun. */
        placement=WORD_AT(g_esp+20)==1?4:3;break;
    case 0x00133AB0u: radar_group=1;break;
    case 0x001358A0u: placement=5;break; /* Zoom controls; centre graphic stays centred. */
    case 0x0013FC50u: /* Story date/title, including all outline/shadow passes. */
    case 0x0012F670u: case 0x0014B930u: case 0x001409C0u: placement=3;break;
    default:original();return;
    }
    if(placement)ui_place(placement,tag);
    original();
    ui_place(old,old_tag);
    radar_group=old_radar;
}
