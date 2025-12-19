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
#include "lua_core.h"
#include "socket.h"
#include "crypt.h"  /* SWAP16, SWAP32 */
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
			client_send_status(sd, 0);
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
			client_send_status(sd, 0);
			client_send_minitext(sd, buff);
		}
	}

	return 0;
}

/* Status update functions */

int client_send_update_status(USER* sd) {
	if (!session[sd->fd])
	{
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
		return 0;
	}

	if (!session[tsd->fd])
	{
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

/* External functions from client.c */
extern int client_send_update_status_onequip(USER* sd);
extern int client_send_update_status_onkill(USER* sd);
extern int client_send_xynoclick(USER* sd);
extern int client_parsewalk(USER* sd);
extern int client_parsewalkpong(USER* sd);
extern int client_parseside(USER* sd);
extern int client_parsechangepos(USER* sd);

int client_send_status(USER* sd, int flags) {
	/* Call the actual status sending function */
	if (!sd) return 0;
	return client_send_update_status_onequip(sd);
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
	char buf[64];
	USER* sd = NULL;
	USER* src_sd = NULL;
	int len = 0;

	nullpo_ret(0, sd = va_arg(ap, USER*));
	nullpo_ret(0, src_sd = (USER*)bl);

	if (!session[sd->fd] || !session[src_sd->fd]) {
		return 0;
	}

	WFIFOHEAD(src_sd->fd, 512);
	WFIFOB(src_sd->fd, 0) = 0xAA;
	WFIFOB(src_sd->fd, 3) = 0x1D;
	WFIFOL(src_sd->fd, 5) = SWAP32(sd->bl.id);

	if (sd->status.state == 4) {
		WFIFOB(src_sd->fd, 9) = 1;
		WFIFOB(src_sd->fd, 10) = 15;
		WFIFOB(src_sd->fd, 11) = sd->status.state;
		WFIFOW(src_sd->fd, 12) = SWAP16(sd->disguise + 32768);
		WFIFOB(src_sd->fd, 14) = sd->disguise_color;

		sprintf(buf, "%s", sd->status.name);

		WFIFOB(src_sd->fd, 16) = strlen(buf);
		len += strlen(sd->status.name) + 1;
		strcpy(WFIFOP(src_sd->fd, 17), buf);

		WFIFOW(src_sd->fd, 1) = SWAP16(len + 13);
		WFIFOSET(src_sd->fd, client_encrypt(src_sd->fd));
	}
	else {
		WFIFOW(src_sd->fd, 9) = SWAP16(sd->status.sex);

		if ((sd->status.state == 2 || (sd->optFlags & optFlag_stealth)) && sd->bl.id != src_sd->bl.id && (src_sd->status.gm_level || client_is_in_group(src_sd, sd) || (sd->gfx.dye == src_sd->gfx.dye && sd->gfx.dye != 0 && src_sd->gfx.dye != 0))) {
			WFIFOB(src_sd->fd, 11) = 5;
		}
		else {
			WFIFOB(src_sd->fd, 11) = sd->status.state;
		}

		if ((sd->optFlags & optFlag_stealth) && !sd->status.state && !src_sd->status.gm_level) WFIFOB(src_sd->fd, 11) = 2;

		if (sd->status.state == 3) {
			WFIFOW(src_sd->fd, 12) = SWAP16(sd->disguise);
		}
		else {
			WFIFOW(src_sd->fd, 12) = SWAP16(0);
		}

		WFIFOB(src_sd->fd, 14) = sd->speed;
		WFIFOB(src_sd->fd, 15) = 0;
		WFIFOB(src_sd->fd, 16) = sd->status.face;
		WFIFOB(src_sd->fd, 17) = sd->status.hair;
		WFIFOB(src_sd->fd, 18) = sd->status.hair_color;
		WFIFOB(src_sd->fd, 19) = sd->status.face_color;
		WFIFOB(src_sd->fd, 20) = sd->status.skin_color;

		/* armor */
		if (!pc_isequip(sd, EQ_ARMOR)) {
			WFIFOW(src_sd->fd, 21) = SWAP16(sd->status.sex);
		}
		else {
			if (sd->status.equip[EQ_ARMOR].customLook != 0) {
				WFIFOW(src_sd->fd, 21) = SWAP16(sd->status.equip[EQ_ARMOR].customLook);
			}
			else {
				WFIFOW(src_sd->fd, 21) = SWAP16(itemdb_look(pc_isequip(sd, EQ_ARMOR)));
			}

			if (sd->status.armor_color > 0) {
				WFIFOB(src_sd->fd, 23) = sd->status.armor_color;
			}
			else {
				if (sd->status.equip[EQ_ARMOR].customLook != 0) {
					WFIFOB(src_sd->fd, 23) = sd->status.equip[EQ_ARMOR].customLookColor;
				}
				else {
					WFIFOB(src_sd->fd, 23) = itemdb_lookcolor(pc_isequip(sd, EQ_ARMOR));
				}
			}
		}

		/* coat */
		if (pc_isequip(sd, EQ_COAT)) {
			WFIFOW(src_sd->fd, 21) = SWAP16(itemdb_look(pc_isequip(sd, EQ_COAT)));
			if (sd->status.armor_color > 0) {
				WFIFOB(src_sd->fd, 23) = sd->status.armor_color;
			}
			else {
				WFIFOB(src_sd->fd, 23) = itemdb_lookcolor(pc_isequip(sd, EQ_COAT));
			}
		}

		/* weapon */
		if (!pc_isequip(sd, EQ_WEAP)) {
			WFIFOW(src_sd->fd, 24) = 0xFFFF;
			WFIFOB(src_sd->fd, 26) = 0x0;
		}
		else {
			if (sd->status.equip[EQ_WEAP].customLook != 0) {
				WFIFOW(src_sd->fd, 24) = SWAP16(sd->status.equip[EQ_WEAP].customLook);
				WFIFOB(src_sd->fd, 26) = sd->status.equip[EQ_WEAP].customLookColor;
			}
			else {
				WFIFOW(src_sd->fd, 24) = SWAP16(itemdb_look(pc_isequip(sd, EQ_WEAP)));
				WFIFOB(src_sd->fd, 26) = itemdb_lookcolor(pc_isequip(sd, EQ_WEAP));
			}
		}

		/* shield */
		if (!pc_isequip(sd, EQ_SHIELD)) {
			WFIFOW(src_sd->fd, 27) = 0xFFFF;
			WFIFOB(src_sd->fd, 29) = 0;
		}
		else {
			if (sd->status.equip[EQ_SHIELD].customLook != 0) {
				WFIFOW(src_sd->fd, 27) = SWAP16(sd->status.equip[EQ_SHIELD].customLook);
				WFIFOB(src_sd->fd, 29) = sd->status.equip[EQ_SHIELD].customLookColor;
			}
			else {
				WFIFOW(src_sd->fd, 27) = SWAP16(itemdb_look(pc_isequip(sd, EQ_SHIELD)));
				WFIFOB(src_sd->fd, 29) = itemdb_lookcolor(pc_isequip(sd, EQ_SHIELD));
			}
		}

		/* helm */
		if (!pc_isequip(sd, EQ_HELM) || !(sd->status.settingFlags & FLAG_HELM) || (itemdb_look(pc_isequip(sd, EQ_HELM)) == -1)) {
			WFIFOB(src_sd->fd, 30) = 0;
			WFIFOW(src_sd->fd, 31) = 0xFFFF;
		}
		else {
			WFIFOB(src_sd->fd, 30) = 1;
			if (sd->status.equip[EQ_HELM].customLook != 0) {
				WFIFOB(src_sd->fd, 31) = sd->status.equip[EQ_HELM].customLook;
				WFIFOB(src_sd->fd, 32) = sd->status.equip[EQ_HELM].customLookColor;
			}
			else {
				WFIFOB(src_sd->fd, 31) = itemdb_look(pc_isequip(sd, EQ_HELM));
				WFIFOB(src_sd->fd, 32) = itemdb_lookcolor(pc_isequip(sd, EQ_HELM));
			}
		}

		/* faceacc */
		if (!pc_isequip(sd, EQ_FACEACC)) {
			WFIFOW(src_sd->fd, 33) = 0xFFFF;
			WFIFOB(src_sd->fd, 35) = 0x0;
		}
		else {
			WFIFOW(src_sd->fd, 33) = SWAP16(itemdb_look(pc_isequip(sd, EQ_FACEACC)));
			WFIFOB(src_sd->fd, 35) = itemdb_lookcolor(pc_isequip(sd, EQ_FACEACC));
		}

		/* crown */
		if (!pc_isequip(sd, EQ_CROWN)) {
			WFIFOW(src_sd->fd, 36) = 0xFFFF;
			WFIFOB(src_sd->fd, 38) = 0x0;
		}
		else {
			WFIFOB(src_sd->fd, 30) = 0;
			if (sd->status.equip[EQ_CROWN].customLook != 0) {
				WFIFOW(src_sd->fd, 36) = SWAP16(sd->status.equip[EQ_CROWN].customLook);
				WFIFOB(src_sd->fd, 38) = sd->status.equip[EQ_CROWN].customLookColor;
			}
			else {
				WFIFOW(src_sd->fd, 36) = SWAP16(itemdb_look(pc_isequip(sd, EQ_CROWN)));
				WFIFOB(src_sd->fd, 38) = itemdb_lookcolor(pc_isequip(sd, EQ_CROWN));
			}
		}

		/* faceacc two */
		if (!pc_isequip(sd, EQ_FACEACCTWO)) {
			WFIFOW(src_sd->fd, 39) = 0xFFFF;
			WFIFOB(src_sd->fd, 41) = 0x0;
		}
		else {
			WFIFOW(src_sd->fd, 39) = SWAP16(itemdb_look(pc_isequip(sd, EQ_FACEACCTWO)));
			WFIFOB(src_sd->fd, 41) = itemdb_lookcolor(pc_isequip(sd, EQ_FACEACCTWO));
		}

		/* mantle */
		if (!pc_isequip(sd, EQ_MANTLE)) {
			WFIFOW(src_sd->fd, 42) = 0xFFFF;
			WFIFOB(src_sd->fd, 44) = 0xFF;
		}
		else {
			WFIFOW(src_sd->fd, 42) = SWAP16(itemdb_look(pc_isequip(sd, EQ_MANTLE)));
			WFIFOB(src_sd->fd, 44) = itemdb_lookcolor(pc_isequip(sd, EQ_MANTLE));
		}

		/* necklace */
		if (!pc_isequip(sd, EQ_NECKLACE) || !(sd->status.settingFlags & FLAG_NECKLACE) || (itemdb_look(pc_isequip(sd, EQ_NECKLACE)) == -1)) {
			WFIFOW(src_sd->fd, 45) = 0xFFFF;
			WFIFOB(src_sd->fd, 47) = 0x0;
		}
		else {
			WFIFOW(src_sd->fd, 45) = SWAP16(itemdb_look(pc_isequip(sd, EQ_NECKLACE)));
			WFIFOB(src_sd->fd, 47) = itemdb_lookcolor(pc_isequip(sd, EQ_NECKLACE));
		}

		/* boots */
		if (!pc_isequip(sd, EQ_BOOTS)) {
			WFIFOW(src_sd->fd, 48) = SWAP16(sd->status.sex);
			WFIFOB(src_sd->fd, 50) = 0x0;
		}
		else {
			if (sd->status.equip[EQ_BOOTS].customLook != 0) {
				WFIFOW(src_sd->fd, 48) = SWAP16(sd->status.equip[EQ_BOOTS].customLook);
				WFIFOB(src_sd->fd, 50) = sd->status.equip[EQ_BOOTS].customLookColor;
			}
			else {
				WFIFOW(src_sd->fd, 48) = SWAP16(itemdb_look(pc_isequip(sd, EQ_BOOTS)));
				WFIFOB(src_sd->fd, 50) = itemdb_lookcolor(pc_isequip(sd, EQ_BOOTS));
			}
		}

		WFIFOB(src_sd->fd, 51) = 0;
		WFIFOB(src_sd->fd, 52) = 128;
		WFIFOB(src_sd->fd, 53) = 0;

		if (sd->gfx.dye != 0 && src_sd->gfx.dye != 0 && src_sd->gfx.dye != sd->gfx.dye && sd->status.state == 2) {
			WFIFOB(src_sd->fd, 51) = 0;
		}
		else {
			if (sd->gfx.dye) WFIFOB(src_sd->fd, 51) = sd->gfx.titleColor;
			else WFIFOB(src_sd->fd, 51) = 0;
		}

		sprintf(buf, "%s", sd->status.name);
		len = strlen(buf);

		if (src_sd->status.clan == sd->status.clan) {
			if (src_sd->status.clan > 0) {
				if (src_sd->status.id != sd->status.id) {
					WFIFOB(src_sd->fd, 53) = 3;
				}
			}
		}

		if (client_is_in_group(src_sd, sd)) {
			if (sd->status.id != src_sd->status.id) {
				WFIFOB(src_sd->fd, 53) = 2;
			}
		}

		if ((sd->status.state != 5) && (sd->status.state != 2)) {
			WFIFOB(src_sd->fd, 54) = len;
			strcpy(WFIFOP(src_sd->fd, 55), buf);
		}
		else {
			WFIFOB(src_sd->fd, 54) = 0;
			len = 0;
		}

		if ((sd->status.gm_level && sd->gfx.toggle) || sd->clone) {
			WFIFOB(src_sd->fd, 16) = sd->gfx.face;
			WFIFOB(src_sd->fd, 17) = sd->gfx.hair;
			WFIFOB(src_sd->fd, 18) = sd->gfx.chair;
			WFIFOB(src_sd->fd, 19) = sd->gfx.cface;
			WFIFOB(src_sd->fd, 20) = sd->gfx.cskin;
			WFIFOW(src_sd->fd, 21) = SWAP16(sd->gfx.armor);
			if (sd->gfx.dye > 0) {
				WFIFOB(src_sd->fd, 23) = sd->gfx.dye;
			}
			else {
				WFIFOB(src_sd->fd, 23) = sd->gfx.carmor;
			}
			WFIFOW(src_sd->fd, 24) = SWAP16(sd->gfx.weapon);
			WFIFOB(src_sd->fd, 26) = sd->gfx.cweapon;
			WFIFOW(src_sd->fd, 27) = SWAP16(sd->gfx.shield);
			WFIFOB(src_sd->fd, 29) = sd->gfx.cshield;

			if (sd->gfx.helm < 255) {
				WFIFOB(src_sd->fd, 30) = 1;
			}
			else if (sd->gfx.crown < 65535) {
				WFIFOB(src_sd->fd, 30) = 0xFF;
			}
			else {
				WFIFOB(src_sd->fd, 30) = 0;
			}

			WFIFOB(src_sd->fd, 31) = sd->gfx.helm;
			WFIFOB(src_sd->fd, 32) = sd->gfx.chelm;
			WFIFOW(src_sd->fd, 33) = SWAP16(sd->gfx.faceAcc);
			WFIFOB(src_sd->fd, 35) = sd->gfx.cfaceAcc;
			WFIFOW(src_sd->fd, 36) = SWAP16(sd->gfx.crown);
			WFIFOB(src_sd->fd, 38) = sd->gfx.ccrown;
			WFIFOW(src_sd->fd, 39) = SWAP16(sd->gfx.faceAccT);
			WFIFOB(src_sd->fd, 41) = sd->gfx.cfaceAccT;
			WFIFOW(src_sd->fd, 42) = SWAP16(sd->gfx.mantle);
			WFIFOB(src_sd->fd, 44) = sd->gfx.cmantle;
			WFIFOW(src_sd->fd, 45) = SWAP16(sd->gfx.necklace);
			WFIFOB(src_sd->fd, 47) = sd->gfx.cnecklace;
			WFIFOW(src_sd->fd, 48) = SWAP16(sd->gfx.boots);
			WFIFOB(src_sd->fd, 50) = sd->gfx.cboots;

			len = strlen(sd->gfx.name);
			if ((sd->status.state != 2) && (sd->status.state != 5) && strcmpi(sd->gfx.name, "")) {
				WFIFOB(src_sd->fd, 52) = len;
				strcpy(WFIFOP(src_sd->fd, 53), sd->gfx.name);
			}
			else {
				WFIFOB(src_sd->fd, 52) = 0;
				len = 1;
			}
		}

		WFIFOW(src_sd->fd, 1) = SWAP16(len + 55 + 3);
		WFIFOSET(src_sd->fd, client_encrypt(src_sd->fd));
	}

	return 0;
}

int client_send_xy(USER* sd) {
	if (!sd) return 0;
	return client_send_xynoclick(sd);
}

int client_send_xy_noclick(USER* sd) {
	if (!sd) return 0;
	return client_send_xynoclick(sd);
}

int client_send_xy_change(USER* sd, int dx, int dy) {
	return 0;
}

int client_parse_walk(USER* sd) {
	if (!sd) return 0;
	return client_parsewalk(sd);
}

int client_noparse_walk(USER* sd, char speed) {
	int moveblock;
	char nothingnew;
	char flag = 0;
	int dx, dy, xold, yold, c = 0;
	struct warp_list* x = NULL;
	int x0 = 0, y0 = 0, x1 = 0, y1 = 0, direction = 0;
	int xcheck, ycheck;
	unsigned short m = sd->bl.m;
	char* buf = NULL;

	xold = dx = sd->bl.x;
	yold = dy = sd->bl.y;

	if (dx != sd->bl.x) {
		client_block_movement(sd, 0);
		map_moveblock(&sd->bl, sd->bl.x, sd->bl.y);
		client_send_xy(sd);
		client_block_movement(sd, 1);
		return 0;
	}

	if (dy != sd->bl.y) {
		client_block_movement(sd, 0);
		map_moveblock(&sd->bl, sd->bl.x, sd->bl.y);
		client_send_xy(sd);
		client_block_movement(sd, 1);
		return 0;
	}

	if (!map[sd->bl.m].canMount && sd->status.state == 3 && !sd->status.gm_level)
		sl_doscript_blargs("onDismount", NULL, 1, &sd->bl);

	direction = sd->status.side;

	switch (direction) {
	case 0:
		dy--;
		x0 = sd->bl.x - (sd->viewx + 1);
		y0 = dy - (sd->viewy + 1);
		x1 = 19;
		y1 = 1;
		break;
	case 1:
		dx++;
		x0 = dx + (18 - (sd->viewx + 1));
		y0 = sd->bl.y - (sd->viewy + 1);
		x1 = 1;
		y1 = 17;
		break;
	case 2:
		dy++;
		x0 = sd->bl.x - (sd->viewx + 1);
		y0 = dy + (16 - (sd->viewy + 1));
		x1 = 19;
		y1 = 1;
		break;
	case 3:
		dx--;
		x0 = dx - (sd->viewx + 1);
		y0 = sd->bl.y - (sd->viewy + 1);
		x1 = 1;
		y1 = 17;
		break;
	}

	if (dx < 0) dx = 0;
	if (dx >= map[m].xs) dx = map[m].xs - 1;
	if (dy < 0) dy = 0;
	if (dy >= map[m].ys) dy = map[m].ys - 1;
	sd->canmove = 0;

	if (!sd->status.gm_level) {
		map_foreachincell(client_can_move_sub, m, dx, dy, BL_PC, sd);
		map_foreachincell(client_can_move_sub, m, dx, dy, BL_MOB, sd);
		map_foreachincell(client_can_move_sub, m, dx, dy, BL_NPC, sd);
		if (read_pass(m, dx, dy)) sd->canmove = 1;
	}

	if (sd->canmove || sd->paralyzed || sd->sleep != 1.0f || sd->snare) {
		client_block_movement(sd, 0);
		client_send_xy(sd);
		client_block_movement(sd, 1);
		return 0;
	}

	if (dx == sd->bl.x && dy == sd->bl.y)
		return 0;

	if (direction == 0 && (dy <= sd->viewy || ((map[m].ys - 1 - dy) < 7 && sd->viewy > 7))) sd->viewy--;
	if (direction == 1 && ((dx < 8 && sd->viewx < 8) || 16 - (map[m].xs - 1 - dx) <= sd->viewx)) sd->viewx++;
	if (direction == 2 && ((dy < 7 && sd->viewy < 7) || 14 - (map[m].ys - 1 - dy) <= sd->viewy)) sd->viewy++;
	if (direction == 3 && (dx <= sd->viewx || ((map[m].xs - 1 - dx) < 8 && sd->viewx > 8))) sd->viewx--;
	if (sd->viewx < 0) sd->viewx = 0;
	if (sd->viewx > 16) sd->viewx = 16;
	if (sd->viewy < 0) sd->viewy = 0;
	if (sd->viewy > 14) sd->viewy = 14;

	if (sd->status.settingFlags & FLAG_FASTMOVE) {
		sd->status.settingFlags ^= FLAG_FASTMOVE;
		client_send_status(sd, 0);
		flag = 1;
	}

	if (!session[sd->fd]) {
		return 0;
	}

	WFIFOHEAD(sd->fd, 15);
	WFIFOB(sd->fd, 0) = 0xAA;
	WFIFOB(sd->fd, 1) = 0x00;
	WFIFOB(sd->fd, 2) = 0x0C;
	WFIFOB(sd->fd, 3) = 0x26;
	WFIFOB(sd->fd, 4) = 0x03;
	WFIFOB(sd->fd, 5) = direction;
	WFIFOW(sd->fd, 6) = SWAP16(xold);
	WFIFOW(sd->fd, 8) = SWAP16(yold);
	WFIFOW(sd->fd, 10) = SWAP16(sd->viewx);
	WFIFOW(sd->fd, 12) = SWAP16(sd->viewy);
	WFIFOB(sd->fd, 14) = 0x00;
	WFIFOSET(sd->fd, client_encrypt(sd->fd));

	if (flag == 1) {
		sd->status.settingFlags ^= FLAG_FASTMOVE;
		client_send_status(sd, 0);
		flag = 0;
	}

	CALLOC(buf, char, 32);
	WBUFB(buf, 0) = 0xAA;
	WBUFB(buf, 1) = 0x00;
	WBUFB(buf, 2) = 0x0C;
	WBUFB(buf, 3) = 0x0C;
	WBUFL(buf, 5) = SWAP32(sd->status.id);
	WBUFW(buf, 9) = SWAP16(xold);
	WBUFW(buf, 11) = SWAP16(yold);
	WBUFB(buf, 13) = direction;
	WBUFB(buf, 14) = 0x00;

	client_send((unsigned char*)buf, 32, &sd->bl, AREA_WOS);
	FREE(buf);

	map_moveblock(&sd->bl, dx, dy);

	if (x0 >= 0 && y0 >= 0 && x0 + (x1 - 1) < map[m].xs && y0 + (y1 - 1) < map[m].ys) {
		client_send_map_data(sd, m, x0, y0, x1, y1, 0);
		client_mob_look_start(sd);
		map_foreachinblock(client_object_look_sub, m, x0, y0, x0 + (x1 - 1), y0 + (y1 - 1), BL_ALL, LOOK_GET, sd);
		client_mob_look_close(sd);
		map_foreachinblock(client_char_look_sub, m, x0, y0, x0 + (x1 - 1), y0 + (y1 - 1), BL_PC, LOOK_GET, sd);
		map_foreachinblock(client_npc_look_sub, m, x0, y0, x0 + (x1 - 1), y0 + (y1 - 1), BL_NPC, LOOK_GET, sd);
		map_foreachinblock(client_mob_look_sub, m, x0, y0, x0 + (x1 - 1), y0 + (y1 - 1), BL_MOB, LOOK_GET, sd);
		map_foreachinblock(client_char_look_sub, m, x0, y0, x0 + (x1 - 1), y0 + (y1 - 1), BL_PC, LOOK_SEND, sd);
	}

	if (session[sd->fd] && session[sd->fd]->eof) printf("%s eof set on.  19", sd->status.name);

	sl_doscript_blargs("onScriptedTile", NULL, 1, &sd->bl);
	pc_runfloor_sub(sd);

	int fm = 0, fx = 0, fy = 0;
	int zm = 0, zx = 0, zy = 0;
	fm = sd->bl.m;
	fx = sd->bl.x;
	fy = sd->bl.y;
	if (fx >= map[fm].xs) fx = map[fm].xs - 1;
	if (fy >= map[fm].ys) fy = map[fm].ys - 1;
	for (x = map[fm].warp[fx / BLOCK_SIZE + (fy / BLOCK_SIZE) * map[fm].bxs]; x && !c; x = x->next) {
		if (x->x == fx && x->y == fy) {
			zm = x->tm;
			zx = x->tx;
			zy = x->ty;
			c = 1;
		}
	}

	if (zx || zy || zm) {
		if ((sd->status.level < map[zm].reqlvl || (sd->status.basehp < map[zm].reqvita && sd->status.basemp < map[zm].reqmana) || sd->status.mark < map[zm].reqmark || (map[zm].reqpath > 0 && sd->status.class != map[zm].reqpath)) && sd->status.gm_level == 0) {
			client_pushback(sd);

			if (strcmpi(map[zm].maprejectmsg, "") == 0) {
				if (abs(map[zm].reqlvl - sd->status.level) >= 10) { client_send_minitext(sd, "Nightmarish visions of your own death repel you."); }
				else if (abs(map[zm].reqlvl - sd->status.level) >= 5 && map[zm].reqlvl - sd->status.level < 10) { client_send_minitext(sd, "You're not quite ready to enter yet."); }
				else if (abs(map[zm].reqlvl - sd->status.level) < 5) { client_send_minitext(sd, "You almost understand the secrets to this entrance."); }
				else if (sd->status.mark < map[zm].reqmark) { client_send_minitext(sd, "You do not understand the secrets to enter."); }
				else if (map[zm].reqpath > 0 && sd->status.class != map[zm].reqpath) { client_send_minitext(sd, "Your path forbids it."); }
				else {
					client_send_minitext(sd, "A powerful force repels you.");
				}
			}
			else { client_send_minitext(sd, map[zm].maprejectmsg); }

			return 0;
		}
		if ((sd->status.level > map[zm].lvlmax || (sd->status.basehp > map[zm].vitamax && sd->status.basemp > map[zm].manamax)) && sd->status.gm_level == 0) {
			client_pushback(sd);
			client_send_minitext(sd, "A magical barrier prevents you from entering.");
			return 0;
		}

		pc_warp(sd, zm, zx, zy);
	}

	return 1;
}

int client_parse_walk_pong(USER* sd) {
	if (!sd) return 0;
	return client_parsewalkpong(sd);
}

int client_parse_side(USER* sd) {
	if (!sd) return 0;
	return client_parseside(sd);
}

int client_parse_change_pos(USER* sd) {
	if (!sd) return 0;
	return client_parsechangepos(sd);
}

int client_can_move(USER* sd, int direct) {
	return 0;
}

int client_object_can_move(int m, int x, int y, int side) {
	int object = read_obj(m, x, y);
	unsigned char flag = objectFlags[object];

	switch (side) {
	case 0: /* heading NORTH */
		if (flag & OBJ_UP)
			return 1;
		break;
	case 1: /* RIGHT */
		if (flag & OBJ_RIGHT)
			return 1;
		break;
	case 2: /* DOWN */
		if (flag & OBJ_DOWN)
			return 1;
		break;
	case 3: /* LEFT */
		if (flag & OBJ_LEFT)
			return 1;
		break;
	}

	return 0;
}

int client_object_can_move_from(int m, int x, int y, int side) {
	int object = read_obj(m, x, y);
	unsigned char flag = objectFlags[object];

	switch (side) {
	case 0: /* heading NORTH */
		if (flag & OBJ_DOWN)
			return 1;
		break;
	case 1: /* RIGHT */
		if (flag & OBJ_LEFT)
			return 1;
		break;
	case 2: /* DOWN */
		if (flag & OBJ_UP)
			return 1;
		break;
	case 3: /* LEFT */
		if (flag & OBJ_RIGHT)
			return 1;
		break;
	}

	return 0;
}

int client_block_movement(USER* sd, int flag) {
	nullpo_ret(0, sd);

	if (!session[sd->fd]) {
		return 0;
	}

	WFIFOHEAD(sd->fd, 8);
	WFIFOHEADER(sd->fd, 0x51, 5);
	WFIFOB(sd->fd, 5) = flag;
	WFIFOB(sd->fd, 6) = 0;
	WFIFOB(sd->fd, 7) = 0;
	WFIFOSET(sd->fd, client_encrypt(sd->fd));
	return 0;
}

int client_group_status(USER* sd) {
	int x, n, w, r, m, p, a, g;
	int len = 0;
	int y;
	int count;
	char buf[32];
	int rogue[256];
	int warrior[256];
	int mage[256];
	int poet[256];
	int peasant[256];
	int gm[256];

	memset(rogue, 0, sizeof(int) * 256);
	memset(warrior, 0, sizeof(int) * 256);
	memset(mage, 0, sizeof(int) * 256);
	memset(poet, 0, sizeof(int) * 256);
	memset(peasant, 0, sizeof(int) * 256);
	memset(gm, 0, sizeof(int) * 256);

	USER* tsd = NULL;
	count = 0;

	if (!session[sd->fd]) {
		return 0;
	}

	WFIFOHEAD(sd->fd, 65535);
	WFIFOB(sd->fd, 0) = 0xAA;
	WFIFOB(sd->fd, 1) = 0x00;
	WFIFOB(sd->fd, 3) = 0x63;
	WFIFOB(sd->fd, 5) = 2;
	WFIFOB(sd->fd, 6) = sd->group_count;

	for (x = 0, n = 0, w = 0, r = 0, m = 0, p = 0, g = 0; (n + w + r + m + p + g) < sd->group_count; x++) {
		tsd = map_id2sd(groups[sd->groupid][x]);
		if (!tsd)
			continue;

		if (tsd->status.level < 99) {
			tsd->status.maxtnl = classdb_level(tsd->status.class, tsd->status.level);
			tsd->status.maxtnl -= classdb_level(tsd->status.class, tsd->status.level - 1);
			tsd->status.tnl = classdb_level(tsd->status.class, tsd->status.level) - (tsd->status.exp);
			tsd->status.percentage = (((float)(tsd->status.maxtnl - tsd->status.tnl) / tsd->status.maxtnl) * 100 + 0.5) + 0.5;
		}
		else {
			tsd->status.percentage = ((float)(tsd->status.exp / 4294967295) * 100) + 0.5;
		}

		tsd->status.intpercentage = (int)tsd->status.percentage;
		count++;

		switch (classdb_path(tsd->status.class)) {
		case 0:
			peasant[n] = groups[sd->groupid][x];
			n++;
			break;
		case 1:
			warrior[w] = groups[sd->groupid][x];
			w++;
			break;
		case 2:
			rogue[r] = groups[sd->groupid][x];
			r++;
			break;
		case 3:
			mage[m] = groups[sd->groupid][x];
			m++;
			break;
		case 4:
			poet[p] = groups[sd->groupid][x];
			p++;
			break;
		default:
			gm[g] = groups[sd->groupid][x];
			g++;
			break;
		}
	}

	for (x = 0, n = 0, w = 0, r = 0, m = 0, p = 0, g = 0; (n + w + r + m + p + g) < sd->group_count; x++) {
		if (rogue[r] != 0) {
			tsd = map_id2sd(rogue[r]);
			r++;
		}
		else if (warrior[w] != 0) {
			tsd = map_id2sd(warrior[w]);
			w++;
		}
		else if (mage[m] != 0) {
			tsd = map_id2sd(mage[m]);
			m++;
		}
		else if (poet[p] != 0) {
			tsd = map_id2sd(poet[p]);
			p++;
		}
		else if (peasant[n] != 0) {
			tsd = map_id2sd(peasant[n]);
			n++;
		}
		else if (gm[g] != 0) {
			tsd = map_id2sd(gm[g]);
			g++;
		}
		if (!tsd)
			continue;

		sprintf(buf, "%s", tsd->status.name);

		WFIFOL(sd->fd, len + 7) = SWAP32(tsd->bl.id);
		WFIFOB(sd->fd, len + 11) = strlen(buf);
		strcpy(WFIFOP(sd->fd, len + 12), buf);

		len += 11;
		len += strlen(buf) + 1;

		if (sd->group_leader == tsd->status.id) {
			WFIFOB(sd->fd, len) = 1;
		}
		else {
			WFIFOB(sd->fd, len) = 0;
		}

		WFIFOB(sd->fd, len + 1) = tsd->status.state;
		WFIFOB(sd->fd, len + 2) = tsd->status.face;
		WFIFOB(sd->fd, len + 3) = tsd->status.hair;
		WFIFOB(sd->fd, len + 4) = tsd->status.hair_color;
		WFIFOB(sd->fd, len + 5) = 0;

		if (!pc_isequip(tsd, EQ_HELM) || !(tsd->status.settingFlags & FLAG_HELM) || (itemdb_look(pc_isequip(tsd, EQ_HELM)) == -1)) {
			WFIFOB(sd->fd, len + 6) = 0;
			WFIFOW(sd->fd, len + 7) = 0xFFFF;
			WFIFOB(sd->fd, len + 9) = 0;
		}
		else {
			WFIFOB(sd->fd, len + 6) = 1;
			if (tsd->status.equip[EQ_HELM].customLook != 0) {
				WFIFOB(sd->fd, len + 7) = tsd->status.equip[EQ_HELM].customLook;
				WFIFOB(sd->fd, len + 8) = tsd->status.equip[EQ_HELM].customLookColor;
			}
			else {
				WFIFOB(sd->fd, len + 7) = itemdb_look(pc_isequip(tsd, EQ_HELM));
				WFIFOB(sd->fd, len + 8) = itemdb_lookcolor(pc_isequip(tsd, EQ_HELM));
			}
		}

		len += 18;
	}

	WFIFOW(sd->fd, 1) = SWAP16(len + 4);
	WFIFOSET(sd->fd, client_encrypt(sd->fd));

	return 0;
}

int client_grouphealth_update(USER* sd) {
	int x;
	int len;
	char buf[32];
	USER* tsd = NULL;

	for (x = 0; x < sd->group_count; x++) {
		len = 0;
		tsd = map_id2sd(groups[sd->groupid][x]);
		if (!tsd)
			continue;

		if (!session[sd->fd]) {
			return 0;
		}

		WFIFOHEAD(sd->fd, 512);
		WFIFOB(sd->fd, 0) = 0xAA;
		WFIFOB(sd->fd, 3) = 0x63;
		WFIFOB(sd->fd, 4) = 0x03;
		WFIFOB(sd->fd, 5) = 0x03;
		WFIFOL(sd->fd, 6) = SWAP32(tsd->bl.id);

		sprintf(buf, "%s", tsd->status.name);

		WFIFOB(sd->fd, 10) = strlen(buf);
		strcpy(WFIFOP(sd->fd, 11), buf);

		len += 10;
		len += strlen(buf) + 1;

		WFIFOL(sd->fd, len) = SWAP32(tsd->status.hp);
		len += 4;
		WFIFOL(sd->fd, len) = SWAP32(tsd->status.mp);
		len += 4;

		WFIFOW(sd->fd, 1) = SWAP16(len + 3);
		WFIFOSET(sd->fd, client_encrypt(sd->fd));

		client_group_status(sd);
	}

	return 0;
}

int client_add_group(USER* sd) {
	int x;
	char nameof[256];
	USER* tsd = NULL;
	int len = 0;
	char buff[256];
	memset(nameof, 0, 256);
	memcpy(nameof, RFIFOP(sd->fd, 6), RFIFOB(sd->fd, 5));

	nullpo_ret(0, tsd = map_name2sd(nameof));

	if (!sd->status.gm_level && tsd->optFlags & optFlag_stealth) return 0;

	if (tsd->status.id == sd->status.id) {
		client_send_minitext(sd, "You can't group yourself...");
		return 0;
	}

	if (tsd->group_count) {
		if (tsd->group_leader == sd->group_leader && sd->group_leader == sd->bl.id) {
			client_leave_group(tsd);
			return 0;
		}
	}
	if (sd->group_count >= MAX_GROUP_MEMBERS) {
		client_send_minitext(sd, "Your group is already full.");
		return 0;
	}
	if (tsd->status.state == 1) {
		client_send_minitext(sd, "They are unable to join your party.");
		return 0;
	}

	if (!map[sd->bl.m].canGroup) {
		client_send_minitext(sd, "You are unable to join a party. (Grouping disabled on map)");
		return 0;
	}
	if (!map[tsd->bl.m].canGroup) {
		client_send_minitext(sd, "They are unable to join your party. (Grouping disabled on map)");
		return 0;
	}

	if (!(tsd->status.settingFlags & FLAG_GROUP)) {
		client_send_minitext(sd, "They have refused to join your party.");
		return 0;
	}
	if (tsd->group_count != 0) {
		client_send_minitext(sd, "They have refused to join your party.");
		return 0;
	}

	if (sd->group_count == 0) {
		for (x = 1; x < MAX_GROUPS; x++) {
			if (groups[x][0] == 0) {
				break;
			}
		}

		if (x == MAX_GROUPS) {
			client_send_minitext(sd, "All groups are currently occupied, please try again later.");
			return 0;
		}

		groups[x][0] = sd->status.id;
		sd->group_leader = groups[x][0];
		groups[x][1] = tsd->status.id;
		sd->group_count = 2;
		sd->groupid = x;
		tsd->groupid = sd->groupid;
	}
	else {
		groups[sd->groupid][sd->group_count] = tsd->status.id;
		sd->group_count++;
		tsd->groupid = sd->groupid;
	}

	len = sprintf(buff, "%s is joining the group.", tsd->status.name);

	client_update_group(sd, buff);
	client_group_status(sd);

	return 0;
}

int client_update_group(USER* sd, char* message) {
	int x, y;
	int len = 0;
	USER* tsd;

	for (x = 0; x < sd->group_count; x++) {
		tsd = map_id2sd(groups[sd->groupid][x]);

		if (!tsd)
			continue;

		tsd->group_count = sd->group_count;
		tsd->group_leader = sd->group_leader;

		if (tsd->group_count == 1) {
			groups[sd->groupid][0] = 0;
			tsd->group_count = 0;
			tsd->groupid = 0;
		}
		client_send_minitext(tsd, message);
		client_grouphealth_update(tsd);
		client_group_status(tsd);
	}

	return 0;
}

int client_leave_group(USER* sd) {
	int x;
	int taken = 0;
	char buff[256];

	for (x = 0; x < sd->group_count; x++) {
		if (taken == 1) {
			groups[sd->groupid][x - 1] = groups[sd->groupid][x];
		}
		else {
			if (groups[sd->groupid][x] == sd->status.id) {
				groups[sd->groupid][x] = 0;
				taken = 1;
				if (sd->group_leader == sd->status.id) {
					sd->group_leader = groups[sd->groupid][0];
				}
			}
		}
	}

	if (sd->group_leader == 0) {
		sd->group_leader = groups[sd->groupid][0];
	}

	sprintf(buff, "%s is leaving the group.", sd->status.name);
	sd->group_count--;
	client_update_group(sd, buff);
	sprintf(buff, "You have left the group.");
	client_send_minitext(sd, buff);

	sd->group_count = 0;
	sd->groupid = 0;
	client_group_status(sd);

	return 0;
}

int client_group_exp(USER* sd, unsigned int exp) {
	/* Not implemented in original - stub is correct */
	return 0;
}

int client_parse_exchange(USER* sd) {
	/* Exchange parsing handled in main client.c parse loop */
	return 0;
}

int client_start_exchange(USER* sd, unsigned int target) {
	int len = 0;
	char buff[256];
	USER* tsd = map_id2sd(target);
	nullpo_ret(0, sd);

	if (target == sd->bl.id) {
		sprintf(buff, "You move your items from one hand to another, but quickly get bored.");
		client_send_minitext(sd, buff);
		return 0;
	}

	if (!tsd) return 0;

	sd->exchange.target = target;
	tsd->exchange.target = sd->bl.id;

	if (tsd->status.settingFlags & FLAG_EXCHANGE) {
		if (classdb_name(tsd->status.class, tsd->status.mark)) {
			sprintf(buff, "%s(%s)", tsd->status.name, classdb_name(tsd->status.class, tsd->status.mark));
		}
		else {
			sprintf(buff, "%s()", tsd->status.name);
		}

		if (!session[sd->fd]) {
			return 0;
		}

		WFIFOHEAD(sd->fd, 512);
		WFIFOB(sd->fd, 0) = 0xAA;
		WFIFOB(sd->fd, 3) = 0x42;
		WFIFOB(sd->fd, 4) = 0x03;
		WFIFOB(sd->fd, 5) = 0x00;
		WFIFOL(sd->fd, 6) = SWAP32(tsd->bl.id);
		len = 4;
		WFIFOB(sd->fd, len + 6) = strlen(buff);
		strcpy(WFIFOP(sd->fd, len + 7), buff);
		len += strlen(buff) + 1;
		WFIFOW(sd->fd, len + 6) = SWAP16(tsd->status.level);
		len += 2;

		WFIFOW(sd->fd, 1) = SWAP16(len + 3);
		WFIFOSET(sd->fd, client_encrypt(sd->fd));

		if (!session[tsd->fd]) {
			return 0;
		}

		WFIFOHEAD(tsd->fd, 512);

		if (classdb_name(sd->status.class, sd->status.mark)) {
			sprintf(buff, "%s(%s)", sd->status.name, classdb_name(sd->status.class, sd->status.mark));
		}
		else {
			sprintf(buff, "%s()", sd->status.name);
		}

		WFIFOB(tsd->fd, 0) = 0xAA;
		WFIFOB(tsd->fd, 3) = 0x42;
		WFIFOB(tsd->fd, 4) = 0x03;
		WFIFOB(tsd->fd, 5) = 0x00;
		WFIFOL(tsd->fd, 6) = SWAP32(sd->bl.id);
		len = 4;
		WFIFOB(tsd->fd, len + 6) = strlen(buff);
		strcpy(WFIFOP(tsd->fd, len + 7), buff);
		len += strlen(buff) + 1;
		WFIFOW(tsd->fd, len + 6) = SWAP16(sd->status.level);
		len += 2;

		WFIFOW(tsd->fd, 1) = SWAP16(len + 3);
		WFIFOSET(tsd->fd, client_encrypt(tsd->fd));
		sd->status.settingFlags ^= FLAG_EXCHANGE;
		tsd->status.settingFlags ^= FLAG_EXCHANGE;

		sd->exchange.item_count = 0;
		tsd->exchange.item_count = 0;
		sd->exchange.list_count = 0;
		tsd->exchange.list_count = 1;
	}
	else {
		sprintf(buff, "They have refused to exchange with you");
		client_send_minitext(sd, buff);
	}

	return 0;
}
