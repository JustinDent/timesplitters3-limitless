/* Native host primitives required by the Xbox runtime; no game or save files. */
#include "platform/win32_compat.h"
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#define CHECK(test) do { if (!(test)) { \
    fprintf(stderr, "FAIL line %d: %s (error %u)\n", __LINE__, #test, (unsigned)GetLastError()); \
    return 1; } } while (0)

static DWORD WINAPI signal_event(void *event)
{
    SetEvent(event);
    return 0;
}

int main(void)
{
    const DWORD size = 64u * 1024u * 1024u;
    int input_was_open = fcntl(STDIN_FILENO, F_GETFD) >= 0;
    HANDLE mapping = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL,
                                       PAGE_READWRITE, 0, size, NULL);
    CHECK(mapping != NULL);
    CHECK(!input_was_open || fcntl(STDIN_FILENO, F_GETFD) >= 0);

    /* Use normal native addresses. This checks the mapping primitive, not
     * the game's complete relocated Xbox address-space initialization. */
    uint32_t *low = MapViewOfFile(mapping, FILE_MAP_ALL_ACCESS, 0, 0, size);
    CHECK(low != NULL);
    uint32_t *mirror = MapViewOfFile(mapping, FILE_MAP_ALL_ACCESS, 0, 0, size);
    CHECK(mirror != NULL && mirror != low);
    low[0] = 0x58424f58u;
    CHECK(mirror[0] == 0x58424f58u);
    mirror[size / sizeof(uint32_t) - 1] = 0x12345678u;
    CHECK(low[size / sizeof(uint32_t) - 1] == 0x12345678u);

    /* An occupied-address request must fail without destroying either view. */
    CHECK(MapViewOfFileEx(mapping, FILE_MAP_ALL_ACCESS, 0, 0, size, low) == NULL);
    CHECK(low[0] == 0x58424f58u && mirror[0] == 0x58424f58u);
    CHECK(UnmapViewOfFile(mirror));
    CHECK(UnmapViewOfFile(low));
    CHECK(CloseHandle(mapping));

    HANDLE event = CreateEventA(NULL, FALSE, FALSE, NULL);
    CHECK(event != NULL);
    CHECK(WaitForSingleObject(event, 0) == WAIT_TIMEOUT);
    HANDLE thread = CreateThread(NULL, 0, signal_event, event, 0, NULL);
    CHECK(thread != NULL);
    CHECK(WaitForSingleObject(event, 2000) == WAIT_OBJECT_0);
    CHECK(WaitForSingleObject(thread, 2000) == WAIT_OBJECT_0);
    CHECK(CloseHandle(thread));
    CHECK(CloseHandle(event));
    puts("PASS: native 64 MiB mapping, alias coherence, collision safety, threads and events");
    return 0;
}
