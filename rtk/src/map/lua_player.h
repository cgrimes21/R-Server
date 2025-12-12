/**
 * @file lua_player.h
 * @brief Player character Lua bindings
 *
 * Renamed from sl_player.h as part of the RTK naming refactor.
 */

#ifndef _LUA_PLAYER_H_
#define _LUA_PLAYER_H_

#include <lua.h>
#include <lauxlib.h>
#include "map.h"
#include "lua_types.h"

/* Player type class */
extern lua_type_class lua_player_type;

/* Type initialization */
void lua_player_staticinit(void);

/* Core type functions */
int lua_player_ctor(lua_State* state);
int lua_player_init(lua_State* state, USER* sd, int dataref, void* param);
int lua_player_getattr(lua_State* state, USER* sd, char* attrname);
int lua_player_setattr(lua_State* state, USER* sd, char* attrname);

/* All player methods - see sl_player.h for full documentation */
/* Health/Combat */
int lua_player_addhealth(lua_State*, USER*);
int lua_player_removehealth(lua_State*, USER*);
int lua_player_resurrect(lua_State*, USER*);
int lua_player_die(lua_State*, USER*);
int lua_player_sendhealth(lua_State*, USER*);
int lua_player_showhealth(lua_State*, USER*);
int lua_player_swing(lua_State*, USER*);
int lua_player_swingtarget(lua_State*, USER*);
int lua_player_addthreat(lua_State*, USER*);
int lua_player_setthreat(lua_State*, USER*);
int lua_player_addthreatgeneral(lua_State*, USER*);

/* Movement */
int lua_player_warp(lua_State*, USER*);
int lua_player_move(lua_State*, USER*);
int lua_player_respawn(lua_State*, USER*);

/* Durations */
int lua_player_setduration(lua_State*, USER*);
int lua_player_hasduration(lua_State*, USER*);
int lua_player_hasdurationid(lua_State*, USER*);
int lua_player_getduration(lua_State*, USER*);
int lua_player_getdurationid(lua_State*, USER*);
int lua_player_getalldurations(lua_State*, USER*);
int lua_player_flushduration(lua_State*, USER*);
int lua_player_flushdurationnouncast(lua_State*, USER*);
int lua_player_refreshdurations(lua_State*, USER*);
int lua_player_durationamount(lua_State*, USER*);
int lua_player_setaether(lua_State*, USER*);
int lua_player_hasaether(lua_State*, USER*);
int lua_player_getaether(lua_State*, USER*);
int lua_player_getallaethers(lua_State*, USER*);
int lua_player_flushaether(lua_State*, USER*);

/* Legend */
int lua_player_addlegend(lua_State*, USER*);
int lua_player_haslegend(lua_State*, USER*);
int lua_player_removelegendbyname(lua_State*, USER*);
int lua_player_removelegendbycolor(lua_State*, USER*);

/* Spells */
int lua_player_addspell(lua_State*, USER*);
int lua_player_removespell(lua_State*, USER*);
int lua_player_hasspell(lua_State*, USER*);
int lua_player_getspells(lua_State*, USER*);
int lua_player_getspellname(lua_State*, USER*);
int lua_player_getspellyname(lua_State*, USER*);
int lua_player_getspellnamefromyname(lua_State*, USER*);
int lua_player_getunknownspells(lua_State*, USER*);
int lua_player_getallclassspells(lua_State*, USER*);
int lua_player_getallspells(lua_State*, USER*);

/* Inventory */
int lua_player_additem(lua_State*, USER*);
int lua_player_hasitem(lua_State*, USER*);
int lua_player_hasitemdura(lua_State*, USER*);
int lua_player_hasspace(lua_State*, USER*);
int lua_player_removeinventoryitem(lua_State*, USER*);
int lua_player_removeitemslot(lua_State*, USER*);
int lua_player_removeitemdura(lua_State*, USER*);
int lua_player_getinventoryitem(lua_State*, USER*);
int lua_player_getboditem(lua_State*, USER*);
int lua_player_getequippeditem(lua_State*, USER*);
int lua_player_getexchangeitem(lua_State*, USER*);
int lua_player_updateinv(lua_State*, USER*);
int lua_player_checkinvbod(lua_State*, USER*);
int lua_player_refreshInventory(lua_State*, USER*);

/* Equipment */
int lua_player_equip(lua_State*, USER*);
int lua_player_forceequip(lua_State*, USER*);
int lua_player_takeoff(lua_State*, USER*);
int lua_player_stripequip(lua_State*, USER*);
int lua_player_hasequipped(lua_State*, USER*);
int lua_player_deductarmor(lua_State*, USER*);
int lua_player_deductweapon(lua_State*, USER*);
int lua_player_deductdura(lua_State*, USER*);
int lua_player_deductdurainv(lua_State*, USER*);
int lua_player_deductduraequip(lua_State*, USER*);

/* Items */
int lua_player_useitem(lua_State*, USER*);
int lua_player_forcedrop(lua_State*, USER*);
int lua_player_throwitem(lua_State*, USER*);
int lua_player_pickup(lua_State*, USER*);
int lua_player_expireitem(lua_State*, USER*);

/* Gifts */
int lua_player_addGift(lua_State*, USER*);
int lua_player_retrieveGift(lua_State*, USER*);

/* Banking */
int lua_player_getbankitem(lua_State*, USER*);
int lua_player_getbankitems(lua_State*, USER*);
int lua_player_bankdeposit(lua_State*, USER*);
int lua_player_bankwithdraw(lua_State*, USER*);
int lua_player_getclanbankitems(lua_State*, USER*);
int lua_player_clanbankdeposit(lua_State*, USER*);
int lua_player_clanbankwithdraw(lua_State*, USER*);
int lua_player_getsubpathbankitems(lua_State*, USER*);
int lua_player_subpathbankdeposit(lua_State*, USER*);
int lua_player_subpathbankwithdraw(lua_State*, USER*);

/* Dialog */
int lua_player_dialog(lua_State*, USER*);
int lua_player_menu(lua_State*, USER*);
int lua_player_menuseq(lua_State*, USER*);
int lua_player_input(lua_State*, USER*);
int lua_player_inputseq(lua_State*, USER*);
int lua_player_popup(lua_State*, USER*);
int lua_player_paperpopup(lua_State*, USER*);
int lua_player_paperpopupwrite(lua_State*, USER*);

/* Shop */
int lua_player_buy(lua_State*, USER*);
int lua_player_sell(lua_State*, USER*);
int lua_player_logbuysell(lua_State*, USER*);

/* Status */
int lua_player_sendstatus(lua_State*, USER*);
int lua_player_calcstat(lua_State*, USER*);
int lua_player_status(lua_State*, USER*);
int lua_player_addEventXP(lua_State*, USER*);

/* Communication */
int lua_player_sendminitext(lua_State*, USER*);
int lua_player_speak(lua_State*, USER*);
int lua_player_talkself(lua_State*, USER*);
int lua_player_guitext(lua_State*, USER*);
int lua_player_sendmail(lua_State*, USER*);
int lua_player_sendurl(lua_State*, USER*);

/* Board */
int lua_player_showboard(lua_State*, USER*);
int lua_player_showpost(lua_State*, USER*);
int lua_player_sendboardquestions(lua_State*, USER*);
int lua_player_powerboard(lua_State*, USER*);
int lua_player_mapselection(lua_State*, USER*);

/* Kills */
int lua_player_killcount(lua_State*, USER*);
int lua_player_setkillcount(lua_State*, USER*);
int lua_player_flushkills(lua_State*, USER*);
int lua_player_flushallkills(lua_State*, USER*);

/* Misc */
int lua_player_addclan(lua_State*, USER*);
int lua_player_addguide(lua_State*, USER*);
int lua_player_delguide(lua_State*, USER*);
int lua_player_getparcel(lua_State*, USER*);
int lua_player_getparcellist(lua_State*, USER*);
int lua_player_removeparcel(lua_State*, USER*);
int lua_player_getcreationitems(lua_State*, USER*);
int lua_player_getcreationamounts(lua_State*, USER*);
int lua_player_setpk(lua_State*, USER*);
int lua_player_getpk(lua_State*, USER*);
int lua_player_lock(lua_State*, USER*);
int lua_player_unlock(lua_State*, USER*);
int lua_player_freeasync(lua_State*, USER*);
int lua_player_forcesave(lua_State*, USER*);
int lua_player_refresh(lua_State*, USER*);
int lua_player_addKan(lua_State*, USER*);
int lua_player_removeKan(lua_State*, USER*);
int lua_player_setKan(lua_State*, USER*);
int lua_player_checkKan(lua_State*, USER*);
int lua_player_claimKan(lua_State*, USER*);
int lua_player_kanBalance(lua_State*, USER*);
int lua_player_updatePath(lua_State*, USER*);
int lua_player_updateCountry(lua_State*, USER*);
int lua_player_updateMail(lua_State*, USER*);
int lua_player_setHeroShow(lua_State*, USER*);
int lua_player_setAccountBan(lua_State*, USER*);
int lua_player_setCaptchaKey(lua_State*, USER*);
int lua_player_getCaptchaKey(lua_State*, USER*);
int lua_player_sendminimap(lua_State*, USER*);
int lua_player_setMiniMapToggle(lua_State*, USER*);
int lua_player_settimevalues(lua_State*, USER*);
int lua_player_gettimevalues(lua_State*, USER*);
int lua_player_settimer(lua_State*, USER*);
int lua_player_addtime(lua_State*, USER*);
int lua_player_removetime(lua_State*, USER*);
int lua_player_lookat(lua_State*, USER*);
int lua_player_changeview(lua_State*, USER*);
int lua_player_testpacket(lua_State*, USER*);
int lua_player_getcasterid(lua_State*, USER*);
int lua_player_checklevel(lua_State*, USER*);

/* Backward compatibility - pcl_* -> lua_player_* */
#define pcl_type lua_player_type
#define pcl_staticinit lua_player_staticinit
#define pcl_ctor lua_player_ctor
#define pcl_init lua_player_init
#define pcl_getattr lua_player_getattr
#define pcl_setattr lua_player_setattr
#define pcl_addhealth lua_player_addhealth
#define pcl_removehealth lua_player_removehealth
#define pcl_warp lua_player_warp
#define pcl_dialog lua_player_dialog
#define pcl_menu lua_player_menu
#define pcl_additem lua_player_additem
#define pcl_hasitem lua_player_hasitem
#define pcl_sendminitext lua_player_sendminitext
/* ... additional compatibility macros as needed */

#endif /* _LUA_PLAYER_H_ */
