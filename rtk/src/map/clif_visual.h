/**
 * @file clif_visual.h
 * @brief Visual updates, animations, look packets, and movement display
 *
 * Extracted from clif.c as part of the RTK refactoring project.
 * Handles all visual/graphical updates sent to clients.
 */

#ifndef _CLIF_VISUAL_H_
#define _CLIF_VISUAL_H_

#include <stdarg.h>
#include "map.h"
#include "mob.h"
#include "npc.h"

/* Animation functions */
int clif_sendanimation(struct block_list* bl, va_list ap);
int clif_sendanimation_xy(struct block_list* bl, va_list ap);
int clif_animation(USER* src, USER* sd, int animation, int duration);
int clif_sendanimations(USER* src, USER* sd);

/* Action functions */
int clif_sendaction(struct block_list* bl, int type, int time, int sound);
int clif_sendmob_action(MOB* mob, int type, int time, int sound);

/* Sound */
int clif_playsound(struct block_list* bl, int sound);

/* Look/appearance packets */
int clif_lookgone(struct block_list* bl);
int clif_cnpclook_sub(struct block_list* bl, va_list ap);
int clif_cmoblook_sub(struct block_list* bl, va_list ap);
int clif_charlook_sub(struct block_list* bl, va_list ap);
int clif_object_look_sub(struct block_list* bl, va_list ap);
int clif_object_look_sub2(struct block_list* bl, va_list ap);
int clif_object_look_specific(USER* sd, unsigned int id);
int clif_show_ghost(USER* sd, USER* tsd);

/* Mob look functions */
int clif_mob_look_start(USER* sd);
int clif_mob_look_close(USER* sd);
int clif_mob_look_start_func(struct block_list* bl, va_list ap);
int clif_mob_look_close_func(struct block_list* bl, va_list ap);

/* Side/direction packets */
int clif_sendside(struct block_list* bl);
int clif_sendmob_side(MOB* mob);

/* Movement display */
int clif_npc_move(struct block_list* bl, va_list ap);
int clif_mob_move(struct block_list* bl, va_list ap);

/* Destroy/remove visual */
int clif_send_destroy(struct block_list* bl, va_list ap);

#endif /* _CLIF_VISUAL_H_ */
