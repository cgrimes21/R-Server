/**
 * @file clif_combat.h
 * @brief Combat-related packets: health bars, damage, attacks, durability
 *
 * Extracted from clif.c as part of the RTK refactoring project.
 * Handles all combat-related visual updates.
 */

#ifndef _CLIF_COMBAT_H_
#define _CLIF_COMBAT_H_

#include <stdarg.h>
#include "map.h"
#include "mob.h"

/* Player damage/health */
int clif_pc_damage(USER* sd, USER* src);
int clif_send_pc_health(USER* src, int damage, int critical);
int clif_send_pc_healthscript(USER* sd, int damage, int critical);

/* Mob damage/health */
int clif_mob_damage(USER* sd, MOB* mob);
int clif_send_mob_health(MOB* mob, int damage, int critical);
int clif_send_mob_healthscript(MOB* mob, int damage, int critical);
int clif_send_mob_health_sub(struct block_list* bl, va_list ap);
int clif_send_mob_health_sub_nosd(struct block_list* bl, va_list ap);
int clif_mob_kill(MOB* mob);

/* Health bars */
void clif_send_selfbar(USER* sd);
void clif_send_groupbars(USER* sd, USER* tsd);
void clif_send_mobbars(struct block_list* bl, va_list ap);

/* Combat calculations */
int clif_calc_critical(USER* sd, struct block_list* bl);
int clif_parseattack(USER* sd);

/* Durability/equipment wear */
int clif_deductweapon(USER* sd, int hit);
int clif_deductarmor(USER* sd, int hit);
int clif_deductdura(USER* sd, int equip, int val);
int clif_deductduraequip(USER* sd);
int clif_checkdura(USER* sd, int equip);

/* Duration/aether effects */
int clif_send_duration(USER* sd, int id, int time, USER* tsd);
int clif_send_aether(USER* sd, int id, int time);
int clif_has_aethers(USER* sd, int spell);
int clif_findspell_pos(USER* sd, int id);

#endif /* _CLIF_COMBAT_H_ */
