/* Deterministic Windows primary-display enumeration; no display/settings changes. */
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static int present=1, calls;
static BOOL WINAPI devices(LPCSTR unused,DWORD index,PDISPLAY_DEVICEA d,DWORD flags)
{
    (void)unused;(void)flags;
    if(!present || index>1) return FALSE;
    strcpy(d->DeviceName,index?"primary":"secondary");
    d->StateFlags=DISPLAY_DEVICE_ATTACHED_TO_DESKTOP|(index?DISPLAY_DEVICE_PRIMARY_DEVICE:0);
    return TRUE;
}
static BOOL WINAPI settings(LPCSTR device,DWORD index,DEVMODEA *d)
{
    const unsigned sizes[][3]={{1920,1080,32},{640,480,32},{1920,1080,32},
        {800,600,16},{3840,2160,32},{1080,1920,32},{7680,4320,32},{1280,720,32}};
    if(strcmp(device,"primary") || d->dmSize!=sizeof *d || d->dmDriverExtra) abort();
    calls++;
    if(index>=sizeof sizes/sizeof sizes[0]) return FALSE;
    d->dmPelsWidth=sizes[index][0];d->dmPelsHeight=sizes[index][1];d->dmBitsPerPel=sizes[index][2];return TRUE;
}
#define EnumDisplayDevicesA devices
#define EnumDisplaySettingsA settings
#include "../src/pc_display.c"
HWND xbox_D3D8Window(void) { return NULL; }
int xbox_D3D8ChangeResolution(unsigned w,unsigned h) { (void)w;(void)h;return 1; }
#define CHECK(x) do { if(!(x)) { fprintf(stderr,"FAIL %d: %s\n",__LINE__,#x);return 1; } } while(0)
int main(int argc,char **argv)
{
    PcDisplayMode modes[128], custom={3000,1400}; int n;
    CHECK(argc==2);CHECK(!_putenv_s("TSFP_DISPLAY_CONFIG",argv[1]));
    {FILE *f=fopen(argv[1],"wb");CHECK(f);fputs("TSFP_DISPLAY_V1 0 0\n",f);fclose(f);}
    n=pc_display_modes((HWND)123,modes,128);
    CHECK(n>40 && calls==9);CHECK(!modes[0].width && !modes[0].height);
    CHECK(modes[1].width==640 && modes[n-1].width==7680);
    for(int i=0;i<n;i++) {
        CHECK(pc_display_valid(modes[i]));
        for(int j=0;j<i;j++)CHECK(modes[i].width!=modes[j].width || modes[i].height!=modes[j].height);
    }
    CHECK(pc_display_aspect((PcDisplayMode){854,480})==3);
    CHECK(pc_display_aspect((PcDisplayMode){1366,768})==3);
    CHECK(pc_display_aspect((PcDisplayMode){3440,1440})==5);
    CHECK(pc_display_aspect((PcDisplayMode){5120,1440})==6);
    CHECK(pc_display_aspect((PcDisplayMode){1280,1024})==2);
    CHECK(pc_display_aspect(custom)==-1);
    CHECK(pc_display_apply(custom));CHECK(pc_display_modes(NULL,modes,128)==n+1);
    CHECK(modes[n].width==3000 && modes[n].height==1400);
    modes[2].width=12345;CHECK(pc_display_modes(NULL,modes,2)==2);CHECK(modes[2].width==12345);
    CHECK(modes[1].width==3000); /* Capacity reserves the saved mode. */
    present=0;CHECK(pc_display_modes(NULL,modes,128)>40);CHECK(!modes[0].width);
    CHECK(!pc_display_modes(NULL,NULL,8) && !pc_display_modes(NULL,modes,0));
    puts("PASS: shared presets, primary modes, deduplication, aspect groups, bounds, custom preservation, fallback and capacity");
    return 0;
}
