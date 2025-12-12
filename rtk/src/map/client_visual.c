/**
 * @file client_visual.c
 * @brief Visual updates, animations, look packets, and movement display
 *
 * Extracted from clif.c as part of the RTK refactoring project.
 * Handles all visual/graphical updates sent to clients.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include "client_visual.h"
#include "client_crypto.h"
#include "client.h"
#include "core.h"
#include "map.h"
#include "socket.h"
#include "malloc.h"
#include "mmo.h"
#include "pc.h"
#include "mob.h"
#include "npc.h"
#include "itemdb.h"
#include "sl.h"

/* ========== Animation Functions ========== */

int client_send_animation_xy(struct block_list* bl, va_list ap) {
	USER* src = NULL;
	int anim, times, x, y;

	anim = va_arg(ap, int);
	times = va_arg(ap, int);
	x = va_arg(ap, int);
	y = va_arg(ap, int);
	nullpo_ret(0, src = (USER*)bl);

	if (!session[src->fd])
	{
		session[src->fd]->eof = 8;
		return 0;
	}

	WFIFOHEAD(src->fd, 0x30);
	WFIFOB(src->fd, 0) = 0xAA;
	WFIFOW(src->fd, 1) = SWAP16(14);
	WFIFOB(src->fd, 3) = 0x29;
	WFIFOL(src->fd, 5) = 0;
	WFIFOW(src->fd, 9) = SWAP16(anim);
	WFIFOW(src->fd, 11) = SWAP16(times);
	WFIFOW(src->fd, 13) = SWAP16(x);
	WFIFOW(src->fd, 15) = SWAP16(y);
	WFIFOSET(src->fd, client_encrypt(src->fd));
	return 0;
}

int client_send_animation(struct block_list* bl, va_list ap) {
	struct block_list* t = NULL;
	USER* sd = NULL;
	int anim, times;

	anim = va_arg(ap, int);
	nullpo_ret(0, t = va_arg(ap, struct block_list*));
	nullpo_ret(0, sd = (USER*)bl);
	times = va_arg(ap, int);

	if (sd->status.settingFlags & FLAG_MAGIC) {
		if (!session[sd->fd])
		{
			session[sd->fd]->eof = 8;
			return 0;
		}

		WFIFOHEAD(sd->fd, 13);
		WFIFOB(sd->fd, 0) = 0xAA;
		WFIFOW(sd->fd, 1) = SWAP16(0x0A);
		WFIFOB(sd->fd, 3) = 0x29;
		WFIFOL(sd->fd, 5) = SWAP32(t->id);
		WFIFOW(sd->fd, 9) = SWAP16(anim);
		WFIFOW(sd->fd, 11) = SWAP16(times);
		WFIFOSET(sd->fd, client_encrypt(sd->fd));
	}

	return 0;
}

int client_animation(USER* src, USER* sd, int animation, int duration) {
	if (!session[sd->fd])
	{
		session[sd->fd]->eof = 8;
		return 0;
	}

	WFIFOHEAD(src->fd, 0x0A + 3);
	if (src->status.settingFlags & FLAG_MAGIC) {
		WFIFOB(src->fd, 0) = 0xAA;
		WFIFOW(src->fd, 1) = SWAP16(0x0A);
		WFIFOB(src->fd, 3) = 0x29;
		WFIFOB(src->fd, 4) = 0x03;
		WFIFOL(src->fd, 5) = SWAP32(sd->bl.id);
		WFIFOW(src->fd, 9) = SWAP16(animation);
		WFIFOW(src->fd, 11) = SWAP16(duration / 1000);
		WFIFOSET(src->fd, client_encrypt(src->fd));
	}
	return 0;
}

int client_send_animations(USER* src, USER* sd) {
	int x;
	for (x = 0; x < MAX_MAGIC_TIMERS; x++) {
		if (sd->status.dura_aether[x].duration > 0 && sd->status.dura_aether[x].animation) {
			client_animation(src, sd, sd->status.dura_aether[x].animation, sd->status.dura_aether[x].duration);
		}
	}
	return 0;
}

/* ========== Action Functions ========== */

int client_send_action(struct block_list* bl, int type, int time, int sound) {
	USER* sd = NULL;
	unsigned char buf[32];

	WBUFB(buf, 0) = 0xAA;
	WBUFB(buf, 1) = 0x00;
	WBUFB(buf, 2) = 0x0B;
	WBUFB(buf, 3) = 0x1A;
	WBUFL(buf, 5) = SWAP32(bl->id);
	WBUFB(buf, 9) = type;
	WBUFB(buf, 10) = 0x00;
	WBUFB(buf, 11) = time;
	WBUFW(buf, 12) = 0x00;
	client_send(buf, 32, bl, SAMEAREA);

	if (sound > 0) client_play_sound(bl, sound);

	if (bl->type == BL_PC) {
		sd = (USER*)bl;
		sd->action = type;
		sl_doscript_blargs("onAction", NULL, 1, &sd->bl);
	}

	return 0;
}

int client_send_mob_action(MOB* mob, int type, int time, int sound) {
	unsigned char buf[32];
	WBUFB(buf, 0) = 0xAA;
	WBUFB(buf, 1) = 0x00;
	WBUFB(buf, 2) = 0x0B;
	WBUFB(buf, 3) = 0x1A;
	WBUFB(buf, 4) = 0x03;
	WBUFL(buf, 5) = SWAP32(mob->bl.id);
	WBUFB(buf, 9) = type;
	WBUFB(buf, 10) = 0x00;
	WBUFB(buf, 11) = time;
	WBUFB(buf, 12) = 0x00;
	client_send(buf, 32, &mob->bl, SAMEAREA);

	if (sound > 0) client_play_sound(&mob->bl, sound);

	return 0;
}

/* ========== Sound ========== */

int client_play_sound(struct block_list* bl, int sound) {
	unsigned char buf2[32];
	WBUFB(buf2, 0) = 0xAA;
	WBUFW(buf2, 1) = SWAP16(0x14);
	WBUFB(buf2, 3) = 0x19;
	WBUFB(buf2, 4) = 0x03;
	WBUFW(buf2, 5) = SWAP16(3);
	WBUFW(buf2, 7) = SWAP16(sound);
	WBUFB(buf2, 9) = 100;
	WBUFW(buf2, 10) = SWAP16(4);
	WBUFL(buf2, 12) = SWAP32(bl->id);
	WBUFB(buf2, 16) = 1;
	WBUFB(buf2, 17) = 0;
	WBUFB(buf2, 18) = 2;
	WBUFB(buf2, 19) = 2;
	WBUFW(buf2, 20) = SWAP16(4);
	WBUFB(buf2, 22) = 0;
	client_send(buf2, 32, bl, SAMEAREA);
	return 0;
}

/* ========== Look/Appearance ========== */

int client_look_gone(struct block_list* bl) {
	char buf[16];

	if (bl->type == BL_PC || (bl->type == BL_NPC && ((NPC*)bl)->npctype == 1) || (bl->type == BL_MOB))
	{
		WBUFB(buf, 0) = 0xAA;
		WBUFW(buf, 1) = SWAP16(6);
		WBUFB(buf, 3) = 0x0E;
		WBUFB(buf, 4) = 0x03;
		WBUFL(buf, 5) = SWAP32(bl->id);
		client_send(buf, 16, bl, AREA_WOS);
	}
	else
	{
		WBUFB(buf, 0) = 0xAA;
		WBUFW(buf, 1) = SWAP16(6);
		WBUFB(buf, 3) = 0x5F;
		WBUFB(buf, 4) = 0x03;
		WBUFL(buf, 5) = SWAP32(bl->id);
		client_send(buf, 16, bl, AREA_WOS);
	}
	return 0;
}

int client_show_ghost(USER* sd, USER* tsd) {
	if (!sd->status.gm_level) {
		if (!map[sd->bl.m].show_ghosts && tsd->status.state == 1 && sd->bl.id != tsd->bl.id) {
			if (map[sd->bl.m].pvp) {
				if (sd->status.state == 1 && sd->optFlags & optFlag_ghosts) return 1;
				else return 0;
			}
			else return 1;
		}
	}

	return 1;
}

/* ========== Side/Direction ========== */

int client_send_side(struct block_list* bl) {
	unsigned char buf[32];
	USER* sd = NULL;
	MOB* mob = NULL;
	NPC* nd = NULL;

	if (bl->type == BL_PC) {
		nullpo_ret(0, sd = (USER*)bl);
	}
	else if (bl->type == BL_MOB) {
		nullpo_ret(0, mob = (MOB*)bl);
	}
	else if (bl->type == BL_NPC) {
		nullpo_ret(0, nd = (NPC*)bl);
	}

	WBUFB(buf, 0) = 0xAA;
	WBUFB(buf, 1) = 0x00;
	WBUFB(buf, 2) = 0x08;
	WBUFB(buf, 3) = 0x11;
	WBUFL(buf, 5) = SWAP32(bl->id);

	if (bl->type == BL_PC) {
		WBUFB(buf, 9) = sd->status.side;
	}
	else if (bl->type == BL_MOB) {
		WBUFB(buf, 9) = mob->side;
	}
	else if (bl->type == BL_NPC) {
		WBUFB(buf, 9) = nd->side;
	}

	WBUFB(buf, 10) = 0;

	if (bl->type == BL_PC) {
		client_send(buf, 32, bl, AREA);
	}
	else {
		client_send(buf, 32, bl, AREA_WOS);
	}
	return 0;
}

int client_send_mob_side(MOB* mob) {
	unsigned char buf[16];
	WBUFB(buf, 0) = 0xAA;
	WBUFB(buf, 1) = 0x00;
	WBUFB(buf, 2) = 0x07;
	WBUFB(buf, 3) = 0x11;
	WBUFB(buf, 4) = 0x03;
	WBUFL(buf, 5) = SWAP32(mob->bl.id);
	WBUFB(buf, 9) = mob->side;
	client_send(buf, 16, &mob->bl, AREA_WOS);
	return 0;
}

/* ========== Movement Display ========== */

int client_npc_move(struct block_list* bl, va_list ap) {
	char* buf;
	int type;
	USER* sd = NULL;
	NPC* nd = NULL;

	type = va_arg(ap, int);
	nullpo_ret(0, sd = (USER*)bl);
	nullpo_ret(0, nd = va_arg(ap, NPC*));

	CALLOC(buf, char, 32);
	WBUFB(buf, 0) = 0xAA;
	WBUFB(buf, 1) = 0x00;
	WBUFB(buf, 2) = 0x0C;
	WBUFB(buf, 3) = 0x0C;
	WBUFL(buf, 5) = SWAP32(nd->bl.id);
	WBUFW(buf, 9) = SWAP16(nd->bl.bx);
	WBUFW(buf, 11) = SWAP16(nd->bl.by);
	WBUFB(buf, 13) = nd->side;
	WBUFB(buf, 14) = 0x00;

	client_send(buf, 32, &nd->bl, AREA_WOS);
	FREE(buf);

	return 0;
}

int client_mob_move(struct block_list* bl, va_list ap) {
	int type;
	USER* sd = NULL;
	MOB* mob = NULL;
	type = va_arg(ap, int);

	if (type == LOOK_GET) {
		nullpo_ret(0, sd = va_arg(ap, USER*));
		nullpo_ret(0, mob = (MOB*)bl);
		if (mob->state == MOB_DEAD)
			return 0;
	}
	else {
		nullpo_ret(0, sd = (USER*)bl);
		nullpo_ret(0, mob = va_arg(ap, MOB*));
		if (mob->state == MOB_DEAD)
			return 0;
	}

	if (!session[sd->fd])
	{
		session[sd->fd]->eof = 8;
		return 0;
	}

	WFIFOHEAD(sd->fd, 14);
	WFIFOHEADER(sd->fd, 0x0C, 11);
	WFIFOL(sd->fd, 5) = SWAP32(mob->bl.id);
	WFIFOW(sd->fd, 9) = SWAP16(mob->bx);
	WFIFOW(sd->fd, 11) = SWAP16(mob->by);
	WFIFOB(sd->fd, 13) = mob->side;
	WFIFOSET(sd->fd, client_encrypt(sd->fd));
	return 0;
}

/* ========== Mob Look Functions ========== */

int client_mob_look_start_func(struct block_list* bl, va_list ap) {
	USER* sd = NULL;

	nullpo_ret(0, sd = (USER*)bl);
	sd->mob_len = 0;
	sd->mob_count = 0;
	sd->mob_item = 0;

	if (!session[sd->fd])
	{
		session[sd->fd]->eof = 8;
		return 0;
	}

	WFIFOHEAD(sd->fd, 65535);
	return 0;
}

int client_mob_look_close_func(struct block_list* bl, va_list ap) {
	USER* sd = NULL;

	nullpo_ret(0, sd = (USER*)bl);

	if (!sd->mob_count) return 0;
	if (!sd->mob_item) {
		WFIFOB(sd->fd, sd->mob_len + 7) = 0;
		sd->mob_len++;
	}
	WFIFOHEADER(sd->fd, 0x07, sd->mob_len + 4);
	WFIFOW(sd->fd, 5) = SWAP16(sd->mob_count);
	WFIFOSET(sd->fd, client_encrypt(sd->fd));
	sd->mob_len = 0;
	sd->mob_count = 0;

	return 0;
}

int client_mob_look_start(USER* sd) {
	sd->mob_count = 0;
	sd->mob_len = 0;
	sd->mob_item = 0;

	if (!session[sd->fd])
	{
		session[sd->fd]->eof = 8;
		return 0;
	}

	WFIFOHEAD(sd->fd, 65535);
	return 0;
}

int client_mob_look_close(USER* sd) {
	if (sd->mob_count) {
		if (!sd->mob_item) {
			WFIFOB(sd->fd, sd->mob_len + 7) = 0;
			sd->mob_len++;
		}
		WFIFOHEADER(sd->fd, 0x07, sd->mob_len + 4);
		WFIFOW(sd->fd, 5) = SWAP16(sd->mob_count);
		WFIFOSET(sd->fd, client_encrypt(sd->fd));
	}
	return 0;
}

/* ========== Object Look Sub Functions ========== */

int client_object_look_sub(struct block_list* bl, va_list ap) {
	USER* sd = NULL;
	MOB* mob = NULL;
	NPC* nd = NULL;
	FLOORITEM* item = NULL;
	struct block_list* b = NULL;
	int type;
	int x;
	int nlen = 0;
	int animlen = 0;
	int len = 0;

	type = va_arg(ap, int);
	if (type == LOOK_SEND) {
		nullpo_ret(0, sd = (USER*)bl);
		nullpo_ret(0, b = va_arg(ap, struct block_list*));
	}
	else {
		nullpo_ret(0, sd = va_arg(ap, USER*));
		nullpo_ret(0, b = bl);
	}

	if (b->type == BL_PC) return 0;
	len = sd->mob_len;
	WFIFOW(sd->fd, len + 7) = SWAP16(b->x);
	WFIFOW(sd->fd, len + 9) = SWAP16(b->y);
	WFIFOL(sd->fd, len + 12) = SWAP32(b->id);

	switch (b->type) {
	case BL_MOB:
		mob = (MOB*)b;

		if (mob->state == MOB_DEAD || mob->data->mobtype == 1) return 0;

		nlen = 0;
		animlen = 0;

		if (mob->data->isnpc == 0) {
			WFIFOB(sd->fd, len + 11) = 0x05;
			WFIFOW(sd->fd, len + 16) = SWAP16(32768 + mob->look);
			WFIFOB(sd->fd, len + 18) = mob->look_color;
			WFIFOB(sd->fd, len + 19) = mob->side;
			WFIFOB(sd->fd, len + 20) = 0;
			WFIFOB(sd->fd, len + 21) = 0;
			for (x = 0; x < 50; x++) {
				if (mob->da[x].duration && mob->da[x].animation) {
					WFIFOW(sd->fd, nlen + len + 22) = SWAP16(mob->da[x].animation);
					WFIFOW(sd->fd, nlen + len + 22 + 2) = SWAP16(mob->da[x].duration / 1000);
					animlen++;
					nlen += 4;
				}
			}

			WFIFOB(sd->fd, len + 21) = animlen;
			WFIFOB(sd->fd, len + 22 + nlen) = 0;
			sd->mob_len += 15 + nlen;
		}
		else if (mob->data->isnpc == 1) {
			WFIFOB(sd->fd, len + 11) = 12;
			WFIFOW(sd->fd, len + 16) = SWAP16(32768 + mob->look);
			WFIFOB(sd->fd, len + 18) = mob->look_color;
			WFIFOB(sd->fd, len + 19) = mob->side;
			WFIFOW(sd->fd, len + 20) = 0;
			WFIFOB(sd->fd, len + 22) = 0;
			sd->mob_len += 15;
		}

		break;
	case BL_NPC:
		nd = (NPC*)b;

		if (b->subtype || nd->bl.subtype || nd->npctype == 1) return 0;

		WFIFOB(sd->fd, len + 11) = 12;
		WFIFOW(sd->fd, len + 16) = SWAP16(32768 + b->graphic_id);
		WFIFOB(sd->fd, len + 18) = b->graphic_color;
		WFIFOB(sd->fd, len + 19) = nd->side;
		WFIFOW(sd->fd, len + 20) = 0;
		WFIFOB(sd->fd, len + 22) = 0;
		sd->mob_len += 15;
		break;
	case BL_ITEM:
		item = (FLOORITEM*)b;

		int inTable = 0;

		for (int j = 0; j < sizeof(item->data.trapsTable); j++) {
			if (item->data.trapsTable[j] == sd->status.id) inTable = 1;
		}

		if (itemdb_type(item->data.id) == ITM_TRAPS && !inTable) {
			return 0;
		}

		WFIFOB(sd->fd, len + 11) = 0x02;

		if (item->data.customIcon != 0) {
			WFIFOW(sd->fd, len + 16) = SWAP16(item->data.customIcon + 49152);
			WFIFOB(sd->fd, len + 18) = item->data.customIconColor;
		}
		else {
			WFIFOW(sd->fd, len + 16) = SWAP16(itemdb_icon(item->data.id));
			WFIFOB(sd->fd, len + 18) = itemdb_iconcolor(item->data.id);
		}

		WFIFOB(sd->fd, len + 19) = 0;
		WFIFOW(sd->fd, len + 20) = 0;
		WFIFOB(sd->fd, len + 22) = 0;
		sd->mob_len += 15;
		sd->mob_item = 1;
		break;
	}
	sd->mob_count++;
	return 0;
}

/* ========== Destroy/Remove Visual ========== */

int client_send_destroy(struct block_list* bl, va_list ap) {
	USER* sd = NULL;
	MOB* mob = NULL;
	int type;
	type = va_arg(ap, int);

	nullpo_ret(0, sd = (USER*)bl);
	nullpo_ret(0, mob = va_arg(ap, MOB*));

	if (!session[sd->fd])
	{
		session[sd->fd]->eof = 8;
		return 0;
	}

	if (mob->data->mobtype == 1) {
		WFIFOHEAD(sd->fd, 9);
		WFIFOB(sd->fd, 0) = 0xAA;
		WFIFOW(sd->fd, 1) = SWAP16(6);
		WFIFOB(sd->fd, 3) = 0x0E;
		WFIFOB(sd->fd, 4) = 0x03;
		WFIFOL(sd->fd, 5) = SWAP32(mob->bl.id);
		WFIFOSET(sd->fd, client_encrypt(sd->fd));
	}
	else {
		WFIFOHEAD(sd->fd, 9);
		WFIFOB(sd->fd, 0) = 0xAA;
		WFIFOW(sd->fd, 1) = SWAP16(6);
		WFIFOB(sd->fd, 3) = 0x5F;
		WFIFOB(sd->fd, 4) = 0x03;
		WFIFOL(sd->fd, 5) = SWAP32(mob->bl.id);
		WFIFOSET(sd->fd, client_encrypt(sd->fd));
	}
	return 0;
}

/* Note: The following functions are very large (200-300 lines each) and remain in client.c:
 * - client_npc_look_sub() - NPC character look packet builder
 * - client_mob_look_sub() - Mob character look packet builder
 * - client_char_look_sub() - Player character look packet builder
 * - client_object_look_sub2() - Object look packet variant
 * - client_object_look_specific() - Specific object look
 *
 * These will be moved in a future iteration or left in client.c
 * with forward declarations in this header.
 */
