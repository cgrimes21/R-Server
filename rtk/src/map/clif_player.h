/**
 * @file clif_player.h
 * @brief Player status, position, groups, exchange, and movement
 *
 * Extracted from clif.c as part of the RTK refactoring project.
 * Handles all player-specific packets.
 */

#ifndef _CLIF_PLAYER_H_
#define _CLIF_PLAYER_H_

#include <stdarg.h>
#include "map.h"

/* Status packets */
int clif_sendstatus(USER* sd, int flags);
int clif_sendstatus2(USER* sd);
int clif_sendstatus3(USER* sd);
int clif_sendupdatestatus(USER* sd);
int clif_sendupdatestatus2(USER* sd);
int clif_sendoptions(USER* sd);
int clif_mystaytus(USER* sd);
int clif_changestatus(USER* sd, int type);
int clif_updatestate(struct block_list* bl, va_list ap);

/* Position packets */
int clif_sendxy(USER* sd);
int clif_sendxynoclick(USER* sd);
int clif_sendxychange(USER* sd, int dx, int dy);

/* Movement parsing */
int clif_parsewalk(USER* sd);
int clif_noparsewalk(USER* sd, char speed);
int clif_parsewalkpong(USER* sd);
int clif_parseside(USER* sd);
int clif_parsechangepos(USER* sd);

/* Movement checks */
int clif_canmove(USER* sd, int direct);
int clif_canmove_sub(struct block_list* bl, va_list ap);
int clif_object_canmove(int m, int x, int y, int side);
int clif_object_canmove_from(int m, int x, int y, int side);
int clif_blockmovement(USER* sd, int flag);

/* Group functions */
int clif_groupstatus(USER* sd);
int clif_grouphealth_update(USER* sd);
int clif_addgroup(USER* sd);
int clif_updategroup(USER* sd, char* message);
int clif_leavegroup(USER* sd);
int clif_isingroup(USER* sd, USER* tsd);
int clif_groupexp(USER* sd, unsigned int exp);

/* Exchange/trade functions */
int clif_parse_exchange(USER* sd);
int clif_startexchange(USER* sd, unsigned int target);
int clif_exchange_additem(USER* sd, USER* tsd, int id, int amount);
int clif_exchange_additem_else(USER* sd, USER* tsd, int id);
int clif_exchange_money(USER* sd, USER* tsd);
int clif_exchange_sendok(USER* sd, USER* tsd);
int clif_exchange_finalize(USER* sd, USER* tsd);
int clif_exchange_message(USER* sd, char* message, int type, int extra);
int clif_exchange_close(USER* sd);
int clif_exchange_cleanup(USER* sd);

/* Refresh functions */
int clif_refresh(USER* sd);
int clif_refreshnoclick(USER* sd);

/* Level/experience */
int clif_getLevelTNL(USER* sd);
float clif_getXPBarPercent(USER* sd);

#endif /* _CLIF_PLAYER_H_ */
