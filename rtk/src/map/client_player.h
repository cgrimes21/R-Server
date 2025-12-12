/**
 * @file client_player.h
 * @brief Player status, position, groups, exchange, and movement
 *
 * Extracted from clif.c as part of the RTK refactoring project.
 * Handles all player-specific packets.
 */

#ifndef _CLIENT_PLAYER_H_
#define _CLIENT_PLAYER_H_

#include <stdarg.h>
#include "map.h"

/* Status packets */
int client_send_status(USER* sd, int flags);
int client_send_status2(USER* sd);
int client_send_status3(USER* sd);
int client_send_update_status(USER* sd);
int client_send_update_status2(USER* sd);
int client_send_options(USER* sd);
int client_my_status(USER* sd);
int client_change_status(USER* sd, int type);
int client_update_state(struct block_list* bl, va_list ap);

/* Position packets */
int client_send_xy(USER* sd);
int client_send_xy_noclick(USER* sd);
int client_send_xy_change(USER* sd, int dx, int dy);

/* Movement parsing */
int client_parse_walk(USER* sd);
int client_noparse_walk(USER* sd, char speed);
int client_parse_walk_pong(USER* sd);
int client_parse_side(USER* sd);
int client_parse_change_pos(USER* sd);

/* Movement checks */
int client_can_move(USER* sd, int direct);
int client_can_move_sub(struct block_list* bl, va_list ap);
int client_object_can_move(int m, int x, int y, int side);
int client_object_can_move_from(int m, int x, int y, int side);
int client_block_movement(USER* sd, int flag);

/* Group functions */
int client_group_status(USER* sd);
int client_grouphealth_update(USER* sd);
int client_add_group(USER* sd);
int client_update_group(USER* sd, char* message);
int client_leave_group(USER* sd);
int client_is_in_group(USER* sd, USER* tsd);
int client_group_exp(USER* sd, unsigned int exp);
int client_add_to_killreg(USER* sd, int mobid);

/* Exchange/trade functions */
int client_parse_exchange(USER* sd);
int client_start_exchange(USER* sd, unsigned int target);
int client_exchange_add_item(USER* sd, USER* tsd, int id, int amount);
int client_exchange_add_item_else(USER* sd, USER* tsd, int id);
int client_exchange_money(USER* sd, USER* tsd);
int client_exchange_send_ok(USER* sd, USER* tsd);
int client_exchange_finalize(USER* sd, USER* tsd);
int client_exchange_message(USER* sd, char* message, int type, int extra);
int client_exchange_close(USER* sd);
int client_exchange_cleanup(USER* sd);

/* Refresh functions */
int client_refresh(USER* sd);
int client_refresh_noclick(USER* sd);

/* Level/experience */
int client_get_level_tnl(USER* sd);
float client_get_xp_bar_percent(USER* sd);

#endif /* _CLIENT_PLAYER_H_ */
