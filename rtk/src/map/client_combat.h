/**
 * @file client_combat.h
 * @brief Combat-related packets: health bars, damage, attacks, durability
 *
 * Extracted from clif.c as part of the RTK refactoring project.
 * Handles all combat-related visual updates.
 */

#ifndef _CLIENT_COMBAT_H_
#define _CLIENT_COMBAT_H_

#include <stdarg.h>
#include "map.h"
#include "mob.h"

/* Player damage/health */
int client_pc_damage(USER* sd, USER* src);
int client_send_pc_health(USER* src, int damage, int critical);
int client_send_pc_healthscript(USER* sd, int damage, int critical);

/* Mob damage/health */
int client_mob_damage(USER* sd, MOB* mob);
int client_send_mob_health(MOB* mob, int damage, int critical);
int client_send_mob_healthscript(MOB* mob, int damage, int critical);
int client_send_mob_health_sub(struct block_list* bl, va_list ap);
int client_send_mob_health_sub_nosd(struct block_list* bl, va_list ap);
int client_mob_kill(MOB* mob);

/* Health bars */
void client_send_selfbar(USER* sd);
void client_send_groupbars(USER* sd, USER* tsd);
void client_send_mobbars(struct block_list* bl, va_list ap);

/* Combat calculations */
int client_calc_critical(USER* sd, struct block_list* bl);
int client_parse_attack(USER* sd);

/* Durability/equipment wear */
int client_deduct_weapon(USER* sd, int hit);
int client_deduct_armor(USER* sd, int hit);
int client_deduct_dura(USER* sd, int equip, int val);
int client_deduct_dura_equip(USER* sd);
int client_check_dura(USER* sd, int equip);

/* Duration/aether effects */
int client_send_duration(USER* sd, int id, int time, USER* tsd);
int client_send_aether(USER* sd, int id, int time);
int client_has_aethers(USER* sd, int spell);
int client_find_spell_pos(USER* sd, int id);

#endif /* _CLIENT_COMBAT_H_ */
