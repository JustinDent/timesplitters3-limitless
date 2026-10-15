#ifndef TSFP_PC_DISPLAY_MENU_H
#define TSFP_PC_DISPLAY_MENU_H
#include <stdint.h>
#define PC_DISPLAY_MENU_CALLBACK 0x00012008u
void pc_display_menu_button(void (*original)(void));
void pc_display_camera(void (*original)(void));
void (*pc_display_menu_lookup(uint32_t address))(void);
#endif
