#include "platform/xbox_winnt.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint8_t memory[0x800000];
static ptrdiff_t g_xbox_mem_offset;
static uint32_t g_esp;
static int64_t counter, deadline;
static unsigned paused_calls, resume_calls, original_calls, frames;
static const char *setting;
const char *recomp_config_lookup(const char *env, const char *key)
{ assert(!strcmp(env,"TSFP_NATIVE_FPS") && !strcmp(key,"native_fps")); return setting; }
static BOOL test_qpf(LARGE_INTEGER *v) { v->QuadPart = 144000000; return TRUE; }
static BOOL test_qpc(LARGE_INTEGER *v) { v->QuadPart = counter; return TRUE; }
static BOOL test_timer(HANDLE a, const LARGE_INTEGER *b, LONG c, void *d, void *e, BOOL f)
{ (void)a;(void)c;(void)d;(void)e;(void)f; deadline = counter + (-b->QuadPart)*144000000/10000000; return TRUE; }
static HANDLE test_create(void) { return (HANDLE)(uintptr_t)1; }
static DWORD test_wait(HANDLE a, DWORD ms)
{ (void)a;(void)ms; counter = deadline > counter ? deadline : counter+1; return WAIT_OBJECT_0; }
static void test_sleep(DWORD ms) { counter += (int64_t)ms*144000; }
#define QueryPerformanceFrequency test_qpf
#define QueryPerformanceCounter test_qpc
#define CreateWaitableTimerExW(a,b,c,d) test_create()
#define CreateWaitableTimerW(a,b,c) test_create()
#define SetWaitableTimer test_timer
#define WaitForSingleObject test_wait
#define Sleep test_sleep
#define WORD(a) (*(uint32_t *)(memory+(a)))
#define FLOAT(a) (*(float *)(memory+(a)))
uint64_t xbox_ReadTimeStampCounter(void) { return (uint64_t)counter*733333333/144000000; }
void sub_000455B0(void) { paused_calls++; g_esp+=4; }
void sub_000455C0(void) { resume_calls++; g_esp+=4; }
void sub_00150190(void) { WORD(0x74914C)+=WORD(0x4E7940); g_esp+=4; }
static void original(void) { original_calls++; g_esp+=4; }
#include "../src/pc_native_timing_guest.inc"
#include "../src/pc_native_numeric.h"
static int test_button;
static void sample_pad(void)
{
    uint32_t record=WORD(g_esp+4),held=WORD(record+12),next=test_button?128:0;
    WORD(record+8)=held;WORD(record+12)=next;
    WORD(record+16)=next&~held;WORD(record+20)=held&~next;
    g_esp+=4;
}
static void input_edges(void)
{
    for(int phase=0;phase<12;phase++) {
        unsigned edges=0,releases=0;
        memset(memory+0x2000,0,64);
        for(int i=0;i<72;i++) {
            /* Every phase of the 144:60 cadence, including zero-tick press
             * frames. A normal held press must arrive once, never disappear. */
            test_button=i>=phase && i<phase+12;
            WORD(0x4E7940)=((i+1)*5/12)-(i*5/12);
            g_esp=0x1000;WORD(g_esp+4)=0x2000;WORD(g_esp+8)=0;
            pc_native_input_guest(sample_pad);
            assert(g_esp==0x1004);
            if(WORD(0x2010)) edges++;
            if(WORD(0x2014)) releases++;
        }
        assert(edges==1 && releases==1 && WORD(0x200C)==0);
    }
    g_tsfp_native_timing_active=0;WORD(0x4E7940)=0;test_button=1;
    g_esp=0x1000;WORD(g_esp+4)=0x2000;WORD(g_esp+8)=0;
    pc_native_input_guest(sample_pad);
    assert(g_esp==0x1004 && WORD(0x2010)==128); /* Original path is ungated. */
    g_tsfp_native_timing_active=1;
}
static void frame(void)
{
    g_esp=0x1000;
    WORD(g_esp)=0x12345678;
    pc_native_timing_guest(original);
    assert(g_esp==0x1004 && WORD(0x1000)==0x12345678);
    assert(WORD(0x563918)==123); /* ISR state was never used as a fake frame counter. */
    frames++;
}
int main(void)
{
    g_xbox_mem_offset=(ptrdiff_t)(uintptr_t)memory;
    WORD(0x563918)=123; WORD(0x4E8CEC)=60;
    setting=NULL; frame(); assert(original_calls==1);
    FLOAT(0x4E794C)=.25f;
    assert(RECOMP_I32_TO_F32(0x4E7944,1)==1);
    assert(RECOMP_I32_TO_F64(0x4E7944,1)==1);
    setting="144oops"; frame(); assert(original_calls==2);
    setting="240"; frame(); assert(original_calls==3);
    setting="144"; frame();
    double distance=0;
    for(unsigned i=0;i<144;i++) { frame(); distance+=FLOAT(0x4E794C); }
    assert(WORD(0x7497B0)==60 && WORD(0x74914C)==60);
    assert(distance>59.999 && distance<60.001);
    assert(WORD(0x7497B8)==145);
    assert(RECOMP_I32_TO_F32(0x4E7944,WORD(0x4E7944))==FLOAT(0x4E794C));
    assert(RECOMP_I32_TO_F64(0x4E7940,WORD(0x4E7940))==FLOAT(0x4E7948));
    assert(RECOMP_I32_TO_F64(0x400000,INT32_MAX)==2147483647.0);
    /* The integer field is still 0/1 for counters, independently from the
     * float conversion used for motion. Guest-local pause scopes win. */
    FLOAT(0x4E794C)=0;
    assert(RECOMP_I32_TO_F32(0x4E7944,1)==0);
    FLOAT(0x4E794C)=.4166667f;
    float saved_delta=pc_native_save_animation_delta();
    FLOAT(0x4E794C)=1; /* Original frozen-pose evaluation restores integer time. */
    pc_native_restore_animation_delta(saved_delta);
    assert(FLOAT(0x4E794C)==.4166667f);
    WORD(0x7497B4)=1;
    for(unsigned i=0;i<144;i++) { frame(); assert(FLOAT(0x4E794C)==0 && WORD(0x4E7944)==0); }
    assert(paused_calls==1 && resume_calls==0);
    assert(WORD(0x7497B0)==120);
    WORD(0x7497B4)=UINT32_MAX; frame();
    assert(resume_calls==1 && WORD(0x7497B4)==0 && FLOAT(0x4E794C)>0);
    assert(original_calls==3 && frames==293);
    input_edges();
    puts("Native guest timing: original fallback, 144 distinct steps, ABI, pause and console clock passed");
    return 0;
}
