#include "pc_input.h"
#include "pc_camera.h"
#include "pc_controls.h"
#include "input_bindings.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(test) do { if (!(test)) { fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #test); return 1; } } while (0)
int main(int argc, char **argv)
{
    PcInput s = {0}, a = {0}, b = {0};
    float yaw = 0, pitch = 0, ya = 0, yb = 0, pa = 0, pb = 0;
    XBOX_GAMEPAD pad;
    char replay_file[MAX_PATH];
    FILE *f;
    unsigned char *guest;
    int i;
    LRESULT message_result = 0;
    int16_t mx = 32767, my = 32767;
    CHECK(argc == 3);
    CHECK(pc_controls_message(NULL, WM_MOUSEACTIVATE, 0, 0, &message_result));
    CHECK(message_result == MA_ACTIVATEANDEAT);
    pc_input_normalize_movement(&mx, &my);
    CHECK(mx == my && hypot(mx, my) <= 32767 && mx > 23000);
    mx = 0; my = -32767; pc_input_normalize_movement(&mx, &my);
    CHECK(mx == 0 && my == -32767);
    pc_input_focus(&s, 1); pc_input_capture(&s, 1);
    pc_input_key(&s, 'W', 1); CHECK(pc_input_down(&s, 'W', 0));
    pc_input_focus(&s, 0); CHECK(!pc_input_down(&s, 'W', 0));
    CHECK(!s.captured);
    pc_input_motion(&s, 100, 100, 100); CHECK(!s.dx && !s.dy);
    pc_input_focus(&s, 1); pc_input_capture(&s, 1);
    pc_input_motion(&s, 100, 20, 200);
    CHECK(pc_input_look(&s, 201, .1f, 0, 1, &yaw, &pitch));
    CHECK(fabs(yaw + 10) < .001 && fabs(pitch + 2) < .001);
    CHECK(!pc_input_look(&s, 202, .1f, 0, 1, &yaw, &pitch));
    pc_input_motion(&s, 0, 20, 210);
    CHECK(pc_input_look(&s, 211, .1f, 1, 1, &yaw, &pitch)); CHECK(fabs(pitch) < .001);
    pc_input_motion(&s, 100, 0, 220);
    CHECK(pc_input_look(&s, 221, .1f, 0, .5f, &yaw, &pitch)); CHECK(fabs(yaw + 15) < .001);
    pc_input_motion(&s, 100, 0, 300);
    CHECK(!pc_input_look(&s, 450, .1f, 0, 1, &yaw, &pitch));
    pc_input_motion(&s, 100, 0, 500); pc_input_capture(&s, 0);
    CHECK(!pc_input_look(&s, 501, .1f, 0, 1, &yaw, &pitch));
    pc_input_capture(&s, 1); pc_input_key(&s, 1, 1); pc_input_capture(&s, 0);
    CHECK(!pc_input_down(&s, 1, 600));
    pc_input_capture(&s, 1); pc_input_wheel(&s, 2);
    CHECK(pc_input_down(&s, 0x86, 700)); CHECK(!pc_input_down(&s, 0x86, 761));
    CHECK(pc_input_down(&s, 0x86, 802));
    pc_input_focus(&a, 1); pc_input_capture(&a, 1);
    pc_input_focus(&b, 1); pc_input_capture(&b, 1);
    /* Equal counts must yield equal angles whether delivered as one large
       event or many events and whether gameplay runs at 30 or 120 Hz. */
    pc_input_motion(&a, 400, 100, 1000); CHECK(pc_input_look(&a, 1001, .1f, 0, 1, &ya, &pa));
    for (i = 0; i < 100; i++) {
        pc_input_motion(&b, 4, 1, 1000 + (uint64_t)i * 8);
        CHECK(pc_input_look(&b, 1001 + (uint64_t)i * 8, .1f, 0, 1, &yb, &pb));
    }
    CHECK(fabs(ya - yb) < .001 && fabs(pa - pb) < .001);
    pc_input_motion(&a, 1000000, 1000000, 2000);
    CHECK(pc_input_look(&a, 2001, .1f, 0, 1, &ya, &pa));
    CHECK(ya >= -180 && ya <= 180 && pa == -80);
    pc_input_motion(&a, NAN, 0, 2100); CHECK(!a.dx);

    /* Exercise the actual binding parser, Windows host adapter, guest
       address guards and direct camera update together, without OS injection. */
    snprintf(replay_file, sizeof replay_file, "%s\\pc-test-replay.txt", argv[2]);
    f = fopen(replay_file, "w"); CHECK(f);
    fputs("0 key 87 1\n0 key 68 1\n0 key 82 1\n0 key 1 1\n0 key 2 1\n0 key 17 1\n0 wheel 1 0\n0 mouse 100 20\n", f); fclose(f);
    _putenv_s("TSFP_PC_INPUT_REPLAY", replay_file);
    _putenv_s("RECOMP_INPUT_CONFIG", argv[1]);
    pc_controls_attach(NULL);
    CHECK(recomp_bindings_sample(0, &pad));
    CHECK(pad.sThumbLY > 23000 && pad.sThumbLX == pad.sThumbLY && hypot(pad.sThumbLX, pad.sThumbLY) <= 32767);
    CHECK(pad.bAnalogButtons[XBOX_BUTTON_X] == 255);
    CHECK(pad.bAnalogButtons[XBOX_BUTTON_RTRIGGER] == 255);
    CHECK(pad.bAnalogButtons[XBOX_BUTTON_LTRIGGER] == 0);
    CHECK(pad.wButtons & XBOX_GAMEPAD_LEFT_THUMB);
    CHECK(pad.wButtons & XBOX_GAMEPAD_RIGHT_THUMB);
    CHECK(pad.wButtons & XBOX_GAMEPAD_DPAD_RIGHT);
    CHECK(recomp_bindings_present_mask() == 1);
    guest = VirtualAlloc(NULL, 0x30000, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE); CHECK(guest);
    *(float *)(guest + 0x20384) = 55;
    /* A network client's current player can have a nonzero guest player ID,
       even though its keyboard/controller is on local port 1 (index 0). */
    *(uint32_t *)(guest + 0x10000) = 1;
    pc_controls_apply_mouse(0x10000, 0x20000, (ptrdiff_t)guest);
    CHECK(fabs(*(float *)(guest + 0x10100) + 10) < .001);
    /* Player IDs must not act as camera eligibility or host port checks. */
    for (i = 0; i < 16; ++i) {
        float *camera_yaw, *camera_pitch, fov;
        *(uint32_t *)(guest + 0x10000) = (uint32_t)i;
        CHECK(pc_camera_fields(0x10000, 0x20000, (ptrdiff_t)guest,
                               &camera_yaw, &camera_pitch, &fov));
        CHECK(camera_yaw == (float *)(guest + 0x10100));
        CHECK(camera_pitch == (float *)(guest + 0x10104) && fov == 55);
        CHECK(!pc_camera_fields(0, 0x20000, (ptrdiff_t)guest,
                                &camera_yaw, &camera_pitch, &fov));
        CHECK(!pc_camera_fields(0x10000, 0, (ptrdiff_t)guest,
                                &camera_yaw, &camera_pitch, &fov));
        *(float *)(guest + 0x20384) = NAN;
        CHECK(!pc_camera_fields(0x10000, 0x20000, (ptrdiff_t)guest,
                                &camera_yaw, &camera_pitch, &fov));
        *(float *)(guest + 0x20384) = 55;
    }
    CHECK(fabs(*(float *)(guest + 0x10104) + 2) < .001);
    pc_controls_apply_mouse(0x10000, 0x20000, (ptrdiff_t)guest);
    CHECK(fabs(*(float *)(guest + 0x10100) + 10) < .001);
    VirtualFree(guest, 0, MEM_RELEASE);
    puts("PASS: PC input focus/capture, released buttons, wheel queue, raw-count camera math, pitch limits, FOV scaling, poll independence, WASD/reload/fire bindings and guest camera integration");
    return 0;
}
