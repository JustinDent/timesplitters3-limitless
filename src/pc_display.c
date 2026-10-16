#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include "pc_display.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
#include <SDL3/SDL.h>
#include "config_files.h"
#define ULONGLONG Uint64
#define GetTickCount64 SDL_GetTicks
#endif

static PcDisplayMode current;
static int initialized;
static int camera_frame;
static PcDisplayMode preview_previous;
static ULONGLONG preview_deadline;
static char path[32768];

int pc_display_valid(PcDisplayMode m)
{
    return (!m.width && !m.height) || (m.width >= 640 && m.height >= 480 &&
        m.width <= 7680 && m.height <= 2160 &&
        (unsigned long long)m.width * m.height <= 16588800 &&
        m.width >= m.height && m.width <= 4 * m.height);
}

static void init(void)
{
    const char *p; FILE *f; unsigned w,h; char tail;
    if(initialized) return;
    initialized=1;
    p=getenv("TSFP_DISPLAY_CONFIG");
    if(p && *p) snprintf(path,sizeof path,"%s",p);
    else {
        p=getenv("RECOMP_INPUT_CONFIG");
        if(p && *p) {
            char *a,*b,*slash;
            snprintf(path,sizeof path,"%s",p);
            a=strrchr(path,'/'); b=strrchr(path,'\\');
            slash=a && b ? (a>b?a:b) : a?a:b;
            if(slash) snprintf(slash+1,sizeof path-(size_t)(slash+1-path),"pc-display.ini");
            else snprintf(path,sizeof path,"pc-display.ini");
        } else snprintf(path,sizeof path,"pc-display.ini");
    }
    f=fopen(path,"rb");
    if(f) {
        if(fscanf(f,"TSFP_DISPLAY_V1 %u %u %c",&w,&h,&tail)==2) {
            PcDisplayMode m={w,h};
            if(pc_display_valid(m)) current=m;
        }
        fclose(f);
    }
    fprintf(stderr,"[PC-DISPLAY] Settings %s, resolution %ux%u\n",path,current.width,current.height);
}

PcDisplayMode pc_display_current(void) { init(); return current; }

static void add_mode(PcDisplayMode *out,int *count,int cap,unsigned w,unsigned h)
{
    int i; PcDisplayMode m={w,h};
    if(!pc_display_valid(m)) return;
    for(i=0;i<*count;i++) if(out[i].width==w && out[i].height==h) return;
    if(*count<cap) out[(*count)++]=m;
}

int pc_display_modes(HWND window,PcDisplayMode *out,int capacity)
{
#ifdef _WIN32
    DISPLAY_DEVICEA primary; DEVMODEA dm; int count=0,reported_capacity; DWORD i;
    (void)window;
    if(!out || capacity<=0) return 0;
    init(); reported_capacity=capacity-(current.width?1:0);
    add_mode(out,&count,capacity,0,0);
    /* Use the primary adapter, even if the game window is on another screen.
       Never manufacture unsupported presets when enumeration is unavailable. */
    for(i=0;;i++) {
        memset(&primary,0,sizeof primary); primary.cb=sizeof primary;
        if(!EnumDisplayDevicesA(NULL,i,&primary,0)) { primary.DeviceName[0]=0; break; }
        if((primary.StateFlags & (DISPLAY_DEVICE_PRIMARY_DEVICE|DISPLAY_DEVICE_ATTACHED_TO_DESKTOP)) ==
            (DISPLAY_DEVICE_PRIMARY_DEVICE|DISPLAY_DEVICE_ATTACHED_TO_DESKTOP)) break;
    }
    if(primary.DeviceName[0]) for(i=0;;i++) {
        memset(&dm,0,sizeof dm); dm.dmSize=sizeof dm;
        if(!EnumDisplaySettingsA(primary.DeviceName,i,&dm)) break;
        if(dm.dmBitsPerPel>=24)
            add_mode(out,&count,reported_capacity,dm.dmPelsWidth,dm.dmPelsHeight);
    }
#else
    int count=0,n=0;
    if(!out || capacity<=0)return 0;
    add_mode(out,&count,capacity,0,0);
    SDL_DisplayID display=window?SDL_GetDisplayForWindow((SDL_Window *)window):SDL_GetPrimaryDisplay();
    const SDL_DisplayMode *desktop=SDL_GetDesktopDisplayMode(display);
    if(desktop)add_mode(out,&count,capacity,desktop->w,desktop->h);
    add_mode(out,&count,capacity,1280,720);
    add_mode(out,&count,capacity,1920,1080);
    add_mode(out,&count,capacity,2560,1440);
    add_mode(out,&count,capacity,3840,2160);
    SDL_DisplayMode **modes=SDL_GetFullscreenDisplayModes(display,&n);
    for(int i=0;modes&&i<n;i++)add_mode(out,&count,capacity,modes[i]->w,modes[i]->h);
    SDL_free(modes);
#endif
    /* Stable ascending dimensions, with the console default first. Keep the
       current custom mode visible separately, so opening cannot change it. */
    for(int a=1;a<count;a++) for(int b=a;b>1;b--) {
        PcDisplayMode left=out[b-1],right=out[b];
        if(left.width<right.width || (left.width==right.width && left.height<=right.height)) break;
        out[b-1]=right; out[b]=left;
    }
    init(); add_mode(out,&count,capacity,current.width,current.height);
    return count;
}

int pc_display_apply(PcDisplayMode m)
{
    PcDisplayMode before;
    init();
    if(!pc_display_valid(m)) return 0;
    before=current;
    current=m;
    if(!xbox_D3D8ChangeResolution(m.width,m.height)) { current=before; return 0; }
    preview_deadline=0;
    fprintf(stderr,"[PC-DISPLAY] Applied %ux%u\n",m.width,m.height);
    return 1;
}

int pc_display_preview(PcDisplayMode mode)
{
    PcDisplayMode before=pc_display_current();
    if(!pc_display_apply(mode)) return 0;
    preview_previous=before; preview_deadline=GetTickCount64()+15000;
    return 1;
}

int pc_display_save(void)
{
#ifndef _WIN32
    char text[128];init();
    snprintf(text,sizeof text,"TSFP_DISPLAY_V1 %u %u\n",current.width,current.height);
    int ok=tsfp_config_write(path,text,0);
    if(ok)preview_deadline=0;
    return ok;
#else
    char temp[32768]; FILE *f; int ok;
    init();
    if(snprintf(temp,sizeof temp,"%s.tmp",path)<0 || strlen(path)+5>=sizeof temp) return 0;
    f=fopen(temp,"wb"); if(!f) return 0;
    ok=fprintf(f,"TSFP_DISPLAY_V1 %u %u\n",current.width,current.height)>0;
    if(fflush(f)) ok=0;
    if(fclose(f)) ok=0;
    if(ok) ok=MoveFileExA(temp,path,MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)!=0;
    if(!ok) DeleteFileA(temp);
    if(ok) { preview_deadline=0; fprintf(stderr,"[PC-DISPLAY] Saved %ux%u\n",current.width,current.height); }
    return ok;
#endif
}

void pc_display_scene_size(unsigned gw,unsigned gh,unsigned *w,unsigned *h)
{
    init();
    if(w) *w=current.width?current.width:gw;
    if(h) *h=current.height?current.height:gh;
}

void pc_display_shape(unsigned *w,unsigned *h)
{
    init();
    if(w) *w=camera_frame && current.width?current.width:4;
    if(h) *h=camera_frame && current.height?current.height:3;
}

float pc_display_camera_factor(void)
{
    init(); camera_frame=1;
    return current.height?(float)current.width*3.f/((float)current.height*4.f):1.f;
}
float pc_display_hud_scale(void)
{
    return camera_frame && current.width?(4.f*(float)current.height)/(3.f*(float)current.width):1.f;
}
int pc_display_wide_now(void) { return camera_frame && current.width>current.height*4u/3u; }
void pc_display_frame_done(void)
{
    camera_frame=0;
    /* Covers leaving the guest menu through a global pause/Start action too.
       Never persist a preview; a crash or forced exit also reloads the old mode. */
    if(preview_deadline && GetTickCount64()>=preview_deadline && pc_display_apply(preview_previous))
        fprintf(stderr,"[PC-DISPLAY] Preview timed out; previous resolution restored\n");
}
