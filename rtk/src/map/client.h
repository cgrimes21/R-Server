/**
 * @file client.h
 * @brief Main client interface header
 *
 * Central header for client communication layer.
 * Renamed from clif.h as part of the RTK naming refactor.
 */

#ifndef _CLIENT_H_
#define _CLIENT_H_

#include <stdarg.h>
#include "map.h"

/* Include split module headers */
#include "client_crypto.h"
#include "client_chat.h"
#include "client_visual.h"
#include "client_combat.h"
#include "client_inventory.h"
#include "client_npc.h"
#include "client_player.h"

/* Global data */
extern unsigned int groups[MAX_GROUPS][MAX_GROUP_MEMBERS];
extern int val[32];

/* Send type enumerations */
enum { ALL_CLIENT, SAMESRV, SAMEMAP, SAMEMAP_WOS, AREA, AREA_WOS, SAMEAREA, SAMEAREA_WOS, CORNER, SELF };
enum { LOOK_GET, LOOK_SEND };

#define META_MAX 20
extern char meta_file[META_MAX][256];
extern int metamax;

/* Core send functions */
int client_send_sub(struct block_list* bl, va_list ap);
int client_send(unsigned char* buf, int len, struct block_list* bl, int type);
void client_send_timer(USER* sd, char type, unsigned int time);
int client_send_to_gm(unsigned char* buf, int len, struct block_list* bl, int type);

/* Connection/session functions */
int client_accept(int fd);
int client_parse(int fd);
int client_timeout(int fd);
int client_handle_disconnect(USER* sd);
int client_quit(USER* sd);
int client_spawn(USER* sd);

/* Hack detection */
int client_hacker(char* name, const char* reason);
int client_is_registered(unsigned int id);

/* Map/world packets */
int client_send_url(USER* sd, int type, char* url);
int client_send_weather(USER* sd);
int client_send_map_info(USER* sd);
int client_send_map_data(USER* sd, int x1, int y1, int x2, int y2, int type, unsigned short val);
int client_send_time(USER* sd);
int client_send_id(USER* sd);
int client_send_towns(USER* sd);
int client_send_heartbeat(int fd, int type);

/* Character appearance/look packets */
int client_npc_look_sub(struct block_list* bl, va_list ap);
int client_mob_look_sub(struct block_list* bl, va_list ap);
int client_char_look_sub(struct block_list* bl, va_list ap);
int client_object_look_sub(struct block_list* bl, va_list ap);
int client_object_look_sub2(struct block_list* bl, va_list ap);
int client_get_char_area(USER* sd);
int client_get_item_area(USER* sd);
int client_send_char_area(USER* sd);
int client_destroy_old(USER* sd);
int client_speak(struct block_list* bl, va_list ap);
int client_update_state(struct block_list* bl, va_list ap);
void client_send_mobbars(struct block_list* bl, va_list ap);
int client_mob_look_start_func(struct block_list* bl, va_list ap);
int client_mob_look_close_func(struct block_list* bl, va_list ap);
int client_npc_move(struct block_list* bl, va_list ap);
int client_mob_move(struct block_list* bl, va_list ap);

/* Animation packets */
int client_send_animation(struct block_list* bl, va_list ap);
int client_send_animation_xy(struct block_list* bl, va_list ap);

/* GUI text */
int client_gui_text(struct block_list* bl, va_list ap);
int client_gui_text_sd(char* msg, USER* sd);

/* Spell/magic functions */
int client_send_magic(USER* sd, int id);
int client_parse_magic(USER* sd);
int client_remove_spell(USER* sd, int id);

/* Map selection */
int client_map_select(USER* sd, char* title, int* icons, int* colors, char** names, unsigned int* warps, int* warp_x, int* warp_y, int count);

/* Test/debug functions */
int client_send_test(USER* sd);
void client_int_check(int a, int b, int c);
void client_mob_item_update();

/* Parser functions */
int client_parse_emotion(USER* sd);
int client_parse_map(USER* sd);
int client_parse_parcel(USER* sd);
int client_parse_ranking(USER* sd, int type);

/* Clan functions */
int client_parse_clan_bank_withdraw(USER* sd);

/* Acknowledgment */
int client_send_ack(USER* sd);

/* Profile */
int client_retrieve_profile(USER* sd);

/* Powerboard */
int client_send_powerboard(USER* sd);

/* AFK */
int client_cancel_afk(USER* sd);

/* Floor item stacking */
int client_add_to_current(struct block_list* bl, va_list ap);

/* Hunter toggle */
int client_hunter_toggle(USER* sd);

/* Reward info */
int client_send_reward_info(USER* sd, int type);

/* Status update functions */
int client_send_update_status(USER* sd);
int client_send_update_status2(USER* sd);
int client_send_xychange(USER* sd, int dx, int dy);

/* Additional functions */
int client_parseattack(USER* sd);
int client_script_menuseq(USER* sd, int id, char* dialog, char** menu, int size, int previous, int next);
int client_send_minimap(USER* sd);
int client_gui_textsd(char* msg, USER* sd);
int client_parselookat_scriptsub(USER* sd, struct block_list* bl);

/* Backward compatibility macros */
#define encrypt client_encrypt
#define decrypt client_decrypt
#define client_sendtime client_send_time
#define client_sendweather client_send_weather
#define client_sendmapdata client_send_map_data
#define client_sendack client_send_ack
#define client_sendid client_send_id
#define client_sendmapinfo client_send_map_info
#define client_sendmagic client_send_magic
#define client_sendurl client_send_url
#define client_sendpowerboard client_send_powerboard
#define client_sendupdatestatus client_send_update_status
#define client_sendupdatestatus_onequip client_send_update_status2
#define client_send_xy_change client_send_xychange
#define client_parse_attack client_parseattack
#define client_script_menu_seq client_script_menuseq
#define client_sendBoardQuestionaire client_send_board_questionnaire
#define client_sendminimap client_send_minimap
#define client_gui_text_sd client_gui_textsd
#define client_parse_look_at_scriptsub client_parselookat_scriptsub

/* Old clif_ names for backward compatibility */
#define clif_send client_send
#define clif_parse client_parse
#define clif_broadcast client_broadcast
#define clif_encrypt client_encrypt
#define clif_decrypt client_decrypt

#endif /* _CLIENT_H_ */
