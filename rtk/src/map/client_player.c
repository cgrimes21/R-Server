/**
 * @file client_player.c
 * @brief Player status, position, groups, exchange, and movement
 *
 * Extracted from clif.c as part of the RTK refactoring project.
 * Handles all player-specific packets.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

#include "client.h"
#include "client_player.h"
#include "client_chat.h"
#include "client_visual.h"
#include "client_inventory.h"
#include "client_crypto.h"
#include "map.h"
#include "mob.h"
#include "pc.h"
#include "itemdb.h"
#include "magic.h"
#include "sl.h"
#include "socket.h"
#include "malloc.h"
#include "class_db.h"

/* Level/experience functions */

int client_get_level_tnl(USER* sd) {
	int tnl = 0;
	int path = sd->status.class;
	int level = sd->status.level;
	if (path > 5) path = classdb_path(path);

	if (level < 99) tnl = classdb_level(path, level) - sd->status.exp;

	return tnl;
}

float client_get_xp_bar_percent(USER* sd) {
	float percentage;

	int path = sd->status.class;
	int level = sd->status.level;
	int expInLevel = 0;
	int tnl = 0;

	if (path > 5) path = classdb_path(path);

	path = sd->status.class;
	level = sd->status.level;
	if (path > 5) path = classdb_path(path);
	if (level < 99) {
		expInLevel = classdb_level(path, level);
		expInLevel -= classdb_level(path, level - 1);
		tnl = classdb_level(path, level) - (sd->status.exp);
		percentage = (((float)(expInLevel - tnl)) / (expInLevel)) * 100;

		if (!sd->underLevelFlag && sd->status.exp < classdb_level(path, level - 1)) sd->underLevelFlag = sd->status.level;

		if (sd->underLevelFlag != sd->status.level) sd->underLevelFlag = 0;

		if (sd->underLevelFlag) percentage = ((float)sd->status.exp / classdb_level(path, level)) * 100;
	}
	else {
		percentage = ((float)sd->status.exp / 4294967295) * 100;
	}

	return percentage;
}

/* Refresh functions */

int client_refresh(USER* sd) {
	client_send_map_info(sd);
	client_send_xy(sd);
	client_mob_look_start(sd);
	map_foreachinarea(client_object_look_sub, sd->bl.m, sd->bl.x, sd->bl.y, SAMEAREA, BL_ALL, LOOK_GET, sd);
	client_mob_look_close(sd);
	client_destroy_old(sd);
	client_send_char_area(sd);
	client_get_char_area(sd);

	if (!session[sd->fd])
	{
		session[sd->fd]->eof = 8;
		return 0;
	}

	WFIFOHEAD(sd->fd, 5);
	WFIFOB(sd->fd, 0) = 0xAA;
	WFIFOW(sd->fd, 1) = SWAP16(2);
	WFIFOB(sd->fd, 3) = 0x22;
	WFIFOB(sd->fd, 4) = 0x03;
	set_packet_indexes(WFIFOP(sd->fd, 0));
	WFIFOSET(sd->fd, 5 + 3);

	if (!map[sd->bl.m].canGroup) {
		char buff[256];
		sd->status.settingFlags ^= FLAG_GROUP;

		if (sd->status.settingFlags & FLAG_GROUP) {
		}
		else {
			if (sd->group_count > 0) {
				client_leave_group(sd);
			}

			sprintf(buff, "Join a group     :OFF");
			client_send_status(sd, NULL);
			client_send_minitext(sd, buff);
		}
	}

	return 0;
}

int client_refresh_noclick(USER* sd)
{
	client_send_map_info(sd);
	client_send_xy_noclick(sd);
	client_mob_look_start(sd);
	map_foreachinarea(client_object_look_sub, sd->bl.m, sd->bl.x, sd->bl.y, SAMEAREA, BL_ALL, LOOK_GET, sd);
	client_mob_look_close(sd);
	client_destroy_old(sd);
	client_send_char_area(sd);
	client_get_char_area(sd);

	if (!session[sd->fd])
	{
		session[sd->fd]->eof = 8;
		return 0;
	}

	WFIFOHEAD(sd->fd, 5);
	WFIFOB(sd->fd, 0) = 0xAA;
	WFIFOW(sd->fd, 1) = SWAP16(2);
	WFIFOB(sd->fd, 3) = 0x22;
	WFIFOB(sd->fd, 4) = 0x03;
	set_packet_indexes(WFIFOP(sd->fd, 0));
	WFIFOSET(sd->fd, 5 + 3);

	if (!map[sd->bl.m].canGroup) {
		char buff[256];
		sd->status.settingFlags ^= FLAG_GROUP;

		if (sd->status.settingFlags & FLAG_GROUP) {
		}
		else {
			if (sd->group_count > 0) {
				client_leave_group(sd);
			}

			sprintf(buff, "Join a group     :OFF");
			client_send_status(sd, NULL);
			client_send_minitext(sd, buff);
		}
	}

	return 0;
}

/* Status update functions */

int client_send_update_status(USER* sd) {
	if (!session[sd->fd])
	{
		session[sd->fd]->eof = 8;
		return 0;
	}

	WFIFOHEAD(sd->fd, 33);
	WFIFOB(sd->fd, 0) = 0xAA;
	WFIFOB(sd->fd, 1) = 0x00;
	WFIFOB(sd->fd, 2) = 0x1C;
	WFIFOB(sd->fd, 3) = 0x08;
	WFIFOB(sd->fd, 5) = 0x38;
	WFIFOL(sd->fd, 6) = SWAP32(sd->status.hp);
	WFIFOL(sd->fd, 10) = SWAP32(sd->status.mp);
	WFIFOL(sd->fd, 14) = SWAP32(sd->status.exp);
	WFIFOL(sd->fd, 18) = SWAP32(sd->status.money);
	WFIFOL(sd->fd, 22) = 0x00;
	WFIFOB(sd->fd, 26) = 0x00;
	WFIFOB(sd->fd, 27) = 0x00;
	WFIFOB(sd->fd, 28) = sd->blind;
	WFIFOB(sd->fd, 29) = sd->drunk;
	WFIFOB(sd->fd, 30) = 0x00;
	WFIFOB(sd->fd, 31) = 0x73;
	WFIFOB(sd->fd, 32) = 0x35;
	WFIFOSET(sd->fd, client_encrypt(sd->fd));
	return 0;
}

int client_send_update_status2(USER* sd) {
	if (!session[sd->fd])
	{
		session[sd->fd]->eof = 8;
		return 0;
	}

	float percentage = client_get_xp_bar_percent(sd);

	WFIFOHEAD(sd->fd, 25);
	WFIFOB(sd->fd, 0) = 0xAA;
	WFIFOB(sd->fd, 3) = 0x08;
	WFIFOB(sd->fd, 5) = 0x18;
	WFIFOL(sd->fd, 6) = SWAP32(sd->status.exp);
	WFIFOL(sd->fd, 10) = SWAP32(sd->status.money);
	WFIFOB(sd->fd, 14) = (int)percentage;
	WFIFOB(sd->fd, 15) = sd->drunk;
	WFIFOB(sd->fd, 16) = sd->blind;
	WFIFOB(sd->fd, 17) = 0x00;
	WFIFOB(sd->fd, 18) = 0x00;
	WFIFOB(sd->fd, 19) = 0x00;
	WFIFOB(sd->fd, 20) = sd->flags;
	WFIFOB(sd->fd, 21) = 0x01;
	WFIFOL(sd->fd, 22) = SWAP32(sd->status.settingFlags);
	WFIFOSET(sd->fd, client_encrypt(sd->fd));
	return 0;
}

/* Group functions */

int client_is_in_group(USER* sd, USER* tsd) {
	int x;
	for (x = 0; x < sd->group_count; x++) {
		if (groups[sd->groupid][x] == tsd->bl.id) {
			return 1;
		}
	}
	return 0;
}

/* Movement check functions */

int client_can_move_sub(struct block_list* bl, va_list ap) {
	USER* sd = NULL;
	USER* tsd = NULL;
	MOB* mob = NULL;
	int i = 0;

	nullpo_ret(0, sd = va_arg(ap, USER*));

	if (sd->canmove == 1) return 0;

	if (bl->type == BL_PC) {
		tsd = (USER*)bl;

		if (tsd) {
			if ((map[tsd->bl.m].show_ghosts && tsd->status.state == 1 && tsd->bl.id != sd->bl.id && sd->status.state != 1 && !(sd->optFlags & optFlag_ghosts))
				|| (tsd->status.state == -1 || (tsd->status.gm_level && (tsd->optFlags & optFlag_stealth)))) {
				return 0;
			}
		}
	}

	if (bl->type == BL_MOB) {
		mob = (MOB*)bl;

		if (mob->state == MOB_DEAD) {
			return 0;
		}
	}

	if (bl->type == BL_NPC && bl->subtype == 2) {
		return 0;
	}

	if (bl && bl->id != sd->bl.id) {
		sd->canmove = 1;
	}

	return 0;
}

/* Exchange/trade functions */

int client_exchange_add_item_else(USER* sd, USER* tsd, int id) {
	int len = 0;
	char buff[256];
	int i;
	float percentage;
	char nameof[255];
	if (!sd) return 0;
	if (!tsd) return 0;
	sprintf(nameof, "%s", sd->exchange.item[sd->exchange.item_count - 1].real_name);
	sd->exchange.list_count++;
	stringTruncate(nameof, 15);

	if (!session[sd->fd])
	{
		session[sd->fd]->eof = 8;
		return 0;
	}

	WFIFOHEAD(sd->fd, 2000);
	WFIFOB(sd->fd, 0) = 0xAA;
	WFIFOB(sd->fd, 3) = 0x42;
	WFIFOB(sd->fd, 4) = 0x03;
	WFIFOB(sd->fd, 5) = 0x02;
	WFIFOB(sd->fd, 6) = 0x00;
	WFIFOB(sd->fd, 7) = sd->exchange.list_count;
	len = 0;
	i = sd->exchange.item_count;

	sprintf(buff, "%s", nameof);
	WFIFOW(sd->fd, len + 8) = 0xFFFF;
	WFIFOB(sd->fd, len + 10) = 0x00;
	WFIFOB(sd->fd, len + 11) = strlen(buff);
	strcpy(WFIFOP(sd->fd, len + 12), buff);
	len += strlen(buff) + 5;

	WFIFOW(sd->fd, 1) = SWAP16(len + 5);
	WFIFOSET(sd->fd, client_encrypt(sd->fd));
	len = 0;

	if (!session[sd->fd])
	{
		session[sd->fd]->eof = 8;
		return 0;
	}

	WFIFOHEAD(tsd->fd, 2000);
	WFIFOB(tsd->fd, 0) = 0xAA;
	WFIFOB(tsd->fd, 3) = 0x42;
	WFIFOB(tsd->fd, 4) = 0x03;
	WFIFOB(tsd->fd, 5) = 0x02;
	WFIFOB(tsd->fd, 6) = 0x01;
	WFIFOB(tsd->fd, 7) = sd->exchange.list_count;

	sprintf(buff, "%s", nameof);
	WFIFOW(tsd->fd, 8) = 0xFFFF;
	WFIFOB(tsd->fd, 10) = 0;
	WFIFOB(tsd->fd, 11) = strlen(buff);
	strcpy(WFIFOP(tsd->fd, 12), buff);
	len += strlen(buff) + 1;
	WFIFOW(tsd->fd, 1) = SWAP16(len + 8);
	WFIFOSET(tsd->fd, client_encrypt(tsd->fd));

	return 0;
}

int client_exchange_add_item(USER* sd, USER* tsd, int id, int amount) {
	int len = 0;
	char buff[256];
	int i;
	float percentage;
	char nameof[255];
	if (!sd) return 0;
	if (!tsd) return 0;

	if (sd->status.inventory[id].id) {
		if (itemdb_exchangeable(sd->status.inventory[id].id)) {
			client_send_minitext(sd, "You cannot exchange that.");
			return 0;
		}
	}

	if (pc_isinvenspace(tsd, sd->status.inventory[id].id, sd->status.inventory[id].owner, sd->status.inventory[id].real_name, sd->status.inventory[id].customLook, sd->status.inventory[id].customLookColor, sd->status.inventory[id].customIcon, sd->status.inventory[id].customIconColor) >= tsd->status.maxinv) {
		client_send_minitext(sd, "Receiving player does not have enough inventory space.");
		return 0;
	}

	sd->exchange.item[sd->exchange.item_count] = sd->status.inventory[id];
	sd->exchange.item[sd->exchange.item_count].amount = amount;
	sprintf(nameof, "%s", itemdb_name(sd->exchange.item[sd->exchange.item_count].id));
	sd->exchange.list_count++;
	stringTruncate(nameof, 15);

	if (!session[sd->fd])
	{
		session[sd->fd]->eof = 8;
		return 0;
	}

	WFIFOHEAD(sd->fd, 2000);
	WFIFOB(sd->fd, 0) = 0xAA;
	WFIFOB(sd->fd, 3) = 0x42;
	WFIFOB(sd->fd, 4) = 0x03;
	WFIFOB(sd->fd, 5) = 0x02;
	WFIFOB(sd->fd, 6) = 0x00;
	WFIFOB(sd->fd, 7) = sd->exchange.list_count;
	len = 0;
	i = sd->exchange.item_count;

	if (amount > 1) {
		sprintf(buff, "%s(%d)", nameof, amount);
	}
	else {
		sprintf(buff, "%s", nameof);
	}

	if (itemdb_type(sd->exchange.item[i].id) > 2 && itemdb_type(sd->exchange.item[i].id) < 17) {
		percentage = ((float)sd->exchange.item[i].dura / itemdb_dura(sd->exchange.item[i].id)) * 100;
		sprintf(buff, "%s (%d%%)", nameof, (int)percentage);
	}
	else if (itemdb_type(sd->exchange.item[i].id) == ITM_SMOKE) {
		sprintf(buff, "%s [%d %s]", nameof, sd->exchange.item[i].dura, itemdb_text(sd->exchange.item[i].id));
	}
	else if (itemdb_type(sd->exchange.item[i].id) == ITM_BAG) {
		sprintf(buff, "%s [%d]", nameof, sd->exchange.item[i].dura);
	}
	else if (itemdb_type(sd->exchange.item[i].id) == ITM_MAP) {
		sprintf(buff, "[T%d] %s", sd->exchange.item[i].dura, nameof);
	}
	else if (itemdb_type(sd->exchange.item[i].id) == ITM_QUIVER) {
		sprintf(buff, "%s [%d]", nameof, sd->exchange.item[i].dura);
	}

	if (sd->exchange.item[i].customIcon != 0) {
		WFIFOW(sd->fd, len + 8) = SWAP16(sd->exchange.item[i].customIcon + 49152);
		WFIFOB(sd->fd, len + 10) = sd->exchange.item[i].customIconColor;
	}
	else {
		WFIFOW(sd->fd, len + 8) = SWAP16(itemdb_icon(sd->exchange.item[i].id));
		WFIFOB(sd->fd, len + 10) = itemdb_iconcolor(sd->exchange.item[i].id);
	}

	WFIFOB(sd->fd, len + 11) = strlen(buff);
	strcpy(WFIFOP(sd->fd, len + 12), buff);
	len += strlen(buff) + 5;

	WFIFOW(sd->fd, 1) = SWAP16(len + 5);
	WFIFOSET(sd->fd, client_encrypt(sd->fd));
	len = 0;

	if (!session[sd->fd])
	{
		session[sd->fd]->eof = 8;
		return 0;
	}

	WFIFOHEAD(tsd->fd, 2000);
	WFIFOB(tsd->fd, 0) = 0xAA;
	WFIFOB(tsd->fd, 3) = 0x42;
	WFIFOB(tsd->fd, 4) = 0x03;
	WFIFOB(tsd->fd, 5) = 0x02;
	WFIFOB(tsd->fd, 6) = 0x01;
	WFIFOB(tsd->fd, 7) = sd->exchange.list_count;

	if (amount > 1) {
		sprintf(buff, "%s (%d)", nameof, amount);
	}
	else {
		sprintf(buff, "%s", nameof);
	}
	if (itemdb_type(sd->exchange.item[i].id) > 2 && itemdb_type(sd->exchange.item[i].id) < 17) {
		percentage = ((float)sd->exchange.item[i].dura / itemdb_dura(sd->exchange.item[i].id)) * 100;
		sprintf(buff, "%s (%d%%)", nameof, (int)percentage);
	}
	else if (itemdb_type(sd->exchange.item[i].id) == ITM_SMOKE) {
		sprintf(buff, "%s [%d %s]", nameof, sd->exchange.item[i].dura, itemdb_text(sd->exchange.item[i].id));
	}
	else if (itemdb_type(sd->exchange.item[i].id) == ITM_BAG) {
		sprintf(buff, "%s [%d]", nameof, sd->exchange.item[i].dura);
	}
	else if (itemdb_type(sd->exchange.item[i].id) == ITM_MAP) {
		sprintf(buff, "[T%d] %s", sd->exchange.item[i].dura, nameof);
	}
	else if (itemdb_type(sd->exchange.item[i].id) == ITM_QUIVER) {
		sprintf(buff, "%s [%d]", nameof, sd->exchange.item[i].dura);
	}

	if (sd->exchange.item[i].customIcon != 0) {
		WFIFOW(tsd->fd, 8) = SWAP16(sd->exchange.item[i].customIcon + 49152);
		WFIFOB(tsd->fd, 10) = sd->exchange.item[i].customIconColor;
	}
	else {
		WFIFOW(tsd->fd, 8) = SWAP16(itemdb_icon(sd->exchange.item[i].id));
		WFIFOB(tsd->fd, 10) = itemdb_iconcolor(sd->exchange.item[i].id);
	}

	WFIFOB(tsd->fd, 11) = strlen(buff);
	strcpy(WFIFOP(tsd->fd, 12), buff);
	len += strlen(buff) + 1;
	WFIFOW(tsd->fd, 1) = SWAP16(len + 8);
	WFIFOSET(tsd->fd, client_encrypt(tsd->fd));

	sd->exchange.item_count++;
	if (strlen(sd->exchange.item[i].real_name)) {
		client_exchange_add_item_else(sd, tsd, id);
	}
	pc_delitem(sd, id, amount, 9);

	return 0;
}

int client_exchange_money(USER* sd, USER* tsd) {
	if (!sd) return 0;
	if (!tsd) return 0;

	if (!session[sd->fd])
	{
		session[sd->fd]->eof = 8;
		return 0;
	}

	if (!session[tsd->fd])
	{
		session[tsd->fd]->eof = 8;
		return 0;
	}

	WFIFOHEAD(sd->fd, 11);
	WFIFOHEAD(tsd->fd, 11);
	WFIFOB(sd->fd, 0) = 0xAA;
	WFIFOW(sd->fd, 1) = SWAP16(8);
	WFIFOB(sd->fd, 3) = 0x42;
	WFIFOB(sd->fd, 4) = 0x03;
	WFIFOB(sd->fd, 5) = 0x03;
	WFIFOB(sd->fd, 6) = 0x00;
	WFIFOL(sd->fd, 7) = SWAP32(sd->exchange.gold);
	WFIFOSET(sd->fd, client_encrypt(sd->fd));

	WFIFOB(tsd->fd, 0) = 0xAA;
	WFIFOW(tsd->fd, 1) = SWAP16(8);
	WFIFOB(tsd->fd, 3) = 0x42;
	WFIFOB(tsd->fd, 4) = 0x03;
	WFIFOB(tsd->fd, 5) = 0x03;
	WFIFOB(tsd->fd, 6) = 0x01;
	WFIFOL(tsd->fd, 7) = SWAP32(sd->exchange.gold);
	WFIFOSET(tsd->fd, client_encrypt(tsd->fd));

	return 0;
}

int client_exchange_send_ok(USER* sd, USER* tsd) {
	if (tsd->exchange.exchange_done == 1) {
		client_exchange_finalize(sd, tsd);

		client_exchange_message(sd, "You exchanged, and gave away ownership of the items.", 5, 0);
		client_exchange_message(tsd, "You exchanged, and gave away ownership of the items.", 5, 0);

		client_exchange_cleanup(sd);
		client_exchange_cleanup(tsd);
	}
	else {
		sd->exchange.exchange_done = 1;
		client_exchange_message(tsd, "You exchanged, and gave away ownership of the items.", 5, 1);
		client_exchange_message(sd, "You exchanged, and gave away ownership of the items.", 5, 1);
	}

	return 0;
}

int client_exchange_finalize(USER* sd, USER* tsd) {
	int i;
	int id;
	int amount;
	struct item* it;
	struct item* it2;
	CALLOC(it, struct item, 1);
	CALLOC(it2, struct item, 1);
	char escape[255];

	sl_doscript_blargs("characterLog", "exchangeLogWrite", 2, &sd->bl, &tsd->bl);

	for (i = 0; i < sd->exchange.item_count; i++) {
		memcpy(it, &sd->exchange.item[i], sizeof(sd->exchange.item[i]));
		Sql_EscapeString(sql_handle, escape, it->real_name);
		pc_additem(tsd, it);
	}

	tsd->status.money += sd->exchange.gold;
	sd->status.money -= sd->exchange.gold;
	sd->exchange.gold = 0;

	for (i = 0; i < tsd->exchange.item_count; i++) {
		memcpy(it2, &tsd->exchange.item[i], sizeof(sd->exchange.item[i]));
		Sql_EscapeString(sql_handle, escape, it2->real_name);
		pc_additem(sd, it2);
	}

	FREE(it);
	FREE(it2);

	sd->status.money += tsd->exchange.gold;
	tsd->status.money -= tsd->exchange.gold;
	tsd->exchange.gold = 0;

	client_send_status(sd, SFLAG_XPMONEY);
	client_send_status(tsd, SFLAG_XPMONEY);
	return 0;
}

int client_exchange_message(USER* sd, char* message, int type, int extra) {
	int len = 0;
	if (extra > 1) extra = 0;
	nullpo_ret(0, sd);
	len = strlen(message) + 5;

	if (!session[sd->fd])
	{
		session[sd->fd]->eof = 8;
		return 0;
	}

	WFIFOHEAD(sd->fd, strlen(message) + 8);
	WFIFOB(sd->fd, 0) = 0xAA;
	WFIFOB(sd->fd, 3) = 0x42;
	WFIFOB(sd->fd, 4) = 0x03;
	WFIFOB(sd->fd, 5) = type;
	WFIFOB(sd->fd, 6) = extra;
	WFIFOB(sd->fd, 7) = strlen(message);
	strcpy(WFIFOP(sd->fd, 8), message);
	WFIFOW(sd->fd, 1) = SWAP16(len + 3);
	WFIFOSET(sd->fd, client_encrypt(sd->fd));
	return 0;
}

int client_exchange_close(USER* sd) {
	int i;
	int id = 0;
	struct item* it;

	nullpo_ret(0, sd);
	CALLOC(it, struct item, 1);
	sd->exchange.target = 0;

	for (i = 0; i < sd->exchange.item_count; i++) {
		memcpy(it, &sd->exchange.item[i], sizeof(sd->exchange.item[i]));
		pc_additemnolog(sd, it);
	}
	FREE(it);
	client_exchange_cleanup(sd);
	return 0;
}

int client_exchange_cleanup(USER* sd) {
	sd->exchange.exchange_done = 0;
	sd->exchange.gold = 0;
	sd->exchange.item_count = 0;
	return 0;
}

/* Stub functions - these remain in clif.c for now due to dependencies */

int client_send_status(USER* sd, int flags) {
	/* Full implementation remains in clif.c */
	return 0;
}

int client_send_status2(USER* sd) {
	return 0;
}

int client_send_status3(USER* sd) {
	return 0;
}

int client_send_options(USER* sd) {
	return 0;
}

int client_my_status(USER* sd) {
	return 0;
}

int client_change_status(USER* sd, int type) {
	return 0;
}

int client_update_state(struct block_list* bl, va_list ap) {
	/* Full implementation remains in clif.c due to complexity */
	return 0;
}

int client_send_xy(USER* sd) {
	return 0;
}

int client_send_xy_noclick(USER* sd) {
	return 0;
}

int client_send_xy_change(USER* sd, int dx, int dy) {
	return 0;
}

int client_parse_walk(USER* sd) {
	return 0;
}

int client_noparse_walk(USER* sd, char speed) {
	return 0;
}

int client_parse_walk_pong(USER* sd) {
	return 0;
}

int client_parse_side(USER* sd) {
	return 0;
}

int client_parse_change_pos(USER* sd) {
	return 0;
}

int client_can_move(USER* sd, int direct) {
	return 0;
}

int client_object_can_move(int m, int x, int y, int side) {
	return 0;
}

int client_object_can_move_from(int m, int x, int y, int side) {
	return 0;
}

int client_block_movement(USER* sd, int flag) {
	return 0;
}

int client_group_status(USER* sd) {
	return 0;
}

int client_grouphealth_update(USER* sd) {
	return 0;
}

int client_add_group(USER* sd) {
	return 0;
}

int client_update_group(USER* sd, char* message) {
	return 0;
}

int client_leave_group(USER* sd) {
	return 0;
}

int client_group_exp(USER* sd, unsigned int exp) {
	return 0;
}

int client_parse_exchange(USER* sd) {
	return 0;
}

int client_start_exchange(USER* sd, unsigned int target) {
	return 0;
}
