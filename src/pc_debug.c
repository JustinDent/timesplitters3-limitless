#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include "pc_debug.h"
#include <shellapi.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <ctype.h>
#include <math.h>

static SRWLOCK debug_lock = SRWLOCK_INIT;
static PcDebugOptions options;
static PcDebugSnapshot stats;
static char history[12][384];
static unsigned history_count, history_next;
static LARGE_INTEGER frequency, previous_frame;
static HWND debug_window;
static HANDLE log_file = INVALID_HANDLE_VALUE;
static WCHAR log_path[1024];
static char partial[1024];
static size_t partial_size;

void pc_debug_get(PcDebugOptions *out)
{
    AcquireSRWLockShared(&debug_lock); *out = options; ReleaseSRWLockShared(&debug_lock);
}

void pc_debug_set(const PcDebugOptions *value)
{
    AcquireSRWLockExclusive(&debug_lock); options = *value; ReleaseSRWLockExclusive(&debug_lock);
}

void pc_debug_toggle_fps(void)
{
    AcquireSRWLockExclusive(&debug_lock); options.show_fps = !options.show_fps; ReleaseSRWLockExclusive(&debug_lock);
}

void pc_debug_init(HWND window)
{
    PcDebugOptions value;
    DWORD length;
    pc_debug_shutdown();
    pc_debug_config_read(&value);
    pc_debug_set(&value);
    QueryPerformanceFrequency(&frequency);
    previous_frame.QuadPart = 0;
    length = GetEnvironmentVariableW(L"RECOMP_LOG_FILE", log_path, (DWORD)(sizeof log_path / sizeof *log_path));
    if (!length || length >= sizeof log_path / sizeof *log_path) log_path[0] = 0;
    debug_window = window;
    if (window) SetTimer(window, PC_DEBUG_TIMER, 500, NULL);
    pc_debug_poll();
}

void pc_debug_shutdown(void)
{
    if (debug_window) KillTimer(debug_window, PC_DEBUG_TIMER);
    debug_window = NULL;
    if (log_file != INVALID_HANDLE_VALUE) CloseHandle(log_file);
    log_file = INVALID_HANDLE_VALUE;
    partial_size = 0;
    AcquireSRWLockExclusive(&debug_lock);
    memset(&stats, 0, sizeof stats);
    history_count = history_next = 0;
    ReleaseSRWLockExclusive(&debug_lock);
}

void pc_debug_frame(void)
{
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    if (previous_frame.QuadPart && frequency.QuadPart &&
        (double)(now.QuadPart - previous_frame.QuadPart) / frequency.QuadPart > .050) {
        AcquireSRWLockExclusive(&debug_lock); stats.slow_frames++; ReleaseSRWLockExclusive(&debug_lock);
    }
    previous_frame = now;
}

void pc_debug_sample(double fps, double shown_fps)
{
    int log;
    double ms;
    if (!isfinite(fps) || !isfinite(shown_fps) || fps <= 0 || shown_fps < 0) return;
    ms = 1000.0 / fps;
    AcquireSRWLockExclusive(&debug_lock);
    stats.fps = fps; stats.shown_fps = shown_fps; stats.frame_ms = ms;
    log = options.log_performance;
    ReleaseSRWLockExclusive(&debug_lock);
    if (log) {
        fprintf(stderr, "[PC-DEBUG] %.1f game fps, %.1f shown fps, %.2f ms average frame interval\n", fps, shown_fps, ms);
        fflush(stderr);
    }
}

int pc_debug_overlay(char *text, size_t bytes, const char *cap)
{
    char fps[48] = "", ms[32] = "", issues[64] = "", cap_text[24] = "";
    int shown;
    AcquireSRWLockShared(&debug_lock);
    shown = options.show_fps || options.show_frame_time || options.show_issues;
    if (options.show_fps) {
        snprintf(cap_text, sizeof cap_text, " cap %s", cap);
        if (!stats.fps) snprintf(fps, sizeof fps, "-- fps  ");
        else if (stats.shown_fps > stats.fps + .1)
            snprintf(fps, sizeof fps, "%.1f/%.1f fps  ", stats.fps, stats.shown_fps);
        else snprintf(fps, sizeof fps, "%.1f fps  ", stats.fps);
    }
    if (options.show_frame_time) {
        if (!stats.fps) snprintf(ms, sizeof ms, "-- ms  ");
        else snprintf(ms, sizeof ms, "%.1f ms  ", stats.frame_ms);
    }
    if (options.show_issues) {
        if (stats.log_available) snprintf(issues, sizeof issues, "E:%lu W:%lu", stats.errors, stats.warnings);
        else snprintf(issues, sizeof issues, "Log unavailable");
    }
    snprintf(text, bytes, "%s%s%s%s", fps, ms, issues, cap_text);
    ReleaseSRWLockShared(&debug_lock);
    return shown;
}

static void record_line(const char *line)
{
    char lower[1024];
    size_t i;
    int severity = 0;
    for (i = 0; line[i] && i < sizeof lower - 1; i++) lower[i] = (char)tolower((unsigned char)line[i]);
    lower[i] = 0;
    /* These are log pattern matches, not an assertion that gameplay is broken. */
    if (strstr(lower, "[pc-debug]")) return;
    if (strstr(lower, "[crash]") || strstr(lower, "[error]") || strstr(lower, "fatal") ||
        strstr(lower, " failed") || strstr(lower, "[icall] unresolved") || strstr(lower, "exception")) severity = 2;
    else if (strstr(lower, "warning") || strstr(lower, "[warn") || strstr(lower, "unsupported") ||
             strstr(lower, "unimplemented") || strstr(lower, "anomaly") || strstr(lower, "unavailable")) severity = 1;
    if (!severity) return;
    AcquireSRWLockExclusive(&debug_lock);
    if (severity == 2) stats.errors++; else stats.warnings++;
    snprintf(history[history_next], sizeof history[history_next], "%s: %.370s", severity == 2 ? "Error" : "Warning", line);
    history_next = (history_next + 1) % 12;
    if (history_count < 12) history_count++;
    ReleaseSRWLockExclusive(&debug_lock);
}

void pc_debug_poll(void)
{
    char buffer[4096];
    DWORD count, budget = 65536, i;
    LARGE_INTEGER size, position, zero = {0};
    int available = 0;
    if (log_file == INVALID_HANDLE_VALUE && log_path[0])
        log_file = CreateFileW(log_path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                               NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (log_file != INVALID_HANDLE_VALUE && GetFileSizeEx(log_file, &size) &&
        SetFilePointerEx(log_file, zero, &position, FILE_CURRENT)) {
        available = 1;
        if (size.QuadPart < position.QuadPart) {
            SetFilePointerEx(log_file, zero, NULL, FILE_BEGIN);
            partial_size = 0;
        }
        /* Bounded work keeps a noisy log from stalling the window's input loop. */
        while (budget) {
            DWORD wanted = budget < sizeof buffer ? budget : (DWORD)sizeof buffer;
            if (!ReadFile(log_file, buffer, wanted, &count, NULL)) { available = 0; break; }
            if (!count) break;
            budget -= count;
            for (i = 0; i < count; i++) {
                if (buffer[i] == '\n') {
                    partial[partial_size] = 0;
                    record_line(partial);
                    partial_size = 0;
                } else if (buffer[i] != '\r' && partial_size < sizeof partial - 1)
                    partial[partial_size++] = buffer[i];
            }
        }
    }
    AcquireSRWLockExclusive(&debug_lock); stats.log_available = available; ReleaseSRWLockExclusive(&debug_lock);
}

void pc_debug_snapshot(PcDebugSnapshot *out)
{
    unsigned i;
    size_t used = 0;
    AcquireSRWLockShared(&debug_lock);
    *out = stats;
    out->recent[0] = 0;
    for (i = 0; i < history_count; i++) {
        unsigned index = (history_next + 12 - history_count + i) % 12;
        used += (size_t)snprintf(out->recent + used, sizeof out->recent - used, "%s\r\n", history[index]);
    }
    ReleaseSRWLockShared(&debug_lock);
}

void pc_debug_clear(void)
{
    LARGE_INTEGER zero = {0};
    if (log_file != INVALID_HANDLE_VALUE) SetFilePointerEx(log_file, zero, NULL, FILE_END);
    partial_size = 0;
    AcquireSRWLockExclusive(&debug_lock);
    stats.errors = stats.warnings = stats.slow_frames = 0;
    history_count = history_next = 0;
    ReleaseSRWLockExclusive(&debug_lock);
}

int pc_debug_open_logs(HWND owner)
{
    WCHAR folder[1024], *slash;
    if (!log_path[0]) return 0;
    wcscpy_s(folder, sizeof folder / sizeof *folder, log_path);
    slash = wcsrchr(folder, L'\\');
    if (!slash) slash = wcsrchr(folder, L'/');
    if (!slash) return 0;
    *slash = 0;
    return (INT_PTR)ShellExecuteW(owner, L"open", folder, NULL, NULL, SW_SHOWNORMAL) > 32;
}
