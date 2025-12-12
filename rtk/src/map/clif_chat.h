/**
 * @file clif_chat.h
 * @brief Chat, whispers, broadcasts, and messaging functions
 *
 * Extracted from clif.c as part of the RTK refactoring project.
 * Handles all chat-related communication between clients.
 */

#ifndef _CLIF_CHAT_H_
#define _CLIF_CHAT_H_

#include <stdarg.h>
#include "map.h"

/* Popup functions */
int clif_popup(USER* sd, const char* buf);
int clif_paperpopup(USER* sd, const char* buf, int width, int height);
int clif_paperpopupwrite(USER* sd, const char* buf, int width, int height, int invslot);
int clif_paperpopupwrite_save(USER* sd);

/* Message sending functions */
int clif_sendmsg(USER* sd, int type, char* buf);
int clif_sendminitext(USER* sd, char* msg);
int clif_sendbluemessage(USER* sd, char* msg);

/* Broadcast functions */
int clif_broadcast(char* msg, int m);
int clif_gmbroadcast(char* msg, int m);
int clif_broadcasttogm(char* msg, int m);
int clif_broadcast_sub(struct block_list* bl, va_list ap);
int clif_gmbroadcast_sub(struct block_list* bl, va_list ap);
int clif_broadcasttogm_sub(struct block_list* bl, va_list ap);

/* Whisper functions */
int clif_sendwisp(USER* sd, char* srcname, unsigned char* msg);
int clif_retrwisp(USER* sd, char* dstname, unsigned char* msg);
int clif_failwisp(USER* sd);
int clif_parsewisp(USER* sd);
int canwhisper(USER* sd, USER* dst_sd);

/* Say/chat functions */
int clif_sendsay(USER* sd, char* msg, int msglen, int type);
int clif_sendscriptsay(USER* sd, char* msg, int msglen, int type);
int clif_parsesay(USER* sd);

/* Group/clan/subpath messaging */
int clif_sendgroupmessage(USER* sd, unsigned char* msg, int msglen);
int clif_sendsubpathmessage(USER* sd, unsigned char* msg, int msglen);
int clif_sendclanmessage(USER* sd, unsigned char* msg, int msglen);
int clif_sendnovicemessage(USER* sd, unsigned char* msg, int msglen);

/* NPC/Mob speech triggers */
int clif_sendnpcsay(struct block_list* bl, va_list ap);
int clif_sendmobsay(struct block_list* bl, va_list ap);
int clif_sendnpcyell(struct block_list* bl, va_list ap);
int clif_sendmobyell(struct block_list* bl, va_list ap);
int clif_speak(struct block_list* bl, va_list ap);

/* Ignore list functions */
int clif_parseignore(USER* sd);
int clif_isignore(USER* sd, USER* dst_sd);
int ignorelist_add(USER* sd, const char* name);
int ignorelist_remove(USER* sd, const char* name);

/* GUI text functions */
int clif_guitext(struct block_list* bl, va_list ap);
int clif_guitextsd(char* msg, USER* sd);

/* Helper functions */
int clif_distance(struct block_list* bl, struct block_list* bl2);

#endif /* _CLIF_CHAT_H_ */
