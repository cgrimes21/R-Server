/**
 * @file sl_player.c
 * @brief Player character (pcl_*) Lua bindings implementation
 *
 * Extracted from sl.c as part of the RTK refactoring project.
 * Contains all player character operations exposed to Lua scripts.
 * Original location: sl.c lines 5847-11155 (~5,300 lines)
 *
 * NOTE: This file contains extracted implementations but sl.c still has originals.
 * To complete extraction: remove pcl_* functions from sl.c, then add sl_player.o to Makefile
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "map.h"
#include "clif.h"
#include "pc.h"
#include "npc.h"
#include "mob.h"
#include "magic.h"
#include "itemdb.h"
#include "intif.h"
#include "board_db.h"
#include "clan_db.h"
#include "guide.h"

#include "sl.h"
#include "sl_types.h"
#include "sl_player.h"

#include "timer.h"
#include "socket.h"
#include "db_mysql.h"
#include "malloc.h"  /* includes nullpo_ret macro */

/* External declarations */
extern Sql* sql_handle;
extern int xp_rate;

/* Player type class definition */
typel_class pcl_type;

/*==============================================================================
 * Type initialization - registers all 150+ methods to pcl_type
 *============================================================================*/
void pcl_staticinit() {
	pcl_type = typel_new("Player", pcl_ctor);
	pcl_type.getattr = pcl_getattr;
	pcl_type.setattr = pcl_setattr;
	pcl_type.init = pcl_init;

	/* Free/Async */
	typel_extendproto(&pcl_type, "freeAsync", pcl_freeasync);

	/* Health/Combat */
	typel_extendproto(&pcl_type, "showHealth", pcl_showhealth);
	typel_extendproto(&pcl_type, "addHealth", pcl_addhealth);
	typel_extendproto(&pcl_type, "removeHealth", pcl_removehealth);
	typel_extendproto(&pcl_type, "sendHealth", pcl_sendhealth);
	typel_extendproto(&pcl_type, "die", pcl_die);
	typel_extendproto(&pcl_type, "resurrect", pcl_resurrect);

	/* Time values */
	typel_extendproto(&pcl_type, "setTimeValues", pcl_settimevalues);
	typel_extendproto(&pcl_type, "getTimeValues", pcl_gettimevalues);

	/* PK system */
	typel_extendproto(&pcl_type, "setPK", pcl_setpk);
	typel_extendproto(&pcl_type, "getPK", pcl_getpk);

	/* Status */
	typel_extendproto(&pcl_type, "status", pcl_status);
	typel_extendproto(&pcl_type, "sendStatus", pcl_sendstatus);
	typel_extendproto(&pcl_type, "calcStat", pcl_calcstat);

	/* Account/Admin */
	typel_extendproto(&pcl_type, "setHeroShow", pcl_setHeroShow);
	typel_extendproto(&pcl_type, "setAccountBan", pcl_setAccountBan);
	typel_extendproto(&pcl_type, "getCaptchaKey", pcl_getCaptchaKey);
	typel_extendproto(&pcl_type, "setCaptchaKey", pcl_setCaptchaKey);

	/* Minimap */
	typel_extendproto(&pcl_type, "sendMiniMap", pcl_sendminimap);
	typel_extendproto(&pcl_type, "setMiniMapToggle", pcl_setMiniMapToggle);

	/* Kan system */
	typel_extendproto(&pcl_type, "addKan", pcl_addKan);
	typel_extendproto(&pcl_type, "removeKan", pcl_removeKan);
	typel_extendproto(&pcl_type, "setKan", pcl_setKan);
	typel_extendproto(&pcl_type, "checkKan", pcl_checkKan);
	typel_extendproto(&pcl_type, "claimKan", pcl_claimKan);
	typel_extendproto(&pcl_type, "kanBalance", pcl_kanBalance);

	/* Path/Country/Mail updates */
	typel_extendproto(&pcl_type, "updatePath", pcl_updatePath);
	typel_extendproto(&pcl_type, "updateCountry", pcl_updateCountry);
	typel_extendproto(&pcl_type, "updateMail", pcl_updateMail);

	/* View/Look */
	typel_extendproto(&pcl_type, "lookAt", pcl_lookat);

	/* Communication */
	typel_extendproto(&pcl_type, "sendURL", pcl_sendurl);
	typel_extendproto(&pcl_type, "sendMinitext", pcl_sendminitext);
	typel_extendproto(&pcl_type, "speak", pcl_speak);
	typel_extendproto(&pcl_type, "talkSelf", pcl_talkself);
	typel_extendproto(&pcl_type, "guitext", pcl_guitext);
	typel_extendproto(&pcl_type, "sendMail", pcl_sendmail);

	/* Board/Post */
	typel_extendproto(&pcl_type, "showBoard", pcl_showboard);
	typel_extendproto(&pcl_type, "showPost", pcl_showpost);
	typel_extendproto(&pcl_type, "sendBoardQuestions", pcl_sendboardquestions);
	typel_extendproto(&pcl_type, "powerBoard", pcl_powerboard);

	/* Refresh */
	typel_extendproto(&pcl_type, "refresh", pcl_refresh);
	typel_extendproto(&pcl_type, "refreshInventory", pcl_refreshInventory);

	/* Dialog/Menu */
	typel_extendproto(&pcl_type, "popUp", pcl_popup);
	typel_extendproto(&pcl_type, "paperpopup", pcl_paperpopup);
	typel_extendproto(&pcl_type, "paperpopupwrite", pcl_paperpopupwrite);
	typel_extendproto(&pcl_type, "input", pcl_input);
	typel_extendproto(&pcl_type, "dialog", pcl_dialog);
	typel_extendproto(&pcl_type, "menu", pcl_menu);
	typel_extendproto(&pcl_type, "menuSeq", pcl_menuseq);
	typel_extendproto(&pcl_type, "inputSeq", pcl_inputseq);

	/* Shop/Trading */
	typel_extendproto(&pcl_type, "buy", pcl_buy);
	typel_extendproto(&pcl_type, "sell", pcl_sell);
	typel_extendproto(&pcl_type, "logBuySell", pcl_logbuysell);

	/* Durations/Aethers */
	typel_extendproto(&pcl_type, "setDuration", pcl_setduration);
	typel_extendproto(&pcl_type, "hasDuration", pcl_hasduration);
	typel_extendproto(&pcl_type, "hasDurationID", pcl_hasdurationid);
	typel_extendproto(&pcl_type, "getDuration", pcl_getduration);
	typel_extendproto(&pcl_type, "getDurationID", pcl_getdurationid);
	typel_extendproto(&pcl_type, "getAllDurations", pcl_getalldurations);
	typel_extendproto(&pcl_type, "forceSave", pcl_forcesave);
	typel_extendproto(&pcl_type, "refreshDurations", pcl_refreshdurations);
	typel_extendproto(&pcl_type, "flushDuration", pcl_flushduration);
	typel_extendproto(&pcl_type, "flushDurationNoUncast", pcl_flushdurationnouncast);
	typel_extendproto(&pcl_type, "getCasterID", pcl_getcasterid);
	typel_extendproto(&pcl_type, "setAether", pcl_setaether);
	typel_extendproto(&pcl_type, "hasAether", pcl_hasaether);
	typel_extendproto(&pcl_type, "getAether", pcl_getaether);
	typel_extendproto(&pcl_type, "getAllAethers", pcl_getallaethers);
	typel_extendproto(&pcl_type, "flushAether", pcl_flushaether);
	typel_extendproto(&pcl_type, "durationAmount", pcl_durationamount);

	/* Gifts */
	typel_extendproto(&pcl_type, "hasSpace", pcl_hasspace);
	typel_extendproto(&pcl_type, "addGift", pcl_addGift);
	typel_extendproto(&pcl_type, "retrieveGift", pcl_retrieveGift);

	/* Inventory */
	typel_extendproto(&pcl_type, "addItem", pcl_additem);
	typel_extendproto(&pcl_type, "getInventoryItem", pcl_getinventoryitem);
	typel_extendproto(&pcl_type, "getExchangeItem", pcl_getexchangeitem);
	typel_extendproto(&pcl_type, "getBODItem", pcl_getboditem);
	typel_extendproto(&pcl_type, "hasItem", pcl_hasitem);
	typel_extendproto(&pcl_type, "hasItemDura", pcl_hasitemdura);
	typel_extendproto(&pcl_type, "removeItem", pcl_removeinventoryitem);
	typel_extendproto(&pcl_type, "removeItemSlot", pcl_removeitemslot);
	typel_extendproto(&pcl_type, "removeItemDura", pcl_removeitemdura);
	typel_extendproto(&pcl_type, "updateInv", pcl_updateinv);
	typel_extendproto(&pcl_type, "checkInvBod", pcl_checkinvbod);

	/* Equipment */
	typel_extendproto(&pcl_type, "getEquippedItem", pcl_getequippeditem);
	typel_extendproto(&pcl_type, "hasEquipped", pcl_hasequipped);
	typel_extendproto(&pcl_type, "pickUp", pcl_pickup);
	typel_extendproto(&pcl_type, "equip", pcl_equip);
	typel_extendproto(&pcl_type, "forceEquip", pcl_forceequip);
	typel_extendproto(&pcl_type, "takeOff", pcl_takeoff);
	typel_extendproto(&pcl_type, "stripEquip", pcl_stripequip);
	typel_extendproto(&pcl_type, "throwItem", pcl_throwitem);
	typel_extendproto(&pcl_type, "deductArmor", pcl_deductarmor);
	typel_extendproto(&pcl_type, "deductWeapon", pcl_deductweapon);
	typel_extendproto(&pcl_type, "deductDura", pcl_deductdura);
	typel_extendproto(&pcl_type, "deductDuraInv", pcl_deductdurainv);
	typel_extendproto(&pcl_type, "deductDuraEquip", pcl_deductduraequip);

	/* Items - drops/use */
	typel_extendproto(&pcl_type, "useItem", pcl_useitem);
	typel_extendproto(&pcl_type, "forceDrop", pcl_forcedrop);
	typel_extendproto(&pcl_type, "expireItem", pcl_expireitem);

	/* Legend/Marks */
	typel_extendproto(&pcl_type, "addLegend", pcl_addlegend);
	typel_extendproto(&pcl_type, "hasLegend", pcl_haslegend);
	typel_extendproto(&pcl_type, "removeLegendbyName", pcl_removelegendbyname);
	typel_extendproto(&pcl_type, "removeLegendbyColor", pcl_removelegendbycolor);

	/* Movement/Position */
	typel_extendproto(&pcl_type, "warp", pcl_warp);
	typel_extendproto(&pcl_type, "move", pcl_move);
	typel_extendproto(&pcl_type, "respawn", pcl_respawn);

	/* Clan */
	typel_extendproto(&pcl_type, "addClan", pcl_addclan);

	/* Kill tracking */
	typel_extendproto(&pcl_type, "killCount", pcl_killcount);
	typel_extendproto(&pcl_type, "setKillCount", pcl_setkillcount);
	typel_extendproto(&pcl_type, "flushAllKills", pcl_flushallkills);
	typel_extendproto(&pcl_type, "flushKills", pcl_flushkills);

	/* Spells */
	typel_extendproto(&pcl_type, "getUnknownSpells", pcl_getunknownspells);
	typel_extendproto(&pcl_type, "getAllSpells", pcl_getallspells);
	typel_extendproto(&pcl_type, "getAllClassSpells", pcl_getallclassspells);
	typel_extendproto(&pcl_type, "addSpell", pcl_addspell);
	typel_extendproto(&pcl_type, "removeSpell", pcl_removespell);
	typel_extendproto(&pcl_type, "hasSpell", pcl_hasspell);
	typel_extendproto(&pcl_type, "getSpells", pcl_getspells);
	typel_extendproto(&pcl_type, "getSpellName", pcl_getspellname);
	typel_extendproto(&pcl_type, "getSpellYName", pcl_getspellyname);
	typel_extendproto(&pcl_type, "getSpellNameFromYName", pcl_getspellnamefromyname);
	typel_extendproto(&pcl_type, "addEventXP", pcl_addEventXP);

	/* Map selection */
	typel_extendproto(&pcl_type, "mapSelection", pcl_mapselection);

	/* Lock/Async */
	typel_extendproto(&pcl_type, "lock", pcl_lock);
	typel_extendproto(&pcl_type, "unlock", pcl_unlock);

	/* Combat */
	typel_extendproto(&pcl_type, "swing", pcl_swing);
	typel_extendproto(&pcl_type, "swingTarget", pcl_swingtarget);

	/* Banking */
	typel_extendproto(&pcl_type, "getBankItem", pcl_getbankitem);
	typel_extendproto(&pcl_type, "getBankItems", pcl_getbankitems);
	typel_extendproto(&pcl_type, "bankDeposit", pcl_bankdeposit);
	typel_extendproto(&pcl_type, "bankWithdraw", pcl_bankwithdraw);

	/* Clan banking */
	typel_extendproto(&pcl_type, "getClanBankItems", pcl_getclanbankitems);
	typel_extendproto(&pcl_type, "clanBankDeposit", pcl_clanbankdeposit);
	typel_extendproto(&pcl_type, "clanBankWithdraw", pcl_clanbankwithdraw);

	/* Subpath banking */
	typel_extendproto(&pcl_type, "getSubpathBankItems", pcl_getsubpathbankitems);
	typel_extendproto(&pcl_type, "subpathBankDeposit", pcl_subpathbankdeposit);
	typel_extendproto(&pcl_type, "subpathBankWithdraw", pcl_subpathbankwithdraw);

	/* Threat system */
	typel_extendproto(&pcl_type, "addThreat", pcl_addthreat);
	typel_extendproto(&pcl_type, "setThreat", pcl_setthreat);
	typel_extendproto(&pcl_type, "addThreatGeneral", pcl_addthreatgeneral);

	/* Creation system */
	typel_extendproto(&pcl_type, "getCreationItems", pcl_getcreationitems);
	typel_extendproto(&pcl_type, "getCreationAmounts", pcl_getcreationamounts);

	/* Parcel */
	typel_extendproto(&pcl_type, "getParcel", pcl_getparcel);
	typel_extendproto(&pcl_type, "getParcelList", pcl_getparcellist);
	typel_extendproto(&pcl_type, "removeParcel", pcl_removeparcel);

	/* Debug/Test */
	typel_extendproto(&pcl_type, "testPacket", pcl_testpacket);
	typel_extendproto(&pcl_type, "changeView", pcl_changeview);

	/* Timer */
	typel_extendproto(&pcl_type, "setTimer", pcl_settimer);
	typel_extendproto(&pcl_type, "addTime", pcl_addtime);
	typel_extendproto(&pcl_type, "removeTime", pcl_removetime);
	typel_extendproto(&pcl_type, "checkLevel", pcl_checklevel);

	/* Extend with block list methods */
	bll_extendproto(&pcl_type);
}

/*==============================================================================
 * Core type functions
 *============================================================================*/

int pcl_ctor(lua_State* state) {
	USER* sd = NULL;
	if (lua_isnumber(state, 2)) {
		unsigned int id = lua_tonumber(state, 2);
		sd = map_id2sd(id);
		if (!sd) {
			lua_pushnil(state);
			return 1;
		}
	}
	else if (lua_isstring(state, 2)) {
		char* name = lua_tostring(state, 2);
		sd = map_name2sd(name);
		if (!sd) {
			lua_pushnil(state);
			return 1;
		}
	}
	else {
		luaL_argerror(state, 1, "expected string or number");
	}
	pcl_pushinst(state, sd);
	return 1;
}

int pcl_init(lua_State* state, USER* sd, int dataref, void* param) {
	return 0;
}

/*==============================================================================
 * Attribute setter - handles all player.attribute = value assignments
 *============================================================================*/
int pcl_setattr(lua_State* state, USER* sd, char* attrname) {
	if (bll_setattr(state, &sd->bl, attrname)) return 1;
	if (!strcmp(attrname, "npcGraphic")) sd->npc_g = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "npcColor")) sd->npc_gc = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "lastClick")) sd->last_click = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "title")) strcpy(sd->status.title, lua_tostring(state, -1));
	else if (!strcmp(attrname, "name")) strcpy(sd->status.name, lua_tostring(state, -1));
	else if (!strcmp(attrname, "clanTitle")) strcpy(sd->status.clan_title, lua_tostring(state, -1));
	else if (!strcmp(attrname, "noviceChat")) sd->status.novice_chat = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "subpathChat")) sd->status.subpath_chat = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "clanChat")) sd->status.clan_chat = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "fakeDrop")) sd->fakeDrop = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "maxHealth")) sd->max_hp = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "maxMagic")) sd->max_mp = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "polearm")) sd->polearm = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "actionTime")) sd->time = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "miniMapToggle")) sd->status.miniMapToggle = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "rage")) sd->rage = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "classRank")) sd->status.classRank = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "clanRank")) sd->status.clanRank = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "clan")) sd->status.clan = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gmLevel")) sd->status.gm_level = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "class")) sd->status.class = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "totem")) sd->status.totem = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "tier")) sd->status.tier = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "mark")) sd->status.mark = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "country")) sd->status.country = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "health")) sd->status.hp = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "magic")) sd->status.mp = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "baseHealth")) sd->status.basehp = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "baseMagic")) sd->status.basemp = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "lastHealth")) sd->lastvita = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "exp")) sd->status.exp = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "expSoldMagic")) sd->status.expsoldmagic = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "expSoldHealth")) sd->status.expsoldhealth = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "expSoldStats")) sd->status.expsoldstats = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "state")) sd->status.state = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "hair")) sd->status.hair = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "paralyzed")) sd->paralyzed = lua_toboolean(state, -1);
	else if (!strcmp(attrname, "blind")) { sd->blind = lua_toboolean(state, -1); }
	else if (!strcmp(attrname, "drunk")) { sd->drunk = lua_tonumber(state, -1); }
	else if (!strcmp(attrname, "sex")) { sd->status.sex = lua_tonumber(state, -1); }
	else if (!strcmp(attrname, "uflags")) { sd->uFlags ^= lua_tointeger(state, -1); }
	else if (!strcmp(attrname, "backstab")) sd->backstab = lua_toboolean(state, -1);
	else if (!strcmp(attrname, "flank")) sd->flank = lua_toboolean(state, -1);
	else if (!strcmp(attrname, "armor")) sd->armor = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "dam")) sd->dam = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "hit")) sd->hit = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "miss")) sd->miss = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "might")) sd->might = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "baseMight")) sd->status.basemight = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "tutor")) sd->status.tutor = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "will")) sd->will = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "baseWill")) sd->status.basewill = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "grace")) sd->grace = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "baseGrace")) sd->status.basegrace = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "baseArmor")) sd->status.basearmor = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "karma")) sd->status.karma = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "alignment")) sd->status.alignment = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "boardDel")) sd->board_candel = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "boardWrite")) sd->board_canwrite = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "boardShow")) sd->boardshow = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "boardNameVal")) sd->boardnameval = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "side")) sd->status.side = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "sleep")) sd->sleep = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "attackSpeed")) sd->attack_speed = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "speech")) strcpy(sd->speech, lua_tostring(state, -1));
	else if (!strcmp(attrname, "enchant")) sd->enchanted = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "money")) sd->status.money = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "settings")) sd->status.settingFlags ^= lua_tointeger(state, -1);
	else if (!strcmp(attrname, "confused")) sd->confused = lua_toboolean(state, -1);
	else if (!strcmp(attrname, "skinColor")) sd->status.skin_color = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "target")) sd->target = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "deduction")) sd->deduction = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "hairColor")) sd->status.hair_color = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "face")) sd->status.face = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "faceColor")) sd->status.face_color = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "speed")) sd->speed = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "armorColor")) sd->status.armor_color = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "partner")) sd->status.partner = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "disguise")) sd->disguise = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "disguiseColor")) sd->disguise_color = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "attacker")) sd->attacker = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "invis")) sd->invis = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "coContainer")) sd->coref_container = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "level")) sd->status.level = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "rangeTarget")) sd->rangeTarget = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "maxSlots"))sd->status.maxslots = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "bankMoney"))sd->status.bankmoney = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "snare"))sd->snare = lua_toboolean(state, -1);
	else if (!strcmp(attrname, "silence"))sd->silence = lua_toboolean(state, -1);
	else if (!strcmp(attrname, "extendHit"))sd->extendhit = lua_toboolean(state, -1);
	else if (!strcmp(attrname, "afk"))sd->afk = lua_toboolean(state, -1);
	else if (!strcmp(attrname, "afkMessage"))strcpy(sd->status.afkmessage, lua_tostring(state, -1));
	else if (!strcmp(attrname, "optFlags"))sd->optFlags ^= lua_tointeger(state, -1);
	else if (!strcmp(attrname, "healing"))sd->healing = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "talkType"))sd->talktype = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "f1Name"))strcpy(sd->status.f1name, lua_tostring(state, -1));
	else if (!strcmp(attrname, "crit"))sd->crit = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "critChance"))sd->critchance = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "critMult"))sd->critmult = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "damage"))sd->damage = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "spotTraps"))sd->spottraps = lua_toboolean(state, -1);
	else if (!strcmp(attrname, "fury"))sd->fury = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "maxInv"))sd->status.maxinv = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "faceAccessoryTwo"))sd->status.equip[EQ_FACEACCTWO].id = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "faceAccessoryTwoColor"))sd->status.equip[EQ_FACEACCTWO].custom = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "crown"))sd->status.equip[EQ_CROWN].id = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "crownColor"))sd->status.equip[EQ_CROWN].custom = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxFace"))sd->gfx.face = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxHair"))sd->gfx.hair = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxHairC"))sd->gfx.chair = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxFaceC"))sd->gfx.cface = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxSkinC"))sd->gfx.cskin = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxDye"))sd->gfx.dye = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxTitleColor"))sd->gfx.titleColor = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxWeap"))sd->gfx.weapon = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxWeapC"))sd->gfx.cweapon = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxArmor"))sd->gfx.armor = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxArmorC"))sd->gfx.carmor = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxShield"))sd->gfx.shield = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxShieldC"))sd->gfx.cshield = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxHelm"))sd->gfx.helm = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxHelmC"))sd->gfx.chelm = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxMantle"))sd->gfx.mantle = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxMantleC"))sd->gfx.cmantle = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxCrown"))sd->gfx.crown = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxCrownC"))sd->gfx.ccrown = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxFaceA"))sd->gfx.faceAcc = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxFaceAC"))sd->gfx.cfaceAcc = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxFaceAT"))sd->gfx.faceAccT = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxFaceATC"))sd->gfx.cfaceAccT = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxBoots"))sd->gfx.boots = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxBootsC"))sd->gfx.cboots = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxNeck"))sd->gfx.necklace = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxNeckC"))sd->gfx.cnecklace = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxName"))strcpy(sd->gfx.name, lua_tostring(state, -1));
	else if (!strcmp(attrname, "gfxClone"))sd->clone = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "PK"))sd->status.pk = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "killedBy"))sd->status.killedby = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "protection"))sd->protection = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "dmgShield")) {
		sd->dmgshield = lua_tonumber(state, -1);
		if (sd->dmgshield <= 0) clif_send_duration(sd, 0, 0, NULL);
	}
	else if (!strcmp(attrname, "dmgDealt"))sd->dmgdealt = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "dmgTaken"))sd->dmgtaken = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "mute"))sd->status.mute = lua_toboolean(state, -1);
	else if (!strcmp(attrname, "wisdom"))sd->wisdom = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "con"))sd->con = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "luaExec"))sd->luaexec = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "deathFlag"))sd->deathflag = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "selfBar"))sd->selfbar = lua_toboolean(state, -1);
	else if (!strcmp(attrname, "groupBars"))sd->groupbars = lua_toboolean(state, -1);
	else if (!strcmp(attrname, "mobBars"))sd->mobbars = lua_toboolean(state, -1);
	else if (!strcmp(attrname, "bindMap"))sd->bindmap = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "bindX"))sd->bindx = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "bindY"))sd->bindy = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "heroShow"))sd->status.heroes = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "ambushTimer"))sd->ambushtimer = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "dialogType"))sd->dialogtype = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "cursed"))sd->cursed = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "profileVitaStats"))sd->status.profile_vitastats = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "profileEquipList"))sd->status.profile_equiplist = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "profileLegends"))sd->status.profile_legends = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "profileSpells"))sd->status.profile_spells = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "profileInventory"))sd->status.profile_inventory = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "profileBankItems"))sd->status.profile_bankitems = lua_tonumber(state, -1);
	else return 0;
	return 1;
}

/*==============================================================================
 * Attribute getter - handles all player.attribute reads
 *============================================================================*/
int pcl_getattr(lua_State* state, USER* sd, char* attrname) {
	if (bll_getattr(state, &sd->bl, attrname)) return 1;
	if (!strcmpi(attrname, "id")) lua_pushnumber(state, sd->status.id);
	else if (!strcmp(attrname, "npcGraphic")) lua_pushnumber(state, sd->npc_g);
	else if (!strcmp(attrname, "npcColor")) lua_pushnumber(state, sd->npc_gc);
	else if (!strcmp(attrname, "groupID")) lua_pushnumber(state, sd->groupid);
	else if (!strcmp(attrname, "health")) lua_pushnumber(state, sd->status.hp);
	else if (!strcmp(attrname, "magic")) lua_pushnumber(state, sd->status.mp);
	else if (!strcmp(attrname, "level")) lua_pushnumber(state, sd->status.level);
	else if (!strcmp(attrname, "name")) lua_pushstring(state, sd->status.name);
	else if (!strcmp(attrname, "title")) lua_pushstring(state, sd->status.title);
	else if (!strcmp(attrname, "actionTime")) lua_pushnumber(state, sd->time);
	else if (!strcmp(attrname, "noviceChat")) lua_pushnumber(state, sd->status.novice_chat);
	else if (!strcmp(attrname, "subpathChat")) lua_pushnumber(state, sd->status.subpath_chat);
	else if (!strcmp(attrname, "clanChat")) lua_pushnumber(state, sd->status.clan_chat);
	else if (!strcmp(attrname, "fakeDrop")) lua_pushnumber(state, sd->fakeDrop);
	else if (!strcmp(attrname, "maxHealth")) lua_pushnumber(state, sd->max_hp);
	else if (!strcmp(attrname, "maxMagic")) lua_pushnumber(state, sd->max_mp);
	else if (!strcmp(attrname, "lastHealth")) lua_pushnumber(state, sd->lastvita);
	else if (!strcmp(attrname, "rage")) lua_pushnumber(state, sd->rage);
	else if (!strcmp(attrname, "clanTitle")) lua_pushstring(state, sd->status.clan_title);
	else if (!strcmp(attrname, "polearm")) lua_pushnumber(state, sd->polearm);
	else if (!strcmp(attrname, "miniMapToggle")) lua_pushnumber(state, sd->status.miniMapToggle);
	else if (!strcmp(attrname, "clan")) lua_pushnumber(state, sd->status.clan);
	else if (!strcmp(attrname, "clanName")) lua_pushstring(state, clandb_name(sd->status.clan));
	else if (!strcmp(attrname, "gmLevel")) lua_pushnumber(state, sd->status.gm_level);
	else if (!strcmp(attrname, "actId")) lua_pushnumber(state, clif_isregistered(sd->status.id));
	else if (!strcmp(attrname, "lastClick")) lua_pushnumber(state, sd->last_click);
	else if (!strcmp(attrname, "class")) lua_pushnumber(state, sd->status.class);
	else if (!strcmp(attrname, "baseClass")) lua_pushnumber(state, classdb_path(sd->status.class));
	else if (!strcmp(attrname, "totem")) lua_pushnumber(state, sd->status.totem);
	else if (!strcmp(attrname, "tier")) lua_pushnumber(state, sd->status.tier);
	else if (!strcmp(attrname, "country")) lua_pushnumber(state, sd->status.country);
	else if (!strcmp(attrname, "mark")) lua_pushnumber(state, sd->status.mark);
	else if (!strcmp(attrname, "baseHealth")) lua_pushnumber(state, sd->status.basehp);
	else if (!strcmp(attrname, "baseMagic")) lua_pushnumber(state, sd->status.basemp);
	else if (!strcmp(attrname, "exp")) lua_pushnumber(state, sd->status.exp);
	else if (!strcmp(attrname, "expSoldMagic")) lua_pushnumber(state, sd->status.expsoldmagic);
	else if (!strcmp(attrname, "expSoldHealth")) lua_pushnumber(state, sd->status.expsoldhealth);
	else if (!strcmp(attrname, "expSoldStats")) lua_pushnumber(state, sd->status.expsoldstats);
	else if (!strcmp(attrname, "state")) lua_pushnumber(state, sd->status.state);
	else if (!strcmp(attrname, "face")) lua_pushnumber(state, sd->status.face);
	else if (!strcmp(attrname, "hair")) lua_pushnumber(state, sd->status.hair);
	else if (!strcmp(attrname, "hairColor")) lua_pushnumber(state, sd->status.hair_color);
	else if (!strcmp(attrname, "faceColor")) lua_pushnumber(state, sd->status.face_color);
	else if (!strcmp(attrname, "armorColor")) lua_pushnumber(state, sd->status.armor_color);
	else if (!strcmp(attrname, "skinColor")) lua_pushnumber(state, sd->status.skin_color);
	else if (!strcmp(attrname, "paralyzed")) lua_pushboolean(state, sd->paralyzed);
	else if (!strcmp(attrname, "blind")) lua_pushboolean(state, sd->blind);
	else if (!strcmp(attrname, "drunk")) lua_pushnumber(state, sd->drunk);
	else if (!strcmp(attrname, "board")) lua_pushnumber(state, sd->board);
	else if (!strcmp(attrname, "boardDel")) lua_pushnumber(state, sd->board_candel);
	else if (!strcmp(attrname, "boardWrite")) lua_pushnumber(state, sd->board_canwrite);
	else if (!strcmp(attrname, "boardShow")) lua_pushnumber(state, sd->boardshow);
	else if (!strcmp(attrname, "boardNameVal")) lua_pushnumber(state, sd->boardnameval);
	else if (!strcmp(attrname, "sex")) lua_pushnumber(state, sd->status.sex);
	else if (!strcmp(attrname, "ping")) lua_pushnumber(state, sd->msPing);
	else if (!strcmp(attrname, "pbColor")) lua_pushnumber(state, sd->pbColor);
	else if (!strcmp(attrname, "coRef")) lua_pushnumber(state, sd->coref);
	else if (!strcmp(attrname, "optFlags")) lua_pushnumber(state, sd->optFlags);
	else if (!strcmp(attrname, "uflags")) lua_pushnumber(state, sd->uFlags);
	else if (!strcmp(attrname, "settings")) lua_pushnumber(state, sd->status.settingFlags);
	else if (!strcmp(attrname, "side")) lua_pushnumber(state, sd->status.side);
	else if (!strcmp(attrname, "grace")) lua_pushnumber(state, sd->grace);
	else if (!strcmp(attrname, "baseGrace")) lua_pushnumber(state, sd->status.basegrace);
	else if (!strcmp(attrname, "might")) lua_pushnumber(state, sd->might);
	else if (!strcmp(attrname, "baseMight")) lua_pushnumber(state, sd->status.basemight);
	else if (!strcmp(attrname, "will")) lua_pushnumber(state, sd->will);
	else if (!strcmp(attrname, "baseWill")) lua_pushnumber(state, sd->status.basewill);
	else if (!strcmp(attrname, "tutor")) lua_pushnumber(state, sd->status.tutor);
	else if (!strcmp(attrname, "karma")) lua_pushnumber(state, sd->status.karma);
	else if (!strcmp(attrname, "alignment")) lua_pushnumber(state, sd->status.alignment);
	else if (!strcmp(attrname, "baseArmor")) lua_pushnumber(state, sd->status.basearmor);
	else if (!strcmp(attrname, "ipaddress")) lua_pushstring(state, sd->ipaddress);
	else if (!strcmp(attrname, "email")) lua_pushstring(state, clif_getaccountemail(sd->status.id));
	else if (!strcmp(attrname, "backstab")) lua_pushboolean(state, sd->backstab);
	else if (!strcmp(attrname, "flank")) lua_pushboolean(state, sd->flank);
	else if (!strcmp(attrname, "armor")) lua_pushnumber(state, sd->armor);
	else if (!strcmp(attrname, "dam")) lua_pushnumber(state, sd->dam);
	else if (!strcmp(attrname, "hit")) lua_pushnumber(state, sd->hit);
	else if (!strcmp(attrname, "miss")) lua_pushnumber(state, sd->miss);
	else if (!strcmp(attrname, "sleep")) lua_pushnumber(state, sd->sleep);
	else if (!strcmp(attrname, "attackSpeed")) lua_pushnumber(state, sd->attack_speed);
	else if (!strcmp(attrname, "question")) lua_pushstring(state, sd->question);
	else if (!strcmp(attrname, "enchant")) lua_pushnumber(state, sd->enchanted);
	else if (!strcmp(attrname, "speech")) lua_pushstring(state, sd->speech);
	else if (!strcmp(attrname, "money")) lua_pushnumber(state, sd->status.money);
	else if (!strcmp(attrname, "confused")) lua_pushboolean(state, sd->confused);
	else if (!strcmp(attrname, "target")) lua_pushnumber(state, sd->target);
	else if (!strcmp(attrname, "deduction")) lua_pushnumber(state, sd->deduction);
	else if (!strcmp(attrname, "speed")) lua_pushnumber(state, sd->speed);
	else if (!strcmp(attrname, "partner")) lua_pushnumber(state, sd->status.partner);
	else if (!strcmp(attrname, "disguise")) lua_pushnumber(state, sd->disguise);
	else if (!strcmp(attrname, "disguiseColor")) lua_pushnumber(state, sd->disguise_color);
	else if (!strcmp(attrname, "attacker")) lua_pushnumber(state, sd->attacker);
	else if (!strcmp(attrname, "invis")) lua_pushnumber(state, sd->invis);
	else if (!strcmp(attrname, "damage")) lua_pushnumber(state, sd->damage);
	else if (!strcmp(attrname, "crit")) lua_pushnumber(state, sd->crit);
	else if (!strcmp(attrname, "critChance")) lua_pushnumber(state, sd->critchance);
	else if (!strcmp(attrname, "critMult")) lua_pushnumber(state, sd->critmult);
	else if (!strcmp(attrname, "rangeTarget")) lua_pushnumber(state, sd->rangeTarget);
	else if (!strcmp(attrname, "maxSlots")) lua_pushnumber(state, sd->status.maxslots);
	else if (!strcmp(attrname, "bankMoney")) lua_pushnumber(state, sd->status.bankmoney);
	else if (!strcmp(attrname, "exchangeMoney")) lua_pushnumber(state, sd->exchange.gold);
	else if (!strcmp(attrname, "exchangeItemCount")) lua_pushnumber(state, sd->exchange.item_count);
	else if (!strcmp(attrname, "BODItemCount")) lua_pushnumber(state, sd->boditems.bod_count);
	else if (!strcmp(attrname, "snare")) lua_pushboolean(state, sd->snare);
	else if (!strcmp(attrname, "silence")) lua_pushboolean(state, sd->silence);
	else if (!strcmp(attrname, "extendHit")) lua_pushboolean(state, sd->extendhit);
	else if (!strcmp(attrname, "afk")) lua_pushboolean(state, sd->afk);
	else if (!strcmp(attrname, "afkTime")) lua_pushnumber(state, sd->afktime);
	else if (!strcmp(attrname, "afkTimeTotal")) lua_pushnumber(state, sd->totalafktime);
	else if (!strcmp(attrname, "afkMessage")) lua_pushstring(state, sd->status.afkmessage);
	else if (!strcmp(attrname, "baseClassName")) lua_pushstring(state, classdb_name(classdb_path(sd->status.class), 0));
	else if (!strcmp(attrname, "className")) lua_pushstring(state, classdb_name(sd->status.class, 0));
	else if (!strcmp(attrname, "classNameMark")) lua_pushstring(state, classdb_name(sd->status.class, sd->status.mark));
	else if (!strcmp(attrname, "classRank")) lua_pushnumber(state, sd->status.classRank);
	else if (!strcmp(attrname, "clanRank")) lua_pushnumber(state, sd->status.clanRank);
	else if (!strcmp(attrname, "healing")) lua_pushnumber(state, sd->healing);
	else if (!strcmp(attrname, "minSDam")) lua_pushnumber(state, sd->minSdam);
	else if (!strcmp(attrname, "maxSDam")) lua_pushnumber(state, sd->maxSdam);
	else if (!strcmp(attrname, "minLDam")) lua_pushnumber(state, sd->minLdam);
	else if (!strcmp(attrname, "maxLDam")) lua_pushnumber(state, sd->maxLdam);
	else if (!strcmp(attrname, "talkType")) lua_pushnumber(state, sd->talktype);
	else if (!strcmp(attrname, "f1Name")) lua_pushstring(state, sd->status.f1name);
	else if (!strcmp(attrname, "equipID")) lua_pushnumber(state, sd->equipid);
	else if (!strcmp(attrname, "takeOffID")) lua_pushnumber(state, sd->takeoffid);
	else if (!strcmp(attrname, "breakID")) lua_pushnumber(state, sd->breakid);
	else if (!strcmp(attrname, "equipSlot")) lua_pushnumber(state, sd->equipslot);
	else if (!strcmp(attrname, "invSlot")) lua_pushnumber(state, sd->invslot);
	else if (!strcmp(attrname, "pickUpType")) lua_pushnumber(state, sd->pickuptype);
	else if (!strcmp(attrname, "spotTraps")) lua_pushboolean(state, sd->spottraps);
	else if (!strcmp(attrname, "fury")) lua_pushnumber(state, sd->fury);
	else if (!strcmp(attrname, "maxInv")) lua_pushnumber(state, sd->status.maxinv);
	else if (!strcmp(attrname, "faceAccessoryTwo")) lua_pushnumber(state, sd->status.equip[EQ_FACEACCTWO].id);
	else if (!strcmp(attrname, "faceAccessoryTwoColor")) lua_pushnumber(state, sd->status.equip[EQ_FACEACCTWO].custom);
	else if (!strcmp(attrname, "gfxFace")) lua_pushnumber(state, sd->gfx.face);
	else if (!strcmp(attrname, "gfxHair")) lua_pushnumber(state, sd->gfx.hair);
	else if (!strcmp(attrname, "gfxHairC")) lua_pushnumber(state, sd->gfx.chair);
	else if (!strcmp(attrname, "gfxFaceC")) lua_pushnumber(state, sd->gfx.cface);
	else if (!strcmp(attrname, "gfxSkinC")) lua_pushnumber(state, sd->gfx.cskin);
	else if (!strcmp(attrname, "gfxDye")) lua_pushnumber(state, sd->gfx.dye);
	else if (!strcmp(attrname, "gfxTitleColor")) lua_pushnumber(state, sd->gfx.dye);
	else if (!strcmp(attrname, "gfxWeap"))lua_pushnumber(state, sd->gfx.weapon);
	else if (!strcmp(attrname, "gfxWeapC"))lua_pushnumber(state, sd->gfx.cweapon);
	else if (!strcmp(attrname, "gfxArmor"))lua_pushnumber(state, sd->gfx.armor);
	else if (!strcmp(attrname, "gfxArmorC"))lua_pushnumber(state, sd->gfx.carmor);
	else if (!strcmp(attrname, "gfxShield"))lua_pushnumber(state, sd->gfx.shield);
	else if (!strcmp(attrname, "gfxShieldC"))lua_pushnumber(state, sd->gfx.cshield);
	else if (!strcmp(attrname, "gfxHelm"))lua_pushnumber(state, sd->gfx.helm);
	else if (!strcmp(attrname, "gfxHelmC"))lua_pushnumber(state, sd->gfx.chelm);
	else if (!strcmp(attrname, "gfxMantle"))lua_pushnumber(state, sd->gfx.mantle);
	else if (!strcmp(attrname, "gfxMantleC"))lua_pushnumber(state, sd->gfx.cmantle);
	else if (!strcmp(attrname, "gfxCrown"))lua_pushnumber(state, sd->gfx.crown);
	else if (!strcmp(attrname, "gfxCrownC"))lua_pushnumber(state, sd->gfx.ccrown);
	else if (!strcmp(attrname, "gfxFaceA"))lua_pushnumber(state, sd->gfx.faceAcc);
	else if (!strcmp(attrname, "gfxFaceAC"))lua_pushnumber(state, sd->gfx.cfaceAcc);
	else if (!strcmp(attrname, "gfxFaceAT"))lua_pushnumber(state, sd->gfx.faceAccT);
	else if (!strcmp(attrname, "gfxFaceATC"))lua_pushnumber(state, sd->gfx.cfaceAccT);
	else if (!strcmp(attrname, "gfxBoots"))lua_pushnumber(state, sd->gfx.boots);
	else if (!strcmp(attrname, "gfxBootsC"))lua_pushnumber(state, sd->gfx.cboots);
	else if (!strcmp(attrname, "gfxNeck"))lua_pushnumber(state, sd->gfx.necklace);
	else if (!strcmp(attrname, "gfxNeckC"))lua_pushnumber(state, sd->gfx.cnecklace);
	else if (!strcmp(attrname, "gfxName"))lua_pushstring(state, sd->gfx.name);
	else if (!strcmp(attrname, "gfxClone"))lua_pushnumber(state, sd->clone);
	else if (!strcmp(attrname, "PK"))lua_pushnumber(state, sd->status.pk);
	else if (!strcmp(attrname, "killedBy"))lua_pushnumber(state, sd->status.killedby);
	else if (!strcmp(attrname, "killsPK"))lua_pushnumber(state, sd->status.killspk);
	else if (!strcmp(attrname, "killsPVP"))lua_pushnumber(state, sd->killspvp);
	else if (!strcmp(attrname, "durationPK"))lua_pushnumber(state, sd->status.pkduration);
	else if (!strcmp(attrname, "protection"))lua_pushnumber(state, sd->protection);
	else if (!strcmp(attrname, "createCount"))lua_pushnumber(state, RFIFOB(sd->fd, 5));
	else if (!strcmp(attrname, "timerTick"))lua_pushnumber(state, sd->scripttick);
	else if (!strcmp(attrname, "dmgShield"))lua_pushnumber(state, sd->dmgshield);
	else if (!strcmp(attrname, "dmgDealt"))lua_pushnumber(state, sd->dmgdealt);
	else if (!strcmp(attrname, "dmgTaken"))lua_pushnumber(state, sd->dmgtaken);
	else if (!strcmp(attrname, "action"))lua_pushnumber(state, sd->action);
	else if (!strcmp(attrname, "mute"))lua_pushboolean(state, sd->status.mute);
	else if (!strcmp(attrname, "wisdom"))lua_pushnumber(state, sd->wisdom);
	else if (!strcmp(attrname, "con"))lua_pushnumber(state, sd->con);
	else if (!strcmp(attrname, "deathFlag"))lua_pushnumber(state, sd->deathflag);
	else if (!strcmp(attrname, "selfBar")) lua_pushboolean(state, sd->selfbar);
	else if (!strcmp(attrname, "groupBars")) lua_pushboolean(state, sd->groupbars);
	else if (!strcmp(attrname, "mobBars")) lua_pushboolean(state, sd->mobbars);
	else if (!strcmp(attrname, "displayTimeLeft")) lua_pushnumber(state, sd->disptimertick);
	else if (!strcmp(attrname, "bindMap")) lua_pushnumber(state, sd->bindmap);
	else if (!strcmp(attrname, "bindX")) lua_pushnumber(state, sd->bindx);
	else if (!strcmp(attrname, "bindY")) lua_pushnumber(state, sd->bindy);
	else if (!strcmp(attrname, "heroShow")) lua_pushnumber(state, sd->status.heroes);
	else if (!strcmp(attrname, "ambushTimer")) lua_pushnumber(state, sd->ambushtimer);
	else if (!strcmp(attrname, "dialogType")) lua_pushnumber(state, sd->dialogtype);
	else if (!strcmp(attrname, "mail")) lua_pushstring(state, sd->mail);
	else if (!strcmp(attrname, "cursed")) lua_pushnumber(state, sd->cursed);
	else if (!strcmp(attrname, "profileVitaStats")) lua_pushnumber(state, sd->status.profile_vitastats);
	else if (!strcmp(attrname, "profileEquipList")) lua_pushnumber(state, sd->status.profile_equiplist);
	else if (!strcmp(attrname, "profileLegends")) lua_pushnumber(state, sd->status.profile_legends);
	else if (!strcmp(attrname, "profileSpells")) lua_pushnumber(state, sd->status.profile_spells);
	else if (!strcmp(attrname, "profileInventory")) lua_pushnumber(state, sd->status.profile_inventory);
	else if (!strcmp(attrname, "profileBankItems")) lua_pushnumber(state, sd->status.profile_bankitems);
	else return 0;
	return 1;
}

/*==============================================================================
 * Combat functions
 *============================================================================*/

int pcl_swing(lua_State* state, USER* sd) {
	clif_parseattack(sd);
	return 0;
}

int pcl_swingtarget(lua_State* state, USER* sd) {
	struct block_list* bl = ((struct block_list*)typel_topointer(state, sl_memberarg(1)));

	nullpo_ret(0, bl);

	if (sd->status.equip[EQ_WEAP].id > 0) {
		clif_playsound(&sd->bl, itemdb_sound(sd->status.equip[EQ_WEAP].id));
	}

	if (bl->type == BL_MOB) {
		clif_mob_damage(sd, typel_topointer(state, sl_memberarg(1)));
	}
	else if (bl->type == BL_PC) {
		clif_pc_damage(sd, typel_topointer(state, sl_memberarg(1)));
	}

	return 0;
}

/*==============================================================================
 * Lock/Unlock functions
 *============================================================================*/

int pcl_lock(lua_State* state, USER* sd) {
	clif_blockmovement(sd, 0);
	return 0;
}

int pcl_unlock(lua_State* state, USER* sd) {
	clif_blockmovement(sd, 1);
	return 0;
}

/*==============================================================================
 * Menu/Dialog functions
 *============================================================================*/

int pcl_menu(lua_State* state, USER* sd) {
	sl_checkargs(state, "st");
	int previous = 0, next = 0;
	int size = lua_objlen(state, sl_memberarg(2));
	char* menuopts[size + 1];
	lua_pushnil(state);

	while (lua_next(state, sl_memberarg(2)) != 0) {
		// key is at -2, value at -1
		int index = lua_tonumber(state, -2);
		menuopts[index] = lua_tostring(state, -1);
		lua_pop(state, 1);
	}

	lua_pushnil(state);

	while (lua_next(state, sl_memberarg(2)) != 0) {
		// key is at -2, value is at -1
		if (!strcmp(lua_tostring(state, -1), "previous")) previous = 1;
		else if (!strcmp(lua_tostring(state, -1), "next")) next = 1;
		lua_pop(state, 1);
	}

	char* topic = lua_tostring(state, sl_memberarg(1));
	clif_scriptmenuseq(sd, sd->last_click, topic, menuopts, size, previous, next);
	return lua_yield(state, 0);
}

int pcl_menuseq(lua_State* state, USER* sd) {
	sl_checkargs(state, "stt");
	int previous = 0, next = 0;
	int size = lua_objlen(state, sl_memberarg(2));
	char* menuopts[size + 1];

	lua_pushnil(state);

	while (lua_next(state, sl_memberarg(2)) != 0) {
		// key is at -2, value at -1
		int index = lua_tonumber(state, -2);
		menuopts[index] = lua_tostring(state, -1);
		lua_pop(state, 1);
	}

	lua_pushnil(state);

	while (lua_next(state, sl_memberarg(2)) != 0) {
		// key is at -2, value is at -1
		if (!strcmp(lua_tostring(state, -1), "previous")) previous = 1;
		else if (!strcmp(lua_tostring(state, -1), "next")) next = 1;
		lua_pop(state, 1);
	}

	char* topic = lua_tostring(state, sl_memberarg(1));
	clif_scriptmenuseq(sd, sd->last_click, topic, menuopts, size, previous, next);
	return lua_yield(state, 0);
}

int pcl_inputseq(lua_State* state, USER* sd) {
	sl_checkargs(state, "ssstt");

	int previous = 0, next = 0;
	int size = lua_objlen(state, sl_memberarg(4));
	char* menuopts[size + 1];

	lua_pushnil(state);

	while (lua_next(state, sl_memberarg(4)) != 0) {
		// key is at -2, value at -1
		int index = lua_tonumber(state, -2);
		menuopts[index] = lua_tostring(state, -1);
		lua_pop(state, 1);
	}

	lua_pushnil(state);

	while (lua_next(state, sl_memberarg(4)) != 0) {
		// key is at -2, value is at -1
		if (!strcmp(lua_tostring(state, -1), "previous")) previous = 1;
		else if (!strcmp(lua_tostring(state, -1), "next")) next = 1;
		lua_pop(state, 1);
	}

	char* topic = lua_tostring(state, sl_memberarg(1));
	char* topic2 = lua_tostring(state, sl_memberarg(2));
	char* topic3 = lua_tostring(state, sl_memberarg(3));

	clif_inputseq(sd, sd->last_click, topic, topic2, topic3, menuopts, size, previous, next);
	return lua_yield(state, 0);
}

int pcl_sendboardquestions(lua_State* state, USER* sd) {
	sl_checkargs(state, "ttt");

	unsigned int amount = lua_objlen(state, sl_memberarg(1));

	struct board_questionaire questions[amount + 1];
	memset(&questions, 0, sizeof(questions));

	lua_pushnil(state);
	while (lua_next(state, sl_memberarg(1)) != 0) {
		// key is at -2, value at -1
		int index = lua_tonumber(state, -2);
		strcpy(&questions[index - 1].header, lua_tostring(state, -1));
		lua_pop(state, 1);
	}

	lua_pushnil(state);
	while (lua_next(state, sl_memberarg(2)) != 0) {
		// key is at -2, value at -1
		int index = lua_tonumber(state, -2);
		strcpy(&questions[index - 1].question, lua_tostring(state, -1));
		lua_pop(state, 1);
	}

	lua_pushnil(state);
	while (lua_next(state, sl_memberarg(3)) != 0) {
		// key is at -2, value at -1
		int index = lua_tonumber(state, -2);
		questions[index - 1].inputLines = lua_tonumber(state, -1);
		lua_pop(state, 1);
	}

	clif_sendBoardQuestionaire(sd, &questions, amount);

	return lua_yield(state, 0);
}

int pcl_popup(lua_State* state, USER* sd) {
	sl_checkargs(state, "s");

	clif_popup(sd, lua_tostring(state, sl_memberarg(1)));
	return 0;
}

int pcl_paperpopup(lua_State* state, USER* sd) {
	if (!sd) return 0;

	sl_checkargs(state, "nns");

	int width = lua_tonumber(state, sl_memberarg(1));
	int height = lua_tonumber(state, sl_memberarg(2));
	unsigned char* string = lua_tostring(state, sl_memberarg(3));

	clif_paperpopup(sd, string, width, height);

	return 0;
}

int pcl_paperpopupwrite(lua_State* state, USER* sd) {
	if (!sd) return 0;

	sl_checkargs(state, "nns");

	int width = lua_tonumber(state, sl_memberarg(1));
	int height = lua_tonumber(state, sl_memberarg(2));
	unsigned char* string = lua_tostring(state, sl_memberarg(3));

	clif_paperpopupwrite(sd, string, width, height, sd->invslot);

	return 0;
}

/*==============================================================================
 * Duration/Aether functions
 *============================================================================*/

int pcl_durationamount(lua_State* state, USER* sd) {
	sl_checkargs(state, "s");
	int x, id;
	id = magicdb_id(lua_tostring(state, sl_memberarg(1)));

	for (x = 0; x < MAX_MAGIC_TIMERS; x++) {
		if (sd->status.dura_aether[x].id == id) {
			if (sd->status.dura_aether[x].duration > 0) {
				lua_pushnumber(state, sd->status.dura_aether[x].duration);
				return 1;
			}
		}
	}
	lua_pushnumber(state, 0);
	return 1;
}

int pcl_flushduration(lua_State* state, USER* sd) {
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
		id = sd->status.dura_aether[x].id;

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
			clif_send_duration(sd, id, 0, map_id2sd(sd->status.dura_aether[x].caster_id));
			sd->status.dura_aether[x].duration = 0;
			map_foreachinarea(clif_sendanimation, sd->bl.m, sd->bl.x, sd->bl.y, AREA, BL_PC, sd->status.dura_aether[x].animation, &sd->bl, -1);
			sd->status.dura_aether[x].animation = 0;
			bl = map_id2bl(sd->status.dura_aether[x].caster_id);
			sd->status.dura_aether[x].caster_id = 0;

			if (sd->status.dura_aether[x].aether == 0) {
				sd->status.dura_aether[x].id = 0;
			}

			if (bl != NULL) {
				sl_doscript_blargs(magicdb_yname(id), "uncast", 2, &sd->bl, bl);
			}
			else {
				sl_doscript_blargs(magicdb_yname(id), "uncast", 1, &sd->bl);
			}
		}
	}

	return 0;
}

int pcl_flushdurationnouncast(lua_State* state, USER* sd) {
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
		id = sd->status.dura_aether[x].id;

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
			clif_send_duration(sd, id, 0, map_id2sd(sd->status.dura_aether[x].caster_id));
			sd->status.dura_aether[x].duration = 0;
			sd->status.dura_aether[x].caster_id = 0;
			map_foreachinarea(clif_sendanimation, sd->bl.m, sd->bl.x, sd->bl.y, AREA, BL_PC, sd->status.dura_aether[x].animation, &sd->bl, -1);
			sd->status.dura_aether[x].animation = 0;

			if (sd->status.dura_aether[x].aether == 0) {
				sd->status.dura_aether[x].id = 0;
			}
		}
	}

	return 0;
}

int pcl_flushaether(lua_State* state, USER* sd) {
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
		id = sd->status.dura_aether[x].id;

		if (magicdb_aether(id) > dis) continue;

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
			clif_send_aether(sd, id, 0);
			sd->status.dura_aether[x].aether = 0;

			if (sd->status.dura_aether[x].duration == 0) {
				sd->status.dura_aether[x].id = 0;
			}
		}
	}

	return 0;
}

/*==============================================================================
 * Kill tracking functions
 *============================================================================*/

int pcl_killcount(lua_State* state, USER* sd) {
	int x;
	int id;

	if (lua_isnumber(state, sl_memberarg(1))) {
		sl_checkargs(state, "n");
		id = lua_tonumber(state, sl_memberarg(1));
	}
	else {
		sl_checkargs(state, "s");
		id = mobdb_id(lua_tostring(state, sl_memberarg(1)));
	}
	for (x = 0; x < MAX_KILLREG; x++) {
		if (sd->status.killreg[x].mob_id == id) {
			lua_pushnumber(state, sd->status.killreg[x].amount);
			return 1;
		}
	}

	lua_pushnumber(state, 0);
	return 1;
}

int pcl_setkillcount(lua_State* state, USER* sd) {
	int id;
	int amount = 0;

	if (lua_isnumber(state, sl_memberarg(1))) {
		sl_checkargs(state, "n");
		id = lua_tonumber(state, sl_memberarg(1));
	}
	else {
		sl_checkargs(state, "s");
		id = mobdb_id(lua_tostring(state, sl_memberarg(1)));
	}

	amount = lua_tonumber(state, sl_memberarg(2));

	for (int x = 0; x < MAX_KILLREG; x++) {
		if (sd->status.killreg[x].mob_id == id) { // finds mob in entries, updates
			sd->status.killreg[x].amount = amount;
			return 1;
		}
	}

	for (int x = 0; x < MAX_KILLREG; x++) { // this executes if mob could not already be found, makes new entry, sets to values
		if (sd->status.killreg[x].mob_id == 0) {
			sd->status.killreg[x].mob_id = id;
			sd->status.killreg[x].amount = amount;
			return 1;
		}
	}

	return 1;
}

int pcl_flushallkills(lua_State* state, USER* sd) {
	for (int x = 0; x < MAX_KILLREG; x++) {
		sd->status.killreg[x].mob_id = 0;
		sd->status.killreg[x].amount = 0;
	}

	return 0;
}

int pcl_flushkills(lua_State* state, USER* sd) {
	int x;
	int flushid = 0;

	if (lua_isnil(state, sl_memberarg(1)) == 0) {
		if (lua_isnumber(state, sl_memberarg(1))) {
			flushid = lua_tonumber(state, sl_memberarg(1));
		}
		else {
			flushid = mobdb_id(lua_tostring(state, sl_memberarg(1)));
		}
	}

	if (flushid == 0) {
		for (x = 0; x < MAX_KILLREG; x++) {
			sd->status.killreg[x].mob_id = 0;
			sd->status.killreg[x].amount = 0;
		}
	}
	else {
		for (x = 0; x < MAX_KILLREG; x++) {
			if (sd->status.killreg[x].mob_id == flushid) {
				sd->status.killreg[x].mob_id = 0;
				sd->status.killreg[x].amount = 0;
			}
		}
	}

	return 0;
}

/*==============================================================================
 * Health display functions
 *============================================================================*/

int pcl_showhealth(lua_State* state, USER* sd) {
	sl_checkargs(state, "nn");
	int type = lua_tonumber(state, sl_memberarg(1));
	int damage = lua_tonumber(state, sl_memberarg(2));
	clif_send_pc_health(sd, damage, type);
	return 0;
}

/*==============================================================================
 * Clan functions
 *============================================================================*/

int pcl_addclan(lua_State* state, USER* sd) {
	struct clan_data* clan;
	sl_checkargs(state, "s");
	char* c_name = lua_tostring(state, sl_memberarg(1));
	clan = clandb_searchname(c_name);
	int newid = 0;

	if (clan) {
		lua_pushnumber(state, 0);
	}
	else {
		newid = clandb_add(sd, c_name);
		lua_pushnumber(state, newid);
	}
	return 1;
}

/*==============================================================================
 * Equipment functions
 *============================================================================*/

int pcl_hasequipped(lua_State* state, USER* sd) {
	int id = 0;
	bool found = false;

	if (lua_isstring(state, sl_memberarg(1))) {
		id = itemdb_id(lua_tostring(state, sl_memberarg(1)));
	}
	else {
		id = lua_tonumber(state, sl_memberarg(1));
	}

	for (int x = 0; x < MAX_EQUIP; x++) {
		if ((sd->status.equip[x].id == id) && (id > 0)) {
			found = true;
			break;
		}
	}

	lua_pushboolean(state, found);

	return 1;
}

int pcl_updateinv(lua_State* state, USER* sd) {
	pc_loaditem(sd);
	return 0;
}

int pcl_deductarmor(lua_State* state, USER* sd) {
	sl_checkargs(state, "n");
	clif_deductarmor(sd, lua_tonumber(state, sl_memberarg(1)));
	return 0;
}

int pcl_deductweapon(lua_State* state, USER* sd) {
	sl_checkargs(state, "n");
	clif_deductweapon(sd, lua_tonumber(state, sl_memberarg(1)));
	return 0;
}

/*==============================================================================
 * Communication functions
 *============================================================================*/

int pcl_sendurl(lua_State* state, USER* sd) {
	int type = lua_tonumber(state, sl_memberarg(1));
	char* url = lua_tostring(state, sl_memberarg(2));

	clif_sendurl(sd, type, url);

	return 1;
}

/*==============================================================================
 * Item/Warp functions
 *============================================================================*/

int pcl_useitem(lua_State* state, USER* sd) {
	sl_checkargs(state, "s");
	char* itemname = lua_tostring(state, sl_memberarg(1));
	int i;
	for (i = 0; i < sd->status.maxinv; i++) {
		if (!strcmpi(itemdb_name(sd->status.inventory[i].id), itemname)) {
			pc_useitem(sd, i);
			return 0;
		}
	}
	return 0;
}

int pcl_warp(lua_State* state, USER* sd) {
	sl_checkargs(state, "nnn");
	int m = lua_tonumber(state, sl_memberarg(1)),
		x = lua_tonumber(state, sl_memberarg(2)),
		y = lua_tonumber(state, sl_memberarg(3));

	pc_warp(sd, m, x, y);
	return 0;
}

int pcl_forcedrop(lua_State* state, USER* sd) {
	sl_checkargs(state, "n");
	int id = lua_tonumber(state, sl_memberarg(1));

	pc_dropitemmap(sd, id, 0);
	return 0;
}

int pcl_resurrect(lua_State* state, USER* sd) {
	pc_res(sd);
	return 0;
}

int pcl_setMiniMapToggle(lua_State* state, USER* sd) {
	sl_checkargs(state, "n");
	unsigned int miniMapToggle = lua_tonumber(state, sl_memberarg(1));

	if (SQL_ERROR == Sql_Query(sql_handle, "UPDATE `Character` SET `ChaMiniMapToggle` = '%u' WHERE `ChaId` = '%d'", miniMapToggle, sd->status.id)) {
		SqlStmt_ShowDebug(sql_handle);
		return 1;
	}

	if (miniMapToggle == 0) clif_sendminitext(sd, "MiniMap: OFF");
	else clif_sendminitext(sd, "MiniMap: ON");

	sd->status.miniMapToggle = miniMapToggle;

	return 0;
}

/*==============================================================================
 * Health/damage functions
 *============================================================================*/

int pcl_addhealth(lua_State* state, USER* sd) {
	sl_checkargs(state, "n");
	int damage = lua_tonumber(state, sl_memberarg(1));
	struct block_list* bl = map_id2bl(sd->attacker);

	if (bl != NULL) {
		if (damage > 0) sl_doscript_blargs("player_combat", "on_healed", 2, &sd->bl, bl);
	}
	else {
		if (damage > 0) sl_doscript_blargs("player_combat", "on_healed", 1, &sd->bl);
	}

	clif_send_pc_healthscript(sd, -damage, 0);
	clif_sendstatus(sd, SFLAG_HPMP);
	return 0;
}

int pcl_removehealth(lua_State* state, USER* sd) {
	sl_checkargs(state, "n");
	int damage = lua_tonumber(state, sl_memberarg(1));
	int caster = lua_tonumber(state, sl_memberarg(2));
	struct block_list* bl = NULL;
	USER* tsd = NULL;
	MOB* tmob = NULL;

	if (caster > 0) {
		bl = map_id2bl(caster);
		sd->attacker = caster;
	}
	else {
		bl = map_id2bl(sd->attacker);
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
		sd->damage = damage;
		sd->critchance = 0;
	}

	if (sd->status.state != PC_DIE) {
		clif_send_pc_healthscript(sd, damage, 0);
		clif_sendstatus(sd, SFLAG_HPMP);
	}
	return 0;
}

int pcl_sendhealth(lua_State* state, USER* sd) {
	clif_sendstatus(sd, SFLAG_HPMP);
	return 0;
}

int pcl_sendminitext(lua_State* state, USER* sd) {
	sl_checkargs(state, "s");
	char* msg;

	msg = lua_tostring(state, sl_memberarg(1));

	clif_sendminitext(sd, msg);
	return 0;
}

/*==============================================================================
 * Duration management functions
 *============================================================================*/

int pcl_setduration(lua_State* state, USER* sd) {
	sl_checkargs(state, "sn");
	struct block_list* bl = NULL;
	int id, time, x, caster_id, recast, alreadycast, mid;

	alreadycast = 0;
	id = magicdb_id(lua_tostring(state, sl_memberarg(1)));
	time = lua_tonumber(state, sl_memberarg(2));
	if (time < 1000 && time > 0) time = 1000;
	caster_id = lua_tonumber(state, sl_memberarg(3)); //sets duration from specific caster
	recast = lua_tonumber(state, sl_memberarg(4));

	for (x = 0; x < MAX_MAGIC_TIMERS; x++) {
		if (sd->status.dura_aether[x].id == id && sd->status.dura_aether[x].caster_id == caster_id && sd->status.dura_aether[x].duration > 0) {
			alreadycast = 1;
			break;
		}
	}

	for (x = 0; x < MAX_MAGIC_TIMERS; x++) {
		mid = sd->status.dura_aether[x].id;
		if (mid == id && time <= 0 && sd->status.dura_aether[x].caster_id == caster_id && alreadycast == 1) {
			clif_send_duration(sd, id, time, map_id2sd(caster_id));
			sd->status.dura_aether[x].duration = 0;
			sd->status.dura_aether[x].caster_id = 0;
			map_foreachinarea(clif_sendanimation, sd->bl.m, sd->bl.x, sd->bl.y, AREA, BL_PC, sd->status.dura_aether[x].animation, &sd->bl, -1);
			sd->status.dura_aether[x].animation = 0;

			if (sd->status.dura_aether[x].aether == 0) {
				sd->status.dura_aether[x].id = 0;
			}

			if (sd->status.dura_aether[x].caster_id != sd->bl.id) {
				bl = map_id2bl(sd->status.dura_aether[x].caster_id);
			}

			if (bl != NULL) {
				sl_doscript_blargs(magicdb_yname(mid), "uncast", 2, &sd->bl, bl);
			}
			else {
				sl_doscript_blargs(magicdb_yname(mid), "uncast", 1, &sd->bl);
			}

			return 0;
		}
		else if (sd->status.dura_aether[x].id == id && sd->status.dura_aether[x].caster_id == caster_id && sd->status.dura_aether[x].aether > 0 && sd->status.dura_aether[x].duration <= 0) {
			sd->status.dura_aether[x].duration = time;
			clif_send_duration(sd, id, time / 1000, map_id2sd(caster_id));
			return 0;
		}
		else if (sd->status.dura_aether[x].id == id && sd->status.dura_aether[x].caster_id == caster_id && (sd->status.dura_aether[x].duration > time || recast == 1) && alreadycast == 1) {
			sd->status.dura_aether[x].duration = time;
			clif_send_duration(sd, id, time / 1000, map_id2sd(caster_id));
			return 0;
		}
		else if (sd->status.dura_aether[x].id == 0 && sd->status.dura_aether[x].duration == 0 && time != 0 && alreadycast != 1) {
			sd->status.dura_aether[x].id = id;
			sd->status.dura_aether[x].duration = time;
			sd->status.dura_aether[x].caster_id = caster_id;
			clif_send_duration(sd, id, time / 1000, map_id2sd(caster_id));
			return 0;
		}
	}

	return 0;
}

int pcl_setaether(lua_State* state, USER* sd) {
	sl_checkargs(state, "sn");
	int id;
	int time;
	int x;
	int alreadycast;

	id = magicdb_id(lua_tostring(state, sl_memberarg(1)));
	time = lua_tonumber(state, sl_memberarg(2));
	if (time < 1000 && time > 0) time = 1000;

	for (x = 0; x < MAX_MAGIC_TIMERS; x++) {
		if (sd->status.dura_aether[x].id == id) {
			alreadycast = 1;
		}
	}

	for (x = 0; x < MAX_MAGIC_TIMERS; x++) {
		if (sd->status.dura_aether[x].id == id && time <= 0) {
			clif_send_aether(sd, id, time);

			if (sd->status.dura_aether[x].duration == 0) {
				sd->status.dura_aether[x].id = 0;
			}

			sd->status.dura_aether[x].aether = 0;
			return 0;
		}
		else if (sd->status.dura_aether[x].id == id && (sd->status.dura_aether[x].aether > time || sd->status.dura_aether[x].duration > 0)) {
			sd->status.dura_aether[x].aether = time;
			clif_send_aether(sd, id, time / 1000);
			return 0;
		}
		else if (sd->status.dura_aether[x].id == 0 && sd->status.dura_aether[x].aether == 0 && time != 0 && alreadycast != 1) {
			sd->status.dura_aether[x].id = id;
			sd->status.dura_aether[x].aether = time;
			clif_send_aether(sd, id, time / 1000);
			return 0;
		}
	}
	return 0;
}

int pcl_hasaether(lua_State* state, USER* sd) {
	sl_checkargs(state, "s");
	int x, id;

	id = magicdb_id(lua_tostring(state, sl_memberarg(1)));

	for (x = 0; x < MAX_MAGIC_TIMERS; x++) {
		if (sd->status.dura_aether[x].id == id) {
			if (sd->status.dura_aether[x].aether > 0) {
				lua_pushboolean(state, 1);
				return 1;
			}
		}
	}
	lua_pushboolean(state, 0);
	return 1;
}

int pcl_getaether(lua_State* state, USER* sd) {
	sl_checkargs(state, "s");
	int x, id;

	id = magicdb_id(lua_tostring(state, sl_memberarg(1)));

	for (x = 0; x < MAX_MAGIC_TIMERS; x++) {
		if (sd->status.dura_aether[x].id == id) {
			if (sd->status.dura_aether[x].aether > 0) {
				lua_pushnumber(state, sd->status.dura_aether[x].aether);
				return 1;
			}
		}
	}

	lua_pushnumber(state, 0);
	return 1;
}

int pcl_getallaethers(lua_State* state, USER* sd) {
	int i, x;
	lua_newtable(state);

	for (i = 0, x = 1; i < MAX_MAGIC_TIMERS; i++, x += 2) {
		if (sd->status.dura_aether[i].id > 0 && sd->status.dura_aether[i].aether > 0) {
			lua_pushnumber(state, sd->status.dura_aether[i].aether);
			lua_rawseti(state, -2, x);
			lua_pushstring(state, magicdb_yname(sd->status.dura_aether[i].id));
			lua_rawseti(state, -2, x + 1);
		}
	}

	return 1;
}

int pcl_hasduration(lua_State* state, USER* sd) {
	sl_checkargs(state, "s");
	int id = 0;

	id = magicdb_id(lua_tostring(state, sl_memberarg(1)));

	for (int x = 0; x < MAX_MAGIC_TIMERS; x++) {
		if (sd->status.dura_aether[x].id > 0 && sd->status.dura_aether[x].id == id) {
			if (sd->status.dura_aether[x].duration > 0) {
				lua_pushboolean(state, 1);
				return 1;
			}
		}
	}
	lua_pushboolean(state, 0);
	return 1;
}

int pcl_hasdurationid(lua_State* state, USER* sd) {
	sl_checkargs(state, "sn");
	int x, id, caster_id;

	caster_id = 0;

	id = magicdb_id(lua_tostring(state, sl_memberarg(1)));
	caster_id = lua_tonumber(state, sl_memberarg(2)); //checks for duration from specific caster

	for (x = 0; x < MAX_MAGIC_TIMERS; x++) {
		if (sd->status.dura_aether[x].id == id && sd->status.dura_aether[x].caster_id == caster_id) {
			if (sd->status.dura_aether[x].duration > 0) {
				lua_pushboolean(state, 1);
				return 1;
			}
		}
	}
	lua_pushboolean(state, 0);
	return 1;
}

int pcl_getduration(lua_State* state, USER* sd) {
	sl_checkargs(state, "s");
	int i, id;

	id = magicdb_id(lua_tostring(state, sl_memberarg(1)));

	for (i = 0; i < MAX_MAGIC_TIMERS; i++) {
		if (sd->status.dura_aether[i].id == id) {
			if (sd->status.dura_aether[i].duration > 0) {
				lua_pushnumber(state, sd->status.dura_aether[i].duration);
				return 1;
			}
		}
	}

	lua_pushnumber(state, 0);
	return 1;
}

int pcl_getdurationid(lua_State* state, USER* sd) {
	sl_checkargs(state, "sn");
	int i, id, caster_id;

	caster_id = 0;
	id = magicdb_id(lua_tostring(state, sl_memberarg(1)));
	caster_id = lua_tonumber(state, sl_memberarg(2));

	for (i = 0; i < MAX_MAGIC_TIMERS; i++) {
		if (sd->status.dura_aether[i].id == id && sd->status.dura_aether[i].caster_id == caster_id) {
			if (sd->status.dura_aether[i].duration > 0) {
				lua_pushnumber(state, sd->status.dura_aether[i].duration);
				return 1;
			}
		}
	}

	lua_pushnumber(state, 0);
	return 1;
}

int pcl_getalldurations(lua_State* state, USER* sd) {
	int i, x;
	lua_newtable(state);

	for (i = 0, x = 1; i < MAX_MAGIC_TIMERS; i++, x++) {
		if (sd->status.dura_aether[i].id > 0 && sd->status.dura_aether[i].duration > 0) {
			lua_pushstring(state, magicdb_yname(sd->status.dura_aether[i].id));
			lua_rawseti(state, -2, x);
		}
	}

	return 1;
}

int pcl_forcesave(lua_State* state, USER* sd) {
	intif_save(sd);

	return 1;
}

int pcl_refreshdurations(lua_State* state, USER* sd) {
	int i = 0;
	int x = 0;

	for (i = 0, x = 1; i < MAX_MAGIC_TIMERS; i++, x++) {
		if (sd->status.dura_aether[i].id > 0 && sd->status.dura_aether[i].duration > 0) {
			clif_send_duration(sd, sd->status.dura_aether[i].id, sd->status.dura_aether[i].duration / 1000, map_id2sd(sd->status.dura_aether[i].caster_id));
		}
	}

	return 1;
}

/*==============================================================================
 * Legend functions
 *============================================================================*/

int pcl_addlegend(lua_State* state, USER* sd) {
	sl_checkargs(state, "ssnn");
	int icon, color, x;
	int found = 0;
	unsigned tchaid = 0;

	icon = lua_tonumber(state, sl_memberarg(3));
	color = lua_tonumber(state, sl_memberarg(4));
	tchaid = lua_tonumber(state, sl_memberarg(5));

	for (x = 0; x < MAX_LEGENDS; x++) {
		if (strcmpi(sd->status.legends[x].name, "") == 0 && strcmpi(sd->status.legends[x + 1].name, "") == 0) {
			strcpy(sd->status.legends[x].text, lua_tostring(state, sl_memberarg(1)));
			strcpy(sd->status.legends[x].name, lua_tostring(state, sl_memberarg(2)));
			sd->status.legends[x].icon = icon;
			sd->status.legends[x].color = color;
			sd->status.legends[x].tchaid = tchaid;
			return 0;
		}
	}

	return 0;
}

int pcl_haslegend(lua_State* state, USER* sd) {
	sl_checkargs(state, "s");
	int i;
	char* legend = lua_tostring(state, sl_memberarg(1));

	for (i = 0; i < MAX_LEGENDS; i++) {
		if (!strcmp(sd->status.legends[i].name, legend)) {
			lua_pushboolean(state, 1);
			return 1;
		}
	}

	lua_pushboolean(state, 0);
	return 1;
}

/*==============================================================================
 * Input/Dialog functions
 *============================================================================*/

int pcl_input(lua_State* state, USER* sd) {
	sl_checkargs(state, "s");
	int count = lua_gettop(state);
	char* dialog = lua_tostring(state, sl_memberarg(1));
	char* name = NULL;

	int temp = 0;

	if (lua_isnumber(state, sl_memberarg(2))) name = itemdb_name(lua_tonumber(state, sl_memberarg(2)));
	else if (lua_isstring(state, sl_memberarg(2))) name = lua_tostring(state, sl_memberarg(2));

	lua_pushnil(state);

	if (name) clif_input(sd, 0, dialog, name);
	else clif_input(sd, sd->last_click, dialog, "");

	return lua_yield(state, 0);
}

/*==============================================================================
 * Status functions
 *============================================================================*/

int pcl_sendstatus(lua_State* state, USER* sd) {
	pc_requestmp(sd);
	clif_sendstatus(sd, SFLAG_FULLSTATS | SFLAG_HPMP | SFLAG_XPMONEY);
	clif_sendupdatestatus_onequip(sd);
	return 0;
}

int pcl_calcstat(lua_State* state, USER* sd) {
	pc_calcstat(sd);
	return 0;
}

/*==============================================================================
 * Item retrieval functions
 *============================================================================*/

int pcl_getboditem(lua_State* state, USER* sd) {
	sl_checkargs(state, "n");
	int num = lua_tonumber(state, sl_memberarg(1));
	if (sd->boditems.item[num].id != 0)
		biteml_pushinst(state, &sd->boditems.item[num]);
	else lua_pushnil(state);
	return 1;
}

int pcl_getexchangeitem(lua_State* state, USER* sd) {
	sl_checkargs(state, "n");
	int num = lua_tonumber(state, sl_memberarg(1));
	if (sd->exchange.item[num].id != 0)
		biteml_pushinst(state, &sd->exchange.item[num]);
	else lua_pushnil(state);
	return 1;
}

int pcl_getinventoryitem(lua_State* state, USER* sd) {
	sl_checkargs(state, "n");
	int num = lua_tonumber(state, sl_memberarg(1));
	if (sd->status.inventory[num].id != 0)
		biteml_pushinst(state, &sd->status.inventory[num]);
	else lua_pushnil(state);
	return 1;
}

int pcl_getequippeditem(lua_State* state, USER* sd) {
	sl_checkargs(state, "n");
	int num = lua_tonumber(state, sl_memberarg(1));
	if (sd->status.equip[num].id != 0)
		biteml_pushinst(state, &sd->status.equip[num]);
	else lua_pushnil(state);
	return 1;
}

/*==============================================================================
 * Legend removal functions
 *============================================================================*/

int pcl_removelegendbyname(lua_State* state, USER* sd) {
	sl_checkargs(state, "s");
	char* find = lua_tostring(state, sl_memberarg(1));
	int x;

	for (x = 0; x < MAX_LEGENDS; x++) {
		if (strcmpi(sd->status.legends[x].name, find) == 0) {
			strcpy(sd->status.legends[x].text, "");
			strcpy(sd->status.legends[x].name, "");
			sd->status.legends[x].icon = 0;
			sd->status.legends[x].color = 0;
			sd->status.legends[x].tchaid = 0;
		}
	}
	for (x = 0; x < MAX_LEGENDS; x++) {
		if (strcmpi(sd->status.legends[x].name, "") == 0 && strcmpi(sd->status.legends[x + 1].name, "") != 0) {
			strcpy(sd->status.legends[x].text, sd->status.legends[x + 1].text);
			strcpy(sd->status.legends[x + 1].text, "");
			strcpy(sd->status.legends[x].name, sd->status.legends[x + 1].name);
			strcpy(sd->status.legends[x + 1].name, "");
			sd->status.legends[x].icon = sd->status.legends[x + 1].icon;
			sd->status.legends[x + 1].icon = 0;
			sd->status.legends[x].color = sd->status.legends[x + 1].color;
			sd->status.legends[x + 1].color = 0;
			sd->status.legends[x].tchaid = sd->status.legends[x + 1].tchaid;
			sd->status.legends[x + 1].tchaid = 0;
		}
	}
	return 0;
}

int pcl_removelegendbycolor(lua_State* state, USER* sd) {
	sl_checkargs(state, "n");
	int find = lua_tonumber(state, sl_memberarg(1));
	int x, count = 0;

	for (x = 0; x < MAX_LEGENDS; x++) {
		if (sd->status.legends[x].color == find) {
			count++;
		}
		if (x + count < MAX_LEGENDS) {
			strcpy(sd->status.legends[x].text, sd->status.legends[x + count].text);
			strcpy(sd->status.legends[x].name, sd->status.legends[x + count].name);
			sd->status.legends[x].icon = sd->status.legends[x + count].icon;
			sd->status.legends[x].color = sd->status.legends[x + count].color;
			sd->status.legends[x].tchaid = sd->status.legends[x + count].tchaid;
		}
	}
	for (x = 0; x < MAX_LEGENDS; x++) {
		if (strcmpi(sd->status.legends[x].name, "") == 0 && strcmpi(sd->status.legends[x + 1].name, "") != 0) {
			strcpy(sd->status.legends[x].text, sd->status.legends[x + 1].text);
			strcpy(sd->status.legends[x + 1].text, "");
			strcpy(sd->status.legends[x].name, sd->status.legends[x + 1].name);
			strcpy(sd->status.legends[x + 1].name, "");
			sd->status.legends[x].icon = sd->status.legends[x + 1].icon;
			sd->status.legends[x + 1].icon = 0;
			sd->status.legends[x].color = sd->status.legends[x + 1].color;
			sd->status.legends[x + 1].color = 0;
			sd->status.legends[x].tchaid = sd->status.legends[x + 1].tchaid;
			sd->status.legends[x + 1].tchaid = 0;
		}
	}
	return 0;
}

/*==============================================================================
 * Item removal functions
 *============================================================================*/

int pcl_removeitemslot(lua_State* state, USER* sd) {
	sl_checkargs(state, "nn");
	int slot, amount, max, type;

	slot = lua_tonumber(state, sl_memberarg(1));
	amount = lua_tonumber(state, sl_memberarg(2));
	type = lua_tonumber(state, sl_memberarg(3));
	max = amount;

	if (sd->status.inventory[slot].id > 0) {
		if ((sd->status.inventory[slot].amount < amount) && (sd->status.inventory[slot].amount > 0)) {
			amount -= sd->status.inventory[slot].amount;
			pc_delitem(sd, slot, sd->status.inventory[slot].amount, type);

			if (amount == 0) {
				lua_pushboolean(state, 1);
				return 1;
			}
		}
		else if (sd->status.inventory[slot].amount >= amount) {
			if (type == 1) {
				pc_delitem(sd, slot, amount, 10);
			}
			else if (type == 2) {
				pc_delitem(sd, slot, amount, 9);
			}
			else {
				pc_delitem(sd, slot, amount, 11);
			}

			lua_pushboolean(state, 1);
			return 1;
		}
	}

	lua_pushboolean(state, 0);
	return 1;
}

int pcl_removeitemdura(lua_State* state, USER* sd) {
	unsigned int id = 0;
	unsigned int amount = 0, max = 0;
	int x;
	int count = 0;
	int type = 0;

	if (lua_isnumber(state, sl_memberarg(1))) {
		sl_checkargs(state, "nn");
		id = lua_tonumber(state, sl_memberarg(1));
	}
	else {
		sl_checkargs(state, "sn");
		id = itemdb_id(lua_tostring(state, sl_memberarg(1)));
	}
	amount = lua_tonumber(state, sl_memberarg(2));
	type = lua_tonumber(state, sl_memberarg(3));
	max = amount;

	for (x = 0; x < sd->status.maxinv; x++) {
		if (sd->status.inventory[x].id == id) {
			if ((sd->status.inventory[x].amount < amount) && (sd->status.inventory[x].amount > 0) && (sd->status.inventory[x].dura == itemdb_dura(sd->status.inventory[x].id))) {
				amount -= sd->status.inventory[x].amount;
				if (type == 1) {
					pc_delitem(sd, x, sd->status.inventory[x].amount, 10);
				}
				else if (type == 2) {
					pc_delitem(sd, x, sd->status.inventory[x].amount, 9);
				}
				else {
					pc_delitem(sd, x, sd->status.inventory[x].amount, 11);
				}

				if (amount == 0) {
					lua_pushboolean(state, 1);
					return 1;
				}
			}
			else if (sd->status.inventory[x].amount >= amount && (sd->status.inventory[x].dura == itemdb_dura(sd->status.inventory[x].id))) {
				if (type == 1) {
					pc_delitem(sd, x, amount, 10);
				}
				else if (type == 2) {
					pc_delitem(sd, x, amount, 9);
				}
				else {
					pc_delitem(sd, x, amount, 11);
				}

				lua_pushboolean(state, 1);
				return 1;
			}
		}
	}

	lua_pushboolean(state, 0);
	return 1;
}

/*==============================================================================
 * Board functions
 *============================================================================*/

int pcl_showboard(lua_State* state, USER* sd) {
	unsigned int id = 0;

	if (lua_isnumber(state, sl_memberarg(1))) {
		sl_checkargs(state, "n");
		id = lua_tonumber(state, sl_memberarg(1));
	}
	else {
		sl_checkargs(state, "s");
		id = boarddb_id(lua_tostring(state, sl_memberarg(1)));
	}

	sd->bcount = 0;
	sd->board_popup = 1;
	boards_showposts(sd, id);
	return 0;
}

int pcl_showpost(lua_State* state, USER* sd) {
	unsigned int id = 0;
	unsigned int post = 0;

	if (lua_isnumber(state, sl_memberarg(1))) {
		sl_checkargs(state, "nn");
		id = lua_tonumber(state, sl_memberarg(1));
	}
	else {
		sl_checkargs(state, "sn");
		id = boarddb_id(lua_tostring(state, sl_memberarg(1)));
	}

	post = lua_tonumber(state, sl_memberarg(2));

	sd->bcount = 0;
	sd->board_popup = 1;
	boards_readpost(sd, id, post);
	return 0;
}

int pcl_powerboard(lua_State* state, USER* sd) {
	clif_sendpowerboard(sd);
	return 0;
}

/*==============================================================================
 * Spell functions
 *============================================================================*/

int pcl_addspell(lua_State* state, USER* sd) {
	int spell;
	int x;

	if (lua_isnumber(state, sl_memberarg(1))) {
		spell = lua_tonumber(state, sl_memberarg(1));
	}
	else if (lua_isstring(state, sl_memberarg(1))) {
		spell = magicdb_id(lua_tostring(state, sl_memberarg(1)));
	}
	else {
		lua_pushnil(state);
		return 0;
	}

	for (x = 0; x < 52; x++) {
		if (sd->status.skill[x] == spell) {
			return 0;
		}
	}
	for (x = 0; x < 52; x++) {
		if (sd->status.skill[x] <= 0) {
			sl_doscript_blargs(magicdb_yname(spell), "on_learn", 1, &sd->bl);
			sd->status.skill[x] = spell;
			pc_loadmagic(sd);
			break;
		}
		if (sd->status.skill[x] == spell) {
			break;
		}
	}

	return 0;
}

int pcl_removespell(lua_State* state, USER* sd) {
	int spell;
	int x;

	if (lua_isnumber(state, sl_memberarg(1))) {
		spell = lua_tonumber(state, sl_memberarg(1));
	}
	else if (lua_isstring(state, sl_memberarg(1))) {
		spell = magicdb_id(lua_tostring(state, sl_memberarg(1)));
	}
	else {
		lua_pushnil(state);
		return 0;
	}

	for (x = 0; x < MAX_SPELLS; x++) {
		if (sd->status.skill[x] == spell) {
			sl_doscript_blargs(magicdb_yname(spell), "on_forget", 1, &sd->bl);
			clif_removespell(sd, x);
			sd->status.skill[x] = 0;
			return 0;
		}
	}

	return 0;
}

int pcl_hasspell(lua_State* state, USER* sd) {
	sl_checkargs(state, "s");
	int spell = magicdb_id(lua_tostring(state, sl_memberarg(1)));
	int x;

	for (x = 0; x < 52; x++) {
		if (sd->status.skill[x] == spell) {
			lua_pushboolean(state, 1);
			return 1;
		}
	}

	lua_pushboolean(state, 0);
	return 1;
}

int pcl_getspells(lua_State* state, USER* sd) {
	int i, x;
	lua_newtable(state);

	for (x = 0, i = 1; x < MAX_SPELLS; x++) {
		if (sd->status.skill[x]) {
			lua_pushnumber(state, sd->status.skill[x]);
			lua_rawseti(state, -2, i);
			i++;
		}
	}

	return 1;
}

int pcl_getspellname(lua_State* state, USER* sd) {
	int i, x;
	lua_newtable(state);

	for (x = 0, i = 1; x < MAX_SPELLS; x++) {
		if (sd->status.skill[x]) {
			lua_pushstring(state, magicdb_name(sd->status.skill[x]));
			lua_rawseti(state, -2, i);
			i++;
		}
	}

	return 1;
}

int pcl_getspellyname(lua_State* state, USER* sd) {
	int i, x;
	lua_newtable(state);

	for (x = 0, i = 1; x < MAX_SPELLS; x++) {
		if (sd->status.skill[x]) {
			lua_pushstring(state, magicdb_yname(sd->status.skill[x]));
			lua_rawseti(state, -2, i);
			i++;
		}
	}

	return 1;
}

int pcl_getspellnamefromyname(lua_State* state, USER* sd) {
	int id = magicdb_id(lua_tostring(state, sl_memberarg(1)));
	lua_pushstring(state, magicdb_name(id));
	return 1;
}

/*==============================================================================
 * Banking functions
 *============================================================================*/

int pcl_getbankitem(lua_State* state, USER* sd) {
	sl_checkargs(state, "n");
	int num = lua_tonumber(state, sl_memberarg(1));
	if (sd->status.banks[num].item_id != 0)
		bankiteml_pushinst(state, &sd->status.banks[num]);
	else lua_pushnil(state);
	return 1;
}

int pcl_getbankitems(lua_State* state, USER* sd) {
	int i, x;
	lua_newtable(state);

	for (i = 0, x = 1; i < MAX_BANK_SLOTS; i++, x += 10) {
		if (sd->status.banks[i].item_id > 0) {
			lua_pushnumber(state, sd->status.banks[i].item_id);
			lua_rawseti(state, -2, x);
			lua_pushnumber(state, sd->status.banks[i].amount);
			lua_rawseti(state, -2, x + 1);
			lua_pushnumber(state, sd->status.banks[i].owner);
			lua_rawseti(state, -2, x + 2);
			lua_pushnumber(state, sd->status.banks[i].time);
			lua_rawseti(state, -2, x + 3);
			lua_pushstring(state, sd->status.banks[i].real_name);
			lua_rawseti(state, -2, x + 4);
			lua_pushnumber(state, sd->status.banks[i].protected);
			lua_rawseti(state, -2, x + 5);
			lua_pushnumber(state, sd->status.banks[i].customIcon);
			lua_rawseti(state, -2, x + 6);
			lua_pushnumber(state, sd->status.banks[i].customIconColor);
			lua_rawseti(state, -2, x + 7);
			lua_pushnumber(state, sd->status.banks[i].customLook);
			lua_rawseti(state, -2, x + 8);
			lua_pushnumber(state, sd->status.banks[i].customLookColor);
			lua_rawseti(state, -2, x + 9);
		}
	}

	return 1;
}

int pcl_bankdeposit(lua_State* state, USER* sd) {
	sl_checkargs(state, "nn");
	int x, deposit, amount, owner;
	unsigned int time;

	unsigned int item;
	char* engrave = NULL;
	unsigned int protected, customIcon, customIconColor, customLook, customLookColor;

	deposit = -1;
	item = lua_tonumber(state, sl_memberarg(1));
	amount = lua_tonumber(state, sl_memberarg(2));
	owner = lua_tonumber(state, sl_memberarg(3));
	time = lua_tonumber(state, sl_memberarg(4));
	engrave = lua_tostring(state, sl_memberarg(5));
	protected = lua_tonumber(state, sl_memberarg(6));
	customIcon = lua_tonumber(state, sl_memberarg(7));
	customIconColor = lua_tonumber(state, sl_memberarg(8));
	customLook = lua_tonumber(state, sl_memberarg(9));
	customLookColor = lua_tonumber(state, sl_memberarg(10));

	for (x = 0; x < MAX_BANK_SLOTS; x++) {
		if (sd->status.banks[x].item_id == item && sd->status.banks[x].owner == owner && sd->status.banks[x].time == time && !strcmpi(sd->status.banks[x].real_name, engrave) && sd->status.banks[x].protected == protected && sd->status.banks[x].customIcon == customIcon && sd->status.banks[x].customIconColor == customIconColor && sd->status.banks[x].customLook == customLook && sd->status.banks[x].customLookColor == customLookColor) {
			deposit = x;
			break;
		}
	}

	if (deposit != -1) {
		sd->status.banks[deposit].amount += amount;
	}
	else {
		for (x = 0; x < MAX_BANK_SLOTS; x++) {
			if (sd->status.banks[x].item_id == 0) {
				sd->status.banks[x].item_id = item;
				sd->status.banks[x].amount = amount;
				sd->status.banks[x].owner = owner;
				sd->status.banks[x].time = time;
				strcpy(sd->status.banks[x].real_name, engrave);
				sd->status.banks[x].protected = protected;
				sd->status.banks[x].customIcon = customIcon;
				sd->status.banks[x].customIconColor = customIconColor;
				sd->status.banks[x].customLook = customLook;
				sd->status.banks[x].customLookColor = customLookColor;
				lua_pushboolean(state, 1);
				return 1;
			}
		}
	}

	lua_pushboolean(state, 0);
	return 0;
}

int pcl_bankwithdraw(lua_State* state, USER* sd) {
	sl_checkargs(state, "nn");
	int x, deposit, amount, owner;
	unsigned int time;
	unsigned int item;
	char* engrave = NULL;
	unsigned int protected, customIcon, customIconColor, customLook, customLookColor;

	deposit = -1;
	item = lua_tonumber(state, sl_memberarg(1));
	amount = lua_tonumber(state, sl_memberarg(2));
	owner = lua_tonumber(state, sl_memberarg(3));
	time = lua_tonumber(state, sl_memberarg(4));
	engrave = lua_tostring(state, sl_memberarg(5));
	protected = lua_tonumber(state, sl_memberarg(6));
	customIcon = lua_tonumber(state, sl_memberarg(7));
	customIconColor = lua_tonumber(state, sl_memberarg(8));
	customLook = lua_tonumber(state, sl_memberarg(9));
	customLookColor = lua_tonumber(state, sl_memberarg(10));

	for (x = 0; x < MAX_BANK_SLOTS; x++) {
		if (sd->status.banks[x].item_id == item && sd->status.banks[x].owner == owner && sd->status.banks[x].time == time && !strcmpi(sd->status.banks[x].real_name, engrave) && sd->status.banks[x].protected == protected && sd->status.banks[x].customIcon == customIcon && sd->status.banks[x].customIconColor == customIconColor && sd->status.banks[x].customLook == customLook && sd->status.banks[x].customLookColor == customLookColor) {
			deposit = x;
			break;
		}
	}

	if (deposit != -1) {
		if ((sd->status.banks[deposit].amount - amount) < 0) {
			lua_pushboolean(state, 0);
			return 1;
		}
		else {
			sd->status.banks[deposit].amount -= amount;

			if (sd->status.banks[deposit].amount <= 0) {
				sd->status.banks[deposit].item_id = 0;
				sd->status.banks[deposit].amount = 0;
				sd->status.banks[deposit].owner = 0;
				sd->status.banks[deposit].time = 0;
				strcpy(sd->status.banks[deposit].real_name, "");
				sd->status.banks[deposit].protected = 0;
				sd->status.banks[deposit].customIcon = 0;
				sd->status.banks[deposit].customIconColor = 0;
				sd->status.banks[deposit].customLook = 0;
				sd->status.banks[deposit].customLookColor = 0;

				for (x = 0; x < MAX_BANK_SLOTS; x++) {
					if (sd->status.banks[x].item_id == 0 && x < MAX_BANK_SLOTS - 1) {
						sd->status.banks[x].item_id = sd->status.banks[x + 1].item_id;
						sd->status.banks[x + 1].item_id = 0;
						sd->status.banks[x].amount = sd->status.banks[x + 1].amount;
						sd->status.banks[x + 1].amount = 0;
						sd->status.banks[x].owner = sd->status.banks[x + 1].owner;
						sd->status.banks[x + 1].owner = 0;
						sd->status.banks[x].time = sd->status.banks[x + 1].time;
						sd->status.banks[x + 1].time = 0;
						strcpy(sd->status.banks[x].real_name, sd->status.banks[x + 1].real_name);
						strcpy(sd->status.banks[x + 1].real_name, "");
						sd->status.banks[x].protected = sd->status.banks[x + 1].protected;
						sd->status.banks[x + 1].protected = 0;
						sd->status.banks[x].customIcon = sd->status.banks[x + 1].customIcon;
						sd->status.banks[x + 1].customIcon = 0;
						sd->status.banks[x].customIconColor = sd->status.banks[x + 1].customIconColor;
						sd->status.banks[x + 1].customIconColor = 0;
						sd->status.banks[x].customLook = sd->status.banks[x + 1].customLook;
						sd->status.banks[x + 1].customLook = 0;
						sd->status.banks[x].customLookColor = sd->status.banks[x + 1].customLookColor;
						sd->status.banks[x + 1].customLookColor = 0;
					}
				}

				lua_pushboolean(state, 1);
				return 1;
			}

			lua_pushboolean(state, 1);
			return 1;
		}
	}

	lua_pushboolean(state, 0);
	return 1;
}

/*==============================================================================
 * Clan banking functions
 *============================================================================*/

int pcl_getclanbankitems(lua_State* state, USER* sd) {
	int i, x;

	struct clan_data* clan = NULL;

	unsigned int id = sd->status.clan;
	clan = clandb_search(id);

	lua_newtable(state);

	for (i = 0, x = 1; i < 255; i++, x += 10) {
		if (clan->clanbanks[i].item_id > 0) {
			lua_pushnumber(state, clan->clanbanks[i].item_id);
			lua_rawseti(state, -2, x);
			lua_pushnumber(state, clan->clanbanks[i].amount);
			lua_rawseti(state, -2, x + 1);
			lua_pushnumber(state, clan->clanbanks[i].owner);
			lua_rawseti(state, -2, x + 2);
			lua_pushnumber(state, clan->clanbanks[i].time);
			lua_rawseti(state, -2, x + 3);
			lua_pushstring(state, clan->clanbanks[i].real_name);
			lua_rawseti(state, -2, x + 4);
			lua_pushnumber(state, clan->clanbanks[i].protected);
			lua_rawseti(state, -2, x + 5);
			lua_pushnumber(state, clan->clanbanks[i].customIcon);
			lua_rawseti(state, -2, x + 6);
			lua_pushnumber(state, clan->clanbanks[i].customIconColor);
			lua_rawseti(state, -2, x + 7);
			lua_pushnumber(state, clan->clanbanks[i].customLook);
			lua_rawseti(state, -2, x + 8);
			lua_pushnumber(state, clan->clanbanks[i].customLookColor);
			lua_rawseti(state, -2, x + 9);
		}
	}

	return 1;
}

int pcl_clanbankdeposit(lua_State* state, USER* sd) {
	sl_checkargs(state, "nn");
	int x, deposit, amount, owner;
	unsigned int time;

	struct clan_data* clan = NULL;

	unsigned int item;
	char* engrave = NULL;
	unsigned int protected, customIcon, customIconColor, customLook, customLookColor;

	unsigned int id = sd->status.clan;
	clan = clandb_search(id);

	deposit = -1;
	item = lua_tonumber(state, sl_memberarg(1));
	amount = lua_tonumber(state, sl_memberarg(2));
	owner = lua_tonumber(state, sl_memberarg(3));
	time = lua_tonumber(state, sl_memberarg(4));
	engrave = lua_tostring(state, sl_memberarg(5));
	protected = lua_tonumber(state, sl_memberarg(6));
	customIcon = lua_tonumber(state, sl_memberarg(7));
	customIconColor = lua_tonumber(state, sl_memberarg(8));
	customLook = lua_tonumber(state, sl_memberarg(9));
	customLookColor = lua_tonumber(state, sl_memberarg(10));

	for (x = 0; x < 255; x++) {
		if (clan->clanbanks[x].item_id == item && clan->clanbanks[x].owner == owner && clan->clanbanks[x].time == time && !strcmpi(clan->clanbanks[x].real_name, engrave) && clan->clanbanks[x].protected == protected && clan->clanbanks[x].customIcon == customIcon && clan->clanbanks[x].customIconColor == customIconColor && clan->clanbanks[x].customLook == customLook && clan->clanbanks[x].customLookColor == customLookColor) {
			deposit = x;
			break;
		}
	}

	if (deposit != -1) {
		clan->clanbanks[deposit].amount += amount;
		map_saveclanbank(id);
		lua_pushboolean(state, 1);
	}
	else {
		for (x = 0; x < 255; x++) {
			if (clan->clanbanks[x].item_id == 0) {
				clan->clanbanks[x].item_id = item;
				clan->clanbanks[x].amount = amount;
				clan->clanbanks[x].owner = owner;
				clan->clanbanks[x].time = time;
				strcpy(clan->clanbanks[x].real_name, engrave);
				clan->clanbanks[x].protected = protected;
				clan->clanbanks[x].customIcon = customIcon;
				clan->clanbanks[x].customIconColor = customIconColor;
				clan->clanbanks[x].customLook = customLook;
				clan->clanbanks[x].customLookColor = customLookColor;
				map_saveclanbank(id);
				lua_pushboolean(state, 1);
				return 1;
			}
		}
	}

	lua_pushboolean(state, 0);
	return 0;
}

int pcl_clanbankwithdraw(lua_State* state, USER* sd) {
	sl_checkargs(state, "nn");
	int x, deposit, amount, owner;
	unsigned int time;
	unsigned int item;
	char* engrave = NULL;
	unsigned int protected, customIcon, customIconColor, customLook, customLookColor;

	unsigned int id = sd->status.clan;
	struct clan_data* clan;

	clan = clandb_search(id);

	deposit = -1;
	item = lua_tonumber(state, sl_memberarg(1));
	amount = lua_tonumber(state, sl_memberarg(2));
	owner = lua_tonumber(state, sl_memberarg(3));
	time = lua_tonumber(state, sl_memberarg(4));
	engrave = lua_tostring(state, sl_memberarg(5));
	protected = lua_tonumber(state, sl_memberarg(6));
	customIcon = lua_tonumber(state, sl_memberarg(7));
	customIconColor = lua_tonumber(state, sl_memberarg(8));
	customLook = lua_tonumber(state, sl_memberarg(9));
	customLookColor = lua_tonumber(state, sl_memberarg(10));

	for (x = 0; x < 255; x++) {
		if (clan->clanbanks[x].item_id == item && clan->clanbanks[x].owner == owner && clan->clanbanks[x].time == time && !strcmpi(clan->clanbanks[x].real_name, engrave) && clan->clanbanks[x].protected == protected && clan->clanbanks[x].customIcon == customIcon && clan->clanbanks[x].customIconColor == customIconColor && clan->clanbanks[x].customLook == customLook && clan->clanbanks[x].customLookColor == customLookColor) {
			deposit = x;
			break;
		}
	}

	if (deposit != -1) {
		if ((clan->clanbanks[deposit].amount - amount) < 0) {
			lua_pushboolean(state, 0);
			return 1;
		}
		else {
			clan->clanbanks[deposit].amount -= amount;

			if (clan->clanbanks[deposit].amount <= 0) {
				clan->clanbanks[deposit].item_id = 0;
				clan->clanbanks[deposit].amount = 0;
				clan->clanbanks[deposit].owner = 0;
				clan->clanbanks[deposit].time = 0;
				strcpy(clan->clanbanks[deposit].real_name, "");
				clan->clanbanks[deposit].protected = 0;
				clan->clanbanks[deposit].customIcon = 0;
				clan->clanbanks[deposit].customIconColor = 0;
				clan->clanbanks[deposit].customLook = 0;
				clan->clanbanks[deposit].customLookColor = 0;

				for (x = 0; x < 255; x++) {
					if (clan->clanbanks[x].item_id == 0 && x < 255 - 1) {
						clan->clanbanks[x].item_id = clan->clanbanks[x + 1].item_id;
						clan->clanbanks[x + 1].item_id = 0;
						clan->clanbanks[x].amount = clan->clanbanks[x + 1].amount;
						clan->clanbanks[x + 1].amount = 0;
						clan->clanbanks[x].owner = clan->clanbanks[x + 1].owner;
						clan->clanbanks[x + 1].owner = 0;
						clan->clanbanks[x].time = clan->clanbanks[x + 1].time;
						clan->clanbanks[x + 1].time = 0;
						strcpy(clan->clanbanks[x].real_name, clan->clanbanks[x + 1].real_name);
						strcpy(clan->clanbanks[x + 1].real_name, "");
						clan->clanbanks[x].protected = clan->clanbanks[x + 1].protected;
						clan->clanbanks[x + 1].protected = 0;
						clan->clanbanks[x].customIcon = clan->clanbanks[x + 1].customIcon;
						clan->clanbanks[x + 1].customIcon = 0;
						clan->clanbanks[x].customIconColor = clan->clanbanks[x + 1].customIconColor;
						clan->clanbanks[x + 1].customIconColor = 0;
						clan->clanbanks[x].customLook = clan->clanbanks[x + 1].customLook;
						clan->clanbanks[x + 1].customLook = 0;
						clan->clanbanks[x].customLookColor = clan->clanbanks[x + 1].customLookColor;
						clan->clanbanks[x + 1].customLookColor = 0;
					}
				}
				map_saveclanbank(id);
				lua_pushboolean(state, 1);
				return 1;
			}

			map_saveclanbank(id);
			lua_pushboolean(state, 1);
			return 1;
		}
	}

	lua_pushboolean(state, 0);
	return 1;
}

/*==============================================================================
 * Threat functions
 *============================================================================*/

int pcl_addthreat(lua_State* state, USER* sd) {
	sl_checkargs(state, "nn");
	unsigned int id, amount;
	int x;
	MOB* tmob = NULL;

	id = lua_tonumber(state, sl_memberarg(1));
	amount = lua_tonumber(state, sl_memberarg(2));
	tmob = (MOB*)map_id2mob(id);
	nullpo_ret(0, tmob);
	tmob->lastaction = time(NULL);

	for (x = 0; x < MAX_THREATCOUNT && tmob != NULL; x++) {
		if (tmob->threat[x].user == sd->bl.id) {
			tmob->threat[x].amount += amount;
			return 0;
		}
		else if (tmob->threat[x].user == 0) {
			tmob->threat[x].user = sd->bl.id;
			tmob->threat[x].amount = amount;
			return 0;
		}
	}

	return 0;
}

int pcl_setthreat(lua_State* state, USER* sd) {
	sl_checkargs(state, "nn");
	unsigned int id, amount;
	int x;
	MOB* tmob = NULL;

	id = lua_tonumber(state, sl_memberarg(1));
	amount = lua_tonumber(state, sl_memberarg(2));
	tmob = (MOB*)map_id2mob(id);
	nullpo_ret(0, tmob);
	tmob->lastaction = time(NULL);

	for (x = 0; x < MAX_THREATCOUNT && tmob != NULL; x++) {
		if (tmob->threat[x].user == sd->bl.id) {
			tmob->threat[x].amount = amount;
			return 0;
		}
		else if (tmob->threat[x].user == 0) {
			tmob->threat[x].user = sd->bl.id;
			tmob->threat[x].amount = amount;
			return 0;
		}
	}

	return 0;
}

int pcl_addthreatgeneral(lua_State* state, USER* sd) {
	sl_checkargs(state, "n");
	unsigned int amount;
	int i;

	amount = lua_tonumber(state, sl_memberarg(1));

	int bx, by, nx, ny, x0, x1, y0, y1, m, x, y;
	int nAreaSizeX = AREAX_SIZE, nAreaSizeY = AREAY_SIZE;
	struct block_list* bl = NULL;
	MOB* tmob = NULL;

	m = sd->bl.m;
	x = sd->bl.x;
	y = sd->bl.y;
	nx = map[m].xs - x;
	ny = map[m].ys - y;

	if (nx < 18) nAreaSizeX = nAreaSizeX * 2;
	if (ny < 16) nAreaSizeY = nAreaSizeY * 2;
	if (x < 18) nAreaSizeX = nAreaSizeX * 2;
	if (y < 16) nAreaSizeY = nAreaSizeY * 2;

	x0 = x - nAreaSizeX;
	x1 = x + nAreaSizeX;
	y0 = y - nAreaSizeY;
	y1 = y + nAreaSizeY;

	if (x0 < 0) x0 = 0;
	if (y0 < 0) y0 = 0;
	if (x1 >= map[m].xs) x1 = map[m].xs - 1;
	if (y1 >= map[m].ys) y1 = map[m].ys - 1;

	for (by = y0 / BLOCK_SIZE; by <= y1 / BLOCK_SIZE; by++) {
		for (bx = x0 / BLOCK_SIZE; bx <= x1 / BLOCK_SIZE; bx++) {
			for (bl = map[m].block_mob[bx + by * map[m].bxs]; bl; bl = bl->next) {
				if (bl->type == BL_MOB && bl->x >= x0 && bl->x <= x1 && bl->y >= y0 && bl->y <= y1) {
					tmob = (MOB*)bl;
					if (!tmob) continue;
					tmob->lastaction = time(NULL);

					for (i = 0; i < MAX_THREATCOUNT && tmob != NULL; i++) {
						if (tmob->threat[i].user == sd->bl.id) {
							tmob->threat[i].amount += amount;
						}
						else if (tmob->threat[i].user == 0) {
							tmob->threat[i].user = sd->bl.id;
							tmob->threat[i].amount = amount;
						}
					}
				}
			}
		}
	}

	return 0;
}

/*==============================================================================
 * Communication functions
 *============================================================================*/

int pcl_speak(lua_State* state, USER* sd) {
	sl_checkargs(state, "sn");
	char* msg = lua_tostring(state, sl_memberarg(1));
	int msglen = strlen(msg);
	int type = lua_tonumber(state, sl_memberarg(2));

	clif_sendscriptsay(sd, msg, msglen, type);
	return 0;
}

int pcl_sendmail(lua_State* state, USER* sd) {
	sl_checkargs(state, "sss");
	char* to_user = lua_tostring(state, sl_memberarg(1));
	char* topic = lua_tostring(state, sl_memberarg(2));
	char* message = lua_tostring(state, sl_memberarg(3));

	nmail_sendmail(sd, to_user, topic, message);
	return 0;
}

int pcl_freeasync(lua_State* state, USER* sd) {
	sl_async_freeco(sd);
	return 0;
}

/*==============================================================================
 * Equipment functions
 *============================================================================*/

int pcl_pickup(lua_State* state, USER* sd) {
	sl_checkargs(state, "n");
	unsigned int id = lua_tonumber(state, sl_memberarg(1));
	pc_getitemscript(sd, id);
	return 0;
}

int pcl_forceequip(lua_State* state, USER* sd) {
	struct item_data* db = NULL;

	int id = 0;

	if (lua_isnumber(state, sl_memberarg(1))) {
		sl_checkargs(state, "nn");
		id = lua_tonumber(state, sl_memberarg(1));
	}
	else {
		sl_checkargs(state, "sn");
		id = itemdb_id(lua_tostring(state, sl_memberarg(1)));
	}

	int slot = lua_tonumber(state, sl_memberarg(2));

	db = itemdb_search(id);

	if (db == NULL) return 1;

	sd->status.equip[slot].id = id;
	sd->status.equip[slot].dura = itemdb_dura(id);
	sd->status.equip[slot].protected = itemdb_protected(id);
	sd->status.equip[slot].owner = 0;
	sd->status.equip[slot].time = 0;
	strcpy(sd->status.equip[slot].real_name, "");
	strcpy(sd->status.equip[slot].note, "");
	sd->status.equip[slot].customLook = 0;
	sd->status.equip[slot].customLookColor = 0;
	sd->status.equip[slot].customIcon = 0;
	sd->status.equip[slot].customIconColor = 0;
	sd->status.equip[slot].custom = 0;
	sd->status.equip[slot].amount = 1;

	return 1;
}

int pcl_equip(lua_State* state, USER* sd) {
	pc_equipscript(sd);
	return 0;
}

int pcl_takeoff(lua_State* state, USER* sd) {
	pc_unequipscript(sd);
	return 0;
}

/*------------------------------------------------------------------------------
 * Strip/Die/Refresh
 *----------------------------------------------------------------------------*/

int pcl_stripequip(lua_State* state, USER* sd) {
	int x, type, force, destroy = 0;
	char minitext[255], itemname[100];
	sl_checkargs(state, "n");
	unsigned int equipType = lua_tonumber(state, sl_memberarg(1));
	FLOORITEM* fl;

	destroy = lua_tonumber(state, sl_memberarg(2));			//if 1 then will be destroyed
	force = lua_tonumber(state, sl_memberarg(3));			//if 1 then will be force to drop if inv full
	if (sd->status.equip[equipType].id != 0) {
		strcpy(itemname, itemdb_name(sd->status.equip[equipType].id));
		for (x = 0; x < sd->status.maxinv; x++) {
			if (!sd->status.inventory[x].id) {
				pc_unequip(sd, equipType);
				clif_unequipit(sd, getclifslotfromequiptype(equipType));
				if (destroy) {
					sprintf(minitext, "Your %s has been destroyed.", itemname);
					pc_delitem(sd, x, 1, 8);				//make the item decayed
				}
				else {
					sprintf(minitext, "Your %s has been stripped.", itemname);
				}
				return 0;
			}
		}
		if (destroy) {
			pc_unequip(sd, equipType);
			fl = (FLOORITEM*)map_firstincell(sd->bl.m, sd->bl.x, sd->bl.y, BL_ITEM);
			nullpo_ret(0, fl);
			clif_unequipit(sd, getclifslotfromequiptype(equipType));
			//need to delete floor item
			clif_lookgone(&fl->bl);
			map_delitem(fl->bl.id);
			sprintf(minitext, "Your %s has been destroyed.", itemname);
		}
		else if (force) {
			pc_unequip(sd, equipType);
			fl = (FLOORITEM*)map_firstincell(sd->bl.m, sd->bl.x, sd->bl.y, BL_ITEM);
			nullpo_ret(0, fl);
			clif_unequipit(sd, getclifslotfromequiptype(equipType));
			sprintf(minitext, "Your %s forcefully stripped.", itemname);
		}
	}
	return 0;
}

int pcl_die(lua_State* state, USER* sd) {
	pc_diescript(sd);
	return 0;
}

int pcl_throwitem(lua_State* state, USER* sd) {
	clif_throwitem_script(sd);
	return 0;
}

int pcl_refresh(lua_State* state, USER* sd) {
	pc_setpos(sd, sd->bl.m, sd->bl.x, sd->bl.y);
	clif_refreshnoclick(sd);
	return 0;
}

int pcl_refreshInventory(lua_State* state, USER* sd) {
	for (int i = 0; i < 52; i++) {
		clif_sendadditem(sd, i);
	}

	return 0;
}

int pcl_move(lua_State* state, USER* sd) {
	//sl_checkargs(state, "n");
	char speed = lua_tonumber(state, sl_memberarg(1));
	lua_pushnumber(state, clif_noparsewalk(sd, speed));
	return 1;
}

int pcl_respawn(lua_State* state, USER* sd) {
	clif_spawn(sd);
	return 0;
}

/*------------------------------------------------------------------------------
 * Durability Management
 *----------------------------------------------------------------------------*/

int pcl_deductdura(lua_State* state, USER* sd) {
	sl_checkargs(state, "nn");
	int equip = lua_tonumber(state, sl_memberarg(1));
	unsigned int amount = lua_tonumber(state, sl_memberarg(2));

	clif_deductdura(sd, equip, amount);
	return 0;
}

int pcl_deductduraequip(lua_State* state, USER* sd) {
	clif_deductduraequip(sd);
	return 0;
}

int pcl_checkinvbod(lua_State* state, USER* sd) {
	clif_checkinvbod(sd);
	return 0;
}

int pcl_deductdurainv(lua_State* state, USER* sd) {
	sl_checkargs(state, "nn");
	int slot = lua_tonumber(state, sl_memberarg(1));
	unsigned int amount = lua_tonumber(state, sl_memberarg(2));

	sd->status.inventory[slot].dura -= amount;
	return 0;
}

/*------------------------------------------------------------------------------
 * PK System
 *----------------------------------------------------------------------------*/

int pcl_setpk(lua_State* state, USER* sd) {
	sl_checkargs(state, "n");
	int id = lua_tonumber(state, sl_memberarg(1));
	int exist = -1;
	int x;

	for (x = 0; x < 20; x++) {
		if (sd->pvp[x][0] == id) {
			exist = x;
			break;
		}
	}

	if (exist != -1) {
		sd->pvp[exist][1] = time(NULL);
	}
	else {
		for (x = 0; x < 20; x++) {
			if (!sd->pvp[x][0]) {
				sd->pvp[x][0] = id;
				sd->pvp[x][1] = time(NULL);
				clif_getchararea(sd);
				break;
			}
			else if (x == 19 && sd->pvp[x][0]) {
				lua_pushboolean(state, 0);
				return 1;
			}
		}
	}

	lua_pushboolean(state, 1);
	return 1;
}

int pcl_getpk(lua_State* state, USER* sd) {
	sl_checkargs(state, "n");
	int id = lua_tonumber(state, sl_memberarg(1));
	int x;

	for (x = 0; x < 20; x++) {
		if (sd->pvp[x][0] == id) {
			lua_pushboolean(state, 1);
			return 1;
		}
	}

	lua_pushboolean(state, 0);
	return 1;
}

/*------------------------------------------------------------------------------
 * GUI/Text
 *----------------------------------------------------------------------------*/

int pcl_guitext(lua_State* state, USER* sd) {
	sl_checkargs(state, "s");

	char* msg = lua_tostring(state, sl_memberarg(1));

	clif_guitextsd(msg, sd);

	return 0;
}

/*------------------------------------------------------------------------------
 * Creation System
 *----------------------------------------------------------------------------*/

int pcl_getcreationitems(lua_State* state, USER* sd) {
	sl_checkargs(state, "n");
	int len = lua_tonumber(state, sl_memberarg(1));
	int curitem;

	curitem = RFIFOB(sd->fd, len) - 1;

	if (sd->status.inventory[curitem].id) {
		lua_pushnumber(state, sd->status.inventory[curitem].id);
		return 1;
	}

	return 0;
}

int pcl_getcreationamounts(lua_State* state, USER* sd) {
	sl_checkargs(state, "nn");
	int len = lua_tonumber(state, sl_memberarg(1));
	int item = lua_tonumber(state, sl_memberarg(2));

	if (itemdb_type(item) < 3 || itemdb_type(item) > 17) {
		lua_pushnumber(state, RFIFOB(sd->fd, len));
		return 1;
	}
	else {
		lua_pushnumber(state, 1);
		return 1;
	}

	return 0;
}

/*------------------------------------------------------------------------------
 * Parcel System
 *----------------------------------------------------------------------------*/

int pcl_getparcel(lua_State* state, USER* sd) {
	struct parcel* item;
	unsigned int id, amount, owner, sender, customLookColor, customIconColor, customIcon, customLook;
	int pos;
	unsigned int protected = 0;
	unsigned int durability = 0;

	char real_name[64];
	char npcflag;
	SqlStmt* stmt = SqlStmt_Malloc(sql_handle);

	if (stmt == NULL) {
		SqlStmt_ShowDebug(stmt);
		lua_pushboolean(state, 0);
		SqlStmt_Free(stmt);
		return 1;
	}

	if (SQL_ERROR == SqlStmt_Prepare(stmt, "SELECT `ParItmId`, `ParAmount`, `ParChaIdOwner`, `ParEngrave`, `ParSender`, `ParPosition`, `ParNpc`, `ParCustomLook`, `ParCustomLookColor`, `ParCustomIcon`, `ParCustomIconColor`, `ParProtected`, `ParItmDura` FROM `Parcels` WHERE `ParChaIdDestination` = '%u'", sd->status.id)
		|| SQL_ERROR == SqlStmt_Execute(stmt)
		|| SQL_ERROR == SqlStmt_BindColumn(stmt, 0, SQLDT_UINT, &id, 0, NULL, NULL)
		|| SQL_ERROR == SqlStmt_BindColumn(stmt, 1, SQLDT_UINT, &amount, 0, NULL, NULL)
		|| SQL_ERROR == SqlStmt_BindColumn(stmt, 2, SQLDT_UINT, &owner, 0, NULL, NULL)
		|| SQL_ERROR == SqlStmt_BindColumn(stmt, 3, SQLDT_STRING, &real_name, sizeof(real_name), NULL, NULL)
		|| SQL_ERROR == SqlStmt_BindColumn(stmt, 4, SQLDT_UINT, &sender, 0, NULL, NULL)
		|| SQL_ERROR == SqlStmt_BindColumn(stmt, 5, SQLDT_INT, &pos, 0, NULL, NULL)
		|| SQL_ERROR == SqlStmt_BindColumn(stmt, 6, SQLDT_CHAR, &npcflag, 0, NULL, NULL)
		|| SQL_ERROR == SqlStmt_BindColumn(stmt, 7, SQLDT_INT, &customLook, 0, NULL, NULL)
		|| SQL_ERROR == SqlStmt_BindColumn(stmt, 8, SQLDT_UCHAR, &customLookColor, 0, NULL, NULL)
		|| SQL_ERROR == SqlStmt_BindColumn(stmt, 9, SQLDT_UINT, &customIcon, 0, NULL, NULL)
		|| SQL_ERROR == SqlStmt_BindColumn(stmt, 10, SQLDT_UINT, &customIconColor, 0, NULL, NULL)
		|| SQL_ERROR == SqlStmt_BindColumn(stmt, 11, SQLDT_UINT, &protected, 0, NULL, NULL)
		|| SQL_ERROR == SqlStmt_BindColumn(stmt, 12, SQLDT_UINT, &durability, 0, NULL, NULL)) {
		SqlStmt_ShowDebug(stmt);
		lua_pushboolean(state, 0);
		SqlStmt_Free(stmt);
		return 1;
	}

	if (SQL_SUCCESS == SqlStmt_NextRow(stmt)) {
		CALLOC(item, struct parcel, 1);
		item->data.id = id;
		item->data.amount = amount;
		item->data.owner = owner;
		item->data.customLook = customLook;
		item->data.customLookColor = customLookColor;
		item->data.customIcon = customIcon;
		item->data.customIconColor = customIconColor;
		item->data.protected = protected;

		if (durability == 0) item->data.dura = itemdb_dura(id);
		else item->data.dura = durability;

		memcpy(item->data.real_name, real_name, sizeof(real_name));
		if (npcflag == 0) {
			item->sender = sender;
		}
		else {
			item->sender = sender + NPC_START_NUM - 2;
		}
		item->pos = pos;
		item->npcflag = npcflag;
	}
	else {
		clif_sendminitext(sd, "You have no pending parcels.");
		lua_pushboolean(state, 0);
		SqlStmt_Free(stmt);
		return 1;
	}

	SqlStmt_Free(stmt);

	parcell_pushinst(state, item);
	return 1;
}

int pcl_getparcellist(lua_State* state, USER* sd) {
	struct parcel* item;
	unsigned int id, amount, owner, sender, customLookColor, customIconColor, customIcon, customLook;
	int pos;
	unsigned int protected = 0;
	unsigned int durability = 0;
	char real_name[64];
	char npcflag;
	int i = 0;
	int x = 0;
	SqlStmt* stmt = SqlStmt_Malloc(sql_handle);

	if (stmt == NULL) {
		SqlStmt_ShowDebug(stmt);
		lua_pushboolean(state, 0);
		SqlStmt_Free(stmt);
		return 1;
	}

	if (SQL_ERROR == SqlStmt_Prepare(stmt, "SELECT `ParItmId`, `ParAmount`, `ParChaIdOwner`, `ParEngrave`, `ParSender`, `ParPosition`, `ParNpc`, `ParCustomLook`, `ParCustomLookColor`, `ParCustomIcon`, `ParCustomIconColor`, `ParProtected`, `ParItmDura` FROM `Parcels` WHERE `ParChaIdDestination` = '%u'", sd->status.id)
		|| SQL_ERROR == SqlStmt_Execute(stmt)
		|| SQL_ERROR == SqlStmt_BindColumn(stmt, 0, SQLDT_UINT, &id, 0, NULL, NULL)
		|| SQL_ERROR == SqlStmt_BindColumn(stmt, 1, SQLDT_UINT, &amount, 0, NULL, NULL)
		|| SQL_ERROR == SqlStmt_BindColumn(stmt, 2, SQLDT_UINT, &owner, 0, NULL, NULL)
		|| SQL_ERROR == SqlStmt_BindColumn(stmt, 3, SQLDT_STRING, &real_name, sizeof(real_name), NULL, NULL)
		|| SQL_ERROR == SqlStmt_BindColumn(stmt, 4, SQLDT_UINT, &sender, 0, NULL, NULL)
		|| SQL_ERROR == SqlStmt_BindColumn(stmt, 5, SQLDT_INT, &pos, 0, NULL, NULL)
		|| SQL_ERROR == SqlStmt_BindColumn(stmt, 6, SQLDT_CHAR, &npcflag, 0, NULL, NULL)
		|| SQL_ERROR == SqlStmt_BindColumn(stmt, 7, SQLDT_UINT, &customLook, 0, NULL, NULL)
		|| SQL_ERROR == SqlStmt_BindColumn(stmt, 8, SQLDT_UINT, &customLookColor, 0, NULL, NULL)
		|| SQL_ERROR == SqlStmt_BindColumn(stmt, 9, SQLDT_UINT, &customIcon, 0, NULL, NULL)
		|| SQL_ERROR == SqlStmt_BindColumn(stmt, 10, SQLDT_UINT, &customIconColor, 0, NULL, NULL)
		|| SQL_ERROR == SqlStmt_BindColumn(stmt, 11, SQLDT_UINT, &protected, 0, NULL, NULL)
		|| SQL_ERROR == SqlStmt_BindColumn(stmt, 12, SQLDT_UINT, &durability, 0, NULL, NULL)) {
		SqlStmt_ShowDebug(stmt);
		SqlStmt_Free(stmt);
		lua_pushboolean(state, 0);
		return 1;
	}

	lua_newtable(state);
	for (x = 0, i = 1; x < floor(sd->status.maxslots / 4) && SQL_SUCCESS == SqlStmt_NextRow(stmt); x++) {
		CALLOC(item, struct parcel, 1);
		item->data.id = id;
		item->data.amount = amount;
		item->data.owner = owner;
		item->data.customLook = customLook;
		item->data.customLookColor = customLookColor;
		item->data.customIcon = customIcon;
		item->data.customIconColor = customIconColor;
		item->data.protected = protected;

		if (durability == 0) item->data.dura = itemdb_dura(id);
		else item->data.dura = durability;

		memcpy(item->data.real_name, real_name, sizeof(real_name));
		if (npcflag == 0) {
			item->sender = sender;
		}
		else {
			item->sender = sender + NPC_START_NUM - 2;
		}
		item->pos = pos;
		item->npcflag = npcflag;

		parcell_pushinst(state, item);
		lua_rawseti(state, -2, i);
		i++;
	}

	SqlStmt_Free(stmt);
	//lua_pushboolean(state, 1);
	return 1;
}

int pcl_removeparcel(lua_State* state, USER* sd) {
	sl_checkargs(state, "nnnn");
	int sender = lua_tonumber(state, sl_memberarg(1));
	unsigned int item = lua_tonumber(state, sl_memberarg(2));
	unsigned int amount = lua_tonumber(state, sl_memberarg(3));
	int pos = lua_tonumber(state, sl_memberarg(4));
	int owner = lua_tonumber(state, sl_memberarg(5));
	char* engrave = lua_tostring(state, sl_memberarg(6));
	char npcflag = lua_tonumber(state, sl_memberarg(7));
	unsigned int customLook = lua_tonumber(state, sl_memberarg(8));
	unsigned int customLookColor = lua_tonumber(state, sl_memberarg(9));
	unsigned int customIcon = lua_tonumber(state, sl_memberarg(10));
	unsigned int customIconColor = lua_tonumber(state, sl_memberarg(11));

	sender -= NPC_START_NUM + 1;

	if (SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `Parcels` WHERE `ParChaIdDestination` = '%u' AND `ParPosition` = '%d'", sd->status.id, pos)) {
		SqlStmt_ShowDebug(sql_handle);
		Sql_FreeResult(sql_handle);
		lua_pushboolean(state, 0);
		return 1;
	}

	Sql_FreeResult(sql_handle);
	lua_pushboolean(state, 1);
	return 1;
}

/*------------------------------------------------------------------------------
 * Item Expiration/Logging
 *----------------------------------------------------------------------------*/

int pcl_expireitem(lua_State* state, USER* sd) {
	int x, eqdel;
	unsigned int t = time(NULL);
	char msg[255];

	for (x = 0; x < sd->status.maxinv; x++) {
		if ((sd->status.inventory[x].time > 0 && sd->status.inventory[x].time < t) || (itemdb_time(sd->status.inventory[x].id) > 0 && itemdb_time(sd->status.inventory[x].id) < t)) {
			sprintf(msg, "Your %s has expired! Please visit the cash shop to purchase another.", itemdb_name(sd->status.inventory[x].id));
			pc_delitem(sd, x, 1, 8);
			clif_sendminitext(sd, msg);
		}
	}

	for (x = 0; x < sd->status.maxinv; x++) {
		if (!sd->status.inventory[x].id) {
			eqdel = x;
			break;
		}
	}

	for (x = 0; x < 14; x++) {
		if ((sd->status.equip[x].time > 0 && sd->status.equip[x].time < t) || (itemdb_time(sd->status.equip[x].id) > 0 && itemdb_time(sd->status.equip[x].id) < t)) {
			sprintf(msg, "Your %s has expired! Please visit the cash shop to purchase another.", itemdb_name(sd->status.equip[x].id));
			pc_unequip(sd, x);
			pc_delitem(sd, eqdel, 1, 8);
			clif_sendminitext(sd, msg);
		}
	}

	return 0;
}

int pcl_logbuysell(lua_State* state, USER* sd) {
	sl_checkargs(state, "nnnn");
	unsigned int item = lua_tonumber(state, sl_memberarg(1));
	unsigned int amount = lua_tonumber(state, sl_memberarg(2));
	unsigned int money = lua_tonumber(state, sl_memberarg(3));
	char buysell = lua_tonumber(state, sl_memberarg(4));

	Sql_FreeResult(sql_handle);
	lua_pushboolean(state, 1);
	return 1;
}

/*------------------------------------------------------------------------------
 * Time Values
 *----------------------------------------------------------------------------*/

int pcl_settimevalues(lua_State* state, USER* sd) {
	sl_checkargs(state, "n");
	int i;
	unsigned int newval = lua_tonumber(state, sl_memberarg(1));

	for (i = (sizeof(sd->timevalues) / sizeof(sd->timevalues[0])) - 1; i >= 0; i--) {
		if (i > 0) {
			sd->timevalues[i] = sd->timevalues[i - 1];
		}
		else {
			sd->timevalues[i] = newval;
		}
	}
	return 0;
}

int pcl_gettimevalues(lua_State* state, USER* sd) {
	int i, x;

	lua_newtable(state);
	for (x = 0, i = 1; x < (sizeof(sd->timevalues) / sizeof(sd->timevalues[0])); x++) {
		lua_pushnumber(state, sd->timevalues[x]);
		lua_rawseti(state, -2, i);
		i++;
	}
	return 1;
}

/*------------------------------------------------------------------------------
 * Look At / View
 *----------------------------------------------------------------------------*/

int pcl_lookat(lua_State* state, USER* sd) {
	sl_checkargs(state, "n");
	struct block_list* bl = map_id2bl(lua_tonumber(state, sl_memberarg(1)));

	clif_parselookat_scriptsub(sd, bl);
	return 0;
}

int pcl_changeview(lua_State* state, USER* sd) {
	sl_checkargs(state, "nn");
	int x = lua_tonumber(state, sl_memberarg(1));
	int y = lua_tonumber(state, sl_memberarg(2));

	clif_sendxychange(sd, x, y);
	clif_mob_look_start(sd);
	map_foreachinarea(clif_object_look_sub, sd->bl.m, sd->bl.x, sd->bl.y, AREA, BL_ALL, LOOK_GET, sd);
	clif_mob_look_close(sd);
	clif_destroyold(sd);
	map_foreachinarea(clif_charlook_sub, sd->bl.m, sd->bl.x, sd->bl.y, AREA, BL_PC, LOOK_SEND, sd);
	map_foreachinarea(clif_charlook_sub, sd->bl.m, sd->bl.x, sd->bl.y, AREA, BL_PC, LOOK_GET, sd);
	map_foreachinarea(clif_cnpclook_sub, sd->bl.m, sd->bl.x, sd->bl.y, AREA, BL_NPC, LOOK_GET, sd);
	map_foreachinarea(clif_cmoblook_sub, sd->bl.m, sd->bl.x, sd->bl.y, AREA, BL_MOB, LOOK_GET, sd);
	return 1;
}

/*------------------------------------------------------------------------------
 * Unknown/All Spells Queries
 *----------------------------------------------------------------------------*/

int pcl_getunknownspells(lua_State* state, USER* sd) {
	int i, j, x;
	unsigned int id;
	unsigned int idlist[255];
	char found = 0;

	unsigned int class1 = lua_tonumber(state, sl_memberarg(1));
	unsigned int class2 = lua_tonumber(state, sl_memberarg(2));

	SqlStmt* stmt = SqlStmt_Malloc(sql_handle);
	lua_newtable(state);

	memset(idlist, 0, sizeof(int) * 255);
	if (stmt == NULL) {
		SqlStmt_ShowDebug(stmt);
		SqlStmt_Free(stmt);
		lua_pushboolean(state, 0);
		return 1;
	}

	if (SQL_ERROR == SqlStmt_Prepare(stmt, "SELECT `SplId` FROM `Spells` WHERE `SplPthId` IN ('0', '%d', '%d') AND `SplMark` <= '%d' AND `SplActive` = '1' AND (`SplAlignment` = '%d' OR `SplAlignment` = '-1')", class1, class2, sd->status.mark, sd->status.alignment)
		|| SQL_ERROR == SqlStmt_Execute(stmt)
		|| SQL_ERROR == SqlStmt_BindColumn(stmt, 0, SQLDT_UINT, &id, 0, NULL, NULL)) {
		SqlStmt_ShowDebug(stmt);
		SqlStmt_Free(stmt);
		lua_pushboolean(state, 0);
		return 1;
	}

	for (i = 0; i < SqlStmt_NumRows(stmt) && SQL_SUCCESS == SqlStmt_NextRow(stmt); i++) {
		switch (id) {
		case 0:
		case 100:
		case 200:
		case 300:
		case 400:
		case 500:
		case 1000:
		case 1500:
		case 2000:
		case 2500:
		case 3000:
		case 3500:
		case 4000:
		case 4500:
		case 5000:
		case 5500:
		case 7000:
		case 10000:
			continue;
		default:
			for (j = 0; j < MAX_SPELLS; j++) {
				if (sd->status.skill[j] == id) {
					found = 1;
					break;
				}
			}

			if (found == 0) {
				for (x = 0; x < 255; x++) {
					if (idlist[x] == 0) {
						idlist[x] = id;
						break;
					}
				}
			}
			else {
				found = 0;
				continue;
			}

			break;
		}
	}

	SqlStmt_Free(stmt);

	for (i = 0, x = 1; i < 255; i++, x += 2) {
		if (idlist[i] > 0) {
			lua_pushstring(state, magicdb_name(idlist[i]));
			lua_rawseti(state, -2, x);
			lua_pushstring(state, magicdb_yname(idlist[i]));
			lua_rawseti(state, -2, x + 1);
		}
		else {
			break;
		}
	}

	return 1;
}

int pcl_getallspells(lua_State* state, USER* sd) {
	int i, j, x;
	unsigned int id;
	unsigned int idlist[255];
	char found = 0;
	SqlStmt* stmt = SqlStmt_Malloc(sql_handle);
	lua_newtable(state);

	memset(idlist, 0, sizeof(int) * 255);
	if (stmt == NULL) {
		SqlStmt_ShowDebug(stmt);
		SqlStmt_Free(stmt);
		lua_pushboolean(state, 0);
		return 1;
	}

	if (SQL_ERROR == SqlStmt_Prepare(stmt, "SELECT `SplId` FROM `Spells` WHERE `SplActive` = '1'")
		|| SQL_ERROR == SqlStmt_Execute(stmt)
		|| SQL_ERROR == SqlStmt_BindColumn(stmt, 0, SQLDT_UINT, &id, 0, NULL, NULL)) {
		SqlStmt_ShowDebug(stmt);
		SqlStmt_Free(stmt);
		lua_pushboolean(state, 0);
		return 1;
	}

	for (i = 0; i < SqlStmt_NumRows(stmt) && SQL_SUCCESS == SqlStmt_NextRow(stmt); i++) {
		switch (id) {
		case 0:
		case 100:
		case 200:
		case 300:
		case 400:
		case 500:
		case 1000:
		case 1500:
		case 2000:
		case 2500:
		case 3000:
		case 3500:
		case 4000:
		case 4500:
		case 5000:
		case 5500:
		case 7000:
		case 10000:
			continue;
		default:
			if (found == 0) {
				for (x = 0; x < 255; x++) {
					if (idlist[x] == 0) {
						idlist[x] = id;
						break;
					}
				}
			}
			else {
				found = 0;
				continue;
			}

			break;
		}
	}

	SqlStmt_Free(stmt);

	for (i = 0, x = 1; i < 255; i++, x += 2) {
		if (idlist[i] > 0) {
			lua_pushstring(state, magicdb_name(idlist[i]));
			lua_rawseti(state, -2, x);
			lua_pushstring(state, magicdb_yname(idlist[i]));
			lua_rawseti(state, -2, x + 1);
		}
		else {
			break;
		}
	}

	return 1;
}

int pcl_getallclassspells(lua_State* state, USER* sd) {
	int i, j, x;
	unsigned int id;
	unsigned int idlist[255];

	unsigned int class = lua_tonumber(state, sl_memberarg(1));

	char found = 0;
	SqlStmt* stmt = SqlStmt_Malloc(sql_handle);
	lua_newtable(state);

	memset(idlist, 0, sizeof(int) * 255);
	if (stmt == NULL) {
		SqlStmt_ShowDebug(stmt);
		SqlStmt_Free(stmt);
		lua_pushboolean(state, 0);
		return 1;
	}

	if (SQL_ERROR == SqlStmt_Prepare(stmt, "SELECT `SplId` FROM `Spells` WHERE `SplActive` = '1' AND `SplPthId` = '%u'", class)
		|| SQL_ERROR == SqlStmt_Execute(stmt)
		|| SQL_ERROR == SqlStmt_BindColumn(stmt, 0, SQLDT_UINT, &id, 0, NULL, NULL)) {
		SqlStmt_ShowDebug(stmt);
		SqlStmt_Free(stmt);
		lua_pushboolean(state, 0);
		return 1;
	}

	for (i = 0; i < SqlStmt_NumRows(stmt) && SQL_SUCCESS == SqlStmt_NextRow(stmt); i++) {
		switch (id) {
		case 0:
		case 100:
		case 200:
		case 300:
		case 400:
		case 500:
		case 1000:
		case 1500:
		case 2000:
		case 2500:
		case 3000:
		case 3500:
		case 4000:
		case 4500:
		case 5000:
		case 5500:
		case 7000:
		case 10000:
			continue;
		default:
			if (found == 0) {
				for (x = 0; x < 255; x++) {
					if (idlist[x] == 0) {
						idlist[x] = id;
						break;
					}
				}
			}
			else {
				found = 0;
				continue;
			}

			break;
		}
	}

	SqlStmt_Free(stmt);

	for (i = 0, x = 1; i < 255; i++, x += 2) {
		if (idlist[i] > 0) {
			lua_pushstring(state, magicdb_name(idlist[i]));
			lua_rawseti(state, -2, x);
			lua_pushstring(state, magicdb_yname(idlist[i]));
			lua_rawseti(state, -2, x + 1);
		}
		else {
			break;
		}
	}

	return 1;
}

/*------------------------------------------------------------------------------
 * Status/Debug
 *----------------------------------------------------------------------------*/

int pcl_status(lua_State* state, USER* sd) {
	clif_mystaytus(sd);
	return 1;
}

int pcl_testpacket(lua_State* state, USER* sd) {
	sl_checkargs(state, "t");
	int i = 0;
	int index = 0;

	int len = 0;

	lua_pushnil(state);

	if (!session[sd->fd])
	{
		session[sd->fd]->eof = 8;
		return 0;
	}

	while (lua_next(state, sl_memberarg(1)) != 0) {
		int packetlen = 0;
		// key is at -2, value at -1
		i = lua_tonumber(state, -2);

		if (i == 2) {
			WFIFOHEAD(sd->fd, lua_tonumber(state, -1) + 3);
			len = lua_tonumber(state, -1);
			index += 1;
			lua_pop(state, 1);
			continue;
		}

		if (lua_isnumber(state, -1)) {
			if (lua_tonumber(state, -1) <= 255) {
				WFIFOB(sd->fd, i - 1 + index) = lua_tonumber(state, -1);
			}
			else if (lua_tonumber(state, -1) > 255 && lua_tonumber(state, -1) <= 65535) {
				WFIFOW(sd->fd, i - 1 + index) = SWAP16((int)lua_tonumber(state, -1));
				index += 1;
			}
			else {
				WFIFOL(sd->fd, i - 1 + index) = SWAP32((int)lua_tonumber(state, -1));
				index += 3;
			}
		}
		else {
			strcpy(WFIFOP(sd->fd, i - 1 + index), lua_tostring(state, -1));
			index += strlen(lua_tostring(state, -1)) - 1;
		}

		lua_pop(state, 1);
	}

	WFIFOW(sd->fd, 1) = SWAP16(len);

	printf("test packet\n");
	for (int i = 0; i < len; i++) {
		printf("%02X ", WFIFOB(sd->fd, i));
	}
	printf("\n");
	printf("end test packet\n");

	WFIFOSET(sd->fd, encrypt(sd->fd));
	return 1;
}

int pcl_getcasterid(lua_State* state, USER* sd) {
	sl_checkargs(state, "s");
	int spell = magicdb_id(lua_tostring(state, sl_memberarg(1)));
	int i, x;
	lua_newtable(state);

	for (i = 0, x = 1; i < MAX_MAGIC_TIMERS; i++) {
		if (sd->status.dura_aether[i].id == spell) {
			lua_pushnumber(state, sd->status.dura_aether[i].caster_id);
			lua_rawseti(state, -2, x);
			x++;
		}
	}

	return 1;
}

/*------------------------------------------------------------------------------
 * Timer Functions
 *----------------------------------------------------------------------------*/

int pcl_settimer(lua_State* state, USER* sd) {
	sl_checkargs(state, "nn");
	char type = lua_tonumber(state, sl_memberarg(1));
	unsigned int length = lua_tonumber(state, sl_memberarg(2));

	if (type == 1 || type == 2) {
		if ((long)length > 4294967295) {
			sd->disptimertick = 4294967295;
		}
		else if ((long)length < 0) {
			lua_pushboolean(state, 0);
			return 0;
		}
		else {
			sd->disptimertick = length;
		}
	}
	else if ((type != 1 && type != 2) || sd->disptimer > 0) {
		lua_pushboolean(state, 0);
		return 0;
	}

	sd->disptimertype = type;
	sd->disptimer = timer_insert(1000, 1000, pc_disptimertick, sd->bl.id, 0);
	clif_send_timer(sd, type, length);
	lua_pushboolean(state, 1);
	return 0;
}

int pcl_addtime(lua_State* state, USER* sd) {
	sl_checkargs(state, "n");
	unsigned int addition = lua_tonumber(state, sl_memberarg(1));

	if (sd->disptimertype == 2) {
		if ((long)(sd->disptimertick + addition) > 4294967295) {
			sd->disptimertick = 4294967295;
		}
		else {
			sd->disptimertick += addition;
		}

		clif_send_timer(sd, sd->disptimertype, sd->disptimertick);
	}
	else {
		lua_pushboolean(state, 0);
		return 0;
	}

	lua_pushboolean(state, 1);
	return 0;
}

int pcl_removetime(lua_State* state, USER* sd) {
	sl_checkargs(state, "n");
	unsigned int addition = lua_tonumber(state, sl_memberarg(1));

	if (sd->disptimertype == 2) {
		if ((long)(sd->disptimertick - addition) < 0) {
			sd->disptimertick = 0;
		}
		else {
			sd->disptimertick -= addition;
		}

		clif_send_timer(sd, sd->disptimertype, sd->disptimertick);
	}
	else {
		lua_pushboolean(state, 0);
		return 0;
	}

	lua_pushboolean(state, 1);
	return 0;
}

int pcl_checklevel(lua_State* state, USER* sd) {
	pc_checklevel(sd);
	return 0;
}

/*------------------------------------------------------------------------------
 * Dialog System
 *----------------------------------------------------------------------------*/

int pcl_dialog(lua_State* state, USER* sd) {
	sl_checkargs(state, "st");
	int previous = 0, next = 0;

	if (!sd) {
		return 0;
	}
	else {
		USER* tsd = map_id2sd(sd->status.id);
		if (!tsd) return 0;
	}

	lua_pushnil(state);

	while (lua_next(state, sl_memberarg(2)) != 0) {
		if (!strcmp(lua_tostring(state, -1), "previous")) previous = 1;
		else if (!strcmp(lua_tostring(state, -1), "next")) next = 1;
		lua_pop(state, 1);
	}

	char* msg = lua_tostring(state, sl_memberarg(1));

	clif_scriptmes(sd, sd->last_click, msg, previous, next);
	return lua_yield(state, 0);
}

/*------------------------------------------------------------------------------
 * Buy/Sell System
 *----------------------------------------------------------------------------*/

int pcl_buy(lua_State* state, USER* sd) {
	sl_checkargs(state, "stt");
	int x;
	unsigned int amount = lua_objlen(state, sl_memberarg(2));

	struct item* item;
	CALLOC(item, struct item, amount + 1);

	int var2[amount + 1];
	char* dialog;

	lua_pushnil(state);
	while (lua_next(state, sl_memberarg(2)) != 0) {
		int index = lua_tonumber(state, -2);
		item[index - 1].id = itemdb_id(lua_tostring(state, -1));
		lua_pop(state, 1);
	}

	lua_pushnil(state);
	while (lua_next(state, sl_memberarg(3)) != 0) {
		int index = lua_tonumber(state, -2);
		var2[index - 1] = lua_tonumber(state, -1);
		lua_pop(state, 1);
	}

	lua_pushnil(state);
	while (lua_next(state, sl_memberarg(4)) != 0) {
		int index = lua_tonumber(state, -2);
		strcpy(item[index - 1].real_name, lua_tostring(state, -1));
		lua_pop(state, 1);
	}

	lua_pushnil(state);
	while (lua_next(state, sl_memberarg(5)) != 0) {
		int index = lua_tonumber(state, -2);
		item[index - 1].owner = lua_tonumber(state, -1);
		lua_pop(state, 1);
	}

	lua_pushnil(state);
	while (lua_next(state, sl_memberarg(6)) != 0) {
		int index = lua_tonumber(state, -2);
		strcpy(item[index - 1].buytext, lua_tostring(state, -1));
		lua_pop(state, 1);
	}

	lua_pushnil(state);
	while (lua_next(state, sl_memberarg(7)) != 0) {
		int index = lua_tonumber(state, -2);
		item[index - 1].customIcon = lua_tonumber(state, -1);
		lua_pop(state, 1);
	}

	lua_pushnil(state);
	while (lua_next(state, sl_memberarg(8)) != 0) {
		int index = lua_tonumber(state, -2);
		item[index - 1].customIconColor = lua_tonumber(state, -1);
		lua_pop(state, 1);
	}

	dialog = lua_tostring(state, sl_memberarg(1));

	clif_buydialog(sd, sd->last_click, dialog, item, var2, amount);

	return lua_yield(state, 0);
}

int pcl_sell(lua_State* state, USER* sd) {
	sl_checkargs(state, "st");
	char* dialog = lua_tostring(state, sl_memberarg(1));
	unsigned int amount = lua_objlen(state, sl_memberarg(2));
	int item[60];
	int taken[26];
	int count = 0;
	int lastcount = 0;
	unsigned int temp;
	int x, y;
	lua_pushnil(state);
	while (lua_next(state, sl_memberarg(2)) != 0) {
		int index = lua_tonumber(state, -2);

		if (lua_isnumber(state, -1)) temp = lua_tonumber(state, -1);
		else if (lua_isstring(state, -1)) temp = itemdb_id(lua_tostring(state, -1));

		for (x = 0; x < sd->status.maxinv; x++) {
			lastcount = 0;
			if (sd->status.inventory[x].id == temp) {
				item[count] = x;
				count++;
			}
		}
		lua_pop(state, 1);
	}
	clif_selldialog(sd, sd->last_click, dialog, item, count);
	return lua_yield(state, 0);
}

/*------------------------------------------------------------------------------
 * Map Selection
 *----------------------------------------------------------------------------*/

int pcl_mapselection(lua_State* state, USER* sd) {
	sl_checkargs(state, "stttttt");
	int key = 0;
	char* wm = NULL;
	int map_x0[255];
	int map_y0[255];
	char* map_name[255];
	unsigned int map_id[255];
	int map_x1[255];
	int map_y1[255];

	wm = lua_tostring(state, sl_memberarg(1));

	int index = lua_objlen(state, sl_memberarg(2));

	lua_pushnil(state);
	while (lua_next(state, sl_memberarg(2)) != 0) {
		int index = lua_tonumber(state, -2);
		map_x0[index - 1] = lua_tonumber(state, -1);
		lua_pop(state, 1);
	}

	lua_pushnil(state);
	while (lua_next(state, sl_memberarg(3)) != 0) {
		int index = lua_tonumber(state, -2);
		map_y0[index - 1] = lua_tonumber(state, -1);
		lua_pop(state, 1);
	}

	lua_pushnil(state);
	while (lua_next(state, sl_memberarg(4)) != 0) {
		int index = lua_tonumber(state, -2);
		map_name[index - 1] = lua_tostring(state, -1);
		lua_pop(state, 1);
	}

	lua_pushnil(state);
	while (lua_next(state, sl_memberarg(5)) != 0) {
		unsigned int index = lua_tonumber(state, -2);
		map_id[index - 1] = lua_tonumber(state, -1);
		lua_pop(state, 1);
	}

	lua_pushnil(state);
	while (lua_next(state, sl_memberarg(6)) != 0) {
		int index = lua_tonumber(state, -2);
		map_x1[index - 1] = lua_tonumber(state, -1);
		lua_pop(state, 1);
	}

	lua_pushnil(state);
	while (lua_next(state, sl_memberarg(7)) != 0) {
		int index = lua_tonumber(state, -2);
		map_y1[index - 1] = lua_tonumber(state, -1);
		lua_pop(state, 1);
	}

	clif_mapselect(sd, wm, map_x0, map_y0, map_name, map_id, map_x1, map_y1, index);

	return 0;
}

/*------------------------------------------------------------------------------
 * Remove Inventory Item
 *----------------------------------------------------------------------------*/

int pcl_removeinventoryitem(lua_State* state, USER* sd) {
	unsigned int id = 0, amount = 0, owner = 0, max = 0;
	int x, count = 0, type = 0;
	char* engrave = NULL;

	if (lua_isnumber(state, sl_memberarg(1)))
	{
		sl_checkargs(state, "nn");
		id = lua_tonumber(state, sl_memberarg(1));
	}
	else
	{
		sl_checkargs(state, "sn");
		id = itemdb_id(lua_tostring(state, sl_memberarg(1)));
	}

	amount = lua_tonumber(state, sl_memberarg(2));
	type = lua_tonumber(state, sl_memberarg(3));
	owner = lua_tonumber(state, sl_memberarg(4));
	engrave = lua_tostring(state, sl_memberarg(5));
	max = amount;

	if (!engrave)
	{
		CALLOC(engrave, char, 64);
	}

	if (owner == 0) {
		for (x = 0; x < sd->status.maxinv; x++)
		{
			if (sd->status.inventory[x].id == id && sd->status.inventory[x].amount < itemdb_stackamount(id) && !(strcmpi(sd->status.inventory[x].real_name, engrave)))
			{
				if (sd->status.inventory[x].amount < amount && sd->status.inventory[x].amount > 0)
				{
					amount -= sd->status.inventory[x].amount;
					pc_delitem(sd, x, sd->status.inventory[x].amount, type);

					if (amount == 0)
					{
						lua_pushboolean(state, 1);
						return 1;
					}
				}
				else if (sd->status.inventory[x].amount >= amount)
				{
					pc_delitem(sd, x, amount, type);
					lua_pushboolean(state, 1);
					return 1;
				}
			}
		}

		for (x = 0; x < sd->status.maxinv; x++)
		{
			if (sd->status.inventory[x].id == id && !(strcmpi(sd->status.inventory[x].real_name, engrave)))
			{
				if (sd->status.inventory[x].amount < amount && sd->status.inventory[x].amount > 0)
				{
					amount -= sd->status.inventory[x].amount;
					pc_delitem(sd, x, sd->status.inventory[x].amount, type);

					if (amount == 0)
					{
						lua_pushboolean(state, 1);
						return 1;
					}
				}
				else if (sd->status.inventory[x].amount >= amount)
				{
					pc_delitem(sd, x, amount, type);
					lua_pushboolean(state, 1);
					return 1;
				}
			}
		}
	}
	else if (owner > 0) {
		for (x = 0; x < sd->status.maxinv; x++)
		{
			if (sd->status.inventory[x].id == id && sd->status.inventory[x].amount < itemdb_stackamount(id) && sd->status.inventory[x].owner == owner && !(strcmpi(sd->status.inventory[x].real_name, engrave)))
			{
				if (sd->status.inventory[x].amount < amount && sd->status.inventory[x].amount > 0)
				{
					amount -= sd->status.inventory[x].amount;
					pc_delitem(sd, x, sd->status.inventory[x].amount, type);

					if (amount == 0)
					{
						lua_pushboolean(state, 1);
						return 1;
					}
				}
				else if (sd->status.inventory[x].amount >= amount)
				{
					pc_delitem(sd, x, amount, type);
					lua_pushboolean(state, 1);
					return 1;
				}
			}
		}

		for (x = 0; x < sd->status.maxinv; x++)
		{
			if (sd->status.inventory[x].id == id && sd->status.inventory[x].owner == owner && !(strcmpi(sd->status.inventory[x].real_name, engrave)))
			{
				if (sd->status.inventory[x].amount < amount && sd->status.inventory[x].amount > 0)
				{
					amount -= sd->status.inventory[x].amount;
					pc_delitem(sd, x, sd->status.inventory[x].amount, type);

					if (amount == 0)
					{
						lua_pushboolean(state, 1);
						return 1;
					}
				}
				else if (sd->status.inventory[x].amount >= amount)
				{
					pc_delitem(sd, x, amount, type);
					lua_pushboolean(state, 1);
					return 1;
				}
			}
		}
	}

	if (engrave)
	{
		engrave = NULL;
	}
	else
	{
		FREE(engrave);
	}

	lua_pushboolean(state, 0);
	return 1;
}

/*------------------------------------------------------------------------------
 * Gift System (commented out in original)
 *----------------------------------------------------------------------------*/

int pcl_addGift(lua_State* state, USER* sd) {
	/* Gift system commented out in original sl.c */
	return 0;
}

int pcl_retrieveGift(lua_State* state, USER* sd) {
	/* Gift system commented out in original sl.c */
	return 1;
}

/*------------------------------------------------------------------------------
 * Add Item
 *----------------------------------------------------------------------------*/

int pcl_additem(lua_State* state, USER* sd) {
	unsigned int id = 0;
	unsigned int amount = 0;
	int dura = 0;
	int owner = 0;
	struct item* fl;
	char* engrave = NULL;
	char* note = NULL;

	unsigned int customLook = 0;
	unsigned int customIcon = 0;
	unsigned int customLookColor = 0;
	unsigned int customIconColor = 0;
	unsigned int protected = 0;
	unsigned int time = 0;
	unsigned int custom = 0;

	CALLOC(fl, struct item, 1);
	if (lua_isnumber(state, sl_memberarg(1))) {
		sl_checkargs(state, "nn");
		id = lua_tonumber(state, sl_memberarg(1));
	}
	else {
		sl_checkargs(state, "sn");
		id = itemdb_id(lua_tostring(state, sl_memberarg(1)));
	}

	amount = lua_tonumber(state, sl_memberarg(2));
	dura = lua_tonumber(state, sl_memberarg(3));
	owner = lua_tonumber(state, sl_memberarg(4));
	time = lua_tonumber(state, sl_memberarg(5));

	engrave = lua_tostring(state, sl_memberarg(6));

	customLook = lua_tonumber(state, sl_memberarg(7));
	customLookColor = lua_tonumber(state, sl_memberarg(8));
	customIcon = lua_tonumber(state, sl_memberarg(9));
	customIconColor = lua_tonumber(state, sl_memberarg(10));
	protected = lua_tonumber(state, sl_memberarg(11));
	note = lua_tostring(state, sl_memberarg(12));
	custom = lua_tonumber(state, sl_memberarg(13));

	fl->id = id;
	fl->amount = amount;

	if (owner) {
		fl->owner = owner;
	}
	else {
		fl->owner = 0;
	}

	if (time) {
		fl->time = time;
	}
	else {
		fl->time = 0;
	}

	if (engrave) {
		strcpy(fl->real_name, engrave);
	}
	else {
		strcpy(fl->real_name, "");
	}

	if (note) {
		strcpy(fl->note, note);
	}
	else {
		strcpy(fl->note, "");
	}

	if (dura) {
		fl->dura = dura;
	}
	else {
		fl->dura = itemdb_dura(id);
	}

	if (protected) {
		fl->protected = protected;
	}
	else {
		fl->protected = itemdb_protected(id);
	}

	fl->customLook = customLook;
	fl->customLookColor = customLookColor;
	fl->customIcon = customIcon;
	fl->customIconColor = customIconColor;
	fl->custom = custom;

	if (!pc_additem(sd, fl)) {
		lua_pushboolean(state, 1);
	}
	else {
		lua_pushboolean(state, 0);
	}
	FREE(fl);
	return 1;
}

/*------------------------------------------------------------------------------
 * Event XP System
 *----------------------------------------------------------------------------*/

int pcl_addEventXP(lua_State* state, USER* sd) {
	char date_string[100] = "";
	char time_string[100] = "";
	time_t t;
	t = time(NULL);

	strftime(date_string, sizeof(date_string), "%Y%m%d", localtime(&t));
	strftime(time_string, sizeof(time_string), "%H%M%S", localtime(&t));

	int date = atoi(date_string);
	int curtime = atoi(time_string);
	int todate = 0;
	int totime = 0;

	int eventid = -1;
	char eventname[40] = "";
	unsigned int score = lua_tonumber(state, sl_memberarg(2));
	unsigned char sql_query[500] = "";

	if (lua_isnumber(state, sl_memberarg(1))) {
		sl_checkargs(state, "nn");
		eventid = lua_tonumber(state, sl_memberarg(1));
	}
	else {
		sl_checkargs(state, "sn");
		strcpy(eventname, lua_tostring(state, sl_memberarg(1)));
	}

	SqlStmt* stmt = SqlStmt_Malloc(sql_handle);

	if (stmt == NULL)
	{
		SqlStmt_ShowDebug(stmt);
		return 0;
	}

	if (strcmp(eventname, "") != 0)
	{
		if (SQL_ERROR == SqlStmt_Prepare(stmt, "SELECT `EventId` FROM `RankingEvents` WHERE `EventName` = '%s'", eventname)
			|| SQL_ERROR == SqlStmt_Execute(stmt)
			|| SQL_ERROR == SqlStmt_BindColumn(stmt, 0, SQLDT_INT, &eventid, 0, NULL, NULL))
		{
			SqlStmt_ShowDebug(stmt);
			SqlStmt_Free(stmt);
		}

		if (SQL_SUCCESS != SqlStmt_NextRow(stmt))
		{
			SqlStmt_Free(stmt);
		}
	}

	if (eventid != -1) {
		if (SQL_ERROR == SqlStmt_Prepare(stmt, "SELECT `ToDate`, `ToTime` FROM `RankingEvents` WHERE `EventId` = '%i'", eventid)
			|| SQL_ERROR == SqlStmt_Execute(stmt)
			|| SQL_ERROR == SqlStmt_BindColumn(stmt, 0, SQLDT_INT, &todate, 0, NULL, NULL)
			|| SQL_ERROR == SqlStmt_BindColumn(stmt, 1, SQLDT_INT, &totime, 0, NULL, NULL))
		{
			SqlStmt_ShowDebug(stmt);
			SqlStmt_Free(stmt);
		}

		if (SQL_SUCCESS != SqlStmt_NextRow(stmt))
		{
			SqlStmt_Free(stmt);
		}

		if (date > todate) return 1;
		if (date == todate) { if (curtime >= totime) return 1; }

		if (SQL_ERROR == SqlStmt_Prepare(stmt, "SELECT * FROM `RankingScores` WHERE `EventId` = '%i' AND `ChaId` = '%i'", eventid, sd->status.id) || SQL_ERROR == SqlStmt_Execute(stmt))
		{
			SqlStmt_ShowDebug(stmt);
			SqlStmt_Free(stmt);
			return 0;
		}

		if (SqlStmt_NumRows(stmt) == 0) {
			if (SQL_ERROR == Sql_Query(sql_handle, "INSERT INTO `RankingScores` (`EventId`, `ChaId`, `ChaName`, `Score`, `EventClaim`) VALUES ('%i', '%i', '%s', '%i', '2')", eventid, sd->status.id, sd->status.name, score))
			{
				Sql_ShowDebug(sql_handle);
				lua_pushboolean(state, 1);
				SqlStmt_Free(stmt);
				return 1;
			}
		}

		else {
			if (SQL_ERROR == Sql_Query(sql_handle, "UPDATE `RankingScores` SET `Score` = `Score` + '%u', `ChaName` = '%s' WHERE `EventId` = '%i' AND `ChaId` = '%d'", score, sd->status.name, eventid, sd->status.id)) {
				SqlStmt_ShowDebug(sql_handle);
				lua_pushboolean(state, 1);
				SqlStmt_Free(stmt);
			}
		}
	}

	return 1;
}

/*------------------------------------------------------------------------------
 * Subpath Banking
 *----------------------------------------------------------------------------*/

int pcl_getsubpathbankitems(lua_State* state, USER* sd) {
	int i, x;

	struct clan_data* clan = NULL;

	unsigned int id = sd->status.clan;
	clan = clandb_search(id);

	lua_newtable(state);

	for (i = 0, x = 1; i < 255; i++, x += 5) {
		if (clan->clanbanks[i].item_id > 0) {
			lua_pushnumber(state, clan->clanbanks[i].item_id);
			lua_rawseti(state, -2, x);
			lua_pushnumber(state, clan->clanbanks[i].amount);
			lua_rawseti(state, -2, x + 1);
			lua_pushnumber(state, clan->clanbanks[i].owner);
			lua_rawseti(state, -2, x + 2);
			lua_pushstring(state, clan->clanbanks[i].real_name);
			lua_rawseti(state, -2, x + 3);
			lua_pushnumber(state, clan->clanbanks[i].time);
			lua_rawseti(state, -2, x + 4);
		}
	}

	return 1;
}

int pcl_subpathbankdeposit(lua_State* state, USER* sd) {
	sl_checkargs(state, "nn");
	int x, deposit, amount, owner;
	unsigned int banktime;

	struct clan_data* clan = NULL;

	unsigned int item;
	char* engrave = NULL;

	unsigned int id = sd->status.clan;
	clan = clandb_search(id);

	deposit = -1;
	item = lua_tonumber(state, sl_memberarg(1));
	amount = lua_tonumber(state, sl_memberarg(2));
	owner = lua_tonumber(state, sl_memberarg(3));
	banktime = lua_tonumber(state, sl_memberarg(4));
	engrave = lua_tostring(state, sl_memberarg(5));

	for (x = 0; x < 255; x++) {
		if (clan->clanbanks[x].item_id == item && clan->clanbanks[x].owner == owner && clan->clanbanks[x].time == banktime && !strcmpi(clan->clanbanks[x].real_name, engrave)) {
			deposit = x;
			break;
		}
	}

	if (deposit != -1) {
		clan->clanbanks[deposit].amount += amount;
		map_saveclanbank(id);
		lua_pushboolean(state, 1);
	}
	else {
		for (x = 0; x < 255; x++) {
			if (clan->clanbanks[x].item_id == 0) {
				clan->clanbanks[x].item_id = item;
				clan->clanbanks[x].amount = amount;
				clan->clanbanks[x].owner = owner;
				clan->clanbanks[x].time = banktime;
				strcpy(clan->clanbanks[x].real_name, engrave);
				map_saveclanbank(id);
				lua_pushboolean(state, 1);
				return 1;
			}
		}
	}

	lua_pushboolean(state, 0);
	return 0;
}

int pcl_subpathbankwithdraw(lua_State* state, USER* sd) {
	sl_checkargs(state, "nn");
	int x, deposit, amount, owner;
	unsigned int banktime;
	unsigned int item;
	char* engrave = NULL;

	unsigned int id = sd->status.clan;
	struct clan_data* clan;

	clan = clandb_search(id);

	deposit = -1;
	item = lua_tonumber(state, sl_memberarg(1));
	amount = lua_tonumber(state, sl_memberarg(2));
	owner = lua_tonumber(state, sl_memberarg(3));
	banktime = lua_tonumber(state, sl_memberarg(4));
	engrave = lua_tostring(state, sl_memberarg(5));

	for (x = 0; x < 255; x++) {
		if (clan->clanbanks[x].item_id == item && clan->clanbanks[x].owner == owner && clan->clanbanks[x].time == banktime && !strcmpi(clan->clanbanks[x].real_name, engrave)) {
			deposit = x;
			break;
		}
	}

	if (deposit != -1) {
		if ((clan->clanbanks[deposit].amount - amount) < 0) {
			lua_pushboolean(state, 0);
			return 1;
		}
		else {
			clan->clanbanks[deposit].amount -= amount;

			if (clan->clanbanks[deposit].amount <= 0) {
				clan->clanbanks[deposit].item_id = 0;
				clan->clanbanks[deposit].amount = 0;
				clan->clanbanks[deposit].owner = 0;
				clan->clanbanks[deposit].time = 0;
				strcpy(clan->clanbanks[deposit].real_name, "");

				for (x = 0; x < 255; x++) {
					if (clan->clanbanks[x].item_id == 0 && x < 255 - 1) {
						clan->clanbanks[x].item_id = clan->clanbanks[x + 1].item_id;
						clan->clanbanks[x + 1].item_id = 0;
						clan->clanbanks[x].amount = clan->clanbanks[x + 1].amount;
						clan->clanbanks[x + 1].amount = 0;
						clan->clanbanks[x].owner = clan->clanbanks[x + 1].owner;
						clan->clanbanks[x + 1].owner = 0;
						clan->clanbanks[x].time = clan->clanbanks[x + 1].time;
						clan->clanbanks[x + 1].time = 0;
						strcpy(clan->clanbanks[x].real_name, clan->clanbanks[x + 1].real_name);
						strcpy(clan->clanbanks[x + 1].real_name, "");
					}
				}
				map_saveclanbank(id);
				lua_pushboolean(state, 1);
				return 1;
			}
		}
	}

	lua_pushboolean(state, 0);
	return 1;
}

/*------------------------------------------------------------------------------
 * Talk Self
 *----------------------------------------------------------------------------*/

int pcl_talkself(lua_State* state, USER* sd) {
	sl_checkargs(state, "ns");
	char* buf;
	int type = lua_tonumber(state, sl_memberarg(1));
	char* msg = lua_tostring(state, sl_memberarg(2));
	unsigned int id = lua_tonumber(state, sl_memberarg(3));
	int msglen = strlen(msg);
	struct block_list* bl = map_id2bl(id);
	USER* tsd = NULL;
	MOB* mob = NULL;
	NPC* npc = NULL;

	if (bl != NULL)
	{
		if (bl->type == BL_PC)
			tsd = (USER*)bl;
		else if (bl->type == BL_MOB)
			mob = (MOB*)bl;
		else if (bl->type == BL_NPC)
			npc = (NPC*)bl;
		else
			return 0;
	}

	WFIFOHEAD(sd->fd, msglen + 13);
	CALLOC(buf, char, 16 + msglen);
	WBUFB(buf, 0) = 0xAA;
	WBUFW(buf, 1) = SWAP16(10 + msglen);
	WBUFB(buf, 3) = 0x0D;
	WBUFB(buf, 5) = type;

	if (id == 0 || bl == NULL)
		WBUFL(buf, 6) = SWAP32(sd->status.id);
	else
		WBUFL(buf, 6) = SWAP32(id);

	WBUFB(buf, 10) = msglen + 2;

	memcpy(WBUFP(buf, 11), msg, msglen);

	clif_send(buf, 16 + msglen, &sd->bl, SELF);
	return 0;
}

/*------------------------------------------------------------------------------
 * Inventory Check Functions
 *----------------------------------------------------------------------------*/

int pcl_hasitem(lua_State* state, USER* sd) {
	unsigned int id = 0, amount = 0, leftover = 0, owner = 0;
	int x, count = 0;
	char* engrave = NULL;
	char* note = NULL;
	int dura = 0;
	unsigned int customLook = 0;
	unsigned int customLookColor = 0;
	unsigned int customIcon = 0;
	unsigned int customIconColor = 0;

	if (lua_isnumber(state, sl_memberarg(1)))
	{
		sl_checkargs(state, "nn");
		id = lua_tonumber(state, sl_memberarg(1));
	}
	else
	{
		sl_checkargs(state, "sn");
		id = itemdb_id(lua_tostring(state, sl_memberarg(1)));
	}
	amount = abs(lua_tonumber(state, sl_memberarg(2)));
	dura = lua_tonumber(state, sl_memberarg(3));
	owner = lua_tonumber(state, sl_memberarg(4));
	engrave = lua_tostring(state, sl_memberarg(5));
	customLook = lua_tonumber(state, sl_memberarg(6));
	customLookColor = lua_tonumber(state, sl_memberarg(7));
	customIcon = lua_tonumber(state, sl_memberarg(8));
	customIconColor = lua_tonumber(state, sl_memberarg(9));
	note = lua_tostring(state, sl_memberarg(10));

	leftover = amount;

	if (!engrave)
	{
		CALLOC(engrave, char, 64);
	}

	if (!note)
	{
		CALLOC(note, char, 300);
	}

	for (x = 0; x < sd->status.maxinv; x++)
	{
		if (owner == 0) {
			if (!dura) {
				if (sd->status.inventory[x].id == id && !(strcmpi(sd->status.inventory[x].real_name, engrave)) && sd->status.inventory[x].customLook == customLook && sd->status.inventory[x].customLookColor == customLookColor && sd->status.inventory[x].customIcon == customIcon && sd->status.inventory[x].customIconColor == customIconColor && !(strcmp(sd->status.inventory[x].note, note)))
				{
					if (sd->status.inventory[x].amount < amount && sd->status.inventory[x].amount > 0)
					{
						amount -= sd->status.inventory[x].amount;

						if (amount == 0)
						{
							lua_pushboolean(state, 1);
							return 1;
						}
					}
					else if (sd->status.inventory[x].amount >= amount)
					{
						lua_pushboolean(state, 1);
						return 1;
					}
				}
			}
			else {
				if (sd->status.inventory[x].id == id && sd->status.inventory[x].dura >= dura && !(strcmpi(sd->status.inventory[x].real_name, engrave)) && sd->status.inventory[x].customLook == customLook && sd->status.inventory[x].customLookColor == customLookColor && sd->status.inventory[x].customIcon == customIcon && sd->status.inventory[x].customIconColor == customIconColor && !(strcmp(sd->status.inventory[x].note, note)))
				{
					if (sd->status.inventory[x].amount < amount && sd->status.inventory[x].amount > 0)
					{
						amount -= sd->status.inventory[x].amount;

						if (amount == 0)
						{
							lua_pushboolean(state, 1);
							return 1;
						}
					}
					else if (sd->status.inventory[x].amount >= amount)
					{
						lua_pushboolean(state, 1);
						return 1;
					}
				}
			}
		}
		else if (owner > 0) {
			if (!dura) {
				if (sd->status.inventory[x].id == id && sd->status.inventory[x].owner == owner && !(strcmpi(sd->status.inventory[x].real_name, engrave)) && sd->status.inventory[x].customLook == customLook && sd->status.inventory[x].customLookColor == customLookColor && sd->status.inventory[x].customIcon == customIcon && sd->status.inventory[x].customIconColor == customIconColor && !(strcmp(sd->status.inventory[x].note, note)))
				{
					if (sd->status.inventory[x].amount < amount && sd->status.inventory[x].amount > 0)
					{
						amount -= sd->status.inventory[x].amount;

						if (amount == 0)
						{
							lua_pushboolean(state, 1);
							return 1;
						}
					}
					else if (sd->status.inventory[x].amount >= amount)
					{
						lua_pushboolean(state, 1);
						return 1;
					}
				}
			}
			else {
				if (sd->status.inventory[x].id == id && sd->status.inventory[x].dura >= dura && sd->status.inventory[x].owner == owner && !(strcmpi(sd->status.inventory[x].real_name, engrave)) && sd->status.inventory[x].customLook == customLook && sd->status.inventory[x].customLookColor == customLookColor && sd->status.inventory[x].customIcon == customIcon && sd->status.inventory[x].customIconColor == customIconColor && !(strcmp(sd->status.inventory[x].note, note)))
				{
					if (sd->status.inventory[x].amount < amount && sd->status.inventory[x].amount > 0)
					{
						amount -= sd->status.inventory[x].amount;

						if (amount == 0)
						{
							lua_pushboolean(state, 1);
							return 1;
						}
					}
					else if (sd->status.inventory[x].amount >= amount)
					{
						lua_pushboolean(state, 1);
						return 1;
					}
				}
			}
		}
	}

	if (engrave)
	{
		engrave = NULL;
	}
	else
	{
		FREE(engrave);
	}

	if (note)
	{
		note = NULL;
	}
	else
	{
		FREE(note);
	}

	leftover -= amount;
	lua_pushnumber(state, leftover);
	return 1;
}

int pcl_hasitemdura(lua_State* state, USER* sd) {
	unsigned int id = 0;
	unsigned int amount = 0, leftover = 0;
	int x;
	int count = 0;
	int dura = 0;

	if (lua_isnumber(state, sl_memberarg(1))) {
		sl_checkargs(state, "nn");
		id = lua_tonumber(state, sl_memberarg(1));
	}
	else {
		sl_checkargs(state, "sn");
		id = itemdb_id(lua_tostring(state, sl_memberarg(1)));
	}
	amount = abs(lua_tonumber(state, sl_memberarg(2)));
	dura = lua_tonumber(state, sl_memberarg(3));

	leftover = amount;

	for (x = 0; x < sd->status.maxinv; x++) {
		if (sd->status.inventory[x].id == id) {
			if (!dura) dura = itemdb_dura(sd->status.inventory[x].id);

			if ((sd->status.inventory[x].amount < amount) && (sd->status.inventory[x].amount > 0) && (sd->status.inventory[x].dura == dura)) {
				amount -= sd->status.inventory[x].amount;
				//pc_delitem(sd,x,sd->status.inventory[x].amount,itemdb_type(sd->status.inventory[x].id));
				if (amount == 0) {
					lua_pushboolean(state, 1);
					return 1;
				}
			}
			else if (sd->status.inventory[x].amount >= amount && (sd->status.inventory[x].dura == dura)) {
				//count+=amount;
				//pc_delitem(sd,x,count,itemdb_type(sd->status.inventory[x].id));
				lua_pushboolean(state, 1);
				return 1;
			}
		}
	}

	leftover -= amount;
	lua_pushnumber(state, leftover);
	return 1;
}

int pcl_hasspace(lua_State* state, USER* sd) {
	unsigned int id = 0;
	int a, x;
	char* engrave = NULL;
	char engraved = 0;
	char* note = NULL;

	unsigned int customLook = 0;
	unsigned int customLookColor = 0;
	unsigned int customIcon = 0;
	unsigned int customIconColor = 0;
	unsigned int protected = 0;

	if (lua_isnumber(state, sl_memberarg(1))) {
		sl_checkargs(state, "nn");
		id = lua_tonumber(state, sl_memberarg(1));
	}
	else {
		sl_checkargs(state, "sn");
		id = itemdb_id(lua_tostring(state, sl_memberarg(1)));
	}

	int incomingAmount = lua_tonumber(state, sl_memberarg(2));
	int owner = lua_tonumber(state, sl_memberarg(3));
	int stackAmount = itemdb_stackamount(id);
	bool hasOpenInventorySlot = false;

	engrave = lua_tostring(state, sl_memberarg(4));
	customLook = lua_tonumber(state, sl_memberarg(5));
	customLookColor = lua_tonumber(state, sl_memberarg(6));
	customIcon = lua_tonumber(state, sl_memberarg(7));
	customIconColor = lua_tonumber(state, sl_memberarg(8));
	protected = lua_tonumber(state, sl_memberarg(9));
	note = lua_tostring(state, sl_memberarg(10));

	if (!engrave)
	{
		CALLOC(engrave, char, 64);
	}

	if (pc_isinvenspace(sd, id, owner, engrave, customLook, customLookColor, customIcon, customIconColor) >= sd->status.maxinv) {
		lua_pushboolean(state, 0);
		return 1;
	}

	for (x = 0; x < sd->status.maxinv; x++) {
		if (!engrave) {
			engraved = -1;
		}
		else {
			engraved = strcmpi(sd->status.inventory[x].real_name, engrave);
		}

		unsigned int itemId = sd->status.inventory[x].id;
		bool idMatches = itemId == id;
		bool ownerMatches = sd->status.inventory[x].owner == owner;
		bool isStackable = stackAmount > 1;

		// If this inventory slot already has some amount of the item being added
		if (idMatches && ownerMatches && isStackable) {
			int currentAmount = sd->status.inventory[x].amount;

			if (currentAmount + incomingAmount <= stackAmount) {
				lua_pushboolean(state, 1);
				return 1;
			} else {
				lua_pushboolean(state, 0);
				return 1;
			}
		}

		if (itemId <= 0) {
			hasOpenInventorySlot = true;
			continue;
		}
	}

	if (engrave)
	{
		engrave = NULL;
	}
	else
	{
		FREE(engrave);
	}

	if (hasOpenInventorySlot) {
		lua_pushboolean(state, 1);
		return 1;
	} else {
		lua_pushboolean(state, 0);
		return 1;
	}
}

/*------------------------------------------------------------------------------
 * Minimap/Account Functions
 *----------------------------------------------------------------------------*/

int pcl_sendminimap(lua_State* state, USER* sd) {
	clif_sendminimap(sd);

	return 0;
}

int pcl_setCaptchaKey(lua_State* state, USER* sd) {
	sl_checkargs(state, "s");

	char* key = lua_tostring(state, sl_memberarg(1));

	char md5[64] = "";

	MD5_String(key, md5);

	if (SQL_ERROR == Sql_Query(sql_handle, "UPDATE `Character` SET `ChaCaptchaKey` = '%s' WHERE `ChaId` = '%u'", md5, sd->status.id)) {
		SqlStmt_ShowDebug(sql_handle);
		Sql_FreeResult(sql_handle);
		lua_pushboolean(state, 0);
		return 1;
	}

	Sql_FreeResult(sql_handle);
	lua_pushboolean(state, 1);
	return 1;
}

int pcl_getCaptchaKey(lua_State* state, USER* sd) {
	char key[64] = "";

	SqlStmt* stmt;
	stmt = SqlStmt_Malloc(sql_handle);
	if (stmt == NULL)
	{
		SqlStmt_ShowDebug(stmt);
		return -1;
	}

	if (SQL_ERROR == SqlStmt_Prepare(stmt, "SELECT `ChaCaptchaKey` FROM `Character` WHERE `ChaId` = '%u'", sd->status.id)
		|| SQL_ERROR == SqlStmt_Execute(stmt)
		|| SQL_ERROR == SqlStmt_BindColumn(stmt, 0, SQLDT_STRING, &key, sizeof(key), NULL, NULL))
	{
		SqlStmt_ShowDebug(stmt);
		SqlStmt_Free(stmt);
		return 1;
	}

	if (SQL_SUCCESS != SqlStmt_NextRow(stmt)) {}

	SqlStmt_Free(stmt);
	lua_pushstring(state, key);

	return 1;
}

int pcl_setAccountBan(lua_State* state, USER* sd) {
	int banned = lua_tonumber(state, sl_memberarg(1));

	int accountid = clif_isregistered(sd->status.id);

	if (accountid == 0) { // not regd
		char name[16];
		memcpy(name, sd->status.name, 16);

		if (banned == 1) session[sd->fd]->eof = 1;

		if (SQL_ERROR == Sql_Query(sql_handle, "UPDATE `Character` SET ChaBanned = '%i' WHERE `ChaName` = '%s'", banned, name)) {
			SqlStmt_ShowDebug(sql_handle);
			Sql_FreeResult(sql_handle);
		}
	}
	else {
		unsigned int ChaIds[6];

		SqlStmt* stmt;

		stmt = SqlStmt_Malloc(sql_handle);
		if (stmt == NULL)
		{
			SqlStmt_ShowDebug(stmt);
			return -1;
		}

		if (SQL_ERROR == SqlStmt_Prepare(stmt, "SELECT `AccountCharId1`, `AccountCharId2`, `AccountCharId3`, `AccountCharId4`, `AccountCharId5`, `AccountCharId6` FROM `Accounts` WHERE `AccountId` = '%u'", accountid)
			|| SQL_ERROR == SqlStmt_Execute(stmt)
			|| SQL_ERROR == SqlStmt_BindColumn(stmt, 0, SQLDT_UINT, &ChaIds[0], 0, NULL, NULL)
			|| SQL_ERROR == SqlStmt_BindColumn(stmt, 1, SQLDT_UINT, &ChaIds[1], 0, NULL, NULL)
			|| SQL_ERROR == SqlStmt_BindColumn(stmt, 2, SQLDT_UINT, &ChaIds[2], 0, NULL, NULL)
			|| SQL_ERROR == SqlStmt_BindColumn(stmt, 3, SQLDT_UINT, &ChaIds[3], 0, NULL, NULL)
			|| SQL_ERROR == SqlStmt_BindColumn(stmt, 4, SQLDT_UINT, &ChaIds[4], 0, NULL, NULL)
			|| SQL_ERROR == SqlStmt_BindColumn(stmt, 5, SQLDT_UINT, &ChaIds[5], 0, NULL, NULL))
		{
			SqlStmt_ShowDebug(stmt);
			SqlStmt_Free(stmt);
			return 1;
		}

		if (SQL_SUCCESS != SqlStmt_NextRow(stmt))
		{
		}

		SqlStmt_Free(stmt);

		if (banned == 1) {
			for (int i = 0; i < sizeof(ChaIds); i++) {	// disconnect all now banned chars
				if (ChaIds[i] > 0) {
					USER* tsd = map_id2sd(ChaIds[i]);
					if (tsd != NULL) session[tsd->fd]->eof = 1;
				}
			}
		}

		if (SQL_ERROR == Sql_Query(sql_handle, "UPDATE `Accounts` SET `AccountBanned` = '%i' WHERE `AccountId` = '%u'", banned, accountid)) {
			SqlStmt_ShowDebug(sql_handle);
			SqlStmt_FreeResult(sql_handle);
		}
	}

	return 1;
}

int pcl_setHeroShow(lua_State* state, USER* sd)
{
	sl_checkargs(state, "n");
	sd->status.heroes = lua_tonumber(state, sl_memberarg(1));

	if (SQL_ERROR == Sql_Query(sql_handle, "UPDATE `Character` SET `ChaHeroes` = '%u' WHERE `ChaName` = '%s'", sd->status.heroes, sd->status.name))
	{
		Sql_ShowDebug(sql_handle);
		Sql_FreeResult(sql_handle);
		return 1;
	}

	Sql_FreeResult(sql_handle);

	return 0;
}

/*------------------------------------------------------------------------------
 * Path/Country/Mail Updates
 *----------------------------------------------------------------------------*/

int pcl_updatePath(lua_State* state, USER* sd) {
	int path = lua_tonumber(state, sl_memberarg(1));
	int mark = lua_tonumber(state, sl_memberarg(2));

	if (path < 0) path = 0;
	if (mark < 0) mark = 0;

	sd->status.class = path;
	sd->status.mark = mark;

	if (SQL_ERROR == Sql_Query(sql_handle, "UPDATE `Character` SET `ChaPthId` = '%u', `ChaMark` = '%u' WHERE `ChaId` = '%u'", path, mark, sd->status.id))
	{
		Sql_ShowDebug(sql_handle);
		Sql_FreeResult(sql_handle);
		return 1;
	}

	Sql_FreeResult(sql_handle);
	clif_mystaytus(sd);

	return 1;
}

int pcl_updateCountry(lua_State* state, USER* sd) {
	unsigned int country = lua_tonumber(state, sl_memberarg(1));

	sd->status.country = country;

	if (SQL_ERROR == Sql_Query(sql_handle, "UPDATE `Character` SET `ChaNation` = '%u' WHERE `ChaId` = '%u'", country, sd->status.id))
	{
		Sql_ShowDebug(sql_handle);
		Sql_FreeResult(sql_handle);
		return 1;
	}

	Sql_FreeResult(sql_handle);
	clif_sendstatus(sd, SFLAG_FULLSTATS | SFLAG_HPMP | SFLAG_XPMONEY);
	clif_sendupdatestatus_onequip(sd);

	return 1;
}

int pcl_updateMail(lua_State* state, USER* sd) {
	char* name = lua_tostring(state, sl_memberarg(1));

	if (SQL_ERROR == Sql_Query(sql_handle, "UPDATE `Mail` SET `MalChaNameDestination` = '%s' WHERE `MalChaNameDestination` = '%s'", sd->status.name, name))
	{
		Sql_ShowDebug(sql_handle);
		Sql_FreeResult(sql_handle);
		lua_pushboolean(state, 0);
		return 1;
	}

	Sql_FreeResult(sql_handle);
	lua_pushboolean(state, 1);

	return 1;
}

/*------------------------------------------------------------------------------
 * Kan System
 *----------------------------------------------------------------------------*/

int pcl_addKan(lua_State* state, USER* sd)
{
	sl_checkargs(state, "n");
	int kan = lua_tonumber(state, sl_memberarg(1));

	char* email = clif_getaccountemail(sd->status.id);

	if (email == NULL) { FREE(email); lua_pushboolean(state, 0); return 1; }

	SqlStmt* stmt;

	stmt = SqlStmt_Malloc(sql_handle);
	if (stmt == NULL)
	{
		SqlStmt_ShowDebug(stmt);
		return -1;
	}

	if (SQL_ERROR == SqlStmt_Prepare(stmt, "SELECT * FROM `Accounts` WHERE `AccountEmail` = '%s'", email)
		|| SQL_ERROR == SqlStmt_Execute(stmt)) {
		SqlStmt_ShowDebug(stmt);
		SqlStmt_Free(stmt);
		free(email);
		lua_pushboolean(state, 0);
		return 1;
	}

	if (SqlStmt_NumRows(stmt) == 0) {
		if (SQL_ERROR == Sql_Query(sql_handle, "INSERT INTO `Accounts` (`AccountEmail`, `AccountKanBalance`) VALUES ('%s','%d')", email, kan))
		{
			Sql_ShowDebug(sql_handle);
			SqlStmt_Free(stmt);
			FREE(email);
			lua_pushboolean(state, 0);
			return 1;
		}
		else { FREE(email); lua_pushboolean(state, 1); return 1; }
	}

	else if (SqlStmt_NumRows(stmt) == 1) {
		if (SQL_ERROR == Sql_Query(sql_handle, "UPDATE `Accounts` SET `AccountKanBalance` = `AccountKanBalance` + '%d' WHERE `AccountEmail` = '%s'", kan, email))
		{
			Sql_ShowDebug(sql_handle);
			SqlStmt_Free(stmt);
			FREE(email);
			lua_pushboolean(state, 0);
			return 1;
		}
		else { FREE(email); lua_pushboolean(state, 1); return 1; }
	}
	else { Sql_ShowDebug(sql_handle); SqlStmt_Free(stmt); FREE(email); lua_pushboolean(state, 0); return 1; }

	FREE(email);
	Sql_FreeResult(sql_handle);
	lua_pushboolean(state, 0);
	return 1;
}

int pcl_removeKan(lua_State* state, USER* sd)
{
	sl_checkargs(state, "n");
	int kan = lua_tonumber(state, sl_memberarg(1));

	char* email = clif_getaccountemail(sd->status.id);
	if (email == NULL) { FREE(email); lua_pushboolean(state, 0); return 1; }

	int kanBalance = 0;

	SqlStmt* stmt;

	stmt = SqlStmt_Malloc(sql_handle);
	if (stmt == NULL)
	{
		SqlStmt_ShowDebug(stmt);
		return -1;
	}

	if (SQL_ERROR == SqlStmt_Prepare(stmt, "SELECT `AccountKanBalance` FROM `Accounts` WHERE `AccountEmail` = '%s'", email)
		|| SQL_ERROR == SqlStmt_Execute(stmt)
		|| SQL_ERROR == SqlStmt_BindColumn(stmt, 0, SQLDT_INT, &kanBalance, 0, NULL, NULL)) {
		SqlStmt_ShowDebug(stmt);
		SqlStmt_Free(stmt);
		FREE(email);
		lua_pushboolean(state, 0);
		return 1;
	}
	if (SQL_SUCCESS != SqlStmt_NextRow(stmt)) {
		SqlStmt_ShowDebug(stmt);
		SqlStmt_Free(stmt);
		FREE(email);
		lua_pushboolean(state, 0);
		return 1;
	}

	if (kanBalance - kan < 0) kanBalance = 0;
	else kanBalance -= kan;

	if (SQL_ERROR == Sql_Query(sql_handle, "UPDATE `Accounts` SET `AccountKanBalance` = '%d' WHERE `AccountEmail` = '%s'", kanBalance, email))
	{
		Sql_ShowDebug(sql_handle);
		Sql_FreeResult(sql_handle);
		FREE(email);
		lua_pushboolean(state, 0);
		return 1;
	}
	FREE(email);
	Sql_FreeResult(sql_handle);
	lua_pushboolean(state, 1);
	return 1;
}

int pcl_kanBalance(lua_State* state, USER* sd)
{
	int kanBalance = 0;

	char* email = clif_getaccountemail(sd->status.id);
	if (email == NULL) { FREE(email); lua_pushnumber(state, 0); return 1; }

	SqlStmt* stmt;

	stmt = SqlStmt_Malloc(sql_handle);
	if (stmt == NULL)
	{
		SqlStmt_ShowDebug(stmt);
		return -1;
	}

	if (SQL_ERROR == SqlStmt_Prepare(stmt, "SELECT `AccountKanBalance` FROM `Accounts` WHERE `AccountEmail` = '%s'", email)
		|| SQL_ERROR == SqlStmt_Execute(stmt)
		|| SQL_ERROR == SqlStmt_BindColumn(stmt, 0, SQLDT_INT, &kanBalance, 0, NULL, NULL)) {
		SqlStmt_ShowDebug(stmt);
		SqlStmt_Free(stmt);
		FREE(email);
		lua_pushnumber(state, 0);
		return 1;
	}
	if (SQL_SUCCESS != SqlStmt_NextRow(stmt)) {
		SqlStmt_Free(stmt);
		FREE(email);
		lua_pushnumber(state, 0);
		return 1;
	}

	FREE(email);
	SqlStmt_Free(stmt);
	lua_pushnumber(state, kanBalance);

	return 1;
}

int pcl_checkKan(lua_State* state, USER* sd)
{
	char kan_amount[80];
	memset(kan_amount, 0, sizeof(kan_amount));

	int number = 0;

	char* email = clif_getaccountemail(sd->status.id);

	if (email == NULL) { FREE(email); lua_pushnumber(state, 0); return 1; }

	SqlStmt* stmt;

	stmt = SqlStmt_Malloc(sql_handle);
	if (stmt == NULL) {
		SqlStmt_ShowDebug(stmt);
		return 0;
	}

	if (SQL_ERROR == SqlStmt_Prepare(stmt, "SELECT `kan_amount` FROM `KanDonations` WHERE `account_email` = '%s' AND `Claimed` = 0 LIMIT 1", email)
		|| SQL_ERROR == SqlStmt_Execute(stmt)
		|| SQL_ERROR == SqlStmt_BindColumn(stmt, 0, SQLDT_STRING, &kan_amount, sizeof(kan_amount), NULL, NULL)) {
		SqlStmt_ShowDebug(stmt);
		SqlStmt_Free(stmt);
		lua_pushnumber(state, 0);
		FREE(email);
		return 1;
	}

	if (SQL_SUCCESS == SqlStmt_NextRow(stmt)) {
		SqlStmt_Free(stmt);
	}

	char* pch;
	pch = strtok(kan_amount, " ");
	if (pch != NULL)
	{
		number = atoi(pch);
		pch = strtok(NULL, " ");
	}

	FREE(email);
	if (number != NULL) { lua_pushnumber(state, number); return 1; }

	return 1;
}

int pcl_claimKan(lua_State* state, USER* sd)
{
	sl_checkargs(state, "n");
	int kan = lua_tonumber(state, sl_memberarg(1));
	int payment_id = 0;

	char* email = clif_getaccountemail(sd->status.id);

	if (email == NULL) { FREE(email); lua_pushboolean(state, 0); return 0; }

	SqlStmt* stmt;

	stmt = SqlStmt_Malloc(sql_handle);
	if (stmt == NULL)
	{
		SqlStmt_ShowDebug(stmt);
		return -1;
	}

	if (SQL_ERROR == SqlStmt_Prepare(stmt, "SELECT min(payment_id) FROM `KanDonations` WHERE `account_email` = '%s' AND `kan_amount` = '%i Kan' AND `Claimed` = 0", email, kan)
		|| SQL_ERROR == SqlStmt_Execute(stmt)
		|| SQL_ERROR == SqlStmt_BindColumn(stmt, 0, SQLDT_INT, &payment_id, 0, NULL, NULL)) {
		SqlStmt_ShowDebug(stmt);
		lua_pushboolean(state, 0);
		SqlStmt_Free(stmt);
		FREE(email);
		return 1;
	}

	if (SQL_SUCCESS == SqlStmt_NextRow(stmt)) {
	}

	if (SqlStmt_NumRows(stmt) != 0 && payment_id != 0) {
		if (SQL_ERROR == Sql_Query(sql_handle, "UPDATE `KanDonations` SET `Claimed` = 1 WHERE `payment_id` = '%i'", payment_id))
		{
			Sql_ShowDebug(sql_handle);
			SqlStmt_Free(stmt);
			FREE(email);
			lua_pushboolean(state, 0);
			return 1;
		}
	}

	FREE(email);
	SqlStmt_Free(stmt);
	lua_pushboolean(state, 1);
	return 1;
}

int pcl_setKan(lua_State* state, USER* sd)
{
	sl_checkargs(state, "n");
	int kan = lua_tonumber(state, sl_memberarg(1));

	char* email = clif_getaccountemail(sd->status.id);

	if (email == NULL) { FREE(email); lua_pushboolean(state, 0); return 1; }

	SqlStmt* stmt;

	stmt = SqlStmt_Malloc(sql_handle);
	if (stmt == NULL)
	{
		SqlStmt_ShowDebug(stmt);
		return -1;
	}

	if (SQL_ERROR == Sql_Query(sql_handle, "UPDATE `Accounts` SET `AccountKanBalance` = '%d' WHERE `AccountEmail` = '%s'", kan, email))
	{
		Sql_ShowDebug(sql_handle);
		Sql_FreeResult(sql_handle);
		FREE(email);
		lua_pushboolean(state, 0);
		return 1;
	}
	Sql_FreeResult(sql_handle);
	lua_pushboolean(state, 1);
	return 1;
}

/*==============================================================================
 * NOTE: All pcl_* method implementations have been extracted from sl.c.
 *
 * To complete the extraction:
 * 1. Remove pcl_* functions from sl.c (they are duplicated there)
 * 2. Add sl_player.o to SL_OBJ in Makefile
 * 3. Rebuild and test
 *============================================================================*/
