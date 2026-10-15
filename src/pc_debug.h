#ifndef TSFP_PC_DEBUG_H
#define TSFP_PC_DEBUG_H
#include <windows.h>
#include <stddef.h>

#define PC_DEBUG_COMMAND 0x7e21
#define PC_DEBUG_TIMER 0x7e30
typedef struct PcDebugOptions {
    int show_fps, show_frame_time, show_issues, log_performance, show_latency;
} PcDebugOptions;
typedef struct PcDebugSnapshot {
    double fps, shown_fps, frame_ms;
    unsigned long errors, warnings, slow_frames;
    int log_available;
    int network_mode;
    double latency_ms;
    unsigned latency_peers;
    char recent[6144];
} PcDebugSnapshot;

/* Config access shares the controls file's parser and atomic save mechanism. */
void pc_debug_config_read(PcDebugOptions *options);
int pc_debug_config_save(const PcDebugOptions *options, char *error, size_t bytes);
void pc_debug_init(HWND window);
void pc_debug_shutdown(void);
void pc_debug_get(PcDebugOptions *options);
void pc_debug_set(const PcDebugOptions *options);
void pc_debug_toggle_fps(void);
void pc_debug_frame(void);
void pc_debug_sample(double fps, double shown_fps);
/* mode: 0 offline, 1 LAN, 2 Online. Negative RTT means no fresh measurement. */
void pc_debug_latency(int mode, double milliseconds, unsigned peers);
const char *pc_debug_watermark(void);
int pc_debug_overlay(char *text, size_t bytes, const char *cap);
void pc_debug_snapshot(PcDebugSnapshot *snapshot);
/* Log polling and clearing run on the window thread, never the render thread. */
void pc_debug_poll(void);
void pc_debug_clear(void);
int pc_debug_open_logs(HWND owner);
void pc_debug_show(HWND owner);
#endif
