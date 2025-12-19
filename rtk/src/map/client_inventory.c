/**
 * @file client_inventory.c
 * @brief Inventory, equipment, and item-related packets
 *
 * Extracted from clif.c as part of the RTK refactoring project.
 * Handles all inventory/item visual updates.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "client.h"
#include "client_inventory.h"
#include "client_chat.h"
#include "client_visual.h"
#include "client_player.h"
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
#include "db_mysql.h"

/* Equipment type conversions */

int client_get_slot_from_equip_type(int equipType) {
	int type;

	switch (equipType) {
	case EQ_WEAP:
		type = 0x01;
		break;
	case EQ_ARMOR:
		type = 0x02;
		break;
	case EQ_SHIELD:
		type = 0x03;
		break;
	case EQ_HELM:
		type = 0x04;
		break;
	case EQ_NECKLACE:
		type = 0x06;
		break;
	case EQ_LEFT:
		type = 0x07;
		break;
	case EQ_RIGHT:
		type = 0x08;
		break;
	case EQ_BOOTS:
		type = 13;
		break;
	case EQ_MANTLE:
		type = 14;
		break;
	case EQ_COAT:
		type = 16;
		break;
	case EQ_SUBLEFT:
		type = 20;
		break;
	case EQ_SUBRIGHT:
		type = 21;
		break;
	case EQ_FACEACC:
		type = 22;
		break;
	case EQ_CROWN:
		type = 23;
		break;
	default:
		type = 0;
	}

	return type;
}

int client_get_equip_type(int val) {
	int type = 0;

	switch (val) {
	case EQ_WEAP:
		type = 1;
		break;
	case EQ_ARMOR:
		type = 2;
		break;
	case EQ_SHIELD:
		type = 3;
		break;
	case EQ_HELM:
		type = 4;
		break;
	case EQ_NECKLACE:
		type = 6;
		break;
	case EQ_LEFT:
		type = 7;
		break;
	case EQ_RIGHT:
		type = 8;
		break;
	case EQ_BOOTS:
		type = 13;
		break;
	case EQ_MANTLE:
		type = 14;
		break;
	case EQ_COAT:
		type = 16;
		break;
	case EQ_SUBLEFT:
		type = 20;
		break;
	case EQ_SUBRIGHT:
		type = 21;
		break;
	case EQ_FACEACC:
		type = 22;
		break;
	case EQ_CROWN:
		type = 23;
		break;

	default:
		return 0;
		break;
	}

	return type;
}

/* Item add/remove display */

int client_send_del_item(USER* sd, int num, int type) {
	/*	Type:
		0 = Remove	5 = Shot	10 = Sold
		1 = Drop	6 = Used	11 = Removed
		2 = Eat		7 = Posted	12 = *Item name*
		3 = Smoke	8 = Decayed	13 = Broke
		4 = Throw	9 = Gave
	*/
	sd->status.inventory[num].id = 0;
	sd->status.inventory[num].dura = 0;
	sd->status.inventory[num].protected = 0;
	sd->status.inventory[num].amount = 0;
	sd->status.inventory[num].owner = 0;
	sd->status.inventory[num].custom = 0;
	sd->status.inventory[num].customLook = 0;
	sd->status.inventory[num].customLookColor = 0;
	sd->status.inventory[num].customIcon = 0;
	sd->status.inventory[num].customIconColor = 0;
	memset(sd->status.inventory[num].trapsTable, 0, 100);

	sd->status.inventory[num].time = 0;
	strcpy(sd->status.inventory[num].real_name, "");

	if (!session[sd->fd])
	{
		return 0;
	}

	WFIFOHEAD(sd->fd, 9);
	WFIFOB(sd->fd, 0) = 0xAA;
	WFIFOB(sd->fd, 1) = 0x00;
	WFIFOB(sd->fd, 2) = 0x06;
	WFIFOB(sd->fd, 3) = 0x10;
	WFIFOB(sd->fd, 5) = num + 1;
	WFIFOB(sd->fd, 6) = type;
	WFIFOB(sd->fd, 7) = 0x00;
	WFIFOB(sd->fd, 8) = 0x00;
	WFIFOSET(sd->fd, client_encrypt(sd->fd));

	return 0;
}

int client_send_add_item(USER* sd, int num) {
	char buf[128];
	char buf2[128];
	char* name = NULL;
	char* owner = NULL;
	int namelen = 0;
	int len = 0;
	int id = 0;

	id = sd->status.inventory[num].id;

	if (id < 4) {
		memset(&sd->status.inventory[num], 0, sizeof(sd->status.inventory[num]));
		return 0;
	}

	if (id > 0 && (!strcmpi(itemdb_name(id), "??"))) {
		memset(&sd->status.inventory[num], 0, sizeof(sd->status.inventory[num]));
		return 0;
	}

	if (strlen(sd->status.inventory[num].real_name)) {
		name = sd->status.inventory[num].real_name;
	}
	else {
		name = itemdb_name(id);
	}

	if (sd->status.inventory[num].amount > 1) {
		sprintf(buf, "%s (%d)", name, sd->status.inventory[num].amount);
	}
	else if (itemdb_type(sd->status.inventory[num].id) == ITM_SMOKE) {
		sprintf(buf, "%s [%d %s]", name, sd->status.inventory[num].dura, itemdb_text(sd->status.inventory[num].id));
	}
	else if (itemdb_type(sd->status.inventory[num].id) == ITM_BAG) {
		sprintf(buf, "%s [%d]", name, sd->status.inventory[num].dura);
	}
	else if (itemdb_type(sd->status.inventory[num].id) == ITM_MAP) {
		sprintf(buf, "[T%d] %s", sd->status.inventory[num].dura, name);
	}
	else if (itemdb_type(sd->status.inventory[num].id) == ITM_QUIVER) {
		sprintf(buf, "%s [%d]", name, sd->status.inventory[num].dura);
	}
	else {
		sprintf(buf, "%s", name);
	}

	if (!session[sd->fd])
	{
		return 0;
	}

	WFIFOHEAD(sd->fd, 255);
	WFIFOB(sd->fd, 0) = 0xAA;
	WFIFOB(sd->fd, 3) = 0x0F;
	WFIFOB(sd->fd, 5) = num + 1;

	if (sd->status.inventory[num].customIcon != 0) {
		WFIFOW(sd->fd, 6) = SWAP16(sd->status.inventory[num].customIcon + 49152);
		WFIFOB(sd->fd, 8) = sd->status.inventory[num].customIconColor;
	}
	else {
		WFIFOW(sd->fd, 6) = SWAP16(itemdb_icon(id));
		WFIFOB(sd->fd, 8) = itemdb_iconcolor(id);
	}

	WFIFOB(sd->fd, 9) = strlen(buf);
	memcpy(WFIFOP(sd->fd, 10), buf, strlen(buf));

	len = strlen(buf) + 10;

	WFIFOB(sd->fd, len) = strlen(itemdb_name(id));
	strcpy(WFIFOP(sd->fd, len + 1), itemdb_name(id));
	len += strlen(itemdb_name(id)) + 1;

	WFIFOL(sd->fd, len) = SWAP32(sd->status.inventory[num].amount);
	len += 4;

	if (itemdb_type(id) >= 3 && itemdb_type(id) <= 17) {
		WFIFOB(sd->fd, len) = 0;
		WFIFOL(sd->fd, len + 1) = SWAP32(sd->status.inventory[num].dura);
		WFIFOB(sd->fd, len + 5) = 0;

		if (sd->status.inventory[num].protected >= itemdb_protected(sd->status.inventory[num].id)) WFIFOB(sd->fd, len + 5) = sd->status.inventory[num].protected;
		if (itemdb_protected(sd->status.inventory[num].id) >= sd->status.inventory[num].protected) WFIFOB(sd->fd, len + 5) = itemdb_protected(sd->status.inventory[num].id);

		len += 6;
	}
	else {
		if (itemdb_stackamount(sd->status.inventory[num].id) > 1) WFIFOB(sd->fd, len) = 1;
		else WFIFOB(sd->fd, len) = 0;

		WFIFOL(sd->fd, len + 1) = 0;
		WFIFOB(sd->fd, len + 5) = 0;

		if (sd->status.inventory[num].protected >= itemdb_protected(sd->status.inventory[num].id)) WFIFOB(sd->fd, len + 5) = sd->status.inventory[num].protected;
		if (itemdb_protected(sd->status.inventory[num].id) >= sd->status.inventory[num].protected) WFIFOB(sd->fd, len + 5) = itemdb_protected(sd->status.inventory[num].id);

		len += 6;
	}

	if (sd->status.inventory[num].owner) {
		owner = map_id2name(sd->status.inventory[num].owner);
		WFIFOB(sd->fd, len) = strlen(owner);
		strcpy(WFIFOP(sd->fd, len + 1), owner);
		len += strlen(owner) + 1;
		FREE(owner);
	}
	else {
		WFIFOB(sd->fd, len) = 0;
		len += 1;
	}
	WFIFOW(sd->fd, len) = 0x00;
	len += 2;
	WFIFOB(sd->fd, len) = 0x00;
	len += 1;

	WFIFOW(sd->fd, 1) = SWAP16(len);
	WFIFOSET(sd->fd, client_encrypt(sd->fd));

	return 0;
}

/* Equipment */

int client_equip_item(USER* sd, int id) {
	int len = 0;
	char* nameof = NULL;

	if (strlen(sd->status.equip[id].real_name)) {
		nameof = sd->status.equip[id].real_name;
	}
	else {
		nameof = itemdb_name(sd->status.equip[id].id);
	}

	if (!session[sd->fd])
	{
		return 0;
	}

	WFIFOHEAD(sd->fd, 255);
	WFIFOB(sd->fd, 5) = client_get_equip_type(id);

	if (sd->status.equip[id].customIcon != 0) {
		WFIFOW(sd->fd, 6) = SWAP16(sd->status.equip[id].customIcon + 49152);
		WFIFOB(sd->fd, 8) = sd->status.equip[id].customIconColor;
	}
	else {
		WFIFOW(sd->fd, 6) = SWAP16(itemdb_icon(sd->status.equip[id].id));
		WFIFOB(sd->fd, 8) = itemdb_iconcolor(sd->status.equip[id].id);
	}

	WFIFOB(sd->fd, 9) = strlen(nameof);
	strcpy(WFIFOP(sd->fd, 10), nameof);
	len += strlen(nameof) + 1;

	WFIFOB(sd->fd, len + 9) = strlen(itemdb_name(sd->status.equip[id].id));
	strcpy(WFIFOP(sd->fd, len + 10), itemdb_name(sd->status.equip[id].id));
	len += strlen(itemdb_name(sd->status.equip[id].id)) + 1;

	WFIFOL(sd->fd, len + 9) = SWAP32(sd->status.equip[id].dura);
	len += 4;
	WFIFOW(sd->fd, len + 9) = 0x0000;
	len += 2;
	WFIFOHEADER(sd->fd, 0x37, len + 6);
	WFIFOSET(sd->fd, client_encrypt(sd->fd));
	return 0;
}

int client_send_equip(USER* sd, int id) {
	char buff[256];
	char* name = NULL;
	int msgnum = 0;

	switch (id) {
	case EQ_HELM:
		msgnum = MAP_EQHELM;
		break;
	case EQ_WEAP:
		msgnum = MAP_EQWEAP;
		break;
	case EQ_ARMOR:
		msgnum = MAP_EQARMOR;
		break;
	case EQ_SHIELD:
		msgnum = MAP_EQSHIELD;
		break;
	case EQ_RIGHT:
		msgnum = MAP_EQRIGHT;
		break;
	case EQ_LEFT:
		msgnum = MAP_EQLEFT;
		break;
	case EQ_SUBLEFT:
		msgnum = MAP_EQSUBLEFT;
		break;
	case EQ_SUBRIGHT:
		msgnum = MAP_EQSUBRIGHT;
		break;
	case EQ_FACEACC:
		msgnum = MAP_EQFACEACC;
		break;
	case EQ_CROWN:
		msgnum = MAP_EQCROWN;
		break;
	case EQ_BOOTS:
		msgnum = MAP_EQBOOTS;
		break;
	case EQ_MANTLE:
		msgnum = MAP_EQMANTLE;
		break;
	case EQ_COAT:
		msgnum = MAP_EQCOAT;
		break;
	case EQ_NECKLACE:
		msgnum = MAP_EQNECKLACE;
		break;

	default:
		return -1;
		break;
	}

	if (sd->status.equip[id].id > 0 && !strcmpi(itemdb_name(sd->status.equip[id].id), "??")) {
		memset(&sd->status.equip[id], 0, sizeof(sd->status.equip[id]));
		return 0;
	}

	if (strlen(sd->status.equip[id].real_name)) {
		name = sd->status.equip[id].real_name;
	}
	else {
		name = itemdb_name(sd->status.equip[id].id);
	}

	sprintf(buff, map_msg[msgnum].message, name);
	client_equip_item(sd, id);
	client_send_minitext(sd, buff);
	return 0;
}

int client_unequip_item(USER* sd, int spot) {
	if (!session[sd->fd])
	{
		return 0;
	}

	WFIFOHEAD(sd->fd, 7);
	WFIFOB(sd->fd, 0) = 0xAA;
	WFIFOW(sd->fd, 1) = SWAP16(4);
	WFIFOB(sd->fd, 3) = 0x38;
	WFIFOB(sd->fd, 4) = 0x03;
	WFIFOB(sd->fd, 5) = spot;
	WFIFOB(sd->fd, 6) = 0x00;
	WFIFOSET(sd->fd, client_encrypt(sd->fd));
	return 0;
}

/* Parse item operations */

int client_parse_drop_item(USER* sd) {
	char RegStr[] = "goldbardupe";
	int DupeTimes = pc_readglobalreg(sd, RegStr);
	if (DupeTimes)
	{
		return 0;
	}

	if (sd->status.gm_level == 0) {
		if (sd->status.state == 3) {
			client_send_minitext(sd, "You cannot do that while riding a mount.");
			return 0;
		}
		if (sd->status.state == 1) {
			client_send_minitext(sd, "Spirits can't do that.");
			return 0;
		}
	}

	sd->fakeDrop = 0;

	int id = RFIFOB(sd->fd, 5) - 1;
	int all = RFIFOB(sd->fd, 6);
	if (id >= sd->status.maxinv) return 0;
	if (sd->status.inventory[id].id) {
		if (itemdb_droppable(sd->status.inventory[id].id)) {
			client_send_minitext(sd, "You can't drop this item.");
			return 0;
		}
	}

	client_send_action(&sd->bl, 5, 20, 0);

	sd->invslot = id;

	sl_doscript_blargs(itemdb_yname(sd->status.inventory[id].id), "on_drop", 1, &sd->bl);

	for (int x = 0; x < MAX_MAGIC_TIMERS; x++) {
		if (sd->status.dura_aether[x].id > 0 && sd->status.dura_aether[x].duration > 0) {
			sl_doscript_blargs(magicdb_yname(sd->status.dura_aether[x].id), "on_drop_while_cast", 1, &sd->bl);
		}
	}

	for (int x = 0; x < MAX_MAGIC_TIMERS; x++) {
		if (sd->status.dura_aether[x].id > 0 && sd->status.dura_aether[x].aether > 0) {
			sl_doscript_blargs(magicdb_yname(sd->status.dura_aether[x].id), "on_drop_while_aether", 1, &sd->bl);
		}
	}

	if (sd->fakeDrop) return 0;

	pc_dropitemmap(sd, id, all);

	return 0;
}

int client_parse_get_item(USER* sd) {
	if (sd->status.state == 1 || sd->status.state == 3) return 0;

	if (sd->status.state == 2)
	{
		sd->status.state = 0;
		sl_doscript_blargs("invis_rogue", "uncast", 1, &sd->bl);
		map_foreachinarea(client_update_state, sd->bl.m, sd->bl.x, sd->bl.y, AREA, BL_PC, sd);
	}

	client_send_action(&sd->bl, 4, 40, 0);

	sd->pickuptype = RFIFOB(sd->fd, 5);

	for (int x = 0; x < MAX_MAGIC_TIMERS; x++) {
		if (sd->status.dura_aether[x].id > 0 && sd->status.dura_aether[x].duration > 0) {
			sl_doscript_blargs(magicdb_yname(sd->status.dura_aether[x].id), "on_pickup_while_cast", 1, &sd->bl);
		}
	}

	sl_doscript_blargs("onPickUp", NULL, 1, &sd->bl);

	return 0;
}

int client_parse_use_item(USER* sd) {
	pc_useitem(sd, RFIFOB(sd->fd, 5) - 1);
	return 0;
}

int client_parse_eat_item(USER* sd) {
	if (itemdb_type(sd->status.inventory[RFIFOB(sd->fd, 5) - 1].id) == ITM_EAT) {
		pc_useitem(sd, RFIFOB(sd->fd, 5) - 1);
	}
	else {
		client_send_minitext(sd, "That item is not edible.");
	}

	return 0;
}

int client_parse_unequip(USER* sd) {
	int type;
	int x;
	if (!sd) return 0;

	switch (RFIFOB(sd->fd, 5)) {
	case 0x01:
		type = EQ_WEAP;
		break;
	case 0x02:
		type = EQ_ARMOR;
		break;
	case 0x03:
		type = EQ_SHIELD;
		break;
	case 0x04:
		type = EQ_HELM;
		break;
	case 0x06:
		type = EQ_NECKLACE;
		break;
	case 0x07:
		type = EQ_LEFT;
		break;
	case 0x08:
		type = EQ_RIGHT;
		break;
	case 13:
		type = EQ_BOOTS;
		break;
	case 14:
		type = EQ_MANTLE;
		break;
	case 16:
		type = EQ_COAT;
		break;
	case 20:
		type = EQ_SUBLEFT;
		break;
	case 21:
		type = EQ_SUBRIGHT;
		break;
	case 22:
		type = EQ_FACEACC;
		break;
	case 23:
		type = EQ_CROWN;
		break;

	default:
		return 0;
	}

	if (itemdb_unequip(sd->status.equip[type].id) == 1 && !sd->status.gm_level) {
		char text[] = "You are unable to unequip that.";
		client_send_minitext(sd, text);
		return 0;
	}

	for (x = 0; x < sd->status.maxinv; x++) {
		if (!sd->status.inventory[x].id) {
			pc_unequip(sd, type);
			client_unequip_item(sd, RFIFOB(sd->fd, 5));
			return 0;
		}
	}

	client_send_minitext(sd, "Your inventory is full.");
	return 0;
}

/* Throw items */

int client_throw_item_sub(USER* sd, int id, int type, int x, int y) {
	FLOORITEM* fl = NULL;

	if (!sd->status.inventory[id].id)
		return 0;

	if (sd->status.inventory[id].amount <= 0) { client_send_del_item(sd, id, 4); return 0; }

	CALLOC(fl, FLOORITEM, 1);
	fl->bl.m = sd->bl.m;
	fl->bl.x = x;
	fl->bl.y = y;
	memcpy(&fl->data, &sd->status.inventory[id], sizeof(struct item));
	sd->invslot = id;
	sd->throwx = x;
	sd->throwy = y;
	sl_doscript_blargs("onThrow", NULL, 2, &sd->bl, &fl->bl);
	return 0;
}

int client_throw_item_script(USER* sd) {
	FLOORITEM* fl = NULL;
	char escape[255];
	char sndbuf[48];
	int len = 0;
	int def[1];
	int id = sd->invslot;
	int x = sd->throwx;
	int y = sd->throwy;
	int type = 0;

	CALLOC(fl, FLOORITEM, 1);
	fl->bl.m = sd->bl.m;
	fl->bl.x = x;
	fl->bl.y = y;
	memcpy(&fl->data, &sd->status.inventory[id], sizeof(struct item));
	def[0] = 0;

	if (fl->data.dura == itemdb_dura(fl->data.id)) {
		map_foreachincell(pc_addtocurrent, sd->bl.m, x, y, BL_ITEM, def, id, type, sd);
	}

	sd->status.inventory[id].amount--;

	if (type || !sd->status.inventory[id].amount) {
		memset(&sd->status.inventory[id], 0, sizeof(struct item));
		client_send_del_item(sd, id, 4);
	}
	else {
		fl->data.amount = 1;
		client_send_add_item(sd, id);
	}

	if (sd->bl.x != x) {
		WBUFB(sndbuf, 0) = 0xAA;
		WBUFW(sndbuf, 1) = SWAP16(0x1B);
		WBUFB(sndbuf, 3) = 0x16;
		WBUFB(sndbuf, 4) = 0x03;
		WBUFL(sndbuf, 5) = SWAP32(sd->bl.id);

		if (fl->data.customIcon != 0) {
			WBUFW(sndbuf, 9) = SWAP16(fl->data.customIcon + 49152);
			WBUFB(sndbuf, 11) = fl->data.customIconColor;
		}
		else {
			WBUFW(sndbuf, 9) = SWAP16(itemdb_icon(fl->data.id));
			WBUFB(sndbuf, 11) = itemdb_iconcolor(fl->data.id);
		}

		if (def[0]) {
			WBUFL(sndbuf, 12) = (unsigned int)def[0];
		}
		else {
			WBUFL(sndbuf, 12) = (unsigned int)fl->bl.id;
		}
		WBUFW(sndbuf, 16) = SWAP16(sd->bl.x);
		WBUFW(sndbuf, 18) = SWAP16(sd->bl.y);
		WBUFW(sndbuf, 20) = SWAP16(x);
		WBUFW(sndbuf, 22) = SWAP16(y);
		WBUFL(sndbuf, 24) = 0;
		WBUFB(sndbuf, 28) = 0x02;
		WBUFB(sndbuf, 29) = 0x00;
		client_send(sndbuf, 48, &sd->bl, SAMEAREA);
	}
	else {
		client_send_action(&sd->bl, 2, 30, 0);
	}

	if (!def[0]) {
		map_additem(&fl->bl);
		map_foreachinarea(client_object_look_sub2, sd->bl.m, sd->bl.x, sd->bl.y, AREA, BL_PC, LOOK_SEND, &fl->bl);
	}
	else {
		FREE(fl);
	}

	return 0;
}

int client_throw_check(struct block_list* bl, va_list ap) {
	MOB* mob = NULL;
	USER* sd = NULL;

	int* found = NULL;
	found = va_arg(ap, int*);
	if (found[0]) return 0;
	if (bl->type == BL_NPC) {
		if (bl->subtype != SCRIPT) return 0;
	}
	if (bl->type == BL_MOB) {
		mob = (MOB*)bl;
		if (mob->state == MOB_DEAD) return 0;
	}
	if (bl->type == BL_PC) {
		sd = (USER*)bl;
		if (sd->status.state == 1 || (sd->optFlags & optFlag_stealth)) return 0;
	}

	found[0] += 1;
	return 0;
}

int client_throw_confirm(USER* sd) {
	WFIFOHEAD(sd->fd, 7);
	WFIFOB(sd->fd, 0) = 0xAA;
	WFIFOW(sd->fd, 1) = SWAP16(7);
	WFIFOB(sd->fd, 3) = 0x4E;
	WFIFOB(sd->fd, 5) = RFIFOB(sd->fd, 6);
	WFIFOB(sd->fd, 6) = 0;
	WFIFOSET(sd->fd, client_encrypt(sd->fd));

	return 0;
}

int client_parse_throw(USER* sd) {
	struct warp_list* x = NULL;

	char RegStr[] = "goldbardupe";
	int DupeTimes = pc_readglobalreg(sd, RegStr);
	if (DupeTimes)
	{
		return 0;
	}

	if (sd->status.gm_level == 0) {
		if (sd->status.state == 1) {
			client_send_minitext(sd, "Spirits can't do that.");
			return 0;
		}
		if (sd->status.state == 3) {
			client_send_minitext(sd, "You cannot do that while riding a mount.");
			return 0;
		}
		if (sd->status.state == 4) {
			client_send_minitext(sd, "You cannot do that while transformed.");
			return 0;
		}
	}

	int pos = RFIFOB(sd->fd, 6) - 1;
	if (itemdb_droppable(sd->status.inventory[pos].id)) {
		client_send_minitext(sd, "You can't throw this item.");
		return 0;
	}

	int max = 8;
	int newx = sd->bl.x;
	int newy = sd->bl.y;
	int xmod = 0, x1;
	int ymod = 0, y1;
	int xside = 0, yside = 0;
	int found[1];
	int i;
	found[0] = 0;
	switch (sd->status.side) {
	case 0:
		ymod = -1;
		break;
	case 1:
		xmod = 1;
		break;
	case 2:
		ymod = 1;
		break;
	case 3:
		xmod = -1;
		break;
	}
	for (i = 0; i < max; i++) {
		x1 = sd->bl.x + (i * xmod) + xmod;
		y1 = sd->bl.y + (i * ymod) + ymod;
		if (x1 < 0) x1 = 0;
		if (y1 < 0) y1 = 0;
		if (x1 >= map[sd->bl.m].xs) x1 = map[sd->bl.m].xs - 1;
		if (y1 >= map[sd->bl.m].ys) y1 = map[sd->bl.m].ys - 1;

		map_foreachincell(client_throw_check, sd->bl.m, x1, y1, BL_NPC, found);
		map_foreachincell(client_throw_check, sd->bl.m, x1, y1, BL_PC, found);
		map_foreachincell(client_throw_check, sd->bl.m, x1, y1, BL_MOB, found);
		found[0] += read_pass(sd->bl.m, x1, y1);
		found[0] += client_object_can_move(sd->bl.m, x1, y1, sd->status.side);
		found[0] += client_object_can_move_from(sd->bl.m, x1, y1, sd->status.side);
		for (x = map[sd->bl.m].warp[x1 / BLOCK_SIZE + (y1 / BLOCK_SIZE) * map[sd->bl.m].bxs]; x && !found[0]; x = x->next) {
			if (x->x == x1 && x->y == y1) {
				found[0] += 1;
			}
		}
		if (found[0]) {
			break;
		}
		newx = x1;
		newy = y1;
	}
	client_throw_item_sub(sd, pos, 0, newx, newy);
	return 0;
}

/* Inventory checks */

int client_check_inv_bod(USER* sd) {
	float percentage;
	int id;
	char buf[255];
	char escape[255];

	nullpo_ret(0, sd);

	for (int x = 0; x < 52; x++) {
		sd->invslot = x;

		if (!sd->status.inventory[x].id) continue;

		id = sd->status.inventory[x].id;

		if (sd->status.state == 1 && itemdb_breakondeath(sd->status.inventory[x].id) == 1) {
			if (itemdb_protected(sd->status.inventory[x].id) || sd->status.inventory[x].protected >= 1) {
				sd->status.inventory[x].protected -= 1;
				sd->status.inventory[x].dura = itemdb_dura(sd->status.inventory[x].id);
				sprintf(buf, "Your %s has been restored!", itemdb_name(id));
				client_send_status(sd, SFLAG_FULLSTATS | SFLAG_HPMP);
				client_send_msg(sd, 5, buf);
				sl_doscript_blargs("characterLog", "invRestore", 1, &sd->bl);
				return 0;
			}

			memcpy(&sd->boditems.item[sd->boditems.bod_count], &sd->status.inventory[x], sizeof(sd->status.inventory[x]));
			sd->boditems.bod_count++;

			sprintf(buf, "Your %s was destroyed!", itemdb_name(id));
			sl_doscript_blargs("characterLog", "invBreak", 1, &sd->bl);

			Sql_EscapeString(sql_handle, escape, sd->status.inventory[x].real_name);

			sd->breakid = id;
			sl_doscript_blargs("onBreak", NULL, 1, &sd->bl);
			sl_doscript_blargs(itemdb_yname(id), "on_break", 1, &sd->bl);

			pc_delitem(sd, x, 1, 9);
			client_send_msg(sd, 5, buf);
		}

		map_foreachinarea(client_update_state, sd->bl.m, sd->bl.x, sd->bl.y, AREA, BL_PC, sd);
	}

	sl_doscript_blargs("characterLog", "bodLog", 1, &sd->bl);
	sd->boditems.bod_count = 0;

	return 0;
}

/* Gold/item handling */

int client_drop_gold(USER* sd, unsigned int amounts) {
	char escape[255];
	char RegStr[] = "goldbardupe";
	int DupeTimes = pc_readglobalreg(sd, RegStr);
	if (DupeTimes)
	{
		return 0;
	}

	if (sd->status.gm_level == 0) {
		if (sd->status.state == 1) {
			client_send_minitext(sd, "Spirits can't do that.");
			return 0;
		}
		if (sd->status.state == 3) {
			client_send_minitext(sd, "You cannot do that while riding a mount.");
			return 0;
		}

		if (sd->status.state == 4) {
			client_send_minitext(sd, "You cannot do that while transformed.");
			return 0;
		}
	}

	FLOORITEM* fl;

	unsigned int amount = amounts;
	if (!sd->status.money) return 0;
	if (!amounts) return 0;
	int def[1];
	client_send_action(&sd->bl, 5, 20, 0);
	def[0] = 0;
	CALLOC(fl, FLOORITEM, 1);
	fl->bl.m = sd->bl.m;
	fl->bl.x = sd->bl.x;
	fl->bl.y = sd->bl.y;

	if (sd->status.money < amount) {
		amount = sd->status.money;
		sd->status.money = 0;
	}
	else {
		sd->status.money -= amount;
	}

	if (amount == 1)
	{
		fl->data.id = 0;
		fl->data.amount = amount;
	}
	else if (amount >= 2 && amount <= 99)
	{
		fl->data.id = 1;
		fl->data.amount = amount;
	}
	else if (amount >= 100 && amount <= 999)
	{
		fl->data.id = 2;
		fl->data.amount = amount;
	}
	else if (amount >= 1000)
	{
		fl->data.id = 3;
		fl->data.amount = amount;
	}

	sd->fakeDrop = 0;

	sl_doscript_blargs("on_drop_gold", NULL, 2, &sd->bl, &fl->bl);

	for (int x = 0; x < MAX_MAGIC_TIMERS; x++) {
		if (sd->status.dura_aether[x].id > 0 && sd->status.dura_aether[x].duration > 0) {
			sl_doscript_blargs(magicdb_yname(sd->status.dura_aether[x].id), "on_drop_gold_while_cast", 2, &sd->bl, &fl->bl);
		}
	}

	for (int x = 0; x < MAX_MAGIC_TIMERS; x++) {
		if (sd->status.dura_aether[x].id > 0 && sd->status.dura_aether[x].aether > 0) {
			sl_doscript_blargs(magicdb_yname(sd->status.dura_aether[x].id), "on_drop_gold_while_aether", 2, &sd->bl, &fl->bl);
		}
	}

	if (sd->fakeDrop) return 0;

	char mini[64];
	sprintf(mini, "You dropped %d coins\0", fl->data.amount);
	client_send_minitext(sd, mini);

	map_foreachincell(client_add_to_current, sd->bl.m, sd->bl.x, sd->bl.y, BL_ITEM, def, amount);

	Sql_EscapeString(sql_handle, escape, fl->data.real_name);

	if (!def[0]) {
		map_additem(&fl->bl);

		sl_doscript_blargs("after_drop_gold", NULL, 2, &sd->bl, &fl->bl);

		for (int x = 0; x < MAX_MAGIC_TIMERS; x++) {
			if (sd->status.dura_aether[x].id > 0 && sd->status.dura_aether[x].duration > 0) {
				sl_doscript_blargs(magicdb_yname(sd->status.dura_aether[x].id), "after_drop_gold_while_cast", 2, &sd->bl, &fl->bl);
			}
		}

		for (int x = 0; x < MAX_MAGIC_TIMERS; x++) {
			if (sd->status.dura_aether[x].id > 0 && sd->status.dura_aether[x].aether > 0) {
				sl_doscript_blargs(magicdb_yname(sd->status.dura_aether[x].id), "after_drop_gold_while_aether", 2, &sd->bl, &fl->bl);
			}
		}

		sl_doscript_blargs("characterLog", "dropWrite", 2, &sd->bl, &fl->bl);
		map_foreachinarea(client_object_look_sub2, sd->bl.m, sd->bl.x, sd->bl.y, AREA, BL_PC, LOOK_SEND, &fl->bl);
	}
	else {
		FREE(fl);
	}

	client_send_status(sd, SFLAG_XPMONEY);

	return 0;
}

int client_hand_gold(USER* sd) {
	char buff[256];

	unsigned int gold = SWAP32(RFIFOL(sd->fd, 5));

	if (gold < 0) gold = 0;
	if (gold == 0) return 0;
	if (gold > sd->status.money) gold = sd->status.money;

	int x = 0;
	int y = 0;

	if (sd->status.side == 0) { x = sd->bl.x, y = sd->bl.y - 1; }
	if (sd->status.side == 1) { x = sd->bl.x + 1, y = sd->bl.y; }
	if (sd->status.side == 2) { x = sd->bl.x; y = sd->bl.y + 1; }
	if (sd->status.side == 3) { x = sd->bl.x - 1; y = sd->bl.y; }

	struct block_list* bl = NULL;

	bl = map_firstincell(sd->bl.m, x, y, BL_ALL);
	USER* tsd = NULL;

	sd->exchange.gold = gold;

	sprintf(buff, "They have refused to exchange with you");

	if (bl != NULL) {
		if (bl->type == BL_PC) {
			tsd = map_id2sd(bl->id);

			if (tsd->status.settingFlags & FLAG_EXCHANGE) {
				client_start_exchange(sd, bl->id);
				client_exchange_money(sd, tsd);
			}
			else client_send_minitext(sd, buff);
		}
	}

	return 0;
}

int client_post_item(USER* sd) {
	struct item_data* item = NULL;

	int slot = RFIFOB(sd->fd, 5) - 1;

	item = itemdb_search(sd->status.inventory[slot].id);

	int x = 0;
	int y = 0;

	if (sd->status.side == 0) { x = sd->bl.x, y = sd->bl.y - 1; }
	if (sd->status.side == 1) { x = sd->bl.x + 1, y = sd->bl.y; }
	if (sd->status.side == 2) { x = sd->bl.x; y = sd->bl.y + 1; }
	if (sd->status.side == 3) { x = sd->bl.x - 1; y = sd->bl.y; }

	if (x < 0 || y < 0) return 0;

	int obj = read_obj(sd->bl.m, x, y);

	if (obj == 1619 || obj == 1620) {
		if (sd->status.inventory[slot].amount > 1)
			client_input(sd, sd->last_click, "How many would you like to post?", "");
	}

	sd->invslot = slot;

	return 0;
}

int client_hand_item(USER* sd) {
	struct mobdb_data* db;

	char buff[256];

	int slot = RFIFOB(sd->fd, 5) - 1;
	int handgive = RFIFOB(sd->fd, 6);
	int amount = 0;

	int itemid = sd->status.inventory[slot].id;

	if (handgive == 0) amount = 1;
	else if (handgive == 1) amount = sd->status.inventory[slot].amount;

	int x = 0;
	int y = 0;

	if (sd->status.side == 0) { x = sd->bl.x, y = sd->bl.y - 1; }
	if (sd->status.side == 1) { x = sd->bl.x + 1, y = sd->bl.y; }
	if (sd->status.side == 2) { x = sd->bl.x; y = sd->bl.y + 1; }
	if (sd->status.side == 3) { x = sd->bl.x - 1; y = sd->bl.y; }

	struct block_list* bl = NULL;
	struct item_data* item = NULL;
	char msg[80];
	char* buf;
	struct npc_data* nd = NULL;

	bl = map_firstincell(sd->bl.m, x, y, BL_ALL);
	USER* tsd = NULL;
	MOB* mob = NULL;

	sprintf(buff, "They have refused to exchange with you");

	sd->invslot = slot;

	if (bl != NULL) {
		if (bl->type == BL_PC) {
			tsd = map_id2sd(bl->id);

			if (tsd->status.settingFlags & FLAG_EXCHANGE) {
				client_start_exchange(sd, bl->id);
				client_exchange_add_item(sd, tsd, slot, amount);
			}
			else client_send_minitext(sd, buff);
		}
		if (bl->type == BL_MOB) {
			mob = (MOB*)map_id2mob(bl->id);

			if (itemdb_exchangeable(sd->status.inventory[slot].id) == 1) return 0;

			for (int i = 0; i < MAX_INVENTORY; i++) {
				if (mob->inventory[i].id == sd->status.inventory[slot].id && mob->inventory[i].dura == sd->status.inventory[slot].dura && mob->inventory[i].owner == sd->status.inventory[slot].owner && mob->inventory[i].protected == sd->status.inventory[slot].protected) {
					mob->inventory[i].amount += amount;
					break;
				}
				else if (mob->inventory[i].id == 0) {
					mob->inventory[i].id = sd->status.inventory[slot].id;
					mob->inventory[i].amount = amount;
					mob->inventory[i].owner = sd->status.inventory[slot].owner;
					mob->inventory[i].dura = sd->status.inventory[slot].dura;
					mob->inventory[i].protected = sd->status.inventory[slot].protected;
					break;
				}
			}

			pc_delitem(sd, slot, amount, 9);
		}
		if (bl->type == BL_NPC) {
			nd = map_id2npc((unsigned int)bl->id);

			if (itemdb_exchangeable(sd->status.inventory[slot].id) || itemdb_droppable(sd->status.inventory[slot].id)) return 0;

			if (nd->receiveItem == 1)
				sl_doscript_blargs(nd->name, "handItem", 2, &sd->bl, &nd->bl);
			else {
				sprintf(msg, "What are you trying to do? Keep your junky %s with you!", itemdb_name(sd->status.inventory[slot].id));

				WFIFOHEAD(sd->fd, strlen(msg) + 11);
				WFIFOB(sd->fd, 5) = 0;
				WFIFOL(sd->fd, 6) = SWAP32((unsigned int)bl->id);
				WFIFOB(sd->fd, 10) = strlen(msg);
				strcpy(WFIFOP(sd->fd, 11), msg);

				WFIFOHEADER(sd->fd, 0x0D, strlen(msg) + 11);
				WFIFOSET(sd->fd, client_encrypt(sd->fd));
			}
		}
	}

	return 0;
}
