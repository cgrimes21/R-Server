/**
 * @file sl_mob.h
 * @brief Mob type Lua bindings
 *
 * Extracted from sl.c as part of the RTK refactoring project.
 * Contains mob-related Lua type bindings.
 */

#ifndef _SL_MOB_H_
#define _SL_MOB_H_

#include <lua.h>
#include <lauxlib.h>
#include "map.h"
#include "sl_types.h"

/* Mob type class */
extern typel_class mobl_type;

/* Type initialization */
void mobl_staticinit(void);

/* Mob type functions */
int mobl_ctor(lua_State* state);
int mobl_init(lua_State* state, MOB* mob, int dataref, void* param);
int mobl_getattr(lua_State* state, MOB* mob, char* attrname);
int mobl_setattr(lua_State* state, MOB* mob, char* attrname);

/* Mob methods */
int mobl_attack(lua_State* state, MOB* mob);
int mobl_addhealth(lua_State* state, MOB* mob);
int mobl_removehealth(lua_State* state, MOB* mob);
int mobl_move(lua_State* state, MOB* mob);
int mobl_move_ignore_object(lua_State* state, MOB* mob);
int mobl_setduration(lua_State* state, MOB* mob);
int mobl_moveintent(lua_State* state, MOB* mob);
int mobl_hasduration(lua_State* state, MOB* mob);
int mobl_hasdurationid(lua_State* state, MOB* mob);
int mobl_getduration(lua_State* state, MOB* mob);
int mobl_getdurationid(lua_State* state, MOB* mob);
int mobl_flushduration(lua_State* state, MOB* mob);
int mobl_flushdurationnouncast(lua_State* state, MOB* mob);
int mobl_durationamount(lua_State* state, MOB* mob);
int mobl_checkthreat(lua_State* state, MOB* mob);
int mobl_sendhealth(lua_State* state, MOB* mob);
int mobl_warp(lua_State* state, MOB* mob);
int mobl_moveghost(lua_State* state, MOB* mob);
int mobl_callbase(lua_State* state, MOB* mob);
int mobl_checkmove(lua_State* state, MOB* mob);
int mobl_setinddmg(lua_State* state, MOB* mob);
int mobl_setgrpdmg(lua_State* state, MOB* mob);
int mobl_getinddmg(lua_State* state, MOB* mob);
int mobl_getgrpdmg(lua_State* state, MOB* mob);
int mobl_getequippeditem(lua_State* state, MOB* mob);
int mobl_calcstat(lua_State* state, MOB* mob);
int mobl_sendstatus(lua_State* state, MOB* mob);
int mobl_sendminitext(lua_State* state, MOB* mob);

#endif /* _SL_MOB_H_ */
