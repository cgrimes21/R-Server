/**
 * @file lua_mob.h
 * @brief Mob type Lua bindings
 *
 * Renamed from sl_mob.h as part of the RTK naming refactor.
 */

#ifndef _LUA_MOB_H_
#define _LUA_MOB_H_

#include <lua.h>
#include <lauxlib.h>
#include "map.h"
#include "lua_types.h"

/* Mob type class */
extern lua_type_class lua_mob_type;

/* Type initialization */
void lua_mob_staticinit(void);

/* Mob type functions */
int lua_mob_ctor(lua_State* state);
int lua_mob_init(lua_State* state, MOB* mob, int dataref, void* param);
int lua_mob_getattr(lua_State* state, MOB* mob, char* attrname);
int lua_mob_setattr(lua_State* state, MOB* mob, char* attrname);

/* Mob methods */
int lua_mob_attack(lua_State* state, MOB* mob);
int lua_mob_addhealth(lua_State* state, MOB* mob);
int lua_mob_removehealth(lua_State* state, MOB* mob);
int lua_mob_move(lua_State* state, MOB* mob);
int lua_mob_move_ignore_object(lua_State* state, MOB* mob);
int lua_mob_setduration(lua_State* state, MOB* mob);
int lua_mob_moveintent(lua_State* state, MOB* mob);
int lua_mob_hasduration(lua_State* state, MOB* mob);
int lua_mob_hasdurationid(lua_State* state, MOB* mob);
int lua_mob_getduration(lua_State* state, MOB* mob);
int lua_mob_getdurationid(lua_State* state, MOB* mob);
int lua_mob_flushduration(lua_State* state, MOB* mob);
int lua_mob_flushdurationnouncast(lua_State* state, MOB* mob);
int lua_mob_durationamount(lua_State* state, MOB* mob);
int lua_mob_checkthreat(lua_State* state, MOB* mob);
int lua_mob_sendhealth(lua_State* state, MOB* mob);
int lua_mob_warp(lua_State* state, MOB* mob);
int lua_mob_moveghost(lua_State* state, MOB* mob);
int lua_mob_callbase(lua_State* state, MOB* mob);
int lua_mob_checkmove(lua_State* state, MOB* mob);
int lua_mob_setinddmg(lua_State* state, MOB* mob);
int lua_mob_setgrpdmg(lua_State* state, MOB* mob);
int lua_mob_getinddmg(lua_State* state, MOB* mob);
int lua_mob_getgrpdmg(lua_State* state, MOB* mob);
int lua_mob_getequippeditem(lua_State* state, MOB* mob);
int lua_mob_calcstat(lua_State* state, MOB* mob);
int lua_mob_sendstatus(lua_State* state, MOB* mob);
int lua_mob_sendminitext(lua_State* state, MOB* mob);

/* Backward compatibility */
#define mobl_type lua_mob_type
#define mobl_staticinit lua_mob_staticinit
#define mobl_ctor lua_mob_ctor
#define mobl_init lua_mob_init
#define mobl_getattr lua_mob_getattr
#define mobl_setattr lua_mob_setattr
#define mobl_attack lua_mob_attack
#define mobl_addhealth lua_mob_addhealth
#define mobl_removehealth lua_mob_removehealth
#define mobl_move lua_mob_move
#define mobl_move_ignore_object lua_mob_move_ignore_object
#define mobl_setduration lua_mob_setduration
#define mobl_moveintent lua_mob_moveintent
#define mobl_hasduration lua_mob_hasduration
#define mobl_hasdurationid lua_mob_hasdurationid
#define mobl_getduration lua_mob_getduration
#define mobl_getdurationid lua_mob_getdurationid
#define mobl_flushduration lua_mob_flushduration
#define mobl_flushdurationnouncast lua_mob_flushdurationnouncast
#define mobl_durationamount lua_mob_durationamount
#define mobl_checkthreat lua_mob_checkthreat
#define mobl_sendhealth lua_mob_sendhealth
#define mobl_warp lua_mob_warp
#define mobl_moveghost lua_mob_moveghost
#define mobl_callbase lua_mob_callbase
#define mobl_checkmove lua_mob_checkmove
#define mobl_setinddmg lua_mob_setinddmg
#define mobl_setgrpdmg lua_mob_setgrpdmg
#define mobl_getinddmg lua_mob_getinddmg
#define mobl_getgrpdmg lua_mob_getgrpdmg
#define mobl_getequippeditem lua_mob_getequippeditem
#define mobl_calcstat lua_mob_calcstat
#define mobl_sendstatus lua_mob_sendstatus
#define mobl_sendminitext lua_mob_sendminitext

#endif /* _LUA_MOB_H_ */
