/* Production display model and guest callbacks with isolated settings and RAM.
 * Original menu drawing/input functions are stubs; no game data is needed. */
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
static ULONGLONG clock_ms;
static ULONGLONG ticks(void) { return clock_ms; }
#define GetTickCount64 ticks
#include "../src/pc_display.c"
#include "../src/pc_display_menu.c"
ptrdiff_t g_xbox_mem_offset;
RECOMP_TLS uint32_t g_eax,g_ecx,g_edx,g_esp,g_ebx,g_esi,g_edi,g_ebp,g_seh_ebp;
RECOMP_TLS RecompXmm g_xmm0,g_xmm1,g_xmm2,g_xmm3,g_xmm4,g_xmm5,g_xmm6,g_xmm7;
RECOMP_TLS double g_fp_stack[8];
RECOMP_TLS int g_fp_top,g_fp_cmp,g_df;
RECOMP_TLS uint16_t g_fp_cc,g_fp_control_word;
static int available=1,returned;
#define CHECK(x) do { if(!(x)) { fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#x); exit(1); } } while(0)
HWND xbox_D3D8Window(void) { return NULL; }
int xbox_D3D8ChangeResolution(unsigned w,unsigned h) { (void)w;(void)h;return available; }
int pc_online_connect(void) { return 0; }
void pc_online_disconnect(void) {}
uint32_t xbox_HeapAlloc(uint32_t size,uint32_t alignment) { (void)size;(void)alignment;return 0x100000; }
#define STUB(name) void name(void) { g_esp+=4; }
STUB(sub_00075B70) STUB(sub_00071B10) STUB(sub_00071C50)
STUB(sub_000771D0) STUB(sub_000770E0)
void sub_00078980(void) { build_menu(); }
STUB(sub_0007D480) STUB(sub_000717D0)
void sub_00075430(void) { WORD_AT(WORD_AT(g_esp+4)+0xc8)=WORD_AT(g_esp+8);g_esp+=4; }
STUB(sub_00079710) STUB(sub_00074910) STUB(sub_001A0A60)
static float drawn;
void sub_000814B0(void) { uint32_t b=WORD_AT(g_esp+4);memcpy(&drawn,&b,4);g_esp+=4; }
void sub_00154FE0(void) { g_eax=WORD_AT(g_esp+4);g_esp+=4; }
void sub_00079B70(void) { returned++;g_esp+=4; }
void sub_00079D60_gen(void) { g_eax=0x200000;g_esp+=4; }
static void call(uint32_t address,int selected,unsigned buttons)
{
    g_esp=0x700000;WORD_AT(g_esp)=0;WORD_AT(g_esp+4)=0x300000;
    WORD_AT(0x30018c)=selected;WORD_AT(0x3000c4)=buttons;
    g_esi=0xABC;g_ebx=0xDEF;g_fp_top=3;
    void (*fn)(void)=pc_display_menu_lookup(address); CHECK(fn);fn();
    CHECK(g_esp==0x700004 && g_esi==0xABC && g_ebx==0xDEF && g_fp_top==3);
}
static void reload(const char *text)
{
    FILE *f=fopen(path,"wb");CHECK(f);CHECK(fputs(text,f)>=0);CHECK(!fclose(f));
    initialized=0;current=(PcDisplayMode){0,0};vertical_fov=PC_FOV_DEFAULT;
    pc_display_current();
}
static void content(const char *expected)
{
    FILE *f=fopen(path,"rb");char text[128]={0};CHECK(f);fread(text,1,sizeof text-1,f);fclose(f);
    CHECK(!strcmp(text,expected));
}
int main(int argc,char **argv)
{
    CHECK(argc==2); CHECK(!_putenv_s("TSFP_DISPLAY_CONFIG",argv[1]));
    void *mem=VirtualAlloc(NULL,0x800000,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);CHECK(mem);
    g_xbox_mem_offset=(ptrdiff_t)(uintptr_t)mem;
    pc_display_current();reload("TSFP_DISPLAY_V1 1280 720\n");
    CHECK(pc_display_fov()==55 && pc_display_fov_scale()==1.f);
    CHECK(!pc_display_set_fov(39) && !pc_display_set_fov(101));
    CHECK(pc_display_set_fov(75) && pc_display_save());content("TSFP_DISPLAY_V2 1280 720 75\n");
    initialized=0;vertical_fov=55;CHECK(pc_display_fov()==75);
    CHECK(fabsf(pc_display_fov_scale()-tanf(75*.00872664626f)/tanf(55*.00872664626f))<.00001f);
    reload("TSFP_DISPLAY_V2 1280 720 999\n");CHECK(pc_display_fov()==55 && !current.width);
    reload("TSFP_DISPLAY_V2 1280 720\n");CHECK(pc_display_fov()==55 && !current.width);
    reload("TSFP_DISPLAY_V1 1280 720 75\n");CHECK(pc_display_fov()==55 && !current.width);
    reload("TSFP_DISPLAY_V1 1280 720\n");
    call(PC_DISPLAY_MENU_CALLBACK,0,0);CHECK(staged_fov==55);
    call(BUILD_CB,0,0);
    for(int i=0;i<100;i++)call(TICK_CB,1,0x80000);CHECK(staged_fov==100 && pc_display_fov()==55);
    for(int i=0;i<100;i++)call(TICK_CB,1,0x40000);CHECK(staged_fov==40);
    WORD_AT(0x70000c)=0x400000;
    { float rectangle[4]={0,0,400,20};memcpy((void *)(g_xbox_mem_offset+0x400000),rectangle,sizeof rectangle); }
    call(FOV_DRAW_CB,1,0);CHECK(drawn==0.f);
    staged_fov=100;call(FOV_DRAW_CB,1,0);CHECK(drawn==1.f);
    /* Leaving discards the draft. No write on slider movement. */
    call(BACK_CB,1,0);content("TSFP_DISPLAY_V1 1280 720\n");
    call(PC_DISPLAY_MENU_CALLBACK,0,0);CHECK(staged_fov==55);
    for(int i=0;i<20;i++)call(TICK_CB,1,0x80000);
    call(SELECT_CB,2,0);CHECK(pc_display_fov()==75);content("TSFP_DISPLAY_V2 1280 720 75\n");
    call(SELECT_CB,3,0);CHECK(staged_fov==55 && pc_display_fov()==75);
    /* Combined resolution/FOV preview rolls both back, including global exit. */
    resolution_row=0;staged=(PcDisplayMode){1920,1080};
    call(SELECT_CB,2,0);CHECK(page==1 && pc_display_fov()==55);
    call(BACK_CB,0,0);CHECK(page==0 && current.width==1280 && pc_display_fov()==75);
    resolution_row=0;staged=(PcDisplayMode){1920,1080};staged_fov=80;
    call(SELECT_CB,2,0);clock_ms=16000;pc_display_frame_done();
    CHECK(current.width==1280 && pc_display_fov()==75);content("TSFP_DISPLAY_V2 1280 720 75\n");
    call(TICK_CB,0,0);CHECK(page==0 && staged_fov==75);
    resolution_row=0;staged=(PcDisplayMode){1920,1080};staged_fov=90;
    call(SELECT_CB,2,0);call(SELECT_CB,0,0);
    CHECK(page==0 && current.width==1920 && pc_display_fov()==90);content("TSFP_DISPLAY_V2 1920 1080 90\n");
    resolution_row=0;staged=current;staged_fov=80;CHECK(SetFileAttributesA(path,FILE_ATTRIBUTE_READONLY));
    call(SELECT_CB,2,0);CHECK(pc_display_fov()==90 && page==2);
    CHECK(SetFileAttributesA(path,FILE_ATTRIBUTE_NORMAL));content("TSFP_DISPLAY_V2 1920 1080 90\n");
    CHECK(pc_display_set_fov(55) && pc_display_save());content("TSFP_DISPLAY_V1 1920 1080\n");
    VirtualFree(mem,0,MEM_RELEASE);
    puts("PASS: FOV config, bounds, staging, reset, cancellation, atomic failure, preview rollback, keep and guest ABI");
    return 0;
}
