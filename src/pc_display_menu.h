#ifndef TSFP_PC_DISPLAY_MENU_H
#define TSFP_PC_DISPLAY_MENU_H
#include <stdint.h>
#define PC_DISPLAY_MENU_CALLBACK 0x00012008u
void pc_display_menu_finish(void (*original)(void));
void pc_display_menu_button(void (*original)(void));
void pc_display_menu_mouse(void (*original)(void));
void pc_display_menu_mouse_mode(void);
void pc_display_menu_mouse_input(void (*original)(void));
void pc_display_menu_mouse_footer(void (*original)(void));
void pc_display_menu_mouse_dispatch(void (*original)(void));
void pc_display_camera(void (*original)(void));
void pc_display_ui(void (*original)(void),uint32_t tag);
void pc_display_scope(void (*original)(void),uint32_t tag);
int pc_display_scope_mask(void (*original)(void));
void pc_display_aim_primitive(void (*original)(void),unsigned kind);
void pc_display_aim_panel(void (*original)(void),uint32_t tag);
void pc_display_effect_scope(void (*original)(void));
void pc_display_weapon_effect(void (*original)(void),unsigned kind);
void pc_display_footer_background(void (*original)(void));
void pc_display_menu_backdrop(void (*original)(void));
void pc_display_main_menu_font(void (*original)(void),int building);
void pc_display_radar_begin(void);
void pc_display_health(void (*original)(void));
void pc_display_health_side(void);
void pc_display_tutorial(void (*original)(void));
void pc_display_tutorial_wrap(void (*original)(void));
void pc_display_tutorial_text(void (*original)(void));
void pc_display_perspective_projection(void (*original)(void));
void (*pc_display_menu_lookup(uint32_t address))(void);
#endif
