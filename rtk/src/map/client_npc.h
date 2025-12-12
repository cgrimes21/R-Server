/**
 * @file client_npc.h
 * @brief NPC dialogs, menus, shops, and interaction packets
 *
 * Extracted from clif.c as part of the RTK refactoring project.
 * Handles all NPC interaction UI.
 */

#ifndef _CLIENT_NPC_H_
#define _CLIENT_NPC_H_

#include <stdarg.h>
#include "map.h"

/* Script messages/dialogs */
int client_script_message(USER* sd, int id, char* msg, int previous, int next);
int client_script_menu(USER* sd, int id, char* dialog, char* menu[], int size);
int client_script_menu_seq(USER* sd, int id, char* dialog, char* menu[], int size, int previous, int next);
int client_parse_menu(USER* sd);
int client_parse_npc_dialog(USER* sd);

/* Buy/sell dialogs */
int client_buy_dialog(USER* sd, unsigned int id, char* dialog, struct item* item, int price[], int count);
int client_parse_buy(USER* sd);
int client_sell_dialog(USER* sd, unsigned int id, char* dialog, int item[], int count);
int client_parse_sell(USER* sd);

/* Input dialogs */
int client_input(USER* sd, int id, char* dialog, char* item);
int client_input_seq(USER* sd, int id, char* dialog, char* dialog2, char* dialog3, char* menu[], int size, int previous, int next);
int client_parse_input(USER* sd);

/* Look at/click functions */
int client_parse_look_at(USER* sd);
int client_parse_look_at_2(USER* sd);
int client_parse_look_at_sub(struct block_list* bl, va_list ap);
int client_parse_look_at_scriptsub(USER* sd, struct block_list* bl);
int client_click_on_player(USER* sd, struct block_list* bl);

/* Hair/face menu */
int client_hair_face_menu(USER* sd, char* dialog, char* menu[], int size);

/* Map message numbers */
int client_map_msgnum(USER* sd, int id);

#endif /* _CLIENT_NPC_H_ */
