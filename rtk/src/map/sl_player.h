/**
 * @file sl_player.h
 * @brief Player character (pcl_*) Lua bindings
 *
 * Extracted from sl.c as part of the RTK refactoring project.
 * Contains all player character operations exposed to Lua scripts.
 */

#ifndef _SL_PLAYER_H_
#define _SL_PLAYER_H_

#include <lua.h>
#include <lauxlib.h>
#include "map.h"
#include "sl_types.h"

/* Player type class */
extern typel_class pcl_type;

/* Type initialization */
void pcl_staticinit(void);

/* Core type functions */
int pcl_ctor(lua_State* state);
int pcl_init(lua_State* state, USER* sd, int dataref, void* param);
int pcl_getattr(lua_State* state, USER* sd, char* attrname);
int pcl_setattr(lua_State* state, USER* sd, char* attrname);

/* Health/Combat */
int pcl_addhealth(lua_State* state, USER* sd);
int pcl_removehealth(lua_State* state, USER* sd);
int pcl_resurrect(lua_State* state, USER* sd);
int pcl_die(lua_State* state, USER* sd);
int pcl_sendhealth(lua_State* state, USER* sd);
/* Declared but not implemented in sl.c:
int pcl_sendhealthscript(lua_State* state, USER* sd);
*/
int pcl_showhealth(lua_State* state, USER* sd);
int pcl_swing(lua_State* state, USER* sd);
int pcl_swingtarget(lua_State* state, USER* sd);

/* Threat system */
int pcl_addthreat(lua_State* state, USER* sd);
int pcl_setthreat(lua_State* state, USER* sd);
int pcl_addthreatgeneral(lua_State* state, USER* sd);

/* Movement/Position */
int pcl_warp(lua_State* state, USER* sd);
int pcl_move(lua_State* state, USER* sd);
int pcl_respawn(lua_State* state, USER* sd);

/* Durations/Aethers */
int pcl_setduration(lua_State* state, USER* sd);
int pcl_hasduration(lua_State* state, USER* sd);
int pcl_hasdurationid(lua_State* state, USER* sd);
int pcl_getduration(lua_State* state, USER* sd);
int pcl_getdurationid(lua_State* state, USER* sd);
int pcl_getalldurations(lua_State* state, USER* sd);
int pcl_flushduration(lua_State* state, USER* sd);
int pcl_flushdurationnouncast(lua_State* state, USER* sd);
int pcl_refreshdurations(lua_State* state, USER* sd);
int pcl_durationamount(lua_State* state, USER* sd);
int pcl_setaether(lua_State* state, USER* sd);
int pcl_hasaether(lua_State* state, USER* sd);
int pcl_getaether(lua_State* state, USER* sd);
int pcl_getallaethers(lua_State* state, USER* sd);
int pcl_flushaether(lua_State* state, USER* sd);

/* Legend/Marks */
int pcl_addlegend(lua_State* state, USER* sd);
int pcl_haslegend(lua_State* state, USER* sd);
/* Declared but not implemented in sl.c:
int pcl_getlegend(lua_State* state, USER* sd);
*/
int pcl_removelegendbyname(lua_State* state, USER* sd);
int pcl_removelegendbycolor(lua_State* state, USER* sd);

/* Spells */
int pcl_addspell(lua_State* state, USER* sd);
int pcl_removespell(lua_State* state, USER* sd);
int pcl_hasspell(lua_State* state, USER* sd);
int pcl_getspells(lua_State* state, USER* sd);
int pcl_getspellname(lua_State* state, USER* sd);
int pcl_getspellyname(lua_State* state, USER* sd);
int pcl_getspellnamefromyname(lua_State* state, USER* sd);
/* Declared but not implemented in sl.c:
int pcl_getspellsubspec(lua_State* state, USER* sd);
*/
int pcl_getunknownspells(lua_State* state, USER* sd);
int pcl_getallclassspells(lua_State* state, USER* sd);
int pcl_getallspells(lua_State* state, USER* sd);

/* Inventory */
int pcl_additem(lua_State* state, USER* sd);
int pcl_hasitem(lua_State* state, USER* sd);
int pcl_hasitemdura(lua_State* state, USER* sd);
int pcl_hasspace(lua_State* state, USER* sd);
int pcl_removeinventoryitem(lua_State* state, USER* sd);
int pcl_removeitemslot(lua_State* state, USER* sd);
int pcl_removeitemdura(lua_State* state, USER* sd);
int pcl_getinventoryitem(lua_State* state, USER* sd);
int pcl_getboditem(lua_State* state, USER* sd);
int pcl_getequippeditem(lua_State* state, USER* sd);
int pcl_getexchangeitem(lua_State* state, USER* sd);
int pcl_updateinv(lua_State* state, USER* sd);
int pcl_checkinvbod(lua_State* state, USER* sd);
int pcl_refreshInventory(lua_State* state, USER* sd);

/* Equipment */
int pcl_equip(lua_State* state, USER* sd);
int pcl_forceequip(lua_State* state, USER* sd);
int pcl_takeoff(lua_State* state, USER* sd);
int pcl_stripequip(lua_State* state, USER* sd);
int pcl_hasequipped(lua_State* state, USER* sd);
int pcl_deductarmor(lua_State* state, USER* sd);
int pcl_deductweapon(lua_State* state, USER* sd);
int pcl_deductdura(lua_State* state, USER* sd);
int pcl_deductdurainv(lua_State* state, USER* sd);
int pcl_deductduraequip(lua_State* state, USER* sd);

/* Items - drops/use */
int pcl_useitem(lua_State* state, USER* sd);
int pcl_forcedrop(lua_State* state, USER* sd);
int pcl_throwitem(lua_State* state, USER* sd);
int pcl_pickup(lua_State* state, USER* sd);
int pcl_expireitem(lua_State* state, USER* sd);

/* Gifts */
int pcl_addGift(lua_State* state, USER* sd);
int pcl_retrieveGift(lua_State* state, USER* sd);

/* Banking */
int pcl_getbankitem(lua_State* state, USER* sd);
int pcl_getbankitems(lua_State* state, USER* sd);
int pcl_bankdeposit(lua_State* state, USER* sd);
int pcl_bankwithdraw(lua_State* state, USER* sd);

/* Clan banking */
int pcl_getclanbankitems(lua_State* state, USER* sd);
int pcl_clanbankdeposit(lua_State* state, USER* sd);
int pcl_clanbankwithdraw(lua_State* state, USER* sd);

/* Subpath banking */
int pcl_getsubpathbankitems(lua_State* state, USER* sd);
int pcl_subpathbankdeposit(lua_State* state, USER* sd);
int pcl_subpathbankwithdraw(lua_State* state, USER* sd);

/* Dialog/Menu */
int pcl_dialog(lua_State* state, USER* sd);
int pcl_menu(lua_State* state, USER* sd);
int pcl_menuseq(lua_State* state, USER* sd);
int pcl_input(lua_State* state, USER* sd);
int pcl_inputseq(lua_State* state, USER* sd);
int pcl_popup(lua_State* state, USER* sd);
int pcl_paperpopup(lua_State* state, USER* sd);
int pcl_paperpopupwrite(lua_State* state, USER* sd);

/* Shop/Trading */
int pcl_buy(lua_State* state, USER* sd);
int pcl_sell(lua_State* state, USER* sd);
int pcl_logbuysell(lua_State* state, USER* sd);

/* Status/Stats */
int pcl_sendstatus(lua_State* state, USER* sd);
int pcl_calcstat(lua_State* state, USER* sd);
int pcl_status(lua_State* state, USER* sd);
/* Declared but not implemented in sl.c:
int pcl_level(lua_State* state, USER* sd);
*/
int pcl_addEventXP(lua_State* state, USER* sd);

/* Communication */
int pcl_sendminitext(lua_State* state, USER* sd);
int pcl_speak(lua_State* state, USER* sd);
int pcl_talkself(lua_State* state, USER* sd);
int pcl_guitext(lua_State* state, USER* sd);
int pcl_sendmail(lua_State* state, USER* sd);
int pcl_sendurl(lua_State* state, USER* sd);

/* Board/Post */
int pcl_showboard(lua_State* state, USER* sd);
int pcl_showpost(lua_State* state, USER* sd);
int pcl_sendboardquestions(lua_State* state, USER* sd);
int pcl_powerboard(lua_State* state, USER* sd);

/* Map selection */
int pcl_mapselection(lua_State* state, USER* sd);

/* Kill tracking */
int pcl_killcount(lua_State* state, USER* sd);
int pcl_setkillcount(lua_State* state, USER* sd);
int pcl_flushkills(lua_State* state, USER* sd);
int pcl_flushallkills(lua_State* state, USER* sd);

/* Clan */
int pcl_addclan(lua_State* state, USER* sd);

/* Guide */
int pcl_addguide(lua_State* state, USER* sd);
int pcl_delguide(lua_State* state, USER* sd);

/* Parcel */
int pcl_getparcel(lua_State* state, USER* sd);
int pcl_getparcellist(lua_State* state, USER* sd);
int pcl_removeparcel(lua_State* state, USER* sd);

/* Creation system */
int pcl_getcreationitems(lua_State* state, USER* sd);
int pcl_getcreationamounts(lua_State* state, USER* sd);

/* PK system */
int pcl_setpk(lua_State* state, USER* sd);
int pcl_getpk(lua_State* state, USER* sd);

/* Lock/Async */
int pcl_lock(lua_State* state, USER* sd);
int pcl_unlock(lua_State* state, USER* sd);
int pcl_freeasync(lua_State* state, USER* sd);

/* Save */
int pcl_forcesave(lua_State* state, USER* sd);

/* Refresh */
/* Declared but not implemented in sl.c:
int pcl_minirefresh(lua_State* state, USER* sd);
*/
int pcl_refresh(lua_State* state, USER* sd);

/* Kan system */
int pcl_addKan(lua_State* state, USER* sd);
int pcl_removeKan(lua_State* state, USER* sd);
int pcl_setKan(lua_State* state, USER* sd);
int pcl_checkKan(lua_State* state, USER* sd);
int pcl_claimKan(lua_State* state, USER* sd);
int pcl_kanBalance(lua_State* state, USER* sd);

/* Path/Country/Mail updates */
int pcl_updatePath(lua_State* state, USER* sd);
int pcl_updateCountry(lua_State* state, USER* sd);
int pcl_updateMail(lua_State* state, USER* sd);

/* Account/Admin */
int pcl_setHeroShow(lua_State* state, USER* sd);
int pcl_setAccountBan(lua_State* state, USER* sd);
int pcl_setCaptchaKey(lua_State* state, USER* sd);
int pcl_getCaptchaKey(lua_State* state, USER* sd);

/* Minimap */
int pcl_sendminimap(lua_State* state, USER* sd);
int pcl_setMiniMapToggle(lua_State* state, USER* sd);

/* Time values */
int pcl_settimevalues(lua_State* state, USER* sd);
int pcl_gettimevalues(lua_State* state, USER* sd);

/* Timer */
int pcl_settimer(lua_State* state, USER* sd);
int pcl_addtime(lua_State* state, USER* sd);
int pcl_removetime(lua_State* state, USER* sd);

/* View/Look */
int pcl_lookat(lua_State* state, USER* sd);
int pcl_changeview(lua_State* state, USER* sd);

/* Debug/Test */
int pcl_testpacket(lua_State* state, USER* sd);
int pcl_getcasterid(lua_State* state, USER* sd);
int pcl_checklevel(lua_State* state, USER* sd);

#endif /* _SL_PLAYER_H_ */
