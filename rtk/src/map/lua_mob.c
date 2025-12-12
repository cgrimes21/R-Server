/**
 * @file sl_mob.c
 * @brief Mob type Lua bindings implementation
 *
 * Extracted from sl.c as part of the RTK refactoring project.
 * Contains mob-related Lua type implementations.
 */

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <string.h>
#include <stdlib.h>

#include "map.h"
#include "mob.h"
#include "magic.h"
#include "client.h"
#include "lua_core.h"
#include "lua_types.h"
#include "lua_blocklist.h"
#include "lua_registry.h"
#include "lua_mob.h"

/*==============================================================================
 * Mob Type Declaration
 *============================================================================*/

lua_type_class lua_mob_type;

/* Push instance macros */
#define lua_mob_pushinst(state, mob) lua_type_pushinst(state, &lua_mob_type, mob, 0)
#define mobregl_pushinst(state, mob) lua_type_pushinst(state, &mobregl_type, mob, 0)
#define mapregl_pushinst(state, mob) lua_type_pushinst(state, &mapregl_type, mob, 0)
#define gameregl_pushinst(state, mob) lua_type_pushinst(state, &gameregl_type, mob, 0)

/*==============================================================================
 * Mob Type Implementation
 *============================================================================*/

void lua_mob_staticinit() {
	lua_mob_type = lua_type_new("Mob", lua_mob_ctor);
	lua_mob_type.getattr = lua_mob_getattr;
	lua_mob_type.setattr = lua_mob_setattr;
	lua_mob_type.init = lua_mob_init;
	lua_type_extendproto(&lua_mob_type, "attack", lua_mob_attack);
	lua_type_extendproto(&lua_mob_type, "addHealth", lua_mob_addhealth);
	lua_type_extendproto(&lua_mob_type, "move", lua_mob_move);
	lua_type_extendproto(&lua_mob_type, "moveIgnoreObject", lua_mob_move_ignore_object);
	lua_type_extendproto(&lua_mob_type, "setDuration", lua_mob_setduration);
	lua_type_extendproto(&lua_mob_type, "moveIntent", lua_mob_moveintent);
	lua_type_extendproto(&lua_mob_type, "hasDuration", lua_mob_hasduration);
	lua_type_extendproto(&lua_mob_type, "hasDurationID", lua_mob_hasdurationid);
	lua_type_extendproto(&lua_mob_type, "getDuration", lua_mob_getduration);
	lua_type_extendproto(&lua_mob_type, "getDurationID", lua_mob_getdurationid);
	lua_type_extendproto(&lua_mob_type, "removeHealth", lua_mob_removehealth);
	lua_type_extendproto(&lua_mob_type, "flushDuration", lua_mob_flushduration);
	lua_type_extendproto(&lua_mob_type, "flushDurationNoUncast", lua_mob_flushdurationnouncast);
	lua_type_extendproto(&lua_mob_type, "durationAmount", lua_mob_durationamount);
	lua_type_extendproto(&lua_mob_type, "checkThreat", lua_mob_checkthreat);
	lua_type_extendproto(&lua_mob_type, "sendHealth", lua_mob_sendhealth);
	lua_type_extendproto(&lua_mob_type, "warp", lua_mob_warp);
	lua_type_extendproto(&lua_mob_type, "moveGhost", lua_mob_moveghost);
	lua_type_extendproto(&lua_mob_type, "callBase", lua_mob_callbase);
	lua_type_extendproto(&lua_mob_type, "checkMove", lua_mob_checkmove);
	lua_type_extendproto(&lua_mob_type, "setIndDmg", lua_mob_setinddmg);
	lua_type_extendproto(&lua_mob_type, "setGrpDmg", lua_mob_setgrpdmg);
	lua_type_extendproto(&lua_mob_type, "getIndDmg", lua_mob_getinddmg);
	lua_type_extendproto(&lua_mob_type, "getGrpDmg", lua_mob_getgrpdmg);
	lua_type_extendproto(&lua_mob_type, "getEquippedItem", lua_mob_getequippeditem);
	lua_type_extendproto(&lua_mob_type, "calcStat", lua_mob_calcstat);
	lua_type_extendproto(&lua_mob_type, "sendStatus", lua_mob_sendstatus);
	lua_type_extendproto(&lua_mob_type, "sendMinitext", lua_mob_sendminitext);

	lua_blocklist_extendproto(&lua_mob_type);
}

int lua_mob_durationamount(lua_State* state, MOB* mob) {
	sl_checkargs(state, "n");
	int x, id;
	id = magicdb_id(lua_tostring(state, sl_memberarg(1)));

	for (x = 0; x < MAX_MAGIC_TIMERS; x++) {
		if (mob->da[x].id == id) {
			if (mob->da[x].duration > 0) {
				lua_pushnumber(state, mob->da[x].duration);
				return 1;
			}
		}
	}
	lua_pushnumber(state, 0);
	return 1;
}

int lua_mob_ctor(lua_State* state) {
	MOB* mob = NULL;
	if (lua_isnumber(state, 2)) {
		unsigned int id = lua_tonumber(state, 2);
		mob = map_id2mob(id);
		if (!mob) {
			lua_pushnil(state);
			return 1;
		}
	}
	else {
		luaL_argerror(state, 1, "expected number");
	}
	lua_mob_pushinst(state, mob);
	return 1;
}

int lua_mob_removehealth(lua_State* state, MOB* mob) {
	sl_checkargs(state, "n");
	int damage = lua_tonumber(state, sl_memberarg(1));
	int caster = lua_tonumber(state, sl_memberarg(2));
	struct block_list* bl = NULL;
	USER* tsd = NULL;
	MOB* tmob = NULL;

	if (caster > 0) {
		bl = map_id2bl(caster);
		mob->attacker = caster;
	}
	else {
		bl = map_id2bl(mob->attacker);
	}

	if (bl != NULL) {
		if (bl->type == BL_PC) {
			tsd = (USER*)bl;
			tsd->damage = damage;
			tsd->critchance = 0;
		}
		else if (bl->type == BL_MOB) {
			tmob = (MOB*)bl;
			tmob->damage = damage;
			tmob->critchance = 0;
		}
	}
	else {
		mob->damage = damage;
		mob->critchance = 0;
	}

	if (mob->state != MOB_DEAD) {
		client_send_mob_healthscript(mob, damage, 0);
	}
	return 0;
}

int lua_mob_moveintent(lua_State* state, MOB* mob) {
	struct block_list* bl = map_id2bl(lua_tonumber(state, sl_memberarg(1)));
	nullpo_ret(0, bl);
	lua_pushnumber(state, move_mob_intent(mob, bl));
	return 1;
}

int lua_mob_setduration(lua_State* state, MOB* mob) {
	sl_checkargs(state, "sn");
	struct block_list* bl = NULL;
	int id, time, x, caster_id, recast, alreadycast, mid;

	alreadycast = 0;
	id = magicdb_id(lua_tostring(state, sl_memberarg(1)));
	time = lua_tonumber(state, sl_memberarg(2));
	if (time < 1000 && time > 0) time = 1000;
	caster_id = lua_tonumber(state, sl_memberarg(3));
	recast = lua_tonumber(state, sl_memberarg(4));

	for (x = 0; x < MAX_MAGIC_TIMERS; x++) {
		if (mob->da[x].id == id && mob->da[x].caster_id == caster_id && mob->da[x].duration > 0) {
			alreadycast = 1;
		}
	}

	for (x = 0; x < MAX_MAGIC_TIMERS; x++) {
		mid = mob->da[x].id;
		if (mid == id && time <= 0 && mob->da[x].caster_id == caster_id && alreadycast == 1) {
			mob->da[x].duration = 0;
			mob->da[x].id = 0;
			mob->da[x].caster_id = 0;
			map_foreachinarea(client_send_animation, mob->bl.m, mob->bl.x, mob->bl.y, AREA, BL_PC, mob->da[x].animation, &mob->bl, -1);
			mob->da[x].animation = 0;

			if (mob->da[x].caster_id != mob->bl.id) {
				bl = map_id2bl(mob->da[x].caster_id);
			}

			if (bl != NULL) {
				sl_doscript_blargs(magicdb_yname(mid), "uncast", 2, &mob->bl, bl);
			}
			else {
				sl_doscript_blargs(magicdb_yname(mid), "uncast", 1, &mob->bl);
			}

			return 0;
		}
		else if (mob->da[x].id == id && mob->da[x].caster_id == caster_id && (mob->da[x].duration > time || recast == 1) && alreadycast == 1) {
			mob->da[x].duration = time;
			return 0;
		}
		else if (mob->da[x].id == 0 && mob->da[x].duration == 0 && time != 0 && alreadycast != 1) {
			mob->da[x].id = id;
			mob->da[x].duration = time;
			mob->da[x].caster_id = caster_id;
			return 0;
		}
	}

	return 0;
}

int lua_mob_flushduration(lua_State* state, MOB* mob) {
	struct block_list* bl = NULL;
	int x;
	int id;
	int dis = lua_tonumber(state, sl_memberarg(1));
	int minid = lua_tonumber(state, sl_memberarg(2));
	int maxid = lua_tonumber(state, sl_memberarg(3));
	char flush = 0;

	if (maxid < minid) {
		maxid = minid;
	}

	for (x = 0; x < MAX_MAGIC_TIMERS; x++) {
		id = mob->da[x].id;

		if (magicdb_dispel(id) > dis) continue;

		if (minid <= 0) {
			flush = 1;
		}
		else if (minid > 0 && maxid <= 0) {
			if (id == minid) {
				flush = 1;
			}
			else {
				flush = 0;
			}
		}
		else {
			if (id >= minid && id <= maxid) {
				flush = 1;
			}
			else {
				flush = 0;
			}
		}

		if (flush == 1) {
			mob->da[x].duration = 0;
			map_foreachinarea(client_send_animation, mob->bl.m, mob->bl.x, mob->bl.y, AREA, BL_PC, mob->da[x].animation, &mob->bl, -1);
			mob->da[x].animation = 0;
			mob->da[x].id = 0;
			bl = map_id2bl(mob->da[x].caster_id);
			mob->da[x].caster_id = 0;

			if (bl != NULL) {
				sl_doscript_blargs(magicdb_yname(id), "uncast", 2, &mob->bl, bl);
			}
			else {
				sl_doscript_blargs(magicdb_yname(id), "uncast", 1, &mob->bl);
			}
		}
	}

	return 0;
}

int lua_mob_flushdurationnouncast(lua_State* state, MOB* mob) {
	int x;
	int id;
	int dis = lua_tonumber(state, sl_memberarg(1));
	int minid = lua_tonumber(state, sl_memberarg(2));
	int maxid = lua_tonumber(state, sl_memberarg(3));
	char flush = 0;

	if (maxid < minid) {
		maxid = minid;
	}

	for (x = 0; x < MAX_MAGIC_TIMERS; x++) {
		id = mob->da[x].id;

		if (magicdb_dispel(id) > dis) continue;

		if (minid <= 0) {
			flush = 1;
		}
		else if (minid > 0 && maxid <= 0) {
			if (id == minid) {
				flush = 1;
			}
		}
		else {
			if (id >= minid && id <= maxid) {
				flush = 1;
			}
		}
		if (flush == 1) {
			mob->da[x].duration = 0;
			mob->da[x].caster_id = 0;
			map_foreachinarea(client_send_animation, mob->bl.m, mob->bl.x, mob->bl.y, AREA, BL_PC, mob->da[x].animation, &mob->bl, -1);
			mob->da[x].animation = 0;
			mob->da[x].id = 0;
		}
	}

	return 0;
}

int lua_mob_hasduration(lua_State* state, MOB* mob) {
	sl_checkargs(state, "s");
	int x, id;

	id = magicdb_id(lua_tostring(state, sl_memberarg(1)));

	for (x = 0; x < MAX_MAGIC_TIMERS; x++) {
		if (mob->da[x].id == id) {
			if (mob->da[x].duration > 0) {
				lua_pushboolean(state, 1);
				return 1;
			}
		}
	}
	lua_pushboolean(state, 0);
	return 1;
}

int lua_mob_hasdurationid(lua_State* state, MOB* mob) {
	sl_checkargs(state, "sn");
	int x, id, caster_id;

	caster_id = 0;

	id = magicdb_id(lua_tostring(state, sl_memberarg(1)));
	caster_id = lua_tonumber(state, sl_memberarg(2));

	for (x = 0; x < MAX_MAGIC_TIMERS; x++) {
		if (mob->da[x].id == id && mob->da[x].caster_id == caster_id) {
			if (mob->da[x].duration > 0) {
				lua_pushboolean(state, 1);
				return 1;
			}
		}
	}
	lua_pushboolean(state, 0);
	return 1;
}

int lua_mob_getduration(lua_State* state, MOB* mob) {
	sl_checkargs(state, "s");
	int i, id;

	id = magicdb_id(lua_tostring(state, sl_memberarg(1)));

	for (i = 0; i < MAX_MAGIC_TIMERS; i++) {
		if (mob->da[i].id == id) {
			if (mob->da[i].duration > 0) {
				lua_pushnumber(state, mob->da[i].duration);
				return 1;
			}
		}
	}

	lua_pushnumber(state, 0);
	return 1;
}

int lua_mob_getdurationid(lua_State* state, MOB* mob) {
	sl_checkargs(state, "sn");
	int i, id, caster_id;

	caster_id = 0;
	id = magicdb_id(lua_tostring(state, sl_memberarg(1)));
	caster_id = lua_tonumber(state, sl_memberarg(2));

	for (i = 0; i < MAX_MAGIC_TIMERS; i++) {
		if (mob->da[i].id == id && mob->da[i].caster_id == caster_id) {
			if (mob->da[i].duration > 0) {
				lua_pushnumber(state, mob->da[i].duration);
				return 1;
			}
		}
	}

	lua_pushnumber(state, 0);
	return 1;
}

int lua_mob_sendhealth(lua_State* state, MOB* mob) {
	sl_checkargs(state, "nn");
	int damage;
	float dmg = lua_tonumber(state, sl_memberarg(1));
	int critical = lua_tonumber(state, sl_memberarg(2));

	if (dmg > 0) {
		damage = (int)(dmg + 0.5f);
	}
	else if (dmg < 0) {
		damage = (int)(dmg - 0.5f);
	}
	else {
		damage = 0;
	}

	if (critical == 1) {
		critical = 33;
	}
	else if (critical == 2) {
		critical = 255;
	}

	client_send_mob_healthscript(mob, damage, critical);
	return 0;
}

int lua_mob_init(lua_State* state, MOB* mob, int dataref, void* param) {
	sl_pushref(state, dataref);

	mobregl_pushinst(state, mob);
	lua_setfield(state, -2, "registry");

	mapregl_pushinst(state, mob);
	lua_setfield(state, -2, "mapRegistry");

	gameregl_pushinst(state, mob);
	lua_setfield(state, -2, "gameRegistry");

	if (mob->target && param == 0) {
		// the param is used to stop infinite recursion
	}
	lua_pop(state, 1); // pop the data table
	return 0;
}

int lua_mob_getattr(lua_State* state, MOB* mob, char* attrname) {
	if (lua_blocklist_getattr(state, &mob->bl, attrname)) return 1;
	if (!strcmp(attrname, "state")) lua_pushnumber(state, mob->state);
	else if (!strcmp(attrname, "startX")) lua_pushnumber(state, mob->startx);
	else if (!strcmp(attrname, "startY")) lua_pushnumber(state, mob->starty);
	else if (!strcmp(attrname, "startM")) lua_pushnumber(state, mob->startm);
	else if (!strcmp(attrname, "mobID")) lua_pushnumber(state, mob->mobid);
	else if (!strcmp(attrname, "id")) lua_pushnumber(state, mob->id);
	else if (!strcmp(attrname, "behavior")) lua_pushnumber(state, mob->data->type);
	else if (!strcmp(attrname, "aiType")) lua_pushnumber(state, mob->data->subtype);
	else if (!strcmp(attrname, "mobType")) lua_pushnumber(state, mob->data->mobtype);
	else if (!strcmp(attrname, "side")) lua_pushnumber(state, mob->side);
	else if (!strcmp(attrname, "amnesia")) lua_pushnumber(state, mob->amnesia);
	else if (!strcmp(attrname, "name")) lua_pushstring(state, mob->data->name);
	else if (!strcmp(attrname, "yname")) lua_pushstring(state, mob->data->yname);
	else if (!strcmp(attrname, "experience")) lua_pushnumber(state, mob->exp);
	else if (!strcmp(attrname, "paralyzed")) lua_pushboolean(state, mob->paralyzed);
	else if (!strcmp(attrname, "blind")) lua_pushboolean(state, mob->blind);
	else if (!strcmp(attrname, "hit")) lua_pushnumber(state, mob->hit);
	else if (!strcmp(attrname, "baseHit")) lua_pushnumber(state, mob->data->hit);
	else if (!strcmp(attrname, "miss")) lua_pushnumber(state, mob->miss);
	else if (!strcmp(attrname, "baseMiss")) lua_pushnumber(state, mob->data->miss);
	else if (!strcmp(attrname, "level")) lua_pushnumber(state, mob->data->level);
	else if (!strcmp(attrname, "tier")) lua_pushnumber(state, mob->data->tier);
	else if (!strcmp(attrname, "mark")) lua_pushnumber(state, mob->data->mark);
	else if (!strcmp(attrname, "minDam")) lua_pushnumber(state, mob->mindam);
	else if (!strcmp(attrname, "baseMinDam")) lua_pushnumber(state, mob->data->mindam);
	else if (!strcmp(attrname, "maxDam")) lua_pushnumber(state, mob->maxdam);
	else if (!strcmp(attrname, "baseMaxDam")) lua_pushnumber(state, mob->data->maxdam);
	else if (!strcmp(attrname, "might")) lua_pushnumber(state, mob->might);
	else if (!strcmp(attrname, "baseMight")) lua_pushnumber(state, mob->data->might);
	else if (!strcmp(attrname, "grace")) lua_pushnumber(state, mob->grace);
	else if (!strcmp(attrname, "baseGrace")) lua_pushnumber(state, mob->data->grace);
	else if (!strcmp(attrname, "will")) lua_pushnumber(state, mob->will);
	else if (!strcmp(attrname, "baseWill")) lua_pushnumber(state, mob->data->will);
	else if (!strcmp(attrname, "health")) lua_pushnumber(state, mob->current_vita);
	else if (!strcmp(attrname, "maxHealth")) lua_pushnumber(state, mob->maxvita);
	else if (!strcmp(attrname, "baseHealth")) lua_pushnumber(state, mob->data->vita);
	else if (!strcmp(attrname, "lastHealth")) lua_pushnumber(state, mob->lastvita);
	else if (!strcmp(attrname, "magic")) lua_pushnumber(state, mob->current_mana);
	else if (!strcmp(attrname, "maxMagic")) lua_pushnumber(state, mob->maxmana);
	else if (!strcmp(attrname, "baseMagic")) lua_pushnumber(state, mob->data->mana);
	else if (!strcmp(attrname, "armor")) lua_pushnumber(state, mob->ac);
	else if (!strcmp(attrname, "baseArmor")) lua_pushnumber(state, mob->data->baseac);
	else if (!strcmp(attrname, "attacker")) lua_pushnumber(state, mob->attacker);
	else if (!strcmp(attrname, "confused")) lua_pushboolean(state, mob->confused);
	else if (!strcmp(attrname, "owner")) lua_pushnumber(state, mob->owner);
	else if (!strcmp(attrname, "sleep")) lua_pushnumber(state, mob->sleep);
	else if (!strcmp(attrname, "target")) lua_pushnumber(state, mob->target);
	else if (!strcmp(attrname, "confuseTarget")) lua_pushnumber(state, mob->confused_target);
	else if (!strcmp(attrname, "deduction")) lua_pushnumber(state, mob->deduction);
	else if (!strcmp(attrname, "sound")) lua_pushnumber(state, mob->data->sound);
	else if (!strcmp(attrname, "damage")) lua_pushnumber(state, mob->damage);
	else if (!strcmp(attrname, "crit")) lua_pushnumber(state, mob->crit);
	else if (!strcmp(attrname, "critChance")) lua_pushnumber(state, mob->critchance);
	else if (!strcmp(attrname, "critMult")) lua_pushnumber(state, mob->critmult);
	else if (!strcmp(attrname, "rangeTarget")) lua_pushnumber(state, mob->rangeTarget);
	else if (!strcmp(attrname, "newMove")) lua_pushnumber(state, mob->newmove);
	else if (!strcmp(attrname, "baseMove")) lua_pushnumber(state, mob->data->movetime);
	else if (!strcmp(attrname, "newAttack")) lua_pushnumber(state, mob->newatk);
	else if (!strcmp(attrname, "baseAttack")) lua_pushnumber(state, mob->data->atktime);
	else if (!strcmp(attrname, "spawnTime")) lua_pushnumber(state, mob->data->spawntime);
	else if (!strcmp(attrname, "snare")) lua_pushboolean(state, mob->snare);
	else if (!strcmp(attrname, "lastAction")) lua_pushnumber(state, mob->lastaction);
	else if (!strcmp(attrname, "summon")) lua_pushboolean(state, mob->summon);
	else if (!strcmp(attrname, "block")) lua_pushnumber(state, mob->block);
	else if (!strcmp(attrname, "baseBlock")) lua_pushnumber(state, mob->data->block);
	else if (!strcmp(attrname, "protection")) lua_pushnumber(state, mob->protection);
	else if (!strcmp(attrname, "baseProtection")) lua_pushnumber(state, mob->data->protection);
	else if (!strcmp(attrname, "retDist")) lua_pushnumber(state, mob->data->retdist);
	else if (!strcmp(attrname, "returning")) lua_pushboolean(state, mob->returning);
	else if (!strcmp(attrname, "race")) lua_pushnumber(state, mob->data->race);
	else if (!strcmp(attrname, "dmgShield")) lua_pushnumber(state, mob->dmgshield);
	else if (!strcmp(attrname, "dmgDealt")) lua_pushnumber(state, mob->dmgdealt);
	else if (!strcmp(attrname, "dmgTaken")) lua_pushnumber(state, mob->dmgtaken);
	else if (!strcmp(attrname, "seeInvis")) lua_pushnumber(state, mob->data->seeinvis);
	else if (!strcmp(attrname, "look")) lua_pushnumber(state, mob->look);
	else if (!strcmp(attrname, "lookColor")) lua_pushnumber(state, mob->look_color);
	else if (!strcmp(attrname, "charState")) lua_pushnumber(state, mob->charstate);
	else if (!strcmp(attrname, "invis")) lua_pushnumber(state, mob->invis);
	else if (!strcmp(attrname, "isBoss")) lua_pushnumber(state, mob->data->isboss);
	else if (!strcmp(attrname, "gfxFace")) lua_pushnumber(state, mob->gfx.face);
	else if (!strcmp(attrname, "gfxFaceC")) lua_pushnumber(state, mob->gfx.cface);
	else if (!strcmp(attrname, "gfxHair")) lua_pushnumber(state, mob->gfx.hair);
	else if (!strcmp(attrname, "gfxHairC")) lua_pushnumber(state, mob->gfx.chair);
	else if (!strcmp(attrname, "gfxSkinC")) lua_pushnumber(state, mob->gfx.cskin);
	else if (!strcmp(attrname, "gfxDye")) lua_pushnumber(state, mob->gfx.dye);
	else if (!strcmp(attrname, "gfxTitleColor")) lua_pushnumber(state, mob->gfx.titleColor);
	else if (!strcmp(attrname, "gfxWeap")) lua_pushnumber(state, mob->gfx.weapon);
	else if (!strcmp(attrname, "gfxWeapC")) lua_pushnumber(state, mob->gfx.cweapon);
	else if (!strcmp(attrname, "gfxArmor")) lua_pushnumber(state, mob->gfx.armor);
	else if (!strcmp(attrname, "gfxArmorC")) lua_pushnumber(state, mob->gfx.carmor);
	else if (!strcmp(attrname, "gfxShield")) lua_pushnumber(state, mob->gfx.shield);
	else if (!strcmp(attrname, "gfxShiedlC")) lua_pushnumber(state, mob->gfx.cshield);
	else if (!strcmp(attrname, "gfxHelm")) lua_pushnumber(state, mob->gfx.helm);
	else if (!strcmp(attrname, "gfxHelmC")) lua_pushnumber(state, mob->gfx.chelm);
	else if (!strcmp(attrname, "gfxMantle")) lua_pushnumber(state, mob->gfx.mantle);
	else if (!strcmp(attrname, "gfxMantleC")) lua_pushnumber(state, mob->gfx.cmantle);
	else if (!strcmp(attrname, "gfxCrown")) lua_pushnumber(state, mob->gfx.crown);
	else if (!strcmp(attrname, "gfxCrownC")) lua_pushnumber(state, mob->gfx.ccrown);
	else if (!strcmp(attrname, "gfxFaceA")) lua_pushnumber(state, mob->gfx.faceAcc);
	else if (!strcmp(attrname, "gfxFaceAC")) lua_pushnumber(state, mob->gfx.cfaceAcc);
	else if (!strcmp(attrname, "gfxFaceAT")) lua_pushnumber(state, mob->gfx.faceAccT);
	else if (!strcmp(attrname, "gfxFaceATC")) lua_pushnumber(state, mob->gfx.cfaceAccT);
	else if (!strcmp(attrname, "gfxBoots")) lua_pushnumber(state, mob->gfx.boots);
	else if (!strcmp(attrname, "gfxBootsC")) lua_pushnumber(state, mob->gfx.cboots);
	else if (!strcmp(attrname, "gfxNeck")) lua_pushnumber(state, mob->gfx.necklace);
	else if (!strcmp(attrname, "gfxNeckC")) lua_pushnumber(state, mob->gfx.cnecklace);
	else if (!strcmp(attrname, "gfxName")) lua_pushstring(state, mob->gfx.name);
	else if (!strcmp(attrname, "gfxClone")) lua_pushnumber(state, mob->clone);
	else if (!strcmp(attrname, "lastDeath")) lua_pushnumber(state, mob->last_death);
	else if (!strcmp(attrname, "cursed")) lua_pushnumber(state, mob->cursed);
	else return 0;

	return 1;
}

int lua_mob_setattr(lua_State* state, MOB* mob, char* attrname) {
	if (lua_blocklist_setattr(state, &mob->bl, attrname)) return 1;
	if (!strcmp(attrname, "side")) mob->side = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "time")) mob->time = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "amnesia")) mob->amnesia = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "paralyzed")) mob->paralyzed = lua_toboolean(state, -1);
	else if (!strcmp(attrname, "blind")) mob->blind = lua_toboolean(state, -1);
	else if (!strcmp(attrname, "hit")) mob->hit = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "minDam")) mob->mindam = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "maxDam")) mob->maxdam = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "might")) mob->might = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "grace")) mob->grace = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "will")) mob->will = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "health")) mob->current_vita = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "maxHealth")) mob->maxvita = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "magic")) mob->current_mana = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "maxMagic")) mob->maxmana = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "baseMagic")) mob->data->mana = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "lastHealth")) mob->lastvita = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "armor")) mob->ac = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "miss")) mob->miss = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "attacker")) mob->attacker = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "confused")) mob->confused = lua_toboolean(state, -1);
	else if (!strcmp(attrname, "owner")) mob->owner = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "experience")) mob->exp = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "sleep")) mob->sleep = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "target")) mob->target = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "confuseTarget")) mob->confused_target = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "deduction")) mob->deduction = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "state")) mob->state = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "rangeTarget")) mob->rangeTarget = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "newMove")) mob->newmove = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "newAttack")) mob->newatk = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "snare")) mob->snare = lua_toboolean(state, -1);
	else if (!strcmp(attrname, "lastAction")) mob->lastaction = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "crit")) mob->crit = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "critChance")) mob->critchance = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "critMult")) mob->critmult = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "damage")) mob->damage = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "summon")) mob->summon = lua_toboolean(state, -1);
	else if (!strcmp(attrname, "block")) mob->block = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "protection")) mob->protection = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "returning")) mob->returning = lua_toboolean(state, -1);
	else if (!strcmp(attrname, "dmgShield")) mob->dmgshield = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "dmgDealt")) mob->dmgdealt = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "dmgTaken")) mob->dmgtaken = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "look")) mob->look = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "lookColor")) mob->look_color = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "isBoss")) mob->data->isboss = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxFace"))mob->gfx.face = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxHair"))mob->gfx.hair = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxHairC"))mob->gfx.chair = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxFaceC"))mob->gfx.cface = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxSkinC"))mob->gfx.cskin = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxDye"))mob->gfx.dye = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxTitleColor"))mob->gfx.titleColor = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxWeap"))mob->gfx.weapon = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxWeapC"))mob->gfx.cweapon = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxArmor"))mob->gfx.armor = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxArmorC"))mob->gfx.carmor = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxShield"))mob->gfx.shield = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxShieldC"))mob->gfx.cshield = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxHelm"))mob->gfx.helm = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxHelmC"))mob->gfx.chelm = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxMantle"))mob->gfx.mantle = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxMantleC"))mob->gfx.cmantle = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxCrown"))mob->gfx.crown = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxCrownC"))mob->gfx.ccrown = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxFaceA"))mob->gfx.faceAcc = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxFaceAC"))mob->gfx.cfaceAcc = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxFaceAT"))mob->gfx.faceAccT = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxFaceATC"))mob->gfx.cfaceAccT = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxBoots"))mob->gfx.boots = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxBootsC"))mob->gfx.cboots = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxNeck"))mob->gfx.necklace = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxNeckC"))mob->gfx.cnecklace = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxName"))strcpy(mob->gfx.name, lua_tostring(state, -1));
	else if (!strcmp(attrname, "gfxClone"))mob->clone = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "charState"))mob->charstate = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "invis"))mob->invis = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "lastDeath"))mob->last_death = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "cursed"))mob->cursed = lua_tonumber(state, -1);
	else return 0;

	return 1;
}

int lua_mob_attack(lua_State* state, MOB* mob) {
	sl_checkargs(state, "n");
	int id = lua_tonumber(state, sl_memberarg(1));
	mob_attack(mob, id);
	return 0;
}

int lua_mob_addhealth(lua_State* state, MOB* mob) {
	sl_checkargs(state, "n");
	int damage = lua_tonumber(state, sl_memberarg(1));
	struct block_list* bl = map_id2bl(mob->attacker);

	if (bl != NULL) {
		if (damage > 0) {
			if (mob->data->subtype == 0) {
				sl_doscript_blargs("mob_ai_basic", "on_healed", 2, &mob->bl, bl);
			}
			else if (mob->data->subtype == 1) {
				sl_doscript_blargs("mob_ai_normal", "on_healed", 2, &mob->bl, bl);
			}
			else if (mob->data->subtype == 2) {
				sl_doscript_blargs("mob_ai_hard", "on_healed", 2, &mob->bl, bl);
			}
			else if (mob->data->subtype == 3) {
				sl_doscript_blargs("mob_ai_boss", "on_healed", 2, &mob->bl, bl);
			}
			else if (mob->data->subtype == 4) {
				sl_doscript_blargs(mob->data->yname, "on_healed", 2, &mob->bl, bl);
			}
			else if (mob->data->subtype == 5) {
				sl_doscript_blargs("mob_ai_ghost", "on_healed", 2, &mob->bl, bl);
			}
		}
	}
	else {
		if (damage > 0) {
			if (mob->data->subtype == 0) {
				sl_doscript_blargs("mob_ai_basic", "on_healed", 1, &mob->bl);
			}
			else if (mob->data->subtype == 1) {
				sl_doscript_blargs("mob_ai_normal", "on_healed", 1, &mob->bl);
			}
			else if (mob->data->subtype == 2) {
				sl_doscript_blargs("mob_ai_hard", "on_healed", 1, &mob->bl);
			}
			else if (mob->data->subtype == 3) {
				sl_doscript_blargs("mob_ai_boss", "on_healed", 1, &mob->bl);
			}
			else if (mob->data->subtype == 4) {
				sl_doscript_blargs(mob->data->yname, "on_healed", 1, &mob->bl);
			}
			else if (mob->data->subtype == 5) {
				sl_doscript_blargs("mob_ai_ghost", "on_healed", 1, &mob->bl);
			}
		}
	}

	client_send_mob_healthscript(mob, -damage, 0);
	return 0;
}

int lua_mob_move(lua_State* state, MOB* mob) {
	lua_pushboolean(state, move_mob(mob));
	return 1;
}

int lua_mob_move_ignore_object(lua_State* state, MOB* mob) {
	lua_pushboolean(state, move_mob_ignore_object(mob));
	return 1;
}

int lua_mob_checkthreat(lua_State* state, MOB* mob) {
	sl_checkargs(state, "n");
	unsigned int id;
	int x;
	USER* tsd = NULL;

	id = lua_tonumber(state, sl_memberarg(1));

	tsd = map_id2sd(id);

	if (!tsd) {
		lua_pushnumber(state, 0);
		return 1;
	}

	for (x = 0; x < MAX_THREATCOUNT; x++) {
		if (mob->threat[x].user == tsd->bl.id) {
			lua_pushnumber(state, mob->threat[x].amount);
			return 1;
		}
	}

	lua_pushnumber(state, 0);
	return 1;
}

int lua_mob_warp(lua_State* state, MOB* mob) {
	sl_checkargs(state, "nnn");
	unsigned short m, x, y;

	m = lua_tonumber(state, sl_memberarg(1));
	x = lua_tonumber(state, sl_memberarg(2));
	y = lua_tonumber(state, sl_memberarg(3));

	mob_warp(mob, m, x, y);
	return 0;
}

int lua_mob_moveghost(lua_State* state, MOB* mob) {
	lua_pushnumber(state, moveghost_mob(mob));
	return 1;
}

int lua_mob_callbase(lua_State* state, MOB* mob) {
	sl_checkargs(state, "s");
	char* script = lua_tostring(state, sl_memberarg(1));
	struct block_list* bl = map_id2bl(mob->attacker);

	if (bl != NULL) {
		sl_doscript_blargs(mob->data->yname, script, 2, &mob->bl, bl);
		lua_pushboolean(state, 1);
		return 1;
	}
	else {
		sl_doscript_blargs(mob->data->yname, script, 2, &mob->bl, &mob->bl);
		lua_pushboolean(state, 1);
		return 1;
	}

	lua_pushboolean(state, 0);
	return 1;
}

int lua_mob_checkmove(lua_State* state, MOB* mob) {
	char direction;
	short backx;
	short backy;
	short dx;
	short dy;
	unsigned short m;
	char c = 0;
	struct warp_list* i = NULL;

	m = mob->bl.m;
	backx = mob->bl.x;
	backy = mob->bl.y;
	dx = backx;
	dy = backy;
	direction = mob->side;

	switch (direction) {
	case 0:
		dy -= 1;
		break;
	case 1:
		dx += 1;
		break;
	case 2:
		dy += 1;
		break;
	case 3:
		dx -= 1;
		break;
	}

	if (dx >= map[m].xs) dx = map[m].xs - 1;
	if (dy >= map[m].ys) dy = map[m].ys - 1;
	for (i = map[m].warp[dx / BLOCK_SIZE + (dy / BLOCK_SIZE) * map[m].bxs]; i && !c; i = i->next) {
		if (i->x == dx && i->y == dy) {
			c = 1;
			lua_pushboolean(state, 0);
			return 1;
		}
	}

	map_foreachincell(mob_move, m, dx, dy, BL_MOB, mob);
	map_foreachincell(mob_move, m, dx, dy, BL_PC, mob);
	map_foreachincell(mob_move, m, dx, dy, BL_NPC, mob);

	if (client_object_can_move(m, dx, dy, direction)) {
		lua_pushboolean(state, 0);
		return 1;
	}

	if (client_object_can_move_from(m, backx, backy, direction)) {
		lua_pushboolean(state, 0);
		return 1;
	}

	if (map_canmove(m, dx, dy) == 1 || mob->canmove == 1) {
		lua_pushboolean(state, 0);
		return 1;
	}

	lua_pushboolean(state, 1);
	return 1;
}

int lua_mob_getgrpdmg(lua_State* state, MOB* mob) {
	int x, y;
	lua_newtable(state);

	for (x = 0, y = 1; x < MAX_THREATCOUNT; x++, y += 2) {
		if (mob->dmggrptable[x][0] > 0) {
			lua_pushnumber(state, mob->dmggrptable[x][0]);
			lua_rawseti(state, -2, y);
			lua_pushnumber(state, mob->dmggrptable[x][1]);
			lua_rawseti(state, -2, y + 1);
		}
	}

	return 1;
}

int lua_mob_getinddmg(lua_State* state, MOB* mob) {
	int x, y;
	lua_newtable(state);

	for (x = 0, y = 1; x < MAX_THREATCOUNT; x++, y += 2) {
		if (mob->dmgindtable[x][0] > 0) {
			lua_pushnumber(state, mob->dmgindtable[x][0]);
			lua_rawseti(state, -2, y);
			lua_pushnumber(state, mob->dmgindtable[x][1]);
			lua_rawseti(state, -2, y + 1);
		}
	}

	return 1;
}

int lua_mob_setgrpdmg(lua_State* state, MOB* mob) {
	int x;
	USER* sd = map_id2sd(lua_tonumber(state, sl_memberarg(1)));
	float dmg = lua_tonumber(state, sl_memberarg(2));

	if (sd == NULL) {
		lua_pushboolean(state, 0);
		return 1;
	}

	for (x = 0; x < MAX_THREATCOUNT; x++) {
		if (mob->dmggrptable[x][0] == sd->groupid || mob->dmggrptable[x][0] == 0) {
			mob->dmggrptable[x][0] = sd->groupid;
			mob->dmggrptable[x][1] += dmg;
			lua_pushboolean(state, 1);
			return 1;
		}
	}

	lua_pushboolean(state, 0);
	return 1;
}

int lua_mob_setinddmg(lua_State* state, MOB* mob) {
	int x;
	USER* sd = map_id2sd(lua_tonumber(state, sl_memberarg(1)));
	float dmg = lua_tonumber(state, sl_memberarg(2));

	if (sd == NULL) {
		lua_pushboolean(state, 0);
		return 1;
	}

	for (x = 0; x < MAX_THREATCOUNT; x++) {
		if (mob->dmgindtable[x][0] == sd->status.id || mob->dmgindtable[x][0] == 0) {
			mob->dmgindtable[x][0] = sd->status.id;
			mob->dmgindtable[x][1] += dmg;
			lua_pushboolean(state, 1);
			return 1;
		}
	}

	lua_pushboolean(state, 0);
	return 1;
}

int lua_mob_getequippeditem(lua_State* state, MOB* mob) {
	sl_checkargs(state, "n");
	int num = lua_tonumber(state, sl_memberarg(1));
	lua_newtable(state);

	if (mob->data->equip[num].id != 0) {
		lua_pushnumber(state, mob->data->equip[num].id);
		lua_rawseti(state, -2, 1);
		lua_pushnumber(state, mob->data->equip[num].custom);
		lua_rawseti(state, -2, 2);
	}
	else {
		lua_pushnil(state);
	}

	return 1;
}

int lua_mob_calcstat(lua_State* state, MOB* mob) {
	mob_calcstat(mob);
	return 0;
}

int lua_mob_sendstatus(lua_State* state, MOB* mob) {
	return 0;
}

int lua_mob_sendminitext(lua_State* state, MOB* mob) {
	return 0;
}

/*==============================================================================
 * NOTE: All mob type implementations have been extracted from sl.c.
 *
 * To complete the extraction:
 * 1. Remove mob type functions from sl.c (they are duplicated there)
 * 2. Add sl_mob.o to SL_OBJ in Makefile
 * 3. Add #include "lua_mob.h" to sl.h
 * 4. Rebuild and test
 *============================================================================*/
