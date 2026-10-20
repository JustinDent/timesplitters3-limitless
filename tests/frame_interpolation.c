/* Exercise the actual runtime matrix path, not a second implementation. */
#include "hle/hle_d3d8_interp.c"
#include <assert.h>

int hle_d3d8_interp_rec;
static int title_projection, title_affine, title_count, idle_installed;
static int allow_redraw, presents, prediction, can_predict=1, device_state;
static int begin_ok=1,begin_calls;
static void set_state(const void *arg);
void xbox_D3D8SetInterpRegisters(int p,int a,int n) { title_projection=p;title_affine=a;title_count=n; }
/* Unused device/recorder dependencies; math below runs the real code. */
BOOL xbox_D3D8InterpBegin(void) { assert(allow_redraw);begin_calls++;return begin_ok; }
BOOL xbox_D3D8InterpCanPredict(void) { return can_predict; }
void xbox_D3D8InterpPredict(BOOL on) { prediction=on; }
BOOL xbox_D3D8InterpPresent(void) { presents++; return TRUE; }
void xbox_D3D8InterpEnd(void) { }
void d3d8_vsh_interp_registers(int *p,int *a,int *n) { *p=60;*a=64;*n=12; }
uint32_t d3d8_vsh_bound_reads(int first,int count) { return 1; }
int hle_d3d8_capture_active(void) { return 0; }
int hle_d3d8_defer_on(void) { return 0; }
static float recorded[80 * 4];
void hle_d3d8_interp_snapshot(void) {
    assert(allow_redraw && g_cur==3);
    hle_d3d8_interp_op(set_state,&device_state,sizeof device_state);
}
const float *hle_d3d8_interp_constants(void) { assert(allow_redraw);return recorded; }
void hle_d3d8_overlay_redraw(void) { assert(allow_redraw); }
int hle_d3d8_movie_layer_shown(void) { return 0; }
int hle_d3d8_dump_back_buffer(const char *path) { assert(0);return -1; }
const char *recomp_config_lookup(const char *env,const char *key) { return NULL; }
int recomp_config_int(const char *env,const char *key,int fallback) { return strcmp(key,"frame_interp")==0?2:fallback; }
void xbox_Nv2aFlipGateSetIdle(xbox_FlipGateIdleFn fn) { idle_installed=fn!=NULL; }
void xbox_SetRecompIdle(void (*fn)(void)) { }
const char *xbox_Nv2aFlipGateModeName(void) { return "60"; }
static void set_state(const void *arg) { device_state=*(const int*)arg; }
static void add_state(const void *arg) { device_state+=*(const int*)arg; }
static void predicted_draw(const void *arg) {
    assert(recorded[64*4+3]>12 && recorded[64*4+3]<14);
    device_state=999;
}
static void partial_draw(const void *arg) { assert(!"Guest partial geometry must not be replayed"); }
void d3d8_vsh_set_constant(int first, const float *values, int count)
{
    assert(first >= 0 && first+count <= 80);
    memcpy(recorded + first*4, values, count*4*sizeof(float));
}

int main(void)
{
    float previous[32] = {0}, current[32] = {0};
    InterpDraw draw = {0};
    Prepared prepared = {0};
    int i, cut = 0;
    hle_d3d8_interp_init();
    assert(title_projection==60 && title_affine==64 && title_count==12 && idle_installed);
    g_target=144;assert(previous_list(0)==2 && previous_list(1)==0 && previous_list(2)==1);
    g_target=0;assert(previous_list(0)==1 && previous_list(1)==0);
    /* A prediction must restore every state/clear op before guest drawing. */
    g_cur=0;g_list[0].whole=1;g_list[0].idle_safe=1;hle_d3d8_interp_rec=1;
    int first=7,second=11;
    hle_d3d8_interp_op(set_state,&first,sizeof first);
    hle_d3d8_interp_op(add_state,&second,sizeof second);
    first=second=0;device_state=999;
    restore_prefix(&g_list[0]);assert(device_state==18 && hle_d3d8_interp_rec==1);
    hle_d3d8_interp_unsafe();assert(!g_list[0].idle_safe);
    free(g_list[0].ops);g_list[0].ops=NULL;g_list[0].len=g_list[0].cap=0;
    for (i = 0; i < 4; i++) {
        previous[i*5] = current[i*5] = 1;
        previous[16+i*5] = current[16+i*5] = 1;
    }
    previous[19] = 10; current[19] = 12;
    prepare_group(&prepared, previous+16, current+16, &cut);
    assert(prepared.ok && !cut);
    g_proj=60;g_aff_first=64;g_aff_count=4;g_groups=1;g_nregs=8;
    g_cur=0;g_list[0].regs=current;g_list[1].regs=previous;
    g_list[0].draws=&draw;g_prep=&prepared;
    draw.match=0;draw.key.uses_proj=1;draw.reads_proj=1;
    set_draw_registers(0, .5);
    assert(fabs(recorded[64*4+3]-11) < .00001);
    set_draw_registers(0, 1.25);
    assert(fabs(recorded[64*4+3]-12.5) < .00001);
    set_draw_registers(0, 1.75);
    assert(fabs(recorded[64*4+3]-13.5) < .00001);
    /* Screen-space UI and camera cuts keep the current pose. */
    draw.key.uses_proj=0;set_draw_registers(0, .5);
    assert(recorded[64*4+3]==12);
    draw.key.uses_proj=1;draw.cut=1;set_draw_registers(0, .5);
    assert(recorded[64*4+3]==12);
    current[19]=100;cut=0;
    prepare_group(&prepared, previous+16, current+16, &cut);
    assert(cut && !prepared.ok);
    /* Run the actual CPU callback while the new frame already has geometry.
     * It must draw the completed pose and restore a fresh state-only snapshot,
     * never replay partial guest geometry or alter its recording list. */
    current[19]=12;draw.cut=0;draw.groups=1;draw.key.prims=1;
    InterpDraw older=draw,newer=draw;
    memset(g_list,0,sizeof g_list);g_prep=NULL;g_prep_cap=0;
    g_list[0].regs=previous;g_list[0].draws=&older;g_list[0].ndraws=1;g_list[0].whole=1;
    g_list[1].regs=current;g_list[1].draws=&newer;g_list[1].ndraws=1;
    g_list[1].whole=g_list[1].idle_safe=1;
    push(&g_list[1].ops,&g_list[1].len,&g_list[1].cap,predicted_draw,NULL,0,0,&g_list[1].whole);
    g_list[2].ndraws=1;g_list[2].whole=g_list[2].idle_safe=1;
    push(&g_list[2].ops,&g_list[2].len,&g_list[2].cap,partial_draw,NULL,0,0,&g_list[2].whole);
    LARGE_INTEGER frequency,now;QueryPerformanceFrequency(&frequency);QueryPerformanceCounter(&now);
    g_qpf=frequency.QuadPart;g_last_present=now.QuadPart-g_qpf/120;
    hle_frame_clock_reset(&g_clock,now.QuadPart-g_qpf/144,g_qpf,144);
    g_target=144;g_cur=2;g_render_thread=GetCurrentThreadId();g_redraw_est=0;
    g_idle_checked=0;g_replayed=0;allow_redraw=1;device_state=37;
    guest_idle();
    assert(presents==1 && g_replayed==1 && g_cur==2 && hle_d3d8_interp_rec==1);
    assert(device_state==37 && !prediction && !hle_d3d8_interp_predicting && g_list[2].ndraws==1);
    can_predict=0;guest_idle();assert(presents==1);
    /* Failed target creation must preserve guest state/cadence, use real
       guest frames, throttle retries, and recover without restarting. */
    can_predict=1;begin_ok=0;g_redraw_est=0;g_frame_ok=1;
    QueryPerformanceCounter(&now);g_last_present=now.QuadPart-g_qpf/120;
    hle_frame_clock_reset(&g_clock,now.QuadPart-g_qpf/144,g_qpf,144);
    LONGLONG old_stamp=g_presented_at;int old_calls=begin_calls;
    guest_idle();
    assert(begin_calls==old_calls+1 && presents==1 && g_on && g_redraw_est==0);
    assert(g_retry_after>now.QuadPart && !hle_d3d8_interp_skip_guest());
    assert(g_cur==2 && device_state==37 && hle_d3d8_interp_rec==1);
    assert(!prediction && !hle_d3d8_interp_predicting && !g_idle_busy);
    assert(g_presented_at==old_stamp && g_clock.slot==1);
    assert(!redraw(1) && begin_calls==old_calls+1); /* No allocation during cooldown. */
    g_retry_after=0;begin_ok=1;g_idle_checked=0;g_redraw_est=0;
    QueryPerformanceCounter(&now);g_last_present=now.QuadPart-g_qpf/120;
    hle_frame_clock_reset(&g_clock,now.QuadPart-g_qpf/144,g_qpf,144);
    guest_idle();assert(presents==2 && g_on && g_retry_after==0 && device_state==37);
    /* Gate and boundary paths must also avoid stale present-time arithmetic. */
    begin_ok=0;g_redraw_est=0;g_prepared=g_frame_ok=1;
    QueryPerformanceCounter(&now);g_last_present=now.QuadPart-g_qpf/120;
    hle_frame_clock_reset(&g_clock,now.QuadPart-g_qpf/144,g_qpf,144);
    assert(gate_idle(now.QuadPart,now.QuadPart+g_qpf/120)==0);
    assert(g_on && g_redraw_est==0 && g_clock.slot==1 && presents==2);
    g_retry_after=0;g_frame_ok=1;
    QueryPerformanceCounter(&now);g_last_present=now.QuadPart-g_qpf/120;
    g_release=now.QuadPart+g_qpf/120;
    hle_frame_clock_reset(&g_clock,now.QuadPart-g_qpf/144,g_qpf,144);
    hle_d3d8_interp_finish();
    assert(g_on && g_redraw_est==0 && g_clock.slot==1 && presents==2);
    g_target=0;assert(!redraw(1) && !g_on); /* Existing legacy/native policy. */
    free(g_prep);for(i=0;i<4;i++)free(g_list[i].ops);
    puts("PASS: camera transforms, partial-frame prediction/state restoration, history, 2D hold and cuts");
    return 0;
}
