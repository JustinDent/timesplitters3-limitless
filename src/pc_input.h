#ifndef TSFP_PC_INPUT_H
#define TSFP_PC_INPUT_H
#include <stdint.h>

typedef struct PcInput {
    unsigned char keys[256];
    int focused, captured;
    double dx, dy;
    uint64_t motion_at;
    int wheel_queue, wheel_key;
    uint64_t wheel_until;
} PcInput;

void pc_input_focus(PcInput *s, int focused);
void pc_input_capture(PcInput *s, int captured);
void pc_input_key(PcInput *s, unsigned key, int down);
void pc_input_motion(PcInput *s, double dx, double dy, uint64_t now);
void pc_input_wheel(PcInput *s, int steps);
int pc_input_down(PcInput *s, unsigned key, uint64_t now);
void pc_input_normalize_movement(int16_t *x, int16_t *y);
int pc_input_look(PcInput *s, uint64_t now, float sensitivity, int invert_y,
                  float fov_scale, float *yaw, float *pitch);
#endif
