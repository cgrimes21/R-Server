/**
 * @file client_combat.c
 * @brief Combat-related packets: health bars, damage, attacks, durability
 *
 * Extracted from clif.c as part of the RTK refactoring project.
 * Handles all combat-related visual updates.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <math.h>

#include "client.h"
#include "client_combat.h"
#include "client_chat.h"
#include "client_visual.h"
#include "client_player.h"
#include "client_inventory.h"
#include "client_crypto.h"
#include "map.h"
#include "mob.h"
#include "pc.h"
#include "itemdb.h"
#include "magic.h"
#include "sl.h"
#include "socket.h"
#include "malloc.h"
#include "db_mysql.h"

/* Player damage/health */

int client_pc_damage(USER* sd, USER* src) {
	USER* tsd = NULL;
	int damage;
	int x;

	nullpo_ret(0, sd);
	nullpo_ret(0, src);

	if (src->status.state == 1) return 0;

	sl_doscript_blargs("hitCritChance", NULL, 2, &sd->bl, &src->bl);

	if (sd->critchance > 0) {
		sl_doscript_blargs("swingDamage", NULL, 2, &sd->bl, &src->bl);
		damage = (int)(sd->damage += 0.5f);

		if (sd->status.equip[EQ_WEAP].id > 0) {
			client_play_sound(&src->bl, itemdb_soundhit(sd->status.equip[EQ_WEAP].id));
		}

		for (x = 0; x < 14; x++) {
			if (sd->status.equip[x].id > 0) {
				sl_doscript_blargs(itemdb_yname(sd->status.equip[x].id), "on_hit", 2, &sd->bl, &src->bl);
			}
		}

		for (x = 0; x < MAX_SPELLS; x++) {
			if (sd->status.skill[x] > 0) {
				sl_doscript_blargs(magicdb_yname(sd->status.skill[x]), "passive_on_hit", 2, &sd->bl, &src->bl);
			}
		}

		for (x = 0; x < MAX_MAGIC_TIMERS; x++) {
			if (sd->status.dura_aether[x].id > 0 && sd->status.dura_aether[x].duration > 0) {
				tsd = map_id2sd(sd->status.dura_aether[x].caster_id);

				if (tsd) {
					sl_doscript_blargs(magicdb_yname(sd->status.dura_aether[x].id), "on_hit_while_cast", 3, &sd->bl, &src->bl, &tsd->bl);
				}
				else {
					sl_doscript_blargs(magicdb_yname(sd->status.dura_aether[x].id), "on_hit_while_cast", 2, &sd->bl, &src->bl);
				}
			}
		}

		if (sd->critchance == 1) {
			client_send_pc_health(src, damage, 33);
		}
		else if (sd->critchance == 2) {
			client_send_pc_health(src, damage, 255);
		}

		client_send_status(src, SFLAG_HPMP);
	}

	return 0;
}

int client_send_pc_health(USER* src, int damage, int critical) {
	struct block_list* bl = map_id2bl(src->attacker);

	if (bl == NULL) {
		bl = map_id2bl(src->bl.id);
	}

	sl_doscript_blargs("player_combat", "on_attacked", 2, &src->bl, bl);
	return 0;
}

int client_send_pc_healthscript(USER* sd, int damage, int critical) {
	unsigned int maxvita;
	unsigned int currentvita;
	float percentage;
	char buf[32];
	int x;
	USER* tsd = NULL;
	MOB* tmob = NULL;
	struct block_list* bl = map_id2bl(sd->attacker);

	nullpo_ret(0, sd);
	maxvita = sd->max_hp;
	currentvita = sd->status.hp;

	if (bl) {
		if (bl->type == BL_MOB) {
			tmob = (MOB*)bl;
			if (tmob->owner < MOB_START_NUM && tmob->owner > 0) {
				tsd = map_id2sd(tmob->owner);
			}
		}
		else if (bl->type == BL_PC) {
			tsd = (USER*)bl;
		}
	}

	if (damage > 0) {
		for (x = 0; x < MAX_SPELLS; x++) {
			if (sd->status.skill[x] > 0) {
				sl_doscript_blargs(magicdb_yname(sd->status.skill[x]), "passive_on_takingdamage", 2, &sd->bl, bl);
			}
		}
	}

	if (damage < 0) {
		sd->lastvita = currentvita;
		currentvita -= damage;
	}
	else {
		if (currentvita < damage) {
			sd->lastvita = currentvita;
			currentvita = 0;
		}
		else {
			sd->lastvita = currentvita;
			currentvita -= damage;
		}
	}

	if (currentvita > maxvita) {
		currentvita = maxvita;
	}

	sd->status.hp = currentvita;

	if (currentvita == 0) {
		percentage = 0;
	}
	else {
		percentage = (float)currentvita / (float)maxvita;
		percentage = (float)percentage * 100;
	}

	if (((int)percentage) == 0 && currentvita != 0) percentage = (float)1;

	WBUFB(buf, 0) = 0xAA;
	WBUFW(buf, 1) = SWAP16(12);
	WBUFB(buf, 3) = 0x13;
	WBUFL(buf, 5) = SWAP32(sd->bl.id);
	WBUFB(buf, 9) = critical;
	WBUFB(buf, 10) = (int)percentage;
	WBUFL(buf, 11) = SWAP32((unsigned int)damage);

	if (sd->status.state == 2) client_send(buf, 32, &sd->bl, SELF);
	else client_send(buf, 32, &sd->bl, AREA);

	if (sd->status.hp && damage > 0) {
		for (x = 0; x < MAX_SPELLS; x++) {
			if (sd->status.skill[x] > 0) {
				sl_doscript_blargs(magicdb_yname(sd->status.skill[x]), "passive_on_takedamage", 2, &sd->bl, bl);
			}
		}
		for (x = 0; x < MAX_MAGIC_TIMERS; x++) {
			if (sd->status.dura_aether[x].id > 0 && sd->status.dura_aether[x].duration > 0) {
				sl_doscript_blargs(magicdb_yname(sd->status.dura_aether[x].id), "on_takedamage_while_cast", 2, &sd->bl, bl);
			}
		}
		for (x = 0; x < 14; x++) {
			if (sd->status.equip[x].id > 0) {
				sl_doscript_blargs(itemdb_yname(sd->status.equip[x].id), "on_takedamage", 2, &sd->bl, bl);
			}
		}
	}

	if (!sd->status.hp) {
		sl_doscript_blargs("onDeathPlayer", NULL, 1, &sd->bl);

		if (tsd != NULL) sl_doscript_blargs("onKill", NULL, 2, &sd->bl, &tsd->bl);
	}

	if (sd->group_count > 0) {
		client_grouphealth_update(sd);
	}

	return 0;
}

/* Mob damage/health */

int client_mob_damage(USER* sd, MOB* mob) {
	int damage;
	int x;

	nullpo_ret(0, sd);
	nullpo_ret(0, mob);

	if (mob->state == MOB_DEAD) return 0;

	sl_doscript_blargs("hitCritChance", NULL, 2, &sd->bl, &mob->bl);

	if (sd->critchance > 0) {
		sl_doscript_blargs("swingDamage", NULL, 2, &sd->bl, &mob->bl);

		if (sd->status.equip[EQ_WEAP].id > 0) {
			client_play_sound(&mob->bl, itemdb_soundhit(sd->status.equip[EQ_WEAP].id));
		}

		if (rnd(100) > 75) {
			client_deduct_dura(sd, EQ_WEAP, 1);
		}

		damage = (int)(sd->damage += 0.5f);
		mob->lastaction = time(NULL);

		for (x = 0; x < MAX_THREATCOUNT; x++) {
			if (mob->threat[x].user == sd->bl.id) {
				mob->threat[x].amount = mob->threat[x].amount + damage;
				break;
			}
			else if (mob->threat[x].user == 0) {
				mob->threat[x].user = sd->bl.id;
				mob->threat[x].amount = damage;
				break;
			}
		}

		for (x = 0; x < 14; x++) {
			if (sd->status.equip[x].id > 0) {
				sl_doscript_blargs(itemdb_yname(sd->status.equip[x].id), "on_hit", 2, &sd->bl, &mob->bl);
			}
		}

		for (x = 0; x < MAX_SPELLS; x++) {
			if (sd->status.skill[x] > 0) {
				sl_doscript_blargs(magicdb_yname(sd->status.skill[x]), "passive_on_hit", 2, &sd->bl, &mob->bl);
			}
		}

		for (x = 0; x < MAX_MAGIC_TIMERS; x++) {
			if (sd->status.dura_aether[x].id > 0 && sd->status.dura_aether[x].duration > 0) {
				sl_doscript_blargs(magicdb_yname(sd->status.dura_aether[x].id), "on_hit_while_cast", 2, &sd->bl, &mob->bl);
			}
		}

		if (sd->critchance == 1) {
			client_send_mob_health(mob, damage, 33);
		}
		else if (sd->critchance == 2) {
			client_send_mob_health(mob, damage, 255);
		}
	}

	return 0;
}

int client_send_mob_health(MOB* mob, int damage, int critical) {
	struct block_list* bl = map_id2bl(mob->attacker);

	if (mob->bl.type != BL_MOB) return 0;

	if (bl == NULL) {
		bl = map_id2bl(mob->bl.id);
	}

	if (mob->data->subtype == 0) {
		sl_doscript_blargs("mob_ai_basic", "on_attacked", 2, &mob->bl, bl);
	}
	else if (mob->data->subtype == 1) {
		sl_doscript_blargs("mob_ai_normal", "on_attacked", 2, &mob->bl, bl);
	}
	else if (mob->data->subtype == 2) {
		sl_doscript_blargs("mob_ai_hard", "on_attacked", 2, &mob->bl, bl);
	}
	else if (mob->data->subtype == 3) {
		sl_doscript_blargs("mob_ai_boss", "on_attacked", 2, &mob->bl, bl);
	}
	else if (mob->data->subtype == 4) {
		sl_doscript_blargs(mob->data->yname, "on_attacked", 2, &mob->bl, bl);
	}
	else if (mob->data->subtype == 5) {
		sl_doscript_blargs("mob_ai_ghost", "on_attacked", 2, &mob->bl, bl);
	}

	return 0;
}

int client_send_mob_healthscript(MOB* mob, int damage, int critical) {
	unsigned int dropid = 0;
	float dmgpct = 0.0f;
	char droptype = 0;
	int maxvita;
	int x;
	int currentvita;
	float percentage;
	struct block_list* bl = NULL;

	if (mob->attacker > 0) {
		bl = map_id2bl(mob->attacker);
	}
	USER* sd = NULL;
	USER* tsd = NULL;
	MOB* tmob = NULL;

	nullpo_ret(0, mob);

	if (bl != NULL) {
		if (bl->type == BL_PC) {
			sd = (USER*)bl;
		}
		else if (bl->type == BL_MOB) {
			tmob = (MOB*)bl;
			if (tmob->owner < MOB_START_NUM && tmob->owner > 0) {
				sd = map_id2sd(tmob->owner);
			}
		}
	}

	if (mob->state == MOB_DEAD) return 0;

	maxvita = mob->maxvita;
	currentvita = mob->current_vita;

	if (damage < 0) {
		if (currentvita - damage > maxvita) {
			mob->maxdmg += (float)(maxvita - currentvita);
		}
		else {
			mob->maxdmg -= (float)(damage);
		}

		mob->lastvita = currentvita;
		currentvita -= damage;
	}
	else {
		if (currentvita < damage) {
			mob->lastvita = currentvita;
			currentvita = 0;
		}
		else {
			mob->lastvita = currentvita;
			currentvita -= damage;
		}
	}

	if (currentvita > maxvita) {
		currentvita = maxvita;
	}

	if (currentvita <= 0) {
		percentage = 0.0f;
	}
	else {
		percentage = (float)currentvita / (float)maxvita;
		percentage = percentage * 100.0f;

		if (percentage < 1.0f && currentvita) percentage = 1.0f;
	}

	if (currentvita > 0 && damage > 0) {
		for (x = 0; x < MAX_MAGIC_TIMERS; x++) {
			struct skill_info* p;
			p = &mob->da[x];

			if (p->id > 0 && p->duration > 0) {
				sl_doscript_blargs(magicdb_yname(p->id), "on_takedamage_while_cast", 2, &mob->bl, bl);
			}
		}
	}

	if (sd != NULL) {
		map_foreachinarea(client_send_mob_health_sub, mob->bl.m, mob->bl.x, mob->bl.y, AREA, BL_PC, sd, mob, critical, (int)percentage, damage);
	}
	else {
		map_foreachinarea(client_send_mob_health_sub_nosd, mob->bl.m, mob->bl.x, mob->bl.y, AREA, BL_PC, mob, critical, (int)percentage, damage);
	}

	mob->current_vita = currentvita;

	if (!mob->current_vita) {
		sl_doscript_blargs(mob->data->yname, "before_death", 2, &mob->bl, &sd->bl);
		sl_doscript_blargs("before_death", NULL, 2, &mob->bl, &sd->bl);

		for (x = 0; x < MAX_MAGIC_TIMERS; x++) {
			if (mob->da[x].id > 0) {
				if (mob->da[x].duration > 0) {
					sl_doscript_blargs(magicdb_yname(mob->da[x].id), "before_death_while_cast", 2, &mob->bl, bl);
				}
			}
		}
	}

	if (!mob->current_vita) {
		mob_flushmagic(mob);
		client_mob_kill(mob);

		if (tmob != NULL && mob->summon == 0) {
			for (x = 0; x < MAX_MAGIC_TIMERS; x++) {
				if (tmob->da[x].id > 0) {
					if (tmob->da[x].duration > 0) {
						sl_doscript_blargs(magicdb_yname(tmob->da[x].id), "on_kill_while_cast", 2, &tmob->bl, &mob->bl);
					}
				}
			}
		}

		if (sd != NULL && mob->summon == 0) {
			if (tmob == NULL) {
				for (x = 0; x < MAX_MAGIC_TIMERS; x++) {
					if (sd->status.dura_aether[x].id > 0) {
						if (sd->status.dura_aether[x].duration > 0) {
							sl_doscript_blargs(magicdb_yname(sd->status.dura_aether[x].id), "on_kill_while_cast", 2, &sd->bl, &mob->bl);
						}
					}
				}

				for (x = 0; x < MAX_SPELLS; x++) {
					if (sd->status.skill[x] > 0) {
						sl_doscript_blargs(magicdb_yname(sd->status.skill[x]), "passive_on_kill", 2, &sd->bl, &mob->bl);
					}
				}
			}

			for (x = 0; x < MAX_THREATCOUNT; x++) {
				if (mob->dmggrptable[x][1] / mob->maxdmg > dmgpct) {
					dropid = mob->dmggrptable[x][0];
					dmgpct = mob->dmggrptable[x][1] / mob->maxdmg;
				}

				if (mob->dmgindtable[x][1] / mob->maxdmg > dmgpct) {
					dropid = mob->dmgindtable[x][0];
					dmgpct = mob->dmgindtable[x][1] / mob->maxdmg;
					droptype = 1;
				}
			}

			if (droptype == 1) {
				tsd = map_id2sd(dropid);
			}
			else {
				tsd = map_id2sd(groups[dropid][0]);
			}

			if (tsd != NULL) {
				mobdb_drops(mob, tsd);
			}
			else {
				mobdb_drops(mob, sd);
			}

			if (sd->group_count == 0) {
				if (mob->exp) addtokillreg(sd, mob->mobid);
			}
			else if (sd->group_count > 0) { client_add_to_killreg(sd, mob->mobid); }

			sl_doscript_blargs("onGetExp", NULL, 2, &sd->bl, &mob->bl);

			if (sd->group_count == 0) {
				pc_checklevel(sd);
			}
			else if (sd->group_count > 0) {
				for (x = 0; x < sd->group_count; x++) {
					tsd = map_id2sd(groups[sd->groupid][x]);
					if (!tsd)
						continue;
					if (tsd->bl.m == sd->bl.m && tsd->status.state != 1) {
						pc_checklevel(tsd);
					}
				}
			}

			sl_doscript_blargs("onKill", NULL, 2, &mob->bl, &sd->bl);
		}

		for (x = 0; x < MAX_MAGIC_TIMERS; x++) {
			if (mob->da[x].id > 0) {
				sl_doscript_blargs(magicdb_yname(mob->da[x].id), "after_death_while_cast", 2, &mob->bl, bl);
			}
		}

		sl_doscript_blargs(mob->data->yname, "after_death", 2, &mob->bl, bl);
		sl_doscript_blargs("after_death", NULL, 2, &mob->bl, &sd->bl);
	}

	return 0;
}

int client_send_mob_health_sub(struct block_list* bl, va_list ap) {
	USER* sd = NULL;
	USER* tsd = NULL;
	MOB* mob = NULL;

	int x;
	int critical;
	int percentage;
	int damage;
	nullpo_ret(0, sd = va_arg(ap, USER*));
	nullpo_ret(0, mob = va_arg(ap, MOB*));
	critical = va_arg(ap, int);
	percentage = va_arg(ap, int);
	damage = va_arg(ap, int);
	nullpo_ret(0, tsd = (USER*)bl);

	if (!client_is_in_group(tsd, sd)) {
		if (sd->bl.id != bl->id) {
			return 0;
		}
	}

	if (!session[sd->fd])
	{
		session[sd->fd]->eof = 8;
		return 0;
	}

	WFIFOHEAD(tsd->fd, 15);
	WFIFOHEADER(tsd->fd, 0x13, 12);
	WFIFOL(tsd->fd, 5) = SWAP32(mob->bl.id);
	WFIFOB(tsd->fd, 9) = critical;
	WFIFOB(tsd->fd, 10) = (int)percentage;
	WFIFOL(tsd->fd, 11) = SWAP32((unsigned int)damage);
	WFIFOSET(tsd->fd, client_encrypt(tsd->fd));
	return 0;
}

int client_send_mob_health_sub_nosd(struct block_list* bl, va_list ap) {
	USER* sd = NULL;
	MOB* mob = NULL;

	int x;
	int critical;
	int percentage;
	int damage;
	nullpo_ret(0, mob = va_arg(ap, MOB*));
	critical = va_arg(ap, int);
	percentage = va_arg(ap, int);
	damage = va_arg(ap, int);
	nullpo_ret(0, sd = (USER*)bl);

	if (!session[sd->fd])
	{
		session[sd->fd]->eof = 8;
		return 0;
	}

	WFIFOHEAD(sd->fd, 15);
	WFIFOHEADER(sd->fd, 0x13, 12);
	WFIFOL(sd->fd, 5) = SWAP32(mob->bl.id);
	WFIFOB(sd->fd, 9) = critical;
	WFIFOB(sd->fd, 10) = (int)percentage;
	WFIFOL(sd->fd, 11) = SWAP32((unsigned int)damage);
	WFIFOSET(sd->fd, client_encrypt(sd->fd));

	return 0;
}

int client_mob_kill(MOB* mob) {
	int x;

	for (x = 0; x < MAX_THREATCOUNT; x++) {
		mob->threat[x].user = 0;
		mob->threat[x].amount = 0;
		mob->dmggrptable[x][0] = 0;
		mob->dmggrptable[x][1] = 0;
		mob->dmgindtable[x][0] = 0;
		mob->dmgindtable[x][1] = 0;
	}

	mob->dmgdealt = 0;
	mob->dmgtaken = 0;
	mob->maxdmg = mob->data->vita;
	mob->state = MOB_DEAD;
	mob->last_death = time(NULL);

	if (!mob->onetime) map_lastdeath_mob(mob);
	map_foreachinarea(client_send_destroy, mob->bl.m, mob->bl.x, mob->bl.y, AREA, BL_PC, LOOK_GET, &mob->bl);
	return 0;
}

/* Health bars */

void client_send_selfbar(USER* sd) {
	float percentage;

	if (sd->status.hp == 0) {
		percentage = 0;
	}
	else {
		percentage = (float)sd->status.hp / (float)sd->max_hp;
		percentage = (float)percentage * 100;
	}

	if ((int)percentage == 0 && sd->status.hp != 0) percentage = (float)1;

	if (!session[sd->fd])
	{
		session[sd->fd]->eof = 8;
		return;
	}

	WFIFOHEAD(sd->fd, 15);
	WFIFOB(sd->fd, 0) = 0xAA;
	WFIFOW(sd->fd, 1) = SWAP16(12);
	WFIFOB(sd->fd, 3) = 0x13;
	WFIFOL(sd->fd, 5) = SWAP32(sd->bl.id);
	WFIFOB(sd->fd, 9) = 0;
	WFIFOB(sd->fd, 10) = (int)percentage;
	WFIFOL(sd->fd, 11) = SWAP32(0);
	WFIFOSET(sd->fd, client_encrypt(sd->fd));
}

void client_send_groupbars(USER* sd, USER* tsd) {
	float percentage;

	nullpo_ret(0, sd);
	nullpo_ret(0, tsd);

	if (tsd->status.hp == 0) {
		percentage = 0;
	}
	else {
		percentage = (float)tsd->status.hp / (float)tsd->max_hp;
		percentage = (float)percentage * 100;
	}

	if ((int)percentage == 0 && tsd->status.hp != 0) percentage = (float)1;

	if (!session[sd->fd])
	{
		session[sd->fd]->eof = 8;
		return;
	}

	WFIFOHEAD(sd->fd, 15);
	WFIFOB(sd->fd, 0) = 0xAA;
	WFIFOW(sd->fd, 1) = SWAP16(12);
	WFIFOB(sd->fd, 3) = 0x13;
	WFIFOL(sd->fd, 5) = SWAP32(tsd->bl.id);
	WFIFOB(sd->fd, 9) = 0;
	WFIFOB(sd->fd, 10) = (int)percentage;
	WFIFOL(sd->fd, 11) = SWAP32(0);
	WFIFOSET(sd->fd, client_encrypt(sd->fd));
}

void client_send_mobbars(struct block_list* bl, va_list ap) {
	USER* sd = NULL;
	MOB* mob = NULL;
	float percentage;

	nullpo_ret(0, sd = va_arg(ap, USER*));
	nullpo_ret(0, mob = (MOB*)bl);

	if (mob->current_vita == 0) {
		percentage = 0;
	}
	else {
		percentage = (float)mob->current_vita / (float)mob->maxvita;
		percentage = (float)percentage * 100;
	}

	if ((int)percentage == 0 && mob->current_vita != 0) percentage = (float)1;

	if (!session[sd->fd])
	{
		session[sd->fd]->eof = 8;
		return;
	}

	WFIFOHEAD(sd->fd, 15);
	WFIFOB(sd->fd, 0) = 0xAA;
	WFIFOW(sd->fd, 1) = SWAP16(12);
	WFIFOB(sd->fd, 3) = 0x13;
	WFIFOL(sd->fd, 5) = SWAP32(mob->bl.id);
	WFIFOB(sd->fd, 9) = 0;
	WFIFOB(sd->fd, 10) = (int)percentage;
	WFIFOL(sd->fd, 11) = SWAP32(0);
	WFIFOSET(sd->fd, client_encrypt(sd->fd));
}

/* Combat calculations */

int client_find_spell_pos(USER* sd, int id) {
	int x;
	for (x = 0; x < 52; x++) {
		if (sd->status.skill[x] == id) {
			return x;
		}
	}

	return -1;
}

int client_calc_critical(USER* sd, struct block_list* bl) {
	int chance;
	int equat = 0;
	MOB* mob = NULL;
	USER* tsd = NULL;
	int crit = 0;
	int max_hit = 95;
	int dx, dy;

	if (bl->type == BL_PC) {
		tsd = (USER*)bl;
		equat = (55 + (sd->grace / 2)) - (tsd->grace / 2) + (sd->hit * 1.5) + (sd->status.level - tsd->status.level);
	}
	else if (bl->type == BL_MOB) {
		mob = (MOB*)bl;
		equat = (55 + (sd->grace / 2)) - (mob->data->grace / 2) + (sd->hit * 1.5) + (sd->status.level - mob->data->level);
	}
	if (equat < 5) equat = 5;
	if (equat > max_hit) equat = max_hit;

	chance = rnd(100);
	if (chance < (equat)) {
		crit = sd->hit / 3;
		if (crit < 1) crit = 1;
		if (crit >= 100) crit = 99;
		if (chance < crit) {
			return 2;
		}
		else {
			return 1;
		}
	}
	return 0;
}

int client_parse_attack(USER* sd) {
	struct block_list* bl = NULL;
	USER* tsd = NULL;

	int id;
	int attackspeed;
	int x;
	int tick = 0;
	int exist = -1;

	attackspeed = sd->attack_speed;

	if (sd->paralyzed || sd->sleep != 1.0f) return 0;

	id = sd->status.equip[EQ_WEAP].id;

	if (sd->status.state == 1 || sd->status.state == 3) return 0;

	if (itemdb_sound(sd->status.equip[EQ_WEAP].id) == 0) {
		client_send_action(&sd->bl, 1, attackspeed, 9);
	}
	else {
		client_send_action(&sd->bl, 1, attackspeed, itemdb_sound(sd->status.equip[EQ_WEAP].id));
	}

	sl_doscript_blargs("swingDamage", NULL, 1, &sd->bl);

	sl_doscript_blargs("swing", NULL, 1, &sd->bl);
	sl_doscript_blargs("onSwing", NULL, 1, &sd->bl);

	if (itemdb_look(sd->status.equip[EQ_WEAP].id) >= 20000 && itemdb_look(sd->status.equip[EQ_WEAP].id) < 30000) {
		sl_doscript_blargs(itemdb_yname(sd->status.equip[EQ_WEAP].id), "shootArrow", NULL, 1, &sd->bl);
		sl_doscript_blargs("shootArrow", NULL, 1, &sd->bl);
	}

	for (x = 0; x < 14; x++) {
		if (sd->status.equip[x].id > 0) {
			sl_doscript_blargs(itemdb_yname(sd->status.equip[x].id), "on_swing", 1, &sd->bl);
		}
	}

	for (x = 0; x < MAX_SPELLS; x++) {
		if (sd->status.skill[x] > 0) {
			sl_doscript_blargs(magicdb_yname(sd->status.skill[x]), "passive_on_swing", 1, &sd->bl);
		}
	}

	for (x = 0; x < MAX_MAGIC_TIMERS; x++) {
		if (sd->status.dura_aether[x].id > 0) {
			if (sd->status.dura_aether[x].duration > 0) {
				sl_doscript_simple(magicdb_yname(sd->status.dura_aether[x].id), "on_swing_while_cast", &sd->bl);
			}
		}
	}

	return 0;
}

/* Durability/equipment wear */

int client_deduct_dura(USER* sd, int equip, int val) {
	unsigned char eth;
	nullpo_ret(0, sd);
	if (!sd->status.equip[equip].id) return 0;
	if (map[sd->bl.m].pvp) return 0;

	eth = itemdb_ethereal(sd->status.equip[equip].id);

	if (!eth) {
		sd->status.equip[equip].dura -= val;
		client_check_dura(sd, equip);
	}
	return 0;
}

int client_deduct_weapon(USER* sd, int hit) {
	if (pc_isequip(sd, EQ_WEAP)) {
		if (rnd(100) > 50) {
			client_deduct_dura(sd, EQ_WEAP, hit);
		}
	}

	return 0;
}

int client_deduct_armor(USER* sd, int hit) {
	if (pc_isequip(sd, EQ_WEAP)) {
		if (rnd(100) > 50) {
			client_deduct_dura(sd, EQ_WEAP, hit);
		}
	}
	if (pc_isequip(sd, EQ_HELM)) {
		if (rnd(100) > 50) {
			client_deduct_dura(sd, EQ_HELM, hit);
		}
	}
	if (pc_isequip(sd, EQ_ARMOR)) {
		if (rnd(100) > 50) {
			client_deduct_dura(sd, EQ_ARMOR, hit);
		}
	}
	if (pc_isequip(sd, EQ_LEFT)) {
		if (rnd(100) > 50) {
			client_deduct_dura(sd, EQ_LEFT, hit);
		}
	}
	if (pc_isequip(sd, EQ_RIGHT)) {
		if (rnd(100) > 50) {
			client_deduct_dura(sd, EQ_RIGHT, hit);
		}
	}
	if (pc_isequip(sd, EQ_SUBLEFT)) {
		if (rnd(100) > 50) {
			client_deduct_dura(sd, EQ_SUBLEFT, hit);
		}
	}
	if (pc_isequip(sd, EQ_SUBRIGHT)) {
		if (rnd(100) > 50) {
			client_deduct_dura(sd, EQ_SUBRIGHT, hit);
		}
	}
	if (pc_isequip(sd, EQ_BOOTS)) {
		if (rnd(100) > 50) {
			client_deduct_dura(sd, EQ_BOOTS, hit);
		}
	}
	if (pc_isequip(sd, EQ_MANTLE)) {
		if (rnd(100) > 50) {
			client_deduct_dura(sd, EQ_MANTLE, hit);
		}
	}
	if (pc_isequip(sd, EQ_COAT)) {
		if (rnd(100) > 50) {
			client_deduct_dura(sd, EQ_COAT, hit);
		}
	}
	if (pc_isequip(sd, EQ_SHIELD)) {
		if (rnd(100) > 50) {
			client_deduct_dura(sd, EQ_SHIELD, hit);
		}
	}
	if (pc_isequip(sd, EQ_FACEACC)) {
		if (rnd(100) > 50) {
			client_deduct_dura(sd, EQ_FACEACC, hit);
		}
	}
	if (pc_isequip(sd, EQ_CROWN)) {
		if (rnd(100) > 50) {
			client_deduct_dura(sd, EQ_CROWN, hit);
		}
	}
	if (pc_isequip(sd, EQ_NECKLACE)) {
		if (rnd(100) > 50) {
			client_deduct_dura(sd, EQ_NECKLACE, hit);
		}
	}
	return 0;
}

int client_check_dura(USER* sd, int equip) {
	float percentage;
	int id;
	char buf[255];
	char escape[255];

	nullpo_ret(0, sd);
	if (!sd->status.equip[equip].id) return 0;
	id = sd->status.equip[equip].id;

	sd->equipslot = equip;

	percentage = (float)sd->status.equip[equip].dura / (float)itemdb_dura(id);

	if (percentage <= .5 && sd->status.equip[equip].repair == 0) {
		sprintf(buf, "Your %s is at 50%%.", itemdb_name(id));
		client_send_msg(sd, 5, buf);
		sd->status.equip[equip].repair = 1;
	}

	if (percentage <= .25 && sd->status.equip[equip].repair == 1) {
		sprintf(buf, "Your %s is at 25%%.", itemdb_name(id));
		client_send_msg(sd, 5, buf);
		sd->status.equip[equip].repair = 2;
	}

	if (percentage <= .1 && sd->status.equip[equip].repair == 2) {
		sprintf(buf, "Your %s is at 10%%.", itemdb_name(id));
		client_send_msg(sd, 5, buf);
		sd->status.equip[equip].repair = 3;
	}

	if (percentage <= .05 && sd->status.equip[equip].repair == 3) {
		sprintf(buf, "Your %s is at 5%%.", itemdb_name(id));
		client_send_msg(sd, 5, buf);
		sd->status.equip[equip].repair = 4;
	}

	if (percentage <= .01 && sd->status.equip[equip].repair == 4) {
		sprintf(buf, "Your %s is at 1%%.", itemdb_name(id));
		client_send_msg(sd, 5, buf);
		sd->status.equip[equip].repair = 5;
	}

	if (sd->status.equip[equip].dura <= 0 || (sd->status.state == 1 && itemdb_breakondeath(sd->status.equip[equip].id) == 1)) {
		if (itemdb_protected(sd->status.equip[equip].id) || sd->status.equip[equip].protected >= 1) {
			sd->status.equip[equip].protected -= 1;
			sd->status.equip[equip].dura = itemdb_dura(sd->status.equip[equip].id);
			sprintf(buf, "Your %s has been restored!", itemdb_name(id));
			client_send_status(sd, SFLAG_FULLSTATS | SFLAG_HPMP);
			client_send_msg(sd, 5, buf);
			sl_doscript_blargs("characterLog", "equipRestore", 1, &sd->bl);
			return 0;
		}

		sl_doscript_blargs("characterLog", "equipBreak", 1, &sd->bl);
		sprintf(buf, "Your %s was destroyed!", itemdb_name(id));

		sd->breakid = id;
		sl_doscript_blargs("onBreak", NULL, 1, &sd->bl);
		sl_doscript_blargs(itemdb_yname(id), "on_break", 1, &sd->bl);

		Sql_EscapeString(sql_handle, escape, sd->status.equip[equip].real_name);

		sd->status.equip[equip].id = 0;
		sd->status.equip[equip].dura = 0;
		sd->status.equip[equip].amount = 0;
		sd->status.equip[equip].protected = 0;
		sd->status.equip[equip].owner = 0;
		sd->status.equip[equip].custom = 0;
		sd->status.equip[equip].customLook = 0;
		sd->status.equip[equip].customLookColor = 0;
		sd->status.equip[equip].customIcon = 0;
		sd->status.equip[equip].customIconColor = 0;
		memset(sd->status.equip[equip].trapsTable, 0, 100);
		sd->status.equip[equip].time = 0;
		sd->status.equip[equip].repair = 0;
		strcpy(sd->status.equip[equip].real_name, "");
		client_unequip_item(sd, client_get_equip_type(equip));

		map_foreachinarea(client_update_state, sd->bl.m, sd->bl.x, sd->bl.y, AREA, BL_PC, sd);
		pc_calcstat(sd);
		client_send_status(sd, SFLAG_FULLSTATS | SFLAG_HPMP);
		client_send_msg(sd, 5, buf);
	}

	return 0;
}

int client_deduct_dura_equip(USER* sd) {
	float percentage;
	int id;
	char buf[255];
	char escape[255];

	nullpo_ret(0, sd);

	for (int equip = 0; equip < 14; equip++) {
		if (!sd->status.equip[equip].id) continue;
		id = sd->status.equip[equip].id;

		unsigned char eth;
		nullpo_ret(0, sd);
		if (!sd->status.equip[equip].id) return 0;
		if (map[sd->bl.m].pvp) return 0;

		eth = itemdb_ethereal(sd->status.equip[equip].id);

		if (eth) continue;

		sd->equipslot = equip;

		sd->status.equip[equip].dura -= floor(itemdb_dura(sd->status.equip[equip].id) * 0.10);

		percentage = (float)sd->status.equip[equip].dura / (float)itemdb_dura(id);

		if (percentage <= .5 && sd->status.equip[equip].repair == 0) {
			sprintf(buf, "Your %s is at 50%%.", itemdb_name(id));
			client_send_msg(sd, 5, buf);
			sd->status.equip[equip].repair = 1;
		}

		if (percentage <= .25 && sd->status.equip[equip].repair == 1) {
			sprintf(buf, "Your %s is at 25%%.", itemdb_name(id));
			client_send_msg(sd, 5, buf);
			sd->status.equip[equip].repair = 2;
		}

		if (percentage <= .1 && sd->status.equip[equip].repair == 2) {
			sprintf(buf, "Your %s is at 10%%.", itemdb_name(id));
			client_send_msg(sd, 5, buf);
			sd->status.equip[equip].repair = 3;
		}

		if (percentage <= .05 && sd->status.equip[equip].repair == 3) {
			sprintf(buf, "Your %s is at 5%%.", itemdb_name(id));
			client_send_msg(sd, 5, buf);
			sd->status.equip[equip].repair = 4;
		}

		if (percentage <= .01 && sd->status.equip[equip].repair == 4) {
			sprintf(buf, "Your %s is at 1%%.", itemdb_name(id));
			client_send_msg(sd, 5, buf);
			sd->status.equip[equip].repair = 5;
		}

		if (sd->status.equip[equip].dura <= 0 || (sd->status.state == 1 && itemdb_breakondeath(sd->status.equip[equip].id) == 1)) {
			if (itemdb_protected(sd->status.equip[equip].id) || sd->status.equip[equip].protected >= 1) {
				sd->status.equip[equip].protected -= 1;
				sd->status.equip[equip].dura = itemdb_dura(sd->status.equip[equip].id);
				sprintf(buf, "Your %s has been restored!", itemdb_name(id));
				client_send_status(sd, SFLAG_FULLSTATS | SFLAG_HPMP);
				client_send_msg(sd, 5, buf);
				sl_doscript_blargs("characterLog", "equipRestore", 1, &sd->bl);
				return 0;
			}

			memcpy(&sd->boditems.item[sd->boditems.bod_count], &sd->status.equip[equip], sizeof(sd->status.equip[equip]));
			sd->boditems.bod_count++;

			sl_doscript_blargs("characterLog", "equipBreak", 1, &sd->bl);
			sprintf(buf, "Your %s was destroyed!", itemdb_name(id));

			Sql_EscapeString(sql_handle, escape, sd->status.equip[equip].real_name);

			sd->breakid = id;
			sl_doscript_blargs("onBreak", NULL, 1, &sd->bl);
			sl_doscript_blargs(itemdb_yname(id), "on_break", 1, &sd->bl);

			sd->status.equip[equip].id = 0;
			sd->status.equip[equip].dura = 0;
			sd->status.equip[equip].amount = 0;
			sd->status.equip[equip].protected = 0;
			sd->status.equip[equip].owner = 0;
			sd->status.equip[equip].custom = 0;
			sd->status.equip[equip].customLook = 0;
			sd->status.equip[equip].customLookColor = 0;
			sd->status.equip[equip].customIcon = 0;
			sd->status.equip[equip].customIconColor = 0;
			memset(sd->status.equip[equip].trapsTable, 0, 100);
			sd->status.equip[equip].time = 0;
			sd->status.equip[equip].repair = 0;
			strcpy(sd->status.equip[equip].real_name, "");
			client_unequip_item(sd, client_get_equip_type(equip));

			map_foreachinarea(client_update_state, sd->bl.m, sd->bl.x, sd->bl.y, AREA, BL_PC, sd);
			pc_calcstat(sd);
			client_send_status(sd, SFLAG_FULLSTATS | SFLAG_HPMP);
			client_send_msg(sd, 5, buf);
		}
	}

	sl_doscript_blargs("characterLog", "bodLog", 1, &sd->bl);
	sd->boditems.bod_count = 0;

	return 0;
}

/* Duration/aether effects */

int client_send_duration(USER* sd, int id, int time, USER* tsd) {
	int len;

	nullpo_ret(0, sd);

	char* name = NULL;
	name = magicdb_name(id);

	if (!magicdb_ticker(id)) return 0;

	if (id == 0) {
		len = 6;
	}
	else if (tsd) {
		len = strlen(name) + strlen(tsd->status.name) + 3;
	}
	else {
		len = strlen(name);
	}

	if (!session[sd->fd])
	{
		session[sd->fd]->eof = 8;
		return 0;
	}

	WFIFOHEAD(sd->fd, len + 10);
	WFIFOB(sd->fd, 5) = len;

	if (id == 0) {
		strcpy(WFIFOP(sd->fd, 6), "Shield");
	}
	else if (tsd != NULL) {
		char buf[len];
		sprintf(buf, "%s (%s)", name, tsd->status.name);
		strcpy(WFIFOP(sd->fd, 6), buf);
	}
	else {
		strcpy(WFIFOP(sd->fd, 6), name);
	}

	WFIFOL(sd->fd, len + 6) = SWAP32(time);

	WFIFOHEADER(sd->fd, 0x3A, len + 7);
	WFIFOSET(sd->fd, client_encrypt(sd->fd));

	return 0;
}

int client_send_aether(USER* sd, int id, int time) {
	int pos;

	nullpo_ret(0, sd);
	pos = client_find_spell_pos(sd, id);
	if (pos < 0) return 0;

	if (!session[sd->fd])
	{
		session[sd->fd]->eof = 8;
		return 0;
	}

	WFIFOHEAD(sd->fd, 11);
	WFIFOHEADER(sd->fd, 63, 8);
	WFIFOW(sd->fd, 5) = SWAP16(pos + 1);
	WFIFOL(sd->fd, 7) = SWAP32(time);
	WFIFOSET(sd->fd, client_encrypt(sd->fd));
	return 0;
}

int client_has_aethers(USER* sd, int spell) {
	int x;

	for (x = 0; x < MAX_MAGIC_TIMERS; x++) {
		if (sd->status.dura_aether[x].id == spell) {
			return sd->status.dura_aether[x].aether;
		}
	}

	return 0;
}
