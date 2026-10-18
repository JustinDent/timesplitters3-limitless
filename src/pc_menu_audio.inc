/* US 45410066 audio rows are custom painters, not native choice sliders.
 * sub_002C3720 edits staged 0..16 values; Accept saves them and Cancel
 * reloads them. Use those same values and preview calls, never profile data. */
extern void sub_00045C60(void),sub_00021780(void);
typedef struct MenuAudioTrack {
    uint32_t menu,descriptor,item;
    float x,y,w,h;
    unsigned sw,sh;
    uint64_t time;
} MenuAudioTrack;
static MenuAudioTrack menu_audio_tracks[2];
static uint32_t menu_audio_drag;

/* Capture the final row rectangle passed to the original painter. These
 * offsets and integer truncations match sub_002C3500 / sub_002C3610 and
 * sub_000814B0, so the paused layout and UI aspect fit share the visible bar. */
void pc_display_menu_audio_draw(void (*original)(void),int compact)
{
    uint32_t menu=WORD_AT(g_esp+4),item=WORD_AT(g_esp+8),rect=WORD_AT(g_esp+12);
    if(guest_writable(menu,0x23c) && guest_writable(item,0x10c) &&
       guest_writable(rect,16) && WORD_AT(item+0x88)==menu) {
        uint32_t id=WORD_AT(item+0x14);
        if(id==1 || id==3) {
            MenuAudioTrack *t=&menu_audio_tracks[id==3];
            float left=FLOAT_AT(rect),top=FLOAT_AT(rect+4);
            float right=FLOAT_AT(rect+8),bottom=FLOAT_AT(rect+12);
            memset(t,0,sizeof *t);
            if(isfinite(left) && isfinite(top) && isfinite(right) && isfinite(bottom) &&
               fabsf(left)<100000 && fabsf(top)<100000 && fabsf(right)<100000 && fabsf(bottom)<100000 &&
               right-left>65 && bottom>top) {
                t->menu=menu;t->descriptor=WORD_AT(menu+4);t->item=item;
                t->x=(float)(int)(left+45.f);t->w=(float)(int)(right-left-65.f);
                t->y=(float)(int)(top+(bottom-top)*(compact?.65f:.5f));
                t->h=compact?10.f:20.f;
                d3d8_display_output_shape(WORD_AT(0x004E8CF0u),WORD_AT(0x004E8CF4u),&t->sw,&t->sh);
                t->time=GetTickCount64();
            }
        }
    }
    original();
}

static int menu_audio_track_active(const MenuAudioTrack *t,uint32_t menu,unsigned sw,unsigned sh)
{
    return t->menu==menu && t->descriptor==WORD_AT(menu+4) &&
        t->sw==sw && t->sh==sh && GetTickCount64()-t->time<=100 &&
        guest_writable(t->item,0x10c) && WORD_AT(t->item+0x88)==menu &&
        !(WORD_AT(t->item+0x28)&5u) && t->w>0 && t->h>0;
}

static int menu_audio_pointer(uint32_t menu,PcMenuPointer p,unsigned actions,
    float x,float y,unsigned sw,unsigned sh)
{
    MenuAudioTrack *track=NULL;
    if(!p.left || p.right || menu_mouse.blocked) menu_audio_drag=0;
    for(unsigned i=0;i<2;i++) {
        MenuAudioTrack *t=&menu_audio_tracks[i];
        if(!menu_audio_track_active(t,menu,sw,sh)) continue;
        if(menu_audio_drag==t->item) {track=t;break;}
        if((actions&2) && !p.right &&
           x>=t->x && x<=t->x+t->w && y>=t->y && y<t->y+t->h &&
           x>=FLOAT_AT(menu+0xe4) && x<FLOAT_AT(menu+0xec) &&
           y>=FLOAT_AT(menu+0xe8) && y<FLOAT_AT(menu+0xf0)) {
            menu_audio_drag=t->item;track=t;break;
        }
    }
    if(!track) {menu_audio_drag=0;return 0;}
    /* Capture lasts only while held in this active menu. Moving over labels,
     * the other slider or footer cannot retarget it or synthesize Confirm. */
    menu_mouse.mouse_navigation=1;
    menu_mouse.wheel_pending=0;menu_mouse.scrolling=0;
    WORD_AT(menu+0xc4)&=~0x001F001Fu;
    WORD_AT(menu+0xc8)&=~0x1Fu;
    if(WORD_AT(menu+0x198)!=track->item) guest_call(sub_00076EF0,2,menu,track->item);
    unsigned value=pc_menu_slider_value(x,track->x,track->w,16);
    uint32_t volume=WORD_AT(track->item+0x14)==1?0x005236BCu:0x005236C0u;
    if(WORD_AT(volume)!=value) {
        WORD_AT(volume)=value;
        if(volume==0x005236BCu) guest_call(sub_00045C60,0); /* live music mix */
        if(WORD_AT(0x007BA16Cu) && guest_writable(WORD_AT(0x007BA178u),12))
            guest_call(sub_00021780,1,WORD_AT(WORD_AT(0x007BA178u)+8)); /* original volume preview */
    }
    return 1;
}
