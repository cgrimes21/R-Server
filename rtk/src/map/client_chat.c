/**
 * @file client_chat.c
 * @brief Chat, whispers, broadcasts, and messaging functions
 *
 * Extracted from clif.c as part of the RTK refactoring project.
 * Handles all chat-related communication between clients.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include "client_chat.h"
#include "client_crypto.h"
#include "client.h"
#include "core.h"
#include "map.h"
#include "socket.h"
#include "crypt.h"  /* SWAP16, SWAP32 */
#include "malloc.h"
#include "mmo.h"
#include "pc.h"
#include "command.h"
#include "lua_core.h"
#include "magic.h"
#include "class_db.h"
#include "../common/db_mysql.h"

/* ========== Popup Functions ========== */

int client_popup(USER* sd, const char* buf) {
	if (!session[sd->fd])
	{
		session[sd->fd]->eof = 8;
		return 0;
	}

	WFIFOHEAD(sd->fd, strlen(buf) + 5 + 3);
	WFIFOB(sd->fd, 0) = 0xAA;
	WFIFOW(sd->fd, 1) = SWAP16(strlen(buf) + 5);
	WFIFOB(sd->fd, 3) = 0x0A;
	WFIFOB(sd->fd, 4) = 0x03;
	WFIFOB(sd->fd, 5) = 0x08;
	WFIFOW(sd->fd, 6) = SWAP16(strlen(buf));
	strcpy(WFIFOP(sd->fd, 8), buf);
	WFIFOSET(sd->fd, client_encrypt(sd->fd));
	return 0;
}

int client_paper_popup(USER* sd, const char* buf, int width, int height) {
	if (!session[sd->fd])
	{
		session[sd->fd]->eof = 8;
		return 0;
	}

	WFIFOHEAD(sd->fd, strlen(buf) + 11 + 3);
	WFIFOB(sd->fd, 0) = 0xAA;
	WFIFOW(sd->fd, 1) = SWAP16(strlen(buf) + 11);
	WFIFOB(sd->fd, 3) = 0x35;
	WFIFOB(sd->fd, 5) = 0;
	WFIFOB(sd->fd, 6) = width;
	WFIFOB(sd->fd, 7) = height;
	WFIFOB(sd->fd, 8) = 0;
	WFIFOW(sd->fd, 9) = SWAP16(strlen(buf));
	strcpy(WFIFOP(sd->fd, 11), buf);
	WFIFOSET(sd->fd, client_encrypt(sd->fd));
	return 0;
}

int client_paper_popup_write(USER* sd, const char* buf, int width, int height, int invslot) {
	if (!session[sd->fd])
	{
		session[sd->fd]->eof = 8;
		return 0;
	}

	WFIFOHEAD(sd->fd, strlen(buf) + 11 + 3);
	WFIFOB(sd->fd, 0) = 0xAA;
	WFIFOW(sd->fd, 1) = SWAP16(strlen(buf) + 11);
	WFIFOB(sd->fd, 3) = 0x1B;
	WFIFOB(sd->fd, 5) = invslot;
	WFIFOB(sd->fd, 6) = 0;
	WFIFOB(sd->fd, 7) = width;
	WFIFOB(sd->fd, 8) = height;
	WFIFOW(sd->fd, 9) = SWAP16(strlen(buf));
	strcpy(WFIFOP(sd->fd, 11), buf);
	WFIFOSET(sd->fd, client_encrypt(sd->fd));
	return 0;
}

int client_paper_popup_write_save(USER* sd) {
	if (!session[sd->fd])
	{
		session[sd->fd]->eof = 8;
		return 0;
	}

	char input[300];
	memset(input, 0, 300);

	memcpy(input, RFIFOP(sd->fd, 8), SWAP16(RFIFOW(sd->fd, 6)));
	unsigned int slot = RFIFOB(sd->fd, 5);

	if (strcmp(sd->status.inventory[slot].note, input) != 0) {
		memcpy(sd->status.inventory[slot].note, input, 300);
	}
	return 0;
}

/* ========== Message Functions ========== */

int client_send_msg(USER* sd, int type, char* buf) {
	nullpo_ret(0, buf);

	int adviceFlag = sd->status.settingFlags & FLAG_ADVICE;
	if (type == 99 && adviceFlag) type = 11;
	else if (type == 99 && !adviceFlag) return 0;

	int len = strlen(buf);

	if (len > strlen(buf)) len = strlen(buf);

	WFIFOHEAD(sd->fd, 8 + len);
	if (!session[sd->fd])
	{
		session[sd->fd]->eof = 8;
		return 0;
	}

	WFIFOHEAD(sd->fd, 8 + len);
	WFIFOB(sd->fd, 0) = 0xAA;
	WFIFOW(sd->fd, 1) = SWAP16(5 + len);
	WFIFOB(sd->fd, 3) = 0x0A;
	WFIFOB(sd->fd, 4) = 0x03;
	WFIFOW(sd->fd, 5) = type;
	WFIFOB(sd->fd, 7) = len;
	memcpy(WFIFOP(sd->fd, 8), buf, len);
	WFIFOSET(sd->fd, client_encrypt(sd->fd));
	return 0;
}

int client_send_minitext(USER* sd, char* msg) {
	nullpo_ret(0, sd);
	if (!strlen(msg)) return 0;
	client_send_msg(sd, 3, msg);
	return 0;
}

int client_send_blue_message(USER* sd, char* msg) {
	if (!session[sd->fd])
	{
		session[sd->fd]->eof = 8;
		return 0;
	}

	WFIFOHEAD(sd->fd, strlen(msg) + 8);
	WFIFOB(sd->fd, 0) = 0xAA;
	WFIFOB(sd->fd, 3) = 0x0A;
	WFIFOB(sd->fd, 4) = 0x03;
	WFIFOW(sd->fd, 5) = 0;
	WFIFOB(sd->fd, 7) = strlen(msg);
	strcpy(WFIFOP(sd->fd, 8), msg);
	WFIFOW(sd->fd, 1) = SWAP16(strlen(msg) + 5);
	WFIFOSET(sd->fd, client_encrypt(sd->fd));
	return 0;
}

/* ========== Broadcast Functions ========== */

int client_broadcast_sub(struct block_list* bl, va_list ap) {
	USER* sd = NULL;
	char* msg = NULL;
	int len = 0;

	nullpo_ret(0, sd = (USER*)bl);
	msg = va_arg(ap, char*);
	len = strlen(msg);

	if (!session[sd->fd])
	{
		session[sd->fd]->eof = 8;
		return 0;
	}

	int flag = sd->status.settingFlags & FLAG_SHOUT;
	if (flag == 0)  return 0;

	WFIFOHEAD(sd->fd, len + 8);
	WFIFOB(sd->fd, 0) = 0xAA;
	WFIFOB(sd->fd, 3) = 0x0A;
	WFIFOB(sd->fd, 4) = 0x03;
	WFIFOB(sd->fd, 5) = 0x05;
	WFIFOW(sd->fd, 6) = SWAP16(len);
	strcpy(WFIFOP(sd->fd, 8), msg);
	WFIFOW(sd->fd, 1) = SWAP16(len + 5);
	WFIFOSET(sd->fd, client_encrypt(sd->fd));

	return 0;
}

int client_gm_broadcast_sub(struct block_list* bl, va_list ap) {
	USER* sd = NULL;
	char* msg = NULL;
	int len = 0;

	nullpo_ret(0, sd = (USER*)bl);
	msg = va_arg(ap, char*);
	len = strlen(msg);

	if (!session[sd->fd])
	{
		session[sd->fd]->eof = 8;
		return 0;
	}

	WFIFOHEAD(sd->fd, len + 8);
	WFIFOB(sd->fd, 0) = 0xAA;
	WFIFOB(sd->fd, 3) = 0x0A;
	WFIFOB(sd->fd, 4) = 0x03;
	WFIFOB(sd->fd, 5) = 0x05;
	WFIFOW(sd->fd, 6) = SWAP16(len);
	strcpy(WFIFOP(sd->fd, 8), msg);
	WFIFOW(sd->fd, 1) = SWAP16(len + 5);
	WFIFOSET(sd->fd, client_encrypt(sd->fd));

	return 0;
}

int client_broadcast_to_gm_sub(struct block_list* bl, va_list ap) {
	USER* sd = NULL;
	char* msg = NULL;
	int len = 0;

	nullpo_ret(0, sd = (USER*)bl);
	if (sd->status.gm_level)
	{
		msg = va_arg(ap, char*);
		len = strlen(msg);

		if (!session[sd->fd])
		{
			session[sd->fd]->eof = 8;
			return 0;
		}

		WFIFOHEAD(sd->fd, len + 8);
		WFIFOB(sd->fd, 0) = 0xAA;
		WFIFOB(sd->fd, 3) = 0x0A;
		WFIFOB(sd->fd, 4) = 0x03;
		WFIFOB(sd->fd, 5) = 0x05;
		WFIFOW(sd->fd, 6) = SWAP16(len);
		strcpy(WFIFOP(sd->fd, 8), msg);
		WFIFOW(sd->fd, 1) = SWAP16(len + 5);
		WFIFOSET(sd->fd, client_encrypt(sd->fd));
	}
	return 0;
}

int client_broadcast(char* msg, int m) {
	if (m == -1) {
		for (int x = 0; x < 65535; x++) {
			if (map_isloaded(x)) {
				map_foreachinarea(client_broadcast_sub, x, 1, 1, SAMEMAP, BL_PC, msg);
			}
		}
	}
	else {
		map_foreachinarea(client_broadcast_sub, m, 1, 1, SAMEMAP, BL_PC, msg);
	}

	return 0;
}

int client_gm_broadcast(char* msg, int m) {
	if (m == -1) {
		for (int x = 0; x < 65535; x++) {
			if (map_isloaded(x)) {
				map_foreachinarea(client_gm_broadcast_sub, x, 1, 1, SAMEMAP, BL_PC, msg);
			}
		}
	}
	else {
		map_foreachinarea(client_gm_broadcast_sub, m, 1, 1, SAMEMAP, BL_PC, msg);
	}
	return 0;
}

int client_broadcast_to_gm(char* msg, int m) {
	if (m == -1) {
		for (int x = 0; x < 65535; x++) {
			if (map_isloaded(x)) {
				map_foreachinarea(client_broadcast_to_gm_sub, x, 1, 1, SAMEMAP, BL_PC, msg);
			}
		}
	}
	else {
		map_foreachinarea(client_broadcast_to_gm_sub, m, 1, 1, SAMEMAP, BL_PC, msg);
	}
	return 0;
}

/* ========== Whisper Functions ========== */

int client_send_whisper(USER* sd, char* srcname, unsigned char* msg) {
	int msglen = strlen(msg);
	int srclen = strlen(srcname);
	int newlen = 0;
	unsigned char* buf;
	unsigned char buf2[255];
	USER* src_sd = map_name2sd(srcname);
	if (strlen(msg) < msglen) msglen = strlen(msg);
	if (src_sd) {
		if (classdb_name(src_sd->status.class, src_sd->status.mark)) {
			newlen = sprintf(buf2, "\" (%s) ", classdb_name(src_sd->status.class, src_sd->status.mark));
		}
		else {
			newlen = sprintf(buf2, "\" () ");
		}

		CALLOC(buf, unsigned char, srclen + msglen + newlen);
		memcpy(WBUFP(buf, 0), srcname, srclen);
		strcpy(WBUFP(buf, srclen), buf2);
		memcpy(WBUFP(buf, srclen + newlen), msg, msglen);

		if (map[sd->bl.m].cantalk == 1 && !sd->status.gm_level) {
			client_send_minitext(sd, "Your voice is carried away.");
			FREE(buf);
			return 0;
		}

		client_send_msg(sd, 0, buf);

		FREE(buf);
	}

	return 0;
}

int client_retr_whisper(USER* sd, char* dstname, unsigned char* msg) {
	int msglen = strlen(msg);
	int dstlen = strlen(dstname);
	unsigned char buf[2 + dstlen + msglen];

	sprintf(buf, "%s> %s", dstname, msg);

	client_send_msg(sd, 0, buf);
	return 0;
}

int client_fail_whisper(USER* sd) {
	client_send_msg(sd, 0, map_msg[MAP_WHISPFAIL].message);
	return 0;
}

int canwhisper(USER* sd, USER* dst_sd)
{
	if (!dst_sd)	return 0;

	if (sd->uFlags & uFlag_silenced) return 0;
	else if (!sd->status.gm_level && !(dst_sd->status.settingFlags & FLAG_WHISPER)) { return 0; }
	else if (!sd->status.gm_level)
	{
		return client_is_ignore(sd, dst_sd);
	}

	return 1;
}

int client_parse_whisper(USER* sd) {
	char dst_name[100];
	char strText[255];
	USER* dst_sd = NULL;
	int dstlen, srclen, msglen, afklen;
	unsigned char buf[255];
	char msg[100];
	char afk[100];
	char escape[255];

	if (!(sd->status.settingFlags & FLAG_WHISPER) && !(sd->status.gm_level))
	{
		client_send_blue_message(sd, "You have whispering turned off.");
		return 0;
	}

	if (sd->uFlags & uFlag_silenced)
	{
		client_send_blue_message(sd, "You are silenced.");
		return 0;
	}

	if (map[sd->bl.m].cantalk == 1 && !sd->status.gm_level) {
		client_send_minitext(sd, "Your voice is swept away by a strange wind.");
		return 0;
	}

	nullpo_ret(0, sd);
	dstlen = RFIFOB(sd->fd, 5);

	srclen = strlen(sd->status.name);
	msglen = RFIFOB(sd->fd, 6 + dstlen);

	if ((msglen > 80) || (dstlen > 80) || (dstlen > RFIFOREST(sd->fd)) || (dstlen > SWAP16(RFIFOW(sd->fd, 1))) || (msglen > RFIFOREST(sd->fd)) || (msglen > SWAP16(RFIFOW(sd->fd, 1)))) {
		client_hacker(sd->status.name, "Whisper packet");
		return 0;
	}

	memset(dst_name, 0, 80);
	memset(msg, 0, 80);

	memcpy(dst_name, RFIFOP(sd->fd, 6), dstlen);
	memcpy(msg, RFIFOP(sd->fd, 7 + dstlen), msglen);

	msg[80] = '\0';

	Sql_EscapeString(sql_handle, escape, msg);

	if (!strcmp(dst_name, "!")) {
		if (sd->status.clan == 0) {
			client_send_blue_message(sd, "You are not in a clan");
		}
		else {
			if (sd->status.clan_chat) {
				sl_doscript_strings("characterLog", "clanChatLog", 2, sd->status.name, msg);
				client_send_clan_message(sd, RFIFOP(sd->fd, 7 + dstlen), msglen);
			}
			else {
				client_send_blue_message(sd, "Clan chat is off.");
			}
		}
	}
	else if (!strcmp(dst_name, "!!")) {
		if (sd->group_count == 0) {
			client_send_blue_message(sd, "You are not in a group");
		}
		else {
			sl_doscript_strings("characterLog", "groupChatLog", 2, sd->status.name, msg);
			client_send_group_message(sd, RFIFOP(sd->fd, 7 + dstlen), msglen);
		}
	}
	else if (!strcmp(dst_name, "@")) {
		if (classdb_chat(sd->status.class)) {
			sl_doscript_strings("characterLog", "subPathChatLog", 2, sd->status.name, msg);
			client_send_subpath_message(sd, RFIFOP(sd->fd, 7 + dstlen), msglen);
		}
		else {
			client_send_blue_message(sd, "You cannot do that.");
		}
	}
	else if (!strcmp(dst_name, "?")) {
		if (sd->status.tutor == 0 && sd->status.gm_level == 0) {
			if (sd->status.level < 99) {
				sl_doscript_strings("characterLog", "noviceChatLog", 2, sd->status.name, msg);
				client_send_novice_message(sd, RFIFOP(sd->fd, 7 + dstlen), msglen);
			}
			else {
				client_send_blue_message(sd, "You cannot do that.");
			}
		}
		else {
			sl_doscript_strings("characterLog", "noviceChatLog", 2, sd->status.name, msg);
			client_send_novice_message(sd, RFIFOP(sd->fd, 7 + dstlen), msglen);
		}
	}
	else {
		dst_sd = map_name2sd(dst_name);
		if (!dst_sd) {
			sprintf(strText, "%s is nowhere to be found.", dst_name);
			client_send_blue_message(sd, strText);
		}
		else {
			if (canwhisper(sd, dst_sd)) {
				if (dst_sd->afk && strcmp(dst_sd->afkmessage, "") != 0) {
					sl_doscript_strings("characterLog", "whisperLog", 3, dst_sd->status.name, sd->status.name, msg);

					client_send_whisper(dst_sd, sd->status.name, msg);
					client_send_whisper(dst_sd, dst_sd->status.name, dst_sd->afkmessage);

					if (!sd->status.gm_level && (dst_sd->optFlags & optFlag_stealth)) {
						sprintf(strText, "%s is nowhere to be found.", dst_name);
					}
					else {
						client_retr_whisper(sd, dst_sd->status.name, msg);
						client_retr_whisper(sd, dst_sd->status.name, dst_sd->afkmessage);
					}
				}
				else {
					sl_doscript_strings("characterLog", "whisperLog", 3, dst_sd->status.name, sd->status.name, msg);

					client_send_whisper(dst_sd, sd->status.name, msg);

					if (!sd->status.gm_level && (dst_sd->optFlags & optFlag_stealth)) {
						sprintf(strText, "%s is nowhere to be found.", dst_name);
						client_send_blue_message(sd, strText);
					}
					else {
						client_retr_whisper(sd, dst_sd->status.name, msg);
					}
				}
			}
			else {
				client_send_blue_message(sd, "They cannot hear you right now.");
			}
		}
	}
	return 0;
}

/* ========== Say/Chat Functions ========== */

int client_send_say(USER* sd, char* msg, int msglen, int type) {
	char i;
	if (type == 1) {
		sd->talktype = 1;
		strcpy(sd->speech, msg);
	}
	else {
		sd->talktype = 0;
		strcpy(sd->speech, msg);
	}

	for (i = 0; i < MAX_SPELLS; i++) {
		if (sd->status.skill[i] > 0) {
			sl_doscript_blargs(magicdb_yname(sd->status.skill[i]), "on_say", 1, &sd->bl);
		}
	}
	sl_doscript_blargs("onSay", NULL, 1, &sd->bl);
	return 0;
}

int client_send_script_say(USER* sd, char* msg, int msglen, int type) {
	char* buf;
	char name[25];
	char escape[255];
	int namelen = strlen(sd->status.name);

	if (map[sd->bl.m].cantalk == 1 && !sd->status.gm_level) {
		client_send_minitext(sd, "Your voice is swept away by a strange wind.");
		return 0;
	}

	Sql_EscapeString(sql_handle, escape, msg);

	if (is_command(sd, msg, msglen)) {
		return 0;
	}

	if (sd->uFlags & uFlag_silenced)
	{
		client_send_minitext(sd, "Shut up for now. ^^");
		return 0;
	}

	if (type >= 10) {
		namelen += 4;

		if (!session[sd->fd])
		{
			session[sd->fd]->eof = 8;
			return 0;
		}

		WFIFOHEAD(sd->fd, msglen + namelen + 13);
		CALLOC(buf, char, 16 + namelen + msglen);
		WBUFB(buf, 0) = 0xAA;
		WBUFW(buf, 1) = SWAP16(10 + namelen + msglen);
		WBUFB(buf, 3) = 0x0D;
		WBUFB(buf, 5) = type;
		WBUFL(buf, 6) = SWAP32(sd->status.id);
		WBUFB(buf, 10) = namelen + msglen + 2;
		switch (type) {
		case 10:
			sprintf(name, "EN[%s]", sd->status.name);
			break;
		case 11:
			sprintf(name, "ES[%s]", sd->status.name);
			break;
		case 12:
			sprintf(name, "FR[%s]", sd->status.name);
			break;
		case 13:
			sprintf(name, "CN[%s]", sd->status.name);
			break;
		case 14:
			sprintf(name, "PT[%s]", sd->status.name);
			break;
		case 15:
			sprintf(name, "ID[%s]", sd->status.name);
			break;
		default:
			break;
		}
		memcpy(WBUFP(buf, 11), name, namelen);
		WBUFB(buf, 11 + namelen) = ':';
		WBUFB(buf, 12 + namelen) = ' ';
		memcpy(WBUFP(buf, 13 + namelen), msg, msglen);
		client_send(buf, 16 + namelen + msglen, &sd->bl, SAMEAREA);
	}
	else {
		if (!session[sd->fd])
		{
			session[sd->fd]->eof = 8;
			return 0;
		}

		WFIFOHEAD(sd->fd, msglen + namelen + 13);
		CALLOC(buf, char, 16 + namelen + msglen);
		WBUFB(buf, 0) = 0xAA;
		WBUFW(buf, 1) = SWAP16(10 + namelen + msglen);
		WBUFB(buf, 3) = 0x0D;
		WBUFB(buf, 5) = type;
		WBUFL(buf, 6) = SWAP32(sd->status.id);
		WBUFB(buf, 10) = namelen + msglen + 2;
		memcpy(WBUFP(buf, 11), sd->status.name, namelen);
		if (type == 1)
			WBUFB(buf, 11 + namelen) = '!';
		else
			WBUFB(buf, 11 + namelen) = ':';
		WBUFB(buf, 12 + namelen) = ' ';
		memcpy(WBUFP(buf, 13 + namelen), msg, msglen);
		if (type == 1) {
			client_send(buf, 16 + namelen + msglen, &sd->bl, SAMEMAP);
		}
		else {
			client_send(buf, 16 + namelen + msglen, &sd->bl, SAMEAREA);
		}
	}

	strcpy(sd->speech, msg);

	if (type == 1) {
		map_foreachinarea(client_send_npc_yell, sd->bl.m, sd->bl.x, sd->bl.y, SAMEMAP, BL_NPC, msg, sd);
		map_foreachinarea(client_send_mob_yell, sd->bl.m, sd->bl.x, sd->bl.y, SAMEMAP, BL_MOB, msg, sd);
	}
	else {
		map_foreachinarea(client_send_npc_say, sd->bl.m, sd->bl.x, sd->bl.y, AREA, BL_NPC, msg, sd);
		map_foreachinarea(client_send_mob_say, sd->bl.m, sd->bl.x, sd->bl.y, AREA, BL_MOB, msg, sd);
	}
	FREE(buf);
	return 0;
}

int client_parse_say(USER* sd) {
	char i;
	char* msg = RFIFOP(sd->fd, 7);

	sd->talktype = RFIFOB(sd->fd, 5);

	if (sd->talktype > 1 || RFIFOB(sd->fd, 6) > 100) {
		client_send_minitext(sd, "I just told the GM on you!");
		printf("Talk Hacker: %s\n", sd->status.name);
		return 0;
	}

	strcpy(sd->speech, msg);
	for (i = 0; i < MAX_SPELLS; i++) {
		if (sd->status.skill[i] > 0) {
			sl_doscript_blargs(magicdb_yname(sd->status.skill[i]), "on_say", 1, &sd->bl);
		}
	}
	sl_doscript_blargs("onSay", NULL, 1, &sd->bl);
	return 0;
}

/* ========== Group/Clan/Subpath Messaging ========== */

int client_send_group_message(USER* sd, unsigned char* msg, int msglen) {
	char buf[300];
	char buf2[65535];
	int i;
	char message[256];

	nullpo_ret(0, sd);
	memset(message, 0, 255);
	memcpy(message, msg, msglen);
	USER* tsd;

	if (classdb_name(sd->status.class, sd->status.mark)) {
		sprintf(buf2, "[!%s] (%s) %s", sd->status.name, classdb_name(sd->status.class, sd->status.mark), message);
	}
	else {
		sprintf(buf2, "[!%s] () %s", sd->status.name, message);
	}

	if (sd->uFlags & uFlag_silenced)
	{
		client_send_blue_message(sd, "You are silenced.");
		return 0;
	}

	if (map[sd->bl.m].cantalk == 1 && !sd->status.gm_level) {
		client_send_minitext(sd, "Your voice is swept away by a strange wind.");
		return 0;
	}

	for (i = 0; i < sd->group_count; i++) {
		tsd = map_id2sd(groups[sd->groupid][i]);
		if (tsd && client_is_ignore(sd, tsd)) {
			client_send_msg(tsd, 11, buf2);
		}
	}
	return 0;
}

int client_send_subpath_message(USER* sd, unsigned char* msg, int msglen) {
	char buf[300];
	char buf2[65535];
	int i;
	char message[256];

	nullpo_ret(0, sd);
	memset(message, 0, 255);
	memcpy(message, msg, msglen);
	USER* tsd;

	if (classdb_name(sd->status.class, sd->status.mark)) {
		sprintf(buf2, "<@%s> (%s) %s", sd->status.name, classdb_name(sd->status.class, sd->status.mark), message);
	}
	else {
		sprintf(buf2, "<@%s> () %s", sd->status.name, message);
	}

	if (sd->uFlags & uFlag_silenced)
	{
		client_send_blue_message(sd, "You are silenced.");
		return 0;
	}

	if (map[sd->bl.m].cantalk == 1 && !sd->status.gm_level) {
		client_send_minitext(sd, "Your voice is swept away by a strange wind.");
		return 0;
	}

	for (i = 0; i < fd_max; i++) {
		if (session[i] && (tsd = session[i]->session_data) && client_is_ignore(sd, tsd)) {
			if (tsd->status.class == sd->status.class) {
				if (tsd->status.subpath_chat) {
					client_send_msg(tsd, 11, buf2);
				}
			}
		}
	}

	return 0;
}

int client_send_clan_message(USER* sd, unsigned char* msg, int msglen) {
	char buf[300];
	char buf2[65535];
	int i;
	char message[256];
	memset(buf2, 0, 65534);
	memset(message, 0, 255);
	memcpy(message, msg, msglen);
	USER* tsd = NULL;

	if (classdb_name(sd->status.class, sd->status.mark)) {
		sprintf(buf2, "<!%s> (%s) %s", sd->status.name, classdb_name(sd->status.class, sd->status.mark), message);
	}
	else {
		sprintf(buf2, "<!%s> () %s", sd->status.name, message);
	}

	if (sd->uFlags & uFlag_silenced)
	{
		client_send_blue_message(sd, "You are silenced.");
		return 0;
	}

	if (map[sd->bl.m].cantalk == 1 && !sd->status.gm_level) {
		client_send_minitext(sd, "Your voice is swept away by a strange wind.");
		return 0;
	}

	for (i = 0; i < fd_max; i++) {
		if (session[i] && (tsd = session[i]->session_data) && client_is_ignore(sd, tsd)) {
			if (tsd->status.clan == sd->status.clan) {
				if (tsd->status.clan_chat) {
					client_send_msg(tsd, 12, buf2);
				}
			}
		}
	}

	return 0;
}

int client_send_novice_message(USER* sd, unsigned char* msg, int msglen) {
	char buf[300];
	char buf2[65535];
	int i;
	char message[256];
	memset(buf2, 0, 65534);
	memset(message, 0, 255);
	memcpy(message, msg, msglen);
	USER* tsd = NULL;

	sprintf(buf2, "[Novice](%s) %s> %s", classdb_name(sd->status.class, sd->status.mark), sd->status.name, message);

	if (sd->uFlags & uFlag_silenced)
	{
		client_send_blue_message(sd, "You are silenced.");
		return 0;
	}

	if (map[sd->bl.m].cantalk == 1 && !sd->status.gm_level) {
		client_send_minitext(sd, "Your voice is swept away by a strange wind.");
		return 0;
	}

	if (sd->status.tutor == 0) {
		client_send_msg(sd, 11, buf2);
	}

	for (i = 0; i < fd_max; i++) {
		if (session[i] && (tsd = session[i]->session_data) && client_is_ignore(sd, tsd)) {
			if ((tsd->status.tutor || tsd->status.gm_level > 0) && tsd->status.novice_chat) {
				client_send_msg(tsd, 12, buf2);
			}
		}
	}

	return 0;
}

/* ========== NPC/Mob Speech Triggers ========== */

int client_distance(struct block_list* bl, struct block_list* bl2) {
	int distance = 0;

	distance += abs(bl->x - bl2->x);
	distance += abs(bl->y - bl2->y);

	return distance;
}

int client_send_npc_say(struct block_list* bl, va_list ap) {
	NPC* nd = NULL;
	USER* sd = NULL;
	char* msg = NULL;
	char temp[256];
	char* temp2 = NULL;
	char temp3[256];

	if (bl->subtype != SCRIPT) return 0;

	msg = va_arg(ap, char*);
	nullpo_ret(0, sd = va_arg(ap, USER*));
	nullpo_ret(0, nd = (NPC*)bl);

	if (client_distance(&sd->bl, &nd->bl) <= 10) {
		sd->last_click = bl->id;
		sl_async_freeco(sd);
		sl_doscript_blargs(nd->name, "onSayClick", 2, &sd->bl, &nd->bl);
	}

	return 0;
}

int client_send_mob_say(struct block_list* bl, va_list ap) {
	return 0;
}

int client_send_npc_yell(struct block_list* bl, va_list ap) {
	NPC* nd = NULL;
	USER* sd = NULL;
	char* msg = NULL;
	char temp[256];
	char* temp2 = NULL;
	char temp3[256];

	if (bl->subtype != SCRIPT) return 0;

	msg = va_arg(ap, char*);
	nullpo_ret(0, sd = va_arg(ap, USER*));
	nullpo_ret(0, nd = (NPC*)bl);

	if (client_distance(&sd->bl, &nd->bl) <= 20) {
		sd->last_click = bl->id;
		sl_async_freeco(sd);
		sl_doscript_blargs(nd->name, "onSayClick", 2, &sd->bl, &nd->bl);
	}

	return 0;
}

int client_send_mob_yell(struct block_list* bl, va_list ap) {
	return 0;
}

int client_speak(struct block_list* bl, va_list ap) {
	struct block_list* nd = NULL;
	USER* sd = NULL;
	char* msg = NULL;
	int len;
	int type;

	msg = va_arg(ap, char*);
	nullpo_ret(0, nd = va_arg(ap, struct block_list*));
	type = va_arg(ap, int);
	nullpo_ret(0, sd = (USER*)bl);
	len = strlen(msg);

	if (!session[sd->fd])
	{
		session[sd->fd]->eof = 8;
		return 0;
	}

	WFIFOHEAD(sd->fd, strlen(msg) + 11);
	WFIFOB(sd->fd, 5) = type;
	WFIFOL(sd->fd, 6) = SWAP32(nd->id);
	WFIFOB(sd->fd, 10) = len;
	len = len + 8;
	strcpy(WFIFOP(sd->fd, 11), msg);

	WFIFOHEADER(sd->fd, 0x0D, len);
	WFIFOSET(sd->fd, client_encrypt(sd->fd));
	return 0;
}

/* ========== Ignore List Functions ========== */

int ignorelist_add(USER* sd, const char* name)
{
	struct sd_ignorelist* Current = sd->IgnoreList;
	while (Current)
	{
		if (strcmpi(Current->name, name) == 0)
			return 1;

		Current = Current->Next;
	}

	struct sd_ignorelist* New = NULL;
	CALLOC(New, struct sd_ignorelist, 1);

	strcpy(New->name, name);
	New->Next = sd->IgnoreList;
	sd->IgnoreList = New;

	return 0;
}

int ignorelist_remove(USER* sd, const char* name)
{
	char IgBuffer[32];
	char IgCmp[32];

	strcpy(IgBuffer, name);

	int ret = 1;

	if (sd->IgnoreList)
	{
		struct sd_ignorelist* Current = sd->IgnoreList;
		struct sd_ignorelist* Prev = NULL;
		while (Current)
		{
			strcpy(IgCmp, Current->name);

			if (strcmpi(IgCmp, IgBuffer) == 0)
			{
				ret = 0; break;
			}

			Prev = Current;
			Current = Current->Next;
		}

		if (ret == 0)
		{
			if (Prev)
			{
				Prev->Next = Current->Next;
				FREE(Current);
			}
			else
			{
				FREE(sd->IgnoreList);
				sd->IgnoreList = NULL;
			}
		}
	}
	else ret = 2;

	return ret;
}

int client_is_ignore(USER* sd, USER* dst_sd) {
	struct sd_ignorelist* Current = dst_sd->IgnoreList;
	char LowerName[32];
	char IgCmp[32];

	strcpy(LowerName, sd->status.name);

	while (Current) {
		strcpy(IgCmp, Current->name);

		if (strcmpi(IgCmp, LowerName) == 0) return 0;

		Current = Current->Next;
	}

	Current = sd->IgnoreList;
	strcpy(LowerName, dst_sd->status.name);

	while (Current) {
		strcpy(IgCmp, Current->name);

		if (strcmpi(IgCmp, LowerName) == 0) return 0;

		Current = Current->Next;
	}

	return 1;
}

int client_parse_ignore(USER* sd) {
	unsigned char iCmd = RFIFOB(sd->fd, 5);
	unsigned char nLen = RFIFOB(sd->fd, 6);
	char nameBuf[32];

	memset(nameBuf, 0, 32);
	if (nLen <= 16)
		switch (iCmd)
		{
		case 0x02:
			memcpy(nameBuf, RFIFOP(sd->fd, 7), nLen);
			ignorelist_add(sd, nameBuf);
			break;

		case 0x03:
			memcpy(nameBuf, RFIFOP(sd->fd, 7), nLen);
			ignorelist_remove(sd, nameBuf);
			break;
		}
	return 0;
}

/* ========== GUI Text Functions ========== */

int client_gui_text_sd(char* msg, USER* sd) {
	int len = 0;

	if (!session[sd->fd])
	{
		session[sd->fd]->eof = 8;
		return 0;
	}

	WFIFOHEAD(sd->fd, 0);
	WFIFOB(sd->fd, 0) = 0xAA;
	WFIFOB(sd->fd, 1) = 0x00;
	WFIFOB(sd->fd, 3) = 0x58;
	WFIFOB(sd->fd, 5) = 0x06;

	WFIFOW(sd->fd, 6) = SWAP16(strlen(msg));
	memcpy(WFIFOP(sd->fd, 8), msg, strlen(msg));

	WFIFOW(sd->fd, 1) = SWAP16(8 + strlen(msg) + 3);

	WFIFOSET(sd->fd, client_encrypt(sd->fd));

	return 0;
}

int client_gui_text(struct block_list* bl, va_list ap) {
	USER* sd = NULL;
	char* msg = NULL;
	int len = 0;

	nullpo_ret(0, sd = (USER*)bl);

	msg = va_arg(ap, char*);
	len = strlen(msg);

	if (!session[sd->fd])
	{
		session[sd->fd]->eof = 8;
		return 0;
	}

	WFIFOHEAD(sd->fd, 0);
	WFIFOB(sd->fd, 0) = 0xAA;
	WFIFOB(sd->fd, 1) = 0x00;
	WFIFOB(sd->fd, 3) = 0x58;
	WFIFOB(sd->fd, 5) = 0x06;

	WFIFOW(sd->fd, 6) = SWAP16(strlen(msg));
	memcpy(WFIFOP(sd->fd, 8), msg, strlen(msg));

	WFIFOW(sd->fd, 1) = SWAP16(8 + strlen(msg) + 3);

	WFIFOSET(sd->fd, client_encrypt(sd->fd));
	return 0;
}
