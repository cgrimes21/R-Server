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

/* Include sub-module headers */
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
int client_get_char_area(USER* sd);
int client_get_item_area(USER* sd);
int client_send_char_area(USER* sd);
int client_destroy_old(USER* sd);

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

#endif /* _CLIENT_H_ */
