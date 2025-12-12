/**
 * @file client_visual.h
 * @brief Visual updates, animations, look packets, and movement display
 *
 * Extracted from clif.c as part of the RTK refactoring project.
 * Handles all visual/graphical updates sent to clients.
 */

#ifndef _CLIENT_VISUAL_H_
#define _CLIENT_VISUAL_H_

#include <stdarg.h>
#include "map.h"
#include "mob.h"
#include "npc.h"

/* Animation functions */
int client_send_animation(struct block_list* bl, va_list ap);
int client_send_animation_xy(struct block_list* bl, va_list ap);
int client_animation(USER* src, USER* sd, int animation, int duration);
int client_send_animations(USER* src, USER* sd);

/* Action functions */
int client_send_action(struct block_list* bl, int type, int time, int sound);
int client_send_mob_action(MOB* mob, int type, int time, int sound);

/* Sound */
int client_play_sound(struct block_list* bl, int sound);

/* Look/appearance packets */
int client_look_gone(struct block_list* bl);
int client_npc_look_sub(struct block_list* bl, va_list ap);
int client_mob_look_sub(struct block_list* bl, va_list ap);
int client_char_look_sub(struct block_list* bl, va_list ap);
int client_object_look_sub(struct block_list* bl, va_list ap);
int client_object_look_sub2(struct block_list* bl, va_list ap);
int client_object_look_specific(USER* sd, unsigned int id);
int client_show_ghost(USER* sd, USER* tsd);

/* Mob look functions */
int client_mob_look_start(USER* sd);
int client_mob_look_close(USER* sd);
int client_mob_look_start_func(struct block_list* bl, va_list ap);
int client_mob_look_close_func(struct block_list* bl, va_list ap);

/* Side/direction packets */
int client_send_side(struct block_list* bl);
int client_send_mob_side(MOB* mob);

/* Movement display */
int client_npc_move(struct block_list* bl, va_list ap);
int client_mob_move(struct block_list* bl, va_list ap);

/* Destroy/remove visual */
int client_send_destroy(struct block_list* bl, va_list ap);

#endif /* _CLIENT_VISUAL_H_ */
