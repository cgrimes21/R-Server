/**
 * @file client_chat.h
 * @brief Chat, whispers, broadcasts, and messaging functions
 *
 * Extracted from clif.c as part of the RTK refactoring project.
 * Handles all chat-related communication between clients.
 */

#ifndef _CLIENT_CHAT_H_
#define _CLIENT_CHAT_H_

#include <stdarg.h>
#include "map.h"

/* Popup functions */
int client_popup(USER* sd, const char* buf);
int client_paper_popup(USER* sd, const char* buf, int width, int height);
int client_paper_popup_write(USER* sd, const char* buf, int width, int height, int invslot);
int client_paper_popup_write_save(USER* sd);

/* Message sending functions */
int client_send_msg(USER* sd, int type, char* buf);
int client_send_minitext(USER* sd, char* msg);
int client_send_blue_message(USER* sd, char* msg);

/* Broadcast functions */
int client_broadcast(char* msg, int m);
int client_gm_broadcast(char* msg, int m);
int client_broadcast_to_gm(char* msg, int m);
int client_broadcast_sub(struct block_list* bl, va_list ap);
int client_gm_broadcast_sub(struct block_list* bl, va_list ap);
int client_broadcast_to_gm_sub(struct block_list* bl, va_list ap);

/* Whisper functions */
int client_send_whisper(USER* sd, char* srcname, unsigned char* msg);
int client_retr_whisper(USER* sd, char* dstname, unsigned char* msg);
int client_fail_whisper(USER* sd);
int client_parse_whisper(USER* sd);
int canwhisper(USER* sd, USER* dst_sd);

/* Say/chat functions */
int client_send_say(USER* sd, char* msg, int msglen, int type);
int client_send_script_say(USER* sd, char* msg, int msglen, int type);
int client_parse_say(USER* sd);

/* Group/clan/subpath messaging */
int client_send_group_message(USER* sd, unsigned char* msg, int msglen);
int client_send_subpath_message(USER* sd, unsigned char* msg, int msglen);
int client_send_clan_message(USER* sd, unsigned char* msg, int msglen);
int client_send_novice_message(USER* sd, unsigned char* msg, int msglen);

/* NPC/Mob speech triggers */
int client_send_npc_say(struct block_list* bl, va_list ap);
int client_send_mob_say(struct block_list* bl, va_list ap);
int client_send_npc_yell(struct block_list* bl, va_list ap);
int client_send_mob_yell(struct block_list* bl, va_list ap);
int client_speak(struct block_list* bl, va_list ap);

/* Ignore list functions */
int client_parse_ignore(USER* sd);
int client_is_ignore(USER* sd, USER* dst_sd);
int ignorelist_add(USER* sd, const char* name);
int ignorelist_remove(USER* sd, const char* name);

/* GUI text functions */
int client_gui_text(struct block_list* bl, va_list ap);
int client_gui_text_sd(char* msg, USER* sd);

/* Helper functions */
int client_distance(struct block_list* bl, struct block_list* bl2);

/* Backward compatibility macros */
#define client_broadcasttogm client_broadcast_to_gm
#define client_broadcasttogm_sub client_broadcast_to_gm_sub
#define client_gui_textsd client_gui_text_sd

#endif /* _CLIENT_CHAT_H_ */
