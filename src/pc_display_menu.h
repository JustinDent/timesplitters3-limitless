#ifndef TSFP_PC_DISPLAY_MENU_H
#define TSFP_PC_DISPLAY_MENU_H
#include <stdint.h>
#define PC_DISPLAY_MENU_CALLBACK 0x00012008u
void pc_display_menu_button(void (*original)(void));
void pc_network_bots_menu(void (*original)(void));
uint32_t pc_network_bots_revive(unsigned slot);
void pc_network_bots_entity_refresh(uint32_t entity);
#include "pc_network_bots_state.h"
void pc_network_bots_present(uint32_t entity,const PcNetworkBotState *state);
void pc_network_bots_pose_weapons(uint32_t entity);
void pc_network_bots_present_shot(uint32_t entity,const PcNetworkBotShot *shot);
void pc_network_bots_request_hit(uint32_t context);
int pc_network_bots_kill_notice(unsigned victim,unsigned killer);
void pc_display_camera(void (*original)(void));
void pc_display_ui(void (*original)(void),uint32_t tag);
void pc_display_radar_begin(void);
void pc_display_health(void (*original)(void));
void pc_display_health_side(void);
void pc_display_tutorial(void (*original)(void));
void pc_display_tutorial_wrap(void (*original)(void));
void pc_display_tutorial_text(void (*original)(void));
void pc_display_perspective_projection(void (*original)(void));
void (*pc_display_menu_lookup(uint32_t address))(void);
#endif
