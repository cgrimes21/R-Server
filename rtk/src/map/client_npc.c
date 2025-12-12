/**
 * @file client_npc.c
 * @brief NPC dialogs, menus, shops, and interaction packets
 *
 * Extracted from clif.c as part of the RTK refactoring project.
 * Handles all NPC interaction UI.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

#include "client.h"
#include "client_npc.h"
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
#include "class_db.h"
#include "clan_db.h"

/* Map message numbers */

int client_map_msgnum(USER* sd, int id) {
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

	return msgnum;
}

/* NPC dialog parsing */

int client_parse_npc_dialog(USER* sd) {
	int npc_choice = RFIFOB(sd->fd, 13);
	int npc_menu = 0;
	char input[100];
	memset(input, 0, 80);

	switch (RFIFOB(sd->fd, 5)) {
	case 0x01:
		sl_resumedialog(npc_choice, sd);
		break;
	case 0x02:
		npc_menu = RFIFOB(sd->fd, 15);
		sl_resumemenuseq(npc_choice, npc_menu, sd);
		break;
	case 0x04:
		if (RFIFOB(sd->fd, 13) != 0x02) { sl_async_freeco(sd); return 1; }
		memcpy(input, RFIFOP(sd->fd, 16), RFIFOB(sd->fd, 15));
		sl_resumeinputseq(npc_choice, input, sd);
		break;
	}

	return 0;
}

int client_parse_menu(USER* sd) {
	int selection;
	unsigned int id;
	id = SWAP32(RFIFOL(sd->fd, 6));
	selection = SWAP16(RFIFOW(sd->fd, 10));
	sl_resumemenu(selection, sd);
	return 0;
}

int client_parse_buy(USER* sd) {
	char itemname[255];
	struct item_data* item = NULL;

	memset(itemname, 0, 255);
	memcpy(itemname, RFIFOP(sd->fd, 13), RFIFOB(sd->fd, 12));

	if (strcmp(itemname, "") != 0) sl_resumebuy(itemname, sd);

	return 0;
}

int client_parse_sell(USER* sd) {
	sl_resumesell(RFIFOB(sd->fd, 12), sd);
	return 0;
}

int client_parse_input(USER* sd) {
	char output[256];
	char output2[256];
	int tlen = 0;

	memset(output, 0, 256);
	memset(output2, 0, 256);
	memcpy(output, RFIFOP(sd->fd, 13), RFIFOB(sd->fd, 12));
	tlen = RFIFOB(sd->fd, 12) + 1;
	memcpy(output2, RFIFOP(sd->fd, tlen + 13), RFIFOB(sd->fd, tlen + 12));

	sl_resumeinput(output, output2, sd);
	return 0;
}

/* Look at functions */

int client_parse_look_at_sub(struct block_list* bl, va_list ap) {
	USER* sd = NULL;
	nullpo_ret(0, bl);
	nullpo_ret(0, sd = va_arg(ap, USER*));

	sl_doscript_blargs("onLook", NULL, 2, &sd->bl, bl);
	return 0;
}

int client_parse_look_at_scriptsub(USER* sd, struct block_list* bl) {
	return 0;
}

int client_parse_look_at_2(USER* sd) {
	int x, dx;
	int y, dy;

	dx = sd->bl.x;
	dy = sd->bl.y;

	switch (sd->status.side) {
	case 0:
		dy--;
		break;
	case 1:
		dx++;
		break;
	case 2:
		dy++;
		break;
	case 3:
		dx--;
		break;
	}

	map_foreachincell(client_parse_look_at_sub, sd->bl.m, dx, dy, BL_PC, sd);
	map_foreachincell(client_parse_look_at_sub, sd->bl.m, dx, dy, BL_MOB, sd);
	map_foreachincell(client_parse_look_at_sub, sd->bl.m, dx, dy, BL_ITEM, sd);
	map_foreachincell(client_parse_look_at_sub, sd->bl.m, dx, dy, BL_NPC, sd);
	return 0;
}

int client_parse_look_at(USER* sd) {
	int x = 0, y = 0;

	x = SWAP16(RFIFOW(sd->fd, 5));
	y = SWAP16(RFIFOW(sd->fd, 7));

	map_foreachincell(client_parse_look_at_sub, sd->bl.m, x, y, BL_PC, sd);
	map_foreachincell(client_parse_look_at_sub, sd->bl.m, x, y, BL_MOB, sd);
	map_foreachincell(client_parse_look_at_sub, sd->bl.m, x, y, BL_ITEM, sd);
	map_foreachincell(client_parse_look_at_sub, sd->bl.m, x, y, BL_NPC, sd);
	return 0;
}

/* Script message dialog */

int client_script_message(USER* sd, int id, char* msg, int previous, int next) {
	int graphic_id = sd->npc_g;
	int color = sd->npc_gc;
	NPC* nd = map_id2npc((unsigned int)id);
	int type = sd->dialogtype;

	if (nd) {
		nd->lastaction = time(NULL);
	}

	if (!session[sd->fd])
	{
		session[sd->fd]->eof = 8;
		return 0;
	}

	WFIFOHEAD(sd->fd, 1024);
	WFIFOB(sd->fd, 0) = 0xAA;
	WFIFOB(sd->fd, 3) = 0x30;
	WFIFOW(sd->fd, 5) = SWAP16(1);
	WFIFOL(sd->fd, 7) = SWAP32(id);

	if (type == 0) {
		if (graphic_id == 0) {
			WFIFOB(sd->fd, 11) = 0;
		}
		else if (graphic_id >= 49152) {
			WFIFOB(sd->fd, 11) = 2;
		}
		else {
			WFIFOB(sd->fd, 11) = 1;
		}

		WFIFOB(sd->fd, 12) = 1;
		WFIFOW(sd->fd, 13) = SWAP16(graphic_id);
		WFIFOB(sd->fd, 15) = color;
		WFIFOB(sd->fd, 16) = 1;
		WFIFOW(sd->fd, 17) = SWAP16(graphic_id);
		WFIFOB(sd->fd, 19) = color;
		WFIFOL(sd->fd, 20) = SWAP32(1);
		WFIFOB(sd->fd, 24) = previous;
		WFIFOB(sd->fd, 25) = next;
		WFIFOW(sd->fd, 26) = SWAP16(strlen(msg));
		strcpy(WFIFOP(sd->fd, 28), msg);
		WFIFOW(sd->fd, 1) = SWAP16(strlen(msg) + 25);
	}
	else if (type == 1) {
		WFIFOB(sd->fd, 11) = 1;
		WFIFOW(sd->fd, 12) = SWAP16(nd->sex);
		WFIFOB(sd->fd, 14) = nd->state;
		WFIFOB(sd->fd, 15) = 0;
		WFIFOW(sd->fd, 16) = SWAP16(nd->equip[EQ_ARMOR].id);
		WFIFOB(sd->fd, 18) = 0;
		WFIFOB(sd->fd, 19) = nd->face;
		WFIFOB(sd->fd, 20) = nd->hair;
		WFIFOB(sd->fd, 21) = nd->hair_color;
		WFIFOB(sd->fd, 22) = nd->face_color;
		WFIFOB(sd->fd, 23) = nd->skin_color;

		if (!nd->equip[EQ_ARMOR].id) {
			WFIFOW(sd->fd, 24) = 0xFFFF;
			WFIFOB(sd->fd, 26) = 0;
		}
		else {
			WFIFOW(sd->fd, 24) = SWAP16(nd->equip[EQ_ARMOR].id);
			if (nd->armor_color) {
				WFIFOB(sd->fd, 26) = nd->armor_color;
			}
			else {
				WFIFOB(sd->fd, 26) = nd->equip[EQ_ARMOR].customLookColor;
			}
		}

		if (nd->equip[EQ_COAT].id) {
			WFIFOW(sd->fd, 24) = SWAP16(nd->equip[EQ_COAT].id);
			WFIFOB(sd->fd, 26) = nd->equip[EQ_COAT].customLookColor;
		}

		if (!nd->equip[EQ_WEAP].id) {
			WFIFOW(sd->fd, 27) = 0xFFFF;
			WFIFOB(sd->fd, 29) = 0;
		}
		else {
			WFIFOW(sd->fd, 27) = SWAP16(nd->equip[EQ_WEAP].id);
			WFIFOB(sd->fd, 29) = nd->equip[EQ_WEAP].customLookColor;
		}

		if (!nd->equip[EQ_SHIELD].id) {
			WFIFOW(sd->fd, 30) = 0xFFFF;
			WFIFOB(sd->fd, 32) = 0;
		}
		else {
			WFIFOW(sd->fd, 30) = SWAP16(nd->equip[EQ_SHIELD].id);
			WFIFOB(sd->fd, 32) = nd->equip[EQ_SHIELD].customLookColor;
		}

		if (!nd->equip[EQ_HELM].id) {
			WFIFOB(sd->fd, 33) = 0;
			WFIFOB(sd->fd, 34) = 0xFF;
			WFIFOB(sd->fd, 35) = 0;
		}
		else {
			WFIFOB(sd->fd, 33) = 1;
			WFIFOB(sd->fd, 34) = nd->equip[EQ_HELM].id;
			WFIFOB(sd->fd, 35) = nd->equip[EQ_HELM].customLookColor;
		}

		if (!nd->equip[EQ_FACEACC].id) {
			WFIFOW(sd->fd, 36) = 0xFFFF;
			WFIFOB(sd->fd, 38) = 0;
		}
		else {
			WFIFOW(sd->fd, 36) = SWAP16(nd->equip[EQ_FACEACC].id);
			WFIFOB(sd->fd, 38) = nd->equip[EQ_FACEACC].customLookColor;
		}

		if (!nd->equip[EQ_CROWN].id) {
			WFIFOW(sd->fd, 39) = 0xFFFF;
			WFIFOB(sd->fd, 41) = 0;
		}
		else {
			WFIFOB(sd->fd, 33) = 0;
			WFIFOW(sd->fd, 39) = SWAP16(nd->equip[EQ_CROWN].id);
			WFIFOB(sd->fd, 41) = nd->equip[EQ_CROWN].customLookColor;
		}

		if (!nd->equip[EQ_FACEACCTWO].id) {
			WFIFOW(sd->fd, 42) = 0xFFFF;
			WFIFOB(sd->fd, 44) = 0;
		}
		else {
			WFIFOW(sd->fd, 42) = SWAP16(nd->equip[EQ_FACEACCTWO].id);
			WFIFOB(sd->fd, 44) = nd->equip[EQ_FACEACCTWO].customLookColor;
		}

		if (!nd->equip[EQ_MANTLE].id) {
			WFIFOW(sd->fd, 45) = 0xFFFF;
			WFIFOB(sd->fd, 47) = 0;
		}
		else {
			WFIFOW(sd->fd, 45) = SWAP16(nd->equip[EQ_MANTLE].id);
			WFIFOB(sd->fd, 47) = nd->equip[EQ_MANTLE].customLookColor;
		}

		if (!nd->equip[EQ_NECKLACE].id) {
			WFIFOW(sd->fd, 48) = 0xFFFF;
			WFIFOB(sd->fd, 50) = 0;
		}
		else {
			WFIFOW(sd->fd, 48) = SWAP16(nd->equip[EQ_NECKLACE].id);
			WFIFOB(sd->fd, 50) = nd->equip[EQ_NECKLACE].customLookColor;
		}

		if (!nd->equip[EQ_BOOTS].id) {
			WFIFOW(sd->fd, 51) = SWAP16(nd->sex);
			WFIFOB(sd->fd, 53) = 0;
		}
		else {
			WFIFOW(sd->fd, 51) = SWAP16(nd->equip[EQ_BOOTS].id);
			WFIFOB(sd->fd, 53) = nd->equip[EQ_BOOTS].customLookColor;
		}

		WFIFOB(sd->fd, 54) = 1;
		WFIFOW(sd->fd, 55) = SWAP16(graphic_id);
		WFIFOB(sd->fd, 57) = color;
		WFIFOL(sd->fd, 58) = SWAP32(1);
		WFIFOB(sd->fd, 62) = previous;
		WFIFOB(sd->fd, 63) = next;
		WFIFOW(sd->fd, 64) = SWAP16(strlen(msg));
		strcpy(WFIFOP(sd->fd, 66), msg);
		WFIFOW(sd->fd, 1) = SWAP16(strlen(msg) + 63);
	}
	else if (type == 2) {
		WFIFOB(sd->fd, 11) = 1;
		WFIFOW(sd->fd, 12) = SWAP16(nd->sex);
		WFIFOB(sd->fd, 14) = nd->state;
		WFIFOB(sd->fd, 15) = 0;
		WFIFOW(sd->fd, 16) = SWAP16(nd->gfx.armor);
		WFIFOB(sd->fd, 18) = 0;
		WFIFOB(sd->fd, 19) = nd->gfx.face;
		WFIFOB(sd->fd, 20) = nd->gfx.hair;
		WFIFOB(sd->fd, 21) = nd->gfx.chair;
		WFIFOB(sd->fd, 22) = nd->gfx.cface;
		WFIFOB(sd->fd, 23) = nd->gfx.cskin;

		WFIFOW(sd->fd, 24) = SWAP16(nd->gfx.armor);
		WFIFOB(sd->fd, 26) = nd->gfx.carmor;

		WFIFOW(sd->fd, 27) = SWAP16(nd->gfx.weapon);
		WFIFOB(sd->fd, 29) = nd->gfx.cweapon;

		WFIFOW(sd->fd, 30) = SWAP16(nd->gfx.shield);
		WFIFOB(sd->fd, 32) = nd->gfx.cshield;

		if (nd->gfx.helm == 65535) {
			WFIFOB(sd->fd, 33) = 0;
			WFIFOB(sd->fd, 34) = 0xFF;
			WFIFOB(sd->fd, 35) = 0;
		}
		else {
			WFIFOB(sd->fd, 33) = 1;
			WFIFOB(sd->fd, 34) = nd->gfx.helm;
			WFIFOB(sd->fd, 35) = nd->gfx.chelm;
		}

		WFIFOW(sd->fd, 36) = SWAP16(nd->gfx.faceAcc);
		WFIFOB(sd->fd, 38) = nd->gfx.cfaceAcc;

		if (nd->gfx.crown == 65535) {
			WFIFOW(sd->fd, 39) = 0xFFFF;
			WFIFOB(sd->fd, 41) = 0;
		}
		else {
			WFIFOB(sd->fd, 33) = 0;
			WFIFOW(sd->fd, 39) = SWAP16(nd->gfx.crown);
			WFIFOB(sd->fd, 41) = nd->gfx.ccrown;
		}

		WFIFOW(sd->fd, 42) = SWAP16(nd->gfx.faceAccT);;
		WFIFOB(sd->fd, 44) = nd->gfx.cfaceAccT;

		WFIFOW(sd->fd, 45) = SWAP16(nd->gfx.mantle);
		WFIFOB(sd->fd, 47) = nd->gfx.cmantle;

		WFIFOW(sd->fd, 48) = SWAP16(nd->gfx.necklace);
		WFIFOB(sd->fd, 50) = nd->gfx.cnecklace;

		WFIFOW(sd->fd, 51) = SWAP16(nd->gfx.boots);
		WFIFOB(sd->fd, 53) = nd->gfx.cboots;

		WFIFOB(sd->fd, 54) = 1;
		WFIFOW(sd->fd, 55) = SWAP16(graphic_id);
		WFIFOB(sd->fd, 57) = color;
		WFIFOL(sd->fd, 58) = SWAP32(1);
		WFIFOB(sd->fd, 62) = previous;
		WFIFOB(sd->fd, 63) = next;
		WFIFOW(sd->fd, 64) = SWAP16(strlen(msg));
		strcpy(WFIFOP(sd->fd, 66), msg);
		WFIFOW(sd->fd, 1) = SWAP16(strlen(msg) + 63);
	}

	WFIFOSET(sd->fd, client_encrypt(sd->fd));
	return 0;
}

/* Script menu dialog */

int client_script_menu(USER* sd, int id, char* dialog, char* menu[], int size) {
	int graphic = sd->npc_g;
	int color = sd->npc_gc;
	int x;
	int len;
	int i;
	NPC* nd = map_id2npc((unsigned int)id);
	int type = sd->dialogtype;

	if (nd) {
		nd->lastaction = time(NULL);
	}

	if (!session[sd->fd])
	{
		session[sd->fd]->eof = 8;
		return 0;
	}

	WFIFOHEAD(sd->fd, 65535);
	WFIFOB(sd->fd, 0) = 0xAA;
	WFIFOB(sd->fd, 3) = 0x2F;
	WFIFOW(sd->fd, 5) = SWAP16(1);
	WFIFOL(sd->fd, 7) = SWAP32(id);

	if (type == 0) {
		if (graphic == 0) {
			WFIFOB(sd->fd, 11) = 0;
		}
		else if (graphic >= 49152) {
			WFIFOB(sd->fd, 11) = 2;
		}
		else {
			WFIFOB(sd->fd, 11) = 1;
		}

		WFIFOB(sd->fd, 12) = 1;
		WFIFOW(sd->fd, 13) = SWAP16(graphic);
		WFIFOB(sd->fd, 15) = color;
		WFIFOB(sd->fd, 16) = 1;
		WFIFOW(sd->fd, 17) = SWAP16(graphic);
		WFIFOB(sd->fd, 19) = color;
		WFIFOW(sd->fd, 20) = SWAP16(strlen(dialog));
		strcpy(WFIFOP(sd->fd, 22), dialog);
		WFIFOB(sd->fd, strlen(dialog) + 22) = size;
		len = strlen(dialog);

		for (x = 1; x < size + 1; x++) {
			WFIFOB(sd->fd, len + 23) = strlen(menu[x]);
			strcpy(WFIFOP(sd->fd, len + 24), menu[x]);
			len += strlen(menu[x]) + 1;
			WFIFOW(sd->fd, len + 23) = SWAP16(x);
			len += 2;
		}

		WFIFOW(sd->fd, 1) = SWAP16(len + 20);
	}
	else if (type == 1) {
		WFIFOB(sd->fd, 11) = 1;
		WFIFOW(sd->fd, 12) = SWAP16(nd->sex);
		WFIFOB(sd->fd, 14) = nd->state;
		WFIFOB(sd->fd, 15) = 0;
		WFIFOW(sd->fd, 16) = SWAP16(nd->equip[EQ_ARMOR].id);
		WFIFOB(sd->fd, 18) = 0;
		WFIFOB(sd->fd, 19) = nd->face;
		WFIFOB(sd->fd, 20) = nd->hair;
		WFIFOB(sd->fd, 21) = nd->hair_color;
		WFIFOB(sd->fd, 22) = nd->face_color;
		WFIFOB(sd->fd, 23) = nd->skin_color;

		if (!nd->equip[EQ_ARMOR].id) {
			WFIFOW(sd->fd, 24) = 0xFFFF;
			WFIFOB(sd->fd, 26) = 0;
		}
		else {
			WFIFOW(sd->fd, 24) = SWAP16(nd->equip[EQ_ARMOR].id);
			if (nd->armor_color) {
				WFIFOB(sd->fd, 26) = nd->armor_color;
			}
			else {
				WFIFOB(sd->fd, 26) = nd->equip[EQ_ARMOR].customLookColor;
			}
		}

		if (nd->equip[EQ_COAT].id) {
			WFIFOW(sd->fd, 24) = SWAP16(nd->equip[EQ_COAT].id);
			WFIFOB(sd->fd, 26) = nd->equip[EQ_COAT].customLookColor;
		}

		if (!nd->equip[EQ_WEAP].id) {
			WFIFOW(sd->fd, 27) = 0xFFFF;
			WFIFOB(sd->fd, 29) = 0;
		}
		else {
			WFIFOW(sd->fd, 27) = SWAP16(nd->equip[EQ_WEAP].id);
			WFIFOB(sd->fd, 29) = nd->equip[EQ_WEAP].customLookColor;
		}

		if (!nd->equip[EQ_SHIELD].id) {
			WFIFOW(sd->fd, 30) = 0xFFFF;
			WFIFOB(sd->fd, 32) = 0;
		}
		else {
			WFIFOW(sd->fd, 30) = SWAP16(nd->equip[EQ_SHIELD].id);
			WFIFOB(sd->fd, 32) = nd->equip[EQ_SHIELD].customLookColor;
		}

		if (!nd->equip[EQ_HELM].id) {
			WFIFOB(sd->fd, 33) = 0;
			WFIFOB(sd->fd, 34) = 0xFF;
			WFIFOB(sd->fd, 35) = 0;
		}
		else {
			WFIFOB(sd->fd, 33) = 1;
			WFIFOB(sd->fd, 34) = nd->equip[EQ_HELM].id;
			WFIFOB(sd->fd, 35) = nd->equip[EQ_HELM].customLookColor;
		}

		if (!nd->equip[EQ_FACEACC].id) {
			WFIFOW(sd->fd, 36) = 0xFFFF;
			WFIFOB(sd->fd, 38) = 0;
		}
		else {
			WFIFOW(sd->fd, 36) = SWAP16(nd->equip[EQ_FACEACC].id);
			WFIFOB(sd->fd, 38) = nd->equip[EQ_FACEACC].customLookColor;
		}

		if (!nd->equip[EQ_CROWN].id) {
			WFIFOW(sd->fd, 39) = 0xFFFF;
			WFIFOB(sd->fd, 41) = 0;
		}
		else {
			WFIFOB(sd->fd, 33) = 0;
			WFIFOW(sd->fd, 39) = SWAP16(nd->equip[EQ_CROWN].id);
			WFIFOB(sd->fd, 41) = nd->equip[EQ_CROWN].customLookColor;
		}

		if (!nd->equip[EQ_FACEACCTWO].id) {
			WFIFOW(sd->fd, 42) = 0xFFFF;
			WFIFOB(sd->fd, 44) = 0;
		}
		else {
			WFIFOW(sd->fd, 42) = SWAP16(nd->equip[EQ_FACEACCTWO].id);
			WFIFOB(sd->fd, 44) = nd->equip[EQ_FACEACCTWO].customLookColor;
		}

		if (!nd->equip[EQ_MANTLE].id) {
			WFIFOW(sd->fd, 45) = 0xFFFF;
			WFIFOB(sd->fd, 47) = 0;
		}
		else {
			WFIFOW(sd->fd, 45) = SWAP16(nd->equip[EQ_MANTLE].id);
			WFIFOB(sd->fd, 47) = nd->equip[EQ_MANTLE].customLookColor;
		}

		if (!nd->equip[EQ_NECKLACE].id) {
			WFIFOW(sd->fd, 48) = 0xFFFF;
			WFIFOB(sd->fd, 50) = 0;
		}
		else {
			WFIFOW(sd->fd, 48) = SWAP16(nd->equip[EQ_NECKLACE].id);
			WFIFOB(sd->fd, 50) = nd->equip[EQ_NECKLACE].customLookColor;
		}

		if (!nd->equip[EQ_BOOTS].id) {
			WFIFOW(sd->fd, 51) = SWAP16(nd->sex);
			WFIFOB(sd->fd, 53) = 0;
		}
		else {
			WFIFOW(sd->fd, 51) = SWAP16(nd->equip[EQ_BOOTS].id);
			WFIFOB(sd->fd, 53) = nd->equip[EQ_BOOTS].customLookColor;
		}

		WFIFOB(sd->fd, 54) = 1;
		WFIFOW(sd->fd, 55) = SWAP16(graphic);
		WFIFOB(sd->fd, 57) = color;
		WFIFOW(sd->fd, 58) = SWAP16(strlen(dialog));
		strcpy(WFIFOP(sd->fd, 60), dialog);
		WFIFOB(sd->fd, strlen(dialog) + 60) = size;
		len = strlen(dialog);

		for (x = 1; x < size + 1; x++) {
			WFIFOB(sd->fd, len + 61) = strlen(menu[x]);
			strcpy(WFIFOP(sd->fd, len + 62), menu[x]);
			len += strlen(menu[x]) + 1;
			WFIFOW(sd->fd, len + 61) = SWAP16(x);
			len += 2;
		}

		WFIFOW(sd->fd, 1) = SWAP16(len + 58);
	}
	else if (type == 2) {
		WFIFOB(sd->fd, 11) = 1;
		WFIFOW(sd->fd, 12) = SWAP16(nd->sex);
		WFIFOB(sd->fd, 14) = nd->state;
		WFIFOB(sd->fd, 15) = 0;
		WFIFOW(sd->fd, 16) = SWAP16(nd->gfx.armor);
		WFIFOB(sd->fd, 18) = 0;
		WFIFOB(sd->fd, 19) = nd->gfx.face;
		WFIFOB(sd->fd, 20) = nd->gfx.hair;
		WFIFOB(sd->fd, 21) = nd->gfx.chair;
		WFIFOB(sd->fd, 22) = nd->gfx.cface;
		WFIFOB(sd->fd, 23) = nd->gfx.cskin;

		WFIFOW(sd->fd, 24) = SWAP16(nd->gfx.armor);
		WFIFOB(sd->fd, 26) = nd->gfx.carmor;

		WFIFOW(sd->fd, 27) = SWAP16(nd->gfx.weapon);
		WFIFOB(sd->fd, 29) = nd->gfx.cweapon;

		WFIFOW(sd->fd, 30) = SWAP16(nd->gfx.shield);
		WFIFOB(sd->fd, 32) = nd->gfx.cshield;

		if (nd->gfx.helm == 65535) {
			WFIFOB(sd->fd, 33) = 0;
			WFIFOB(sd->fd, 34) = 0xFF;
			WFIFOB(sd->fd, 35) = 0;
		}
		else {
			WFIFOB(sd->fd, 33) = 1;
			WFIFOB(sd->fd, 34) = nd->gfx.helm;
			WFIFOB(sd->fd, 35) = nd->gfx.chelm;
		}

		WFIFOW(sd->fd, 36) = SWAP16(nd->gfx.faceAcc);
		WFIFOB(sd->fd, 38) = nd->gfx.cfaceAcc;

		if (nd->gfx.crown == 65535) {
			WFIFOW(sd->fd, 39) = 0xFFFF;
			WFIFOB(sd->fd, 41) = 0;
		}
		else {
			WFIFOB(sd->fd, 33) = 0;
			WFIFOW(sd->fd, 39) = SWAP16(nd->gfx.crown);
			WFIFOB(sd->fd, 41) = nd->gfx.ccrown;
		}

		WFIFOW(sd->fd, 42) = SWAP16(nd->gfx.faceAccT);
		WFIFOB(sd->fd, 44) = nd->gfx.cfaceAccT;

		WFIFOW(sd->fd, 45) = SWAP16(nd->gfx.mantle);
		WFIFOB(sd->fd, 47) = nd->gfx.cmantle;

		WFIFOW(sd->fd, 48) = SWAP16(nd->gfx.necklace);
		WFIFOB(sd->fd, 50) = nd->gfx.cnecklace;

		WFIFOW(sd->fd, 51) = SWAP16(nd->gfx.boots);
		WFIFOB(sd->fd, 53) = nd->gfx.cboots;

		WFIFOB(sd->fd, 54) = 1;
		WFIFOW(sd->fd, 55) = SWAP16(graphic);
		WFIFOB(sd->fd, 57) = color;
		WFIFOW(sd->fd, 58) = SWAP16(strlen(dialog));
		strcpy(WFIFOP(sd->fd, 60), dialog);
		WFIFOB(sd->fd, strlen(dialog) + 60) = size;
		len = strlen(dialog);

		for (x = 1; x < size + 1; x++) {
			WFIFOB(sd->fd, len + 61) = strlen(menu[x]);
			strcpy(WFIFOP(sd->fd, len + 62), menu[x]);
			len += strlen(menu[x]) + 1;
			WFIFOW(sd->fd, len + 61) = SWAP16(x);
			len += 2;
		}

		WFIFOW(sd->fd, 1) = SWAP16(len + 58);
	}

	WFIFOSET(sd->fd, client_encrypt(sd->fd));
	return 0;
}

/* Script menu sequence dialog */

int client_script_menu_seq(USER* sd, int id, char* dialog, char* menu[], int size, int previous, int next) {
	int graphic_id = sd->npc_g;
	int color = sd->npc_gc;
	int x;
	int len = 0;
	NPC* nd = map_id2npc((unsigned int)id);
	int type = sd->dialogtype;

	if (nd) {
		nd->lastaction = time(NULL);
	}

	if (!session[sd->fd])
	{
		session[sd->fd]->eof = 8;
		return 0;
	}

	WFIFOHEAD(sd->fd, 65535);
	WFIFOB(sd->fd, 0) = 0xAA;
	WFIFOB(sd->fd, 3) = 0x30;
	WFIFOB(sd->fd, 4) = 0x03;
	WFIFOB(sd->fd, 5) = 0x02;

	WFIFOB(sd->fd, 6) = 0x02;
	WFIFOL(sd->fd, 7) = SWAP32(id);

	if (type == 0) {
		if (graphic_id == 0)
			WFIFOB(sd->fd, 11) = 0;
		else if (graphic_id >= 49152)
			WFIFOB(sd->fd, 11) = 2;
		else
			WFIFOB(sd->fd, 11) = 1;

		WFIFOB(sd->fd, 12) = 1;
		WFIFOW(sd->fd, 13) = SWAP16(graphic_id);

		WFIFOB(sd->fd, 15) = color;
		WFIFOB(sd->fd, 16) = 1;
		WFIFOW(sd->fd, 17) = SWAP16(graphic_id);
		WFIFOB(sd->fd, 19) = color;
		WFIFOL(sd->fd, 20) = SWAP32(1);
		WFIFOB(sd->fd, 24) = previous;
		WFIFOB(sd->fd, 25) = next;
		WFIFOW(sd->fd, 26) = SWAP16(strlen(dialog));
		strcpy(WFIFOP(sd->fd, 28), dialog);
		len = strlen(dialog) + 1;
		WFIFOB(sd->fd, len + 27) = size;
		len += 1;
		for (x = 1; x < size + 1; x++) {
			WFIFOB(sd->fd, len + 27) = strlen(menu[x]);
			strcpy(WFIFOP(sd->fd, len + 28), menu[x]);
			len += strlen(menu[x]) + 1;
		}
		WFIFOW(sd->fd, 1) = SWAP16(len + 24);
	}
	else if (type == 1) {
		WFIFOB(sd->fd, 11) = 1;
		WFIFOW(sd->fd, 12) = SWAP16(nd->sex);
		WFIFOB(sd->fd, 14) = nd->state;
		WFIFOB(sd->fd, 15) = 0;
		WFIFOW(sd->fd, 16) = SWAP16(nd->equip[EQ_ARMOR].id);
		WFIFOB(sd->fd, 18) = 0;
		WFIFOB(sd->fd, 19) = nd->face;
		WFIFOB(sd->fd, 20) = nd->hair;
		WFIFOB(sd->fd, 21) = nd->hair_color;
		WFIFOB(sd->fd, 22) = nd->face_color;
		WFIFOB(sd->fd, 23) = nd->skin_color;

		if (!nd->equip[EQ_ARMOR].id) {
			WFIFOW(sd->fd, 24) = 0xFFFF;
			WFIFOB(sd->fd, 26) = 0;
		}
		else {
			WFIFOW(sd->fd, 24) = SWAP16(nd->equip[EQ_ARMOR].id);
			if (nd->armor_color) {
				WFIFOB(sd->fd, 26) = nd->armor_color;
			}
			else {
				WFIFOB(sd->fd, 26) = nd->equip[EQ_ARMOR].customLookColor;
			}
		}

		if (nd->equip[EQ_COAT].id) {
			WFIFOW(sd->fd, 24) = SWAP16(nd->equip[EQ_COAT].id);
			WFIFOB(sd->fd, 26) = nd->equip[EQ_COAT].customLookColor;
		}

		if (!nd->equip[EQ_WEAP].id) {
			WFIFOW(sd->fd, 27) = 0xFFFF;
			WFIFOB(sd->fd, 29) = 0;
		}
		else {
			WFIFOW(sd->fd, 27) = SWAP16(nd->equip[EQ_WEAP].id);
			WFIFOB(sd->fd, 29) = nd->equip[EQ_WEAP].customLookColor;
		}

		if (!nd->equip[EQ_SHIELD].id) {
			WFIFOW(sd->fd, 30) = 0xFFFF;
			WFIFOB(sd->fd, 32) = 0;
		}
		else {
			WFIFOW(sd->fd, 30) = SWAP16(nd->equip[EQ_SHIELD].id);
			WFIFOB(sd->fd, 32) = nd->equip[EQ_SHIELD].customLookColor;
		}

		if (!nd->equip[EQ_HELM].id) {
			WFIFOB(sd->fd, 33) = 0;
			WFIFOB(sd->fd, 34) = 0xFF;
			WFIFOB(sd->fd, 35) = 0;
		}
		else {
			WFIFOB(sd->fd, 33) = 1;
			WFIFOB(sd->fd, 34) = nd->equip[EQ_HELM].id;
			WFIFOB(sd->fd, 35) = nd->equip[EQ_HELM].customLookColor;
		}

		if (!nd->equip[EQ_FACEACC].id) {
			WFIFOW(sd->fd, 36) = 0xFFFF;
			WFIFOB(sd->fd, 38) = 0;
		}
		else {
			WFIFOW(sd->fd, 36) = SWAP16(nd->equip[EQ_FACEACC].id);
			WFIFOB(sd->fd, 38) = nd->equip[EQ_FACEACC].customLookColor;
		}

		if (!nd->equip[EQ_CROWN].id) {
			WFIFOW(sd->fd, 39) = 0xFFFF;
			WFIFOB(sd->fd, 41) = 0;
		}
		else {
			WFIFOB(sd->fd, 33) = 0;
			WFIFOW(sd->fd, 39) = SWAP16(nd->equip[EQ_CROWN].id);
			WFIFOB(sd->fd, 41) = nd->equip[EQ_CROWN].customLookColor;
		}

		if (!nd->equip[EQ_FACEACCTWO].id) {
			WFIFOW(sd->fd, 42) = 0xFFFF;
			WFIFOB(sd->fd, 44) = 0;
		}
		else {
			WFIFOW(sd->fd, 42) = SWAP16(nd->equip[EQ_FACEACCTWO].id);
			WFIFOB(sd->fd, 44) = nd->equip[EQ_FACEACCTWO].customLookColor;
		}

		if (!nd->equip[EQ_MANTLE].id) {
			WFIFOW(sd->fd, 45) = 0xFFFF;
			WFIFOB(sd->fd, 47) = 0;
		}
		else {
			WFIFOW(sd->fd, 45) = SWAP16(nd->equip[EQ_MANTLE].id);
			WFIFOB(sd->fd, 47) = nd->equip[EQ_MANTLE].customLookColor;
		}

		if (!nd->equip[EQ_NECKLACE].id) {
			WFIFOW(sd->fd, 48) = 0xFFFF;
			WFIFOB(sd->fd, 50) = 0;
		}
		else {
			WFIFOW(sd->fd, 48) = SWAP16(nd->equip[EQ_NECKLACE].id);
			WFIFOB(sd->fd, 50) = nd->equip[EQ_NECKLACE].customLookColor;
		}

		if (!nd->equip[EQ_BOOTS].id) {
			WFIFOW(sd->fd, 51) = SWAP16(nd->sex);
			WFIFOB(sd->fd, 53) = 0;
		}
		else {
			WFIFOW(sd->fd, 51) = SWAP16(nd->equip[EQ_BOOTS].id);
			WFIFOB(sd->fd, 53) = nd->equip[EQ_BOOTS].customLookColor;
		}

		WFIFOB(sd->fd, 55) = 0;
		WFIFOB(sd->fd, 56) = 1;
		WFIFOW(sd->fd, 55) = SWAP16(graphic_id);
		WFIFOB(sd->fd, 59) = color;
		WFIFOL(sd->fd, 60) = SWAP32(1);
		WFIFOB(sd->fd, 64) = previous;
		WFIFOB(sd->fd, 65) = next;
		WFIFOW(sd->fd, 66) = SWAP16(strlen(dialog));
		strcpy(WFIFOP(sd->fd, 68), dialog);
		len = strlen(dialog) + 68;
		WFIFOB(sd->fd, len) = size;
		len += 1;
		for (x = 1; x < size + 1; x++) {
			WFIFOB(sd->fd, len) = strlen(menu[x]);
			strcpy(WFIFOP(sd->fd, len + 1), menu[x]);
			len += strlen(menu[x]) + 1;
		}

		WFIFOW(sd->fd, 1) = SWAP16(len + 68);
	}
	else if (type == 2) {
		WFIFOB(sd->fd, 11) = 1;
		WFIFOW(sd->fd, 12) = SWAP16(sd->status.sex);
		WFIFOB(sd->fd, 14) = sd->status.state;
		WFIFOB(sd->fd, 15) = 0;
		WFIFOW(sd->fd, 16) = SWAP16(sd->gfx.armor);
		WFIFOB(sd->fd, 18) = 0;
		WFIFOB(sd->fd, 19) = sd->gfx.face;
		WFIFOB(sd->fd, 20) = sd->gfx.hair;
		WFIFOB(sd->fd, 21) = sd->gfx.chair;
		WFIFOB(sd->fd, 22) = sd->gfx.cface;
		WFIFOB(sd->fd, 23) = sd->gfx.cskin;

		WFIFOW(sd->fd, 24) = SWAP16(sd->gfx.armor);
		WFIFOB(sd->fd, 26) = sd->gfx.carmor;

		WFIFOW(sd->fd, 27) = SWAP16(sd->gfx.weapon);
		WFIFOB(sd->fd, 29) = sd->gfx.cweapon;

		WFIFOW(sd->fd, 30) = SWAP16(sd->gfx.shield);
		WFIFOB(sd->fd, 32) = sd->gfx.cshield;

		if (sd->gfx.helm == 65535) {
			WFIFOB(sd->fd, 33) = 0;
			WFIFOB(sd->fd, 34) = 0xFF;
			WFIFOB(sd->fd, 35) = 0;
		}
		else {
			WFIFOB(sd->fd, 33) = 1;
			WFIFOB(sd->fd, 34) = sd->gfx.helm;
			WFIFOB(sd->fd, 35) = sd->gfx.chelm;
		}

		WFIFOW(sd->fd, 36) = SWAP16(sd->gfx.faceAcc);
		WFIFOB(sd->fd, 38) = sd->gfx.cfaceAcc;

		if (sd->gfx.crown == 65535) {
			WFIFOW(sd->fd, 39) = 0xFFFF;
			WFIFOB(sd->fd, 41) = 0;
		}
		else {
			WFIFOB(sd->fd, 33) = 0;
			WFIFOW(sd->fd, 39) = SWAP16(sd->gfx.crown);
			WFIFOB(sd->fd, 41) = sd->gfx.ccrown;
		}

		WFIFOW(sd->fd, 42) = SWAP16(sd->gfx.faceAccT);
		WFIFOB(sd->fd, 44) = sd->gfx.cfaceAccT;

		WFIFOW(sd->fd, 45) = SWAP16(sd->gfx.mantle);
		WFIFOB(sd->fd, 47) = sd->gfx.cmantle;

		WFIFOW(sd->fd, 48) = SWAP16(sd->gfx.necklace);
		WFIFOB(sd->fd, 50) = sd->gfx.cnecklace;

		WFIFOW(sd->fd, 51) = SWAP16(sd->gfx.boots);
		WFIFOB(sd->fd, 53) = sd->gfx.cboots;

		WFIFOB(sd->fd, 55) = 0;
		WFIFOB(sd->fd, 56) = 1;
		WFIFOW(sd->fd, 55) = SWAP16(graphic_id);
		WFIFOB(sd->fd, 59) = color;
		WFIFOL(sd->fd, 60) = SWAP32(1);
		WFIFOB(sd->fd, 64) = previous;
		WFIFOB(sd->fd, 65) = next;
		WFIFOW(sd->fd, 66) = SWAP16(strlen(dialog));
		strcpy(WFIFOP(sd->fd, 68), dialog);
		len = strlen(dialog) + 68;
		WFIFOB(sd->fd, len) = size;
		len += 1;
		for (x = 1; x < size + 1; x++) {
			WFIFOB(sd->fd, len) = strlen(menu[x]);
			strcpy(WFIFOP(sd->fd, len + 1), menu[x]);
			len += strlen(menu[x]) + 1;
		}

		WFIFOW(sd->fd, 1) = SWAP16(len + 68);
	}

	WFIFOSET(sd->fd, client_encrypt(sd->fd));
	return 0;
}

/* Input sequence dialog */

int client_input_seq(USER* sd, int id, char* dialog, char* dialog2, char* dialog3, char* menu[], int size, int previous, int next) {
	int graphic_id = sd->npc_g;
	int color = sd->npc_gc;
	int x;
	int len = 0;

	NPC* nd = map_id2npc((unsigned int)id);
	int type = sd->dialogtype;

	if (nd) {
		nd->lastaction = time(NULL);
	}

	if (!session[sd->fd])
	{
		session[sd->fd]->eof = 8;
		return 0;
	}

	WFIFOHEAD(sd->fd, 65535);
	WFIFOB(sd->fd, 0) = 0xAA;
	WFIFOB(sd->fd, 3) = 0x30;
	WFIFOB(sd->fd, 4) = 0x5C;
	WFIFOL(sd->fd, 7) = SWAP32(id);

	WFIFOB(sd->fd, 5) = 0x04;
	WFIFOB(sd->fd, 6) = 0x04;

	if (graphic_id == 0)
		WFIFOB(sd->fd, 11) = 0;
	else if (graphic_id >= 49152)
		WFIFOB(sd->fd, 11) = 2;
	else
		WFIFOB(sd->fd, 11) = 1;

	WFIFOB(sd->fd, 12) = 1;
	WFIFOW(sd->fd, 13) = SWAP16(graphic_id);

	WFIFOB(sd->fd, 15) = color;
	WFIFOB(sd->fd, 16) = 1;
	WFIFOW(sd->fd, 17) = SWAP16(graphic_id);
	WFIFOB(sd->fd, 19) = color;
	WFIFOL(sd->fd, 20) = SWAP32(1);
	WFIFOB(sd->fd, 24) = previous;
	WFIFOB(sd->fd, 25) = next;

	WFIFOW(sd->fd, 26) = SWAP16(strlen(dialog));
	strcpy(WFIFOP(sd->fd, 28), dialog);
	len += strlen(dialog) + 28;

	WFIFOB(sd->fd, len) = strlen(dialog2);
	strcpy(WFIFOP(sd->fd, len + 1), dialog2);
	len += strlen(dialog2) + 1;

	WFIFOB(sd->fd, len) = 42;
	len += 1;
	WFIFOB(sd->fd, len) = strlen(dialog3);
	strcpy(WFIFOP(sd->fd, len + 1), dialog3);
	len += strlen(dialog3) + 3;

	WFIFOW(sd->fd, 1) = SWAP16(len);

	WFIFOSET(sd->fd, client_encrypt(sd->fd));

	return 0;
}

/* Buy dialog */

int client_buy_dialog(USER* sd, unsigned int id, char* dialog, struct item* item, int price[], int count) {
	NPC* nd = NULL;
	int graphic = sd->npc_g;
	int color = sd->npc_gc;
	int x = 0;
	int len;
	int i;

	char name[64];
	char buff[64];

	if (!session[sd->fd])
	{
		session[sd->fd]->eof = 8;
		return 0;
	}

	WFIFOHEAD(sd->fd, 65535);
	WFIFOB(sd->fd, 0) = 0xAA;
	WFIFOB(sd->fd, 3) = 47;
	WFIFOB(sd->fd, 5) = 4;
	WFIFOB(sd->fd, 6) = 2;
	WFIFOL(sd->fd, 7) = SWAP32(id);

	if (graphic > 0) {
		if (graphic > 49152)
			WFIFOB(sd->fd, 11) = 2;
		else
			WFIFOB(sd->fd, 11) = 1;

		WFIFOB(sd->fd, 12) = 1;
		WFIFOW(sd->fd, 13) = SWAP16(graphic);
		WFIFOB(sd->fd, 15) = color;
		WFIFOB(sd->fd, 16) = 1;
		WFIFOW(sd->fd, 17) = SWAP16(graphic);
		WFIFOB(sd->fd, 19) = color;

		WFIFOW(sd->fd, 20) = SWAP16(strlen(dialog));
		strcpy(WFIFOP(sd->fd, 22), dialog);
		len = strlen(dialog);
		WFIFOW(sd->fd, len + 22) = strlen(dialog);
		len += 2;
		WFIFOW(sd->fd, len + 22) = SWAP16(count);
		len += 2;

		for (x = 0; x < count; x++) {
			memset(name, 0, 64);

			if (item[x].customIcon > 0)
			{
				WFIFOW(sd->fd, len + 22) = SWAP16(item[x].customIcon + 49152);
				WFIFOB(sd->fd, len + 24) = item[x].customIconColor;
			}
			else {
				WFIFOW(sd->fd, len + 22) = SWAP16(itemdb_icon(item[x].id));
				WFIFOB(sd->fd, len + 24) = itemdb_iconcolor(item[x].id);
			}

			len += 3;
			WFIFOL(sd->fd, len + 22) = SWAP32(price[x]);
			len += 4;

			if (strcmp(item[x].real_name, "") != 0) {
				sprintf(name, "%s", item[x].real_name);
			}
			else
			{
				sprintf(name, "%s", itemdb_name(item[x].id));
			}

			if (item[x].owner != 0) {
				sprintf(name + strlen(name), " - BONDED");
			}

			WFIFOB(sd->fd, len + 22) = strlen(name);
			strcpy(WFIFOP(sd->fd, len + 23), name);
			len += strlen(name) + 1;

			if (strcmp(itemdb_buytext(item[x].id), "") != 0) {
				strcpy(buff, itemdb_buytext(item[x].id));
			}
			else if (strcmp(item[x].buytext, "") != 0) {
				strcpy(buff, item[x].buytext);
			}
			else {
				char* path = classdb_name(itemdb_class(item[x].id), itemdb_rank(item[x].id));
				sprintf(buff, "%s level %u", path, itemdb_level(item[x].id));
			}

			WFIFOB(sd->fd, len + 22) = strlen(buff);
			memcpy(WFIFOP(sd->fd, len + 23), buff, strlen(buff));
			len += strlen(buff) + 1;
		}

		WFIFOW(sd->fd, 1) = SWAP16(len + 19);
		WFIFOSET(sd->fd, client_encrypt(sd->fd));
	}
	else {
		nd = map_id2npc(id);
		WFIFOB(sd->fd, 11) = 1;
		WFIFOW(sd->fd, 12) = SWAP16(nd->sex);
		WFIFOB(sd->fd, 14) = nd->state;
		WFIFOB(sd->fd, 15) = 0;
		WFIFOW(sd->fd, 16) = SWAP16(nd->equip[EQ_ARMOR].id);
		WFIFOB(sd->fd, 18) = 0;
		WFIFOB(sd->fd, 19) = nd->face;
		WFIFOB(sd->fd, 20) = nd->hair;
		WFIFOB(sd->fd, 21) = nd->hair_color;
		WFIFOB(sd->fd, 22) = nd->face_color;
		WFIFOB(sd->fd, 23) = nd->skin_color;

		if (!nd->equip[EQ_ARMOR].id) {
			WFIFOW(sd->fd, 24) = 0xFFFF;
			WFIFOB(sd->fd, 26) = 0;
		}
		else {
			WFIFOW(sd->fd, 24) = SWAP16(nd->equip[EQ_ARMOR].id);
			if (nd->armor_color) {
				WFIFOB(sd->fd, 26) = nd->armor_color;
			}
			else {
				WFIFOB(sd->fd, 26) = nd->equip[EQ_ARMOR].customLookColor;
			}
		}

		if (nd->equip[EQ_COAT].id) {
			WFIFOW(sd->fd, 24) = SWAP16(nd->equip[EQ_COAT].id);
			WFIFOB(sd->fd, 26) = nd->equip[EQ_COAT].customLookColor;
		}

		if (!nd->equip[EQ_WEAP].id) {
			WFIFOW(sd->fd, 27) = 0xFFFF;
			WFIFOB(sd->fd, 29) = 0;
		}
		else {
			WFIFOW(sd->fd, 27) = SWAP16(nd->equip[EQ_WEAP].id);
			WFIFOB(sd->fd, 29) = nd->equip[EQ_WEAP].customLookColor;
		}

		if (!nd->equip[EQ_SHIELD].id) {
			WFIFOW(sd->fd, 30) = 0xFFFF;
			WFIFOB(sd->fd, 32) = 0;
		}
		else {
			WFIFOW(sd->fd, 30) = SWAP16(nd->equip[EQ_SHIELD].id);
			WFIFOB(sd->fd, 32) = nd->equip[EQ_SHIELD].customLookColor;
		}

		if (!nd->equip[EQ_HELM].id) {
			WFIFOB(sd->fd, 33) = 0;
			WFIFOW(sd->fd, 34) = 0xFF;
			WFIFOB(sd->fd, 35) = 0;
		}
		else {
			WFIFOB(sd->fd, 33) = 1;
			WFIFOW(sd->fd, 34) = nd->equip[EQ_HELM].id;
			WFIFOB(sd->fd, 35) = nd->equip[EQ_HELM].customLookColor;
		}

		if (!nd->equip[EQ_FACEACC].id) {
			WFIFOW(sd->fd, 36) = 0xFFFF;
			WFIFOB(sd->fd, 38) = 0;
		}
		else {
			WFIFOW(sd->fd, 36) = SWAP16(nd->equip[EQ_FACEACC].id);
			WFIFOB(sd->fd, 38) = nd->equip[EQ_FACEACC].customLookColor;
		}

		if (!nd->equip[EQ_CROWN].id) {
			WFIFOW(sd->fd, 39) = 0xFFFF;
			WFIFOB(sd->fd, 41) = 0;
		}
		else {
			WFIFOB(sd->fd, 33) = 0;
			WFIFOW(sd->fd, 39) = SWAP16(nd->equip[EQ_CROWN].id);
			WFIFOB(sd->fd, 41) = nd->equip[EQ_CROWN].customLookColor;
		}

		if (!nd->equip[EQ_FACEACCTWO].id) {
			WFIFOW(sd->fd, 42) = 0xFFFF;
			WFIFOB(sd->fd, 44) = 0;
		}
		else {
			WFIFOW(sd->fd, 42) = SWAP16(nd->equip[EQ_FACEACCTWO].id);
			WFIFOB(sd->fd, 44) = nd->equip[EQ_FACEACCTWO].customLookColor;
		}

		if (!nd->equip[EQ_MANTLE].id) {
			WFIFOW(sd->fd, 45) = 0xFFFF;
			WFIFOB(sd->fd, 47) = 0;
		}
		else {
			WFIFOW(sd->fd, 45) = SWAP16(nd->equip[EQ_MANTLE].id);
			WFIFOB(sd->fd, 47) = nd->equip[EQ_MANTLE].customLookColor;
		}

		if (!nd->equip[EQ_NECKLACE].id) {
			WFIFOW(sd->fd, 48) = 0xFFFF;
			WFIFOB(sd->fd, 50) = 0;
		}
		else {
			WFIFOW(sd->fd, 48) = SWAP16(nd->equip[EQ_NECKLACE].id);
			WFIFOB(sd->fd, 50) = nd->equip[EQ_NECKLACE].customLookColor;
		}

		if (!nd->equip[EQ_BOOTS].id) {
			WFIFOW(sd->fd, 51) = SWAP16(nd->sex);
			WFIFOB(sd->fd, 53) = 0;
		}
		else {
			WFIFOW(sd->fd, 51) = SWAP16(nd->equip[EQ_BOOTS].id);
			WFIFOB(sd->fd, 53) = nd->equip[EQ_BOOTS].customLookColor;
		}

		WFIFOB(sd->fd, 54) = 1;
		WFIFOW(sd->fd, 55) = SWAP16(graphic);
		WFIFOB(sd->fd, 57) = color;

		WFIFOW(sd->fd, 60) = SWAP16(strlen(dialog));
		strcpy(WFIFOP(sd->fd, 62), dialog);
		len = strlen(dialog);
		WFIFOW(sd->fd, len + 62) = strlen(dialog);
		len += 2;
		WFIFOW(sd->fd, len + 62) = SWAP16(count);
		len += 2;

		for (x = 0; x < count; x++) {
			memset(name, 0, 64);

			if (item[x].customIcon > 0)
			{
				WFIFOW(sd->fd, len + 62) = SWAP16(item[x].customIcon);
				WFIFOB(sd->fd, len + 64) = item[x].customIconColor;
			}
			else {
				WFIFOW(sd->fd, len + 62) = SWAP16(itemdb_icon(item[x].id));
				WFIFOB(sd->fd, len + 64) = itemdb_iconcolor(item[x].id);
			}

			len += 3;
			WFIFOL(sd->fd, len + 62) = SWAP32(price[x]);
			len += 4;

			if (strcmp(item[x].real_name, "") != 0) {
				strcpy(name, item[x].real_name);
			}
			else
			{
				strcpy(name, itemdb_name(item[x].id));
			}

			if (item[x].owner != 0) {
				sprintf(name + strlen(name), " - BONDED");
			}

			WFIFOB(sd->fd, len + 62) = strlen(name);
			strcpy(WFIFOP(sd->fd, len + 63), name);
			len += strlen(name) + 1;

			if (strcmp(itemdb_buytext(item[x].id), "") != 0) {
				strcpy(buff, itemdb_buytext(item[x].id));
			}
			else if (strcmp(item[x].buytext, "") != 0) {
				strcpy(buff, item[x].buytext);
			}
			else {
				char* path = classdb_name(itemdb_class(item[x].id), itemdb_rank(item[x].id));
				sprintf(buff, "%s level %u", path, itemdb_level(item[x].id));
			}

			WFIFOB(sd->fd, len + 62) = strlen(buff);
			memcpy(WFIFOP(sd->fd, len + 63), buff, strlen(buff));
			len += strlen(buff) + 1;
		}
		WFIFOW(sd->fd, 1) = SWAP16(len + 63);
		WFIFOSET(sd->fd, client_encrypt(sd->fd));
	}

	FREE(item);
	return 0;
}

/* Sell dialog */

int client_sell_dialog(USER* sd, unsigned int id, char* dialog, int item[], int count) {
	NPC* nd = NULL;
	int graphic = sd->npc_g;
	int color = sd->npc_gc;
	int x;
	int len;
	int i;

	if (!session[sd->fd])
	{
		session[sd->fd]->eof = 8;
		return 0;
	}

	WFIFOHEAD(sd->fd, 65535);
	WFIFOB(sd->fd, 0) = 0xAA;
	WFIFOB(sd->fd, 3) = 47;
	WFIFOB(sd->fd, 4) = 3;
	WFIFOB(sd->fd, 5) = 5;
	WFIFOB(sd->fd, 6) = 4;
	WFIFOL(sd->fd, 7) = SWAP32(id);

	if (graphic > 0) {
		if (graphic > 49152) {
			WFIFOB(sd->fd, 11) = 2;
		}
		else {
			WFIFOB(sd->fd, 11) = 1;
		}
		WFIFOB(sd->fd, 12) = 1;
		WFIFOW(sd->fd, 13) = SWAP16(graphic);
		WFIFOB(sd->fd, 15) = color;
		WFIFOB(sd->fd, 16) = 1;
		WFIFOW(sd->fd, 17) = SWAP16(graphic);
		WFIFOB(sd->fd, 19) = color;

		WFIFOW(sd->fd, 20) = SWAP16(strlen(dialog));
		strcpy(WFIFOP(sd->fd, 22), dialog);
		len = strlen(dialog) + 2;
		WFIFOW(sd->fd, len + 20) = SWAP16(strlen(dialog));
		len += 2;
		WFIFOB(sd->fd, len + 20) = count;
		len += 1;
		for (i = 0; i < count; i++) {
			WFIFOB(sd->fd, len + 20) = item[i] + 1;
			len += 1;
		}
		WFIFOW(sd->fd, 1) = SWAP16(len + 17);
		WFIFOSET(sd->fd, client_encrypt(sd->fd));
	}
	else {
		nd = map_id2npc(id);
		WFIFOB(sd->fd, 11) = 1;
		WFIFOW(sd->fd, 12) = SWAP16(nd->sex);
		WFIFOB(sd->fd, 14) = nd->state;
		WFIFOB(sd->fd, 15) = 0;
		WFIFOW(sd->fd, 16) = SWAP16(nd->equip[EQ_ARMOR].id);
		WFIFOB(sd->fd, 18) = 0;
		WFIFOB(sd->fd, 19) = nd->face;
		WFIFOB(sd->fd, 20) = nd->hair;
		WFIFOB(sd->fd, 21) = nd->hair_color;
		WFIFOB(sd->fd, 22) = nd->face_color;
		WFIFOB(sd->fd, 23) = nd->skin_color;

		if (!nd->equip[EQ_ARMOR].id) {
			WFIFOW(sd->fd, 24) = 0xFFFF;
			WFIFOB(sd->fd, 26) = 0;
		}
		else {
			WFIFOW(sd->fd, 24) = SWAP16(nd->equip[EQ_ARMOR].id);
			if (nd->armor_color) {
				WFIFOB(sd->fd, 26) = nd->armor_color;
			}
			else {
				WFIFOB(sd->fd, 26) = nd->equip[EQ_ARMOR].customLookColor;
			}
		}

		if (nd->equip[EQ_COAT].id) {
			WFIFOW(sd->fd, 24) = SWAP16(nd->equip[EQ_COAT].id);
			WFIFOB(sd->fd, 26) = nd->equip[EQ_COAT].customLookColor;
		}

		if (!nd->equip[EQ_WEAP].id) {
			WFIFOW(sd->fd, 27) = 0xFFFF;
			WFIFOB(sd->fd, 29) = 0;
		}
		else {
			WFIFOW(sd->fd, 27) = SWAP16(nd->equip[EQ_WEAP].id);
			WFIFOB(sd->fd, 29) = nd->equip[EQ_WEAP].customLookColor;
		}

		if (!nd->equip[EQ_SHIELD].id) {
			WFIFOW(sd->fd, 30) = 0xFFFF;
			WFIFOB(sd->fd, 32) = 0;
		}
		else {
			WFIFOW(sd->fd, 30) = SWAP16(nd->equip[EQ_SHIELD].id);
			WFIFOB(sd->fd, 32) = nd->equip[EQ_SHIELD].customLookColor;
		}

		if (!nd->equip[EQ_HELM].id) {
			WFIFOB(sd->fd, 33) = 0;
			WFIFOW(sd->fd, 34) = 0xFF;
			WFIFOB(sd->fd, 35) = 0;
		}
		else {
			WFIFOB(sd->fd, 33) = 1;
			WFIFOW(sd->fd, 34) = nd->equip[EQ_HELM].id;
			WFIFOB(sd->fd, 35) = nd->equip[EQ_HELM].customLookColor;
		}

		if (!nd->equip[EQ_FACEACC].id) {
			WFIFOW(sd->fd, 36) = 0xFFFF;
			WFIFOB(sd->fd, 38) = 0;
		}
		else {
			WFIFOW(sd->fd, 36) = SWAP16(nd->equip[EQ_FACEACC].id);
			WFIFOB(sd->fd, 38) = nd->equip[EQ_FACEACC].customLookColor;
		}

		if (!nd->equip[EQ_CROWN].id) {
			WFIFOW(sd->fd, 39) = 0xFFFF;
			WFIFOB(sd->fd, 41) = 0;
		}
		else {
			WFIFOB(sd->fd, 33) = 0;
			WFIFOW(sd->fd, 39) = SWAP16(nd->equip[EQ_CROWN].id);
			WFIFOB(sd->fd, 41) = nd->equip[EQ_CROWN].customLookColor;
		}

		if (!nd->equip[EQ_FACEACCTWO].id) {
			WFIFOW(sd->fd, 42) = 0xFFFF;
			WFIFOB(sd->fd, 44) = 0;
		}
		else {
			WFIFOW(sd->fd, 42) = SWAP16(nd->equip[EQ_FACEACCTWO].id);
			WFIFOB(sd->fd, 44) = nd->equip[EQ_FACEACCTWO].customLookColor;
		}

		if (!nd->equip[EQ_MANTLE].id) {
			WFIFOW(sd->fd, 45) = 0xFFFF;
			WFIFOB(sd->fd, 47) = 0;
		}
		else {
			WFIFOW(sd->fd, 45) = SWAP16(nd->equip[EQ_MANTLE].id);
			WFIFOB(sd->fd, 47) = nd->equip[EQ_MANTLE].customLookColor;
		}

		if (!nd->equip[EQ_NECKLACE].id) {
			WFIFOW(sd->fd, 48) = 0xFFFF;
			WFIFOB(sd->fd, 50) = 0;
		}
		else {
			WFIFOW(sd->fd, 48) = SWAP16(nd->equip[EQ_NECKLACE].id);
			WFIFOB(sd->fd, 50) = nd->equip[EQ_NECKLACE].customLookColor;
		}

		if (!nd->equip[EQ_BOOTS].id) {
			WFIFOW(sd->fd, 51) = SWAP16(nd->sex);
			WFIFOB(sd->fd, 53) = 0;
		}
		else {
			WFIFOW(sd->fd, 51) = SWAP16(nd->equip[EQ_BOOTS].id);
			WFIFOB(sd->fd, 53) = nd->equip[EQ_BOOTS].customLookColor;
		}

		WFIFOB(sd->fd, 54) = 1;
		WFIFOW(sd->fd, 55) = SWAP16(graphic);
		WFIFOB(sd->fd, 57) = color;

		WFIFOW(sd->fd, 60) = SWAP16(strlen(dialog));
		strcpy(WFIFOP(sd->fd, 62), dialog);
		len = strlen(dialog);
		WFIFOW(sd->fd, len + 62) = strlen(dialog);
		len += 2;
		WFIFOB(sd->fd, len + 62) = count;
		len += 1;

		for (i = 0; i < count; i++) {
			WFIFOB(sd->fd, len + 62) = item[i] + 1;
			len += 1;
		}

		WFIFOW(sd->fd, 1) = SWAP16(len + 62);
		WFIFOSET(sd->fd, client_encrypt(sd->fd));
	}
	return 0;
}

/* Input dialog */

int client_input(USER* sd, int id, char* dialog, char* item) {
	int graphic = sd->npc_g;
	int color = sd->npc_gc;

	int x, i, len;
	NPC* nd = map_id2npc((unsigned int)id);
	int type = sd->dialogtype;

	if (nd) {
		nd->lastaction = time(NULL);
	}

	if (!session[sd->fd])
	{
		session[sd->fd]->eof = 8;
		return 0;
	}

	WFIFOHEAD(sd->fd, 1000);
	WFIFOB(sd->fd, 0) = 0xAA;
	WFIFOB(sd->fd, 3) = 0x2F;
	WFIFOB(sd->fd, 5) = 3;
	WFIFOB(sd->fd, 6) = 3;
	WFIFOL(sd->fd, 7) = SWAP32(id);

	if (type == 0) {
		if (graphic == 0)
			WFIFOB(sd->fd, 11) = 0;
		else if (graphic >= 49152) {
			WFIFOB(sd->fd, 11) = 2;
		}
		else {
			WFIFOB(sd->fd, 11) = 1;
		}
		WFIFOB(sd->fd, 12) = 1;
		WFIFOW(sd->fd, 13) = SWAP16(graphic);
		WFIFOB(sd->fd, 15) = color;
		WFIFOB(sd->fd, 16) = 1;
		WFIFOW(sd->fd, 17) = SWAP16(graphic);
		WFIFOB(sd->fd, 19) = color;

		WFIFOW(sd->fd, 20) = SWAP16(strlen(dialog));
		strcpy(WFIFOP(sd->fd, 22), dialog);
		len = strlen(dialog);
		WFIFOB(sd->fd, len + 22) = strlen(item);
		len += 1;
		strcpy(WFIFOP(sd->fd, len + 23), item);
		len += strlen(item) + 1;
		WFIFOW(sd->fd, len + 22) = SWAP16(76);
		len += 2;

		WFIFOW(sd->fd, 1) = SWAP16(len + 19);
		WFIFOSET(sd->fd, client_encrypt(sd->fd));
	}
	else if (type == 1) {
		WFIFOB(sd->fd, 11) = 1;
		WFIFOW(sd->fd, 12) = SWAP16(nd->sex);
		WFIFOB(sd->fd, 14) = nd->state;
		WFIFOB(sd->fd, 15) = 0;
		WFIFOW(sd->fd, 16) = SWAP16(nd->equip[EQ_ARMOR].id);
		WFIFOB(sd->fd, 18) = 0;
		WFIFOB(sd->fd, 19) = nd->face;
		WFIFOB(sd->fd, 20) = nd->hair;
		WFIFOB(sd->fd, 21) = nd->hair_color;
		WFIFOB(sd->fd, 22) = nd->face_color;
		WFIFOB(sd->fd, 23) = nd->skin_color;

		if (!nd->equip[EQ_ARMOR].id) {
			WFIFOW(sd->fd, 24) = 0xFFFF;
			WFIFOB(sd->fd, 26) = 0;
		}
		else {
			WFIFOW(sd->fd, 24) = SWAP16(nd->equip[EQ_ARMOR].id);
			if (nd->armor_color) {
				WFIFOB(sd->fd, 26) = nd->armor_color;
			}
			else {
				WFIFOB(sd->fd, 26) = nd->equip[EQ_ARMOR].customLookColor;
			}
		}

		if (nd->equip[EQ_COAT].id) {
			WFIFOW(sd->fd, 24) = SWAP16(nd->equip[EQ_COAT].id);
			WFIFOB(sd->fd, 26) = nd->equip[EQ_COAT].customLookColor;
		}

		if (!nd->equip[EQ_WEAP].id) {
			WFIFOW(sd->fd, 27) = 0xFFFF;
			WFIFOB(sd->fd, 29) = 0;
		}
		else {
			WFIFOW(sd->fd, 27) = SWAP16(nd->equip[EQ_WEAP].id);
			WFIFOB(sd->fd, 29) = nd->equip[EQ_WEAP].customLookColor;
		}

		if (!nd->equip[EQ_SHIELD].id) {
			WFIFOW(sd->fd, 30) = 0xFFFF;
			WFIFOB(sd->fd, 32) = 0;
		}
		else {
			WFIFOW(sd->fd, 30) = SWAP16(nd->equip[EQ_SHIELD].id);
			WFIFOB(sd->fd, 32) = nd->equip[EQ_SHIELD].customLookColor;
		}

		if (!nd->equip[EQ_HELM].id) {
			WFIFOB(sd->fd, 33) = 0;
			WFIFOB(sd->fd, 34) = 0xFF;
			WFIFOB(sd->fd, 35) = 0;
		}
		else {
			WFIFOB(sd->fd, 33) = 1;
			WFIFOB(sd->fd, 34) = nd->equip[EQ_HELM].id;
			WFIFOB(sd->fd, 35) = nd->equip[EQ_HELM].customLookColor;
		}

		if (!nd->equip[EQ_FACEACC].id) {
			WFIFOW(sd->fd, 36) = 0xFFFF;
			WFIFOB(sd->fd, 38) = 0;
		}
		else {
			WFIFOW(sd->fd, 36) = SWAP16(nd->equip[EQ_FACEACC].id);
			WFIFOB(sd->fd, 38) = nd->equip[EQ_FACEACC].customLookColor;
		}

		if (!nd->equip[EQ_CROWN].id) {
			WFIFOW(sd->fd, 39) = 0xFFFF;
			WFIFOB(sd->fd, 41) = 0;
		}
		else {
			WFIFOB(sd->fd, 33) = 0;
			WFIFOW(sd->fd, 39) = SWAP16(nd->equip[EQ_CROWN].id);
			WFIFOB(sd->fd, 41) = nd->equip[EQ_CROWN].customLookColor;
		}

		if (!nd->equip[EQ_FACEACCTWO].id) {
			WFIFOW(sd->fd, 42) = 0xFFFF;
			WFIFOB(sd->fd, 44) = 0;
		}
		else {
			WFIFOW(sd->fd, 42) = SWAP16(nd->equip[EQ_FACEACCTWO].id);
			WFIFOB(sd->fd, 44) = nd->equip[EQ_FACEACCTWO].customLookColor;
		}

		if (!nd->equip[EQ_MANTLE].id) {
			WFIFOW(sd->fd, 45) = 0xFFFF;
			WFIFOB(sd->fd, 47) = 0;
		}
		else {
			WFIFOW(sd->fd, 45) = SWAP16(nd->equip[EQ_MANTLE].id);
			WFIFOB(sd->fd, 47) = nd->equip[EQ_MANTLE].customLookColor;
		}

		if (!nd->equip[EQ_NECKLACE].id) {
			WFIFOW(sd->fd, 48) = 0xFFFF;
			WFIFOB(sd->fd, 50) = 0;
		}
		else {
			WFIFOW(sd->fd, 48) = SWAP16(nd->equip[EQ_NECKLACE].id);
			WFIFOB(sd->fd, 50) = nd->equip[EQ_NECKLACE].customLookColor;
		}

		if (!nd->equip[EQ_BOOTS].id) {
			WFIFOW(sd->fd, 51) = SWAP16(nd->sex);
			WFIFOB(sd->fd, 53) = 0;
		}
		else {
			WFIFOW(sd->fd, 51) = SWAP16(nd->equip[EQ_BOOTS].id);
			WFIFOB(sd->fd, 53) = nd->equip[EQ_BOOTS].customLookColor;
		}

		WFIFOB(sd->fd, 54) = 1;
		WFIFOW(sd->fd, 55) = SWAP16(graphic);
		WFIFOB(sd->fd, 57) = color;
		WFIFOW(sd->fd, 58) = SWAP16(strlen(dialog));
		strcpy(WFIFOP(sd->fd, 60), dialog);
		len = strlen(dialog);
		WFIFOB(sd->fd, len + 60) = strlen(item);
		len += 1;
		strcpy(WFIFOP(sd->fd, len + 61), item);
		len += strlen(item) + 1;
		WFIFOW(sd->fd, len + 60) = SWAP16(76);
		len += 2;

		WFIFOW(sd->fd, 1) = SWAP16(len + 57);
		WFIFOSET(sd->fd, client_encrypt(sd->fd));
	}
	else if (type == 2) {
		WFIFOB(sd->fd, 11) = 1;
		WFIFOW(sd->fd, 12) = SWAP16(nd->sex);
		WFIFOB(sd->fd, 14) = nd->state;
		WFIFOB(sd->fd, 15) = 0;
		WFIFOW(sd->fd, 16) = SWAP16(nd->gfx.armor);
		WFIFOB(sd->fd, 18) = 0;
		WFIFOB(sd->fd, 19) = nd->gfx.face;
		WFIFOB(sd->fd, 20) = nd->gfx.hair;
		WFIFOB(sd->fd, 21) = nd->gfx.chair;
		WFIFOB(sd->fd, 22) = nd->gfx.cface;
		WFIFOB(sd->fd, 23) = nd->gfx.cskin;

		WFIFOW(sd->fd, 24) = SWAP16(nd->gfx.armor);
		WFIFOB(sd->fd, 26) = nd->gfx.carmor;

		WFIFOW(sd->fd, 27) = SWAP16(nd->gfx.weapon);
		WFIFOB(sd->fd, 29) = nd->gfx.cweapon;

		WFIFOW(sd->fd, 30) = SWAP16(nd->gfx.shield);
		WFIFOB(sd->fd, 32) = nd->gfx.cshield;

		if (nd->gfx.helm == 65535) {
			WFIFOB(sd->fd, 33) = 0;
			WFIFOB(sd->fd, 34) = 0xFF;
			WFIFOB(sd->fd, 35) = 0;
		}
		else {
			WFIFOB(sd->fd, 33) = 1;
			WFIFOB(sd->fd, 34) = nd->gfx.helm;
			WFIFOB(sd->fd, 35) = nd->gfx.chelm;
		}

		WFIFOW(sd->fd, 36) = SWAP16(nd->gfx.faceAcc);
		WFIFOB(sd->fd, 38) = nd->gfx.cfaceAcc;

		if (nd->gfx.crown == 65535) {
			WFIFOW(sd->fd, 39) = 0xFFFF;
			WFIFOB(sd->fd, 41) = 0;
		}
		else {
			WFIFOB(sd->fd, 33) = 0;
			WFIFOW(sd->fd, 39) = SWAP16(nd->gfx.crown);
			WFIFOB(sd->fd, 41) = nd->gfx.ccrown;
		}

		WFIFOW(sd->fd, 42) = SWAP16(nd->gfx.faceAccT);
		WFIFOB(sd->fd, 44) = nd->gfx.cfaceAccT;

		WFIFOW(sd->fd, 45) = SWAP16(nd->gfx.mantle);
		WFIFOB(sd->fd, 47) = nd->gfx.cmantle;

		WFIFOW(sd->fd, 48) = SWAP16(nd->gfx.necklace);
		WFIFOB(sd->fd, 50) = nd->gfx.cnecklace;

		WFIFOW(sd->fd, 51) = SWAP16(nd->gfx.boots);
		WFIFOB(sd->fd, 53) = nd->gfx.cboots;

		WFIFOB(sd->fd, 54) = 1;
		WFIFOW(sd->fd, 55) = SWAP16(graphic);
		WFIFOB(sd->fd, 57) = color;
		WFIFOW(sd->fd, 58) = SWAP16(strlen(dialog));
		strcpy(WFIFOP(sd->fd, 60), dialog);
		len = strlen(dialog);
		WFIFOB(sd->fd, len + 60) = strlen(item);
		len += 1;
		strcpy(WFIFOP(sd->fd, len + 61), item);
		len += strlen(item) + 1;
		WFIFOW(sd->fd, len + 60) = SWAP16(76);
		len += 2;

		WFIFOW(sd->fd, 1) = SWAP16(len + 57);
		WFIFOSET(sd->fd, client_encrypt(sd->fd));
	}

	return 0;
}

/* Hair/face menu - stub (declared but not commonly used) */

int client_hair_face_menu(USER* sd, char* dialog, char* menu[], int size) {
	return 0;
}

/* Click on player - this will be moved to client_player.c eventually */

int client_click_on_player(USER* sd, struct block_list* bl) {
	/* This function is large and player-related, will remain in clif.c for now
	 * and be moved to client_player.c in the next phase */
	return 0;
}
