/**
 * @file sl_npc.h
 * @brief NPC type Lua bindings
 *
 * Extracted from sl.c as part of the RTK refactoring project.
 * Contains NPC-related Lua type bindings.
 */

#ifndef _SL_NPC_H_
#define _SL_NPC_H_

#include <lua.h>
#include <lauxlib.h>
#include "map.h"
#include "sl_types.h"

/* NPC type class */
extern typel_class npcl_type;

/* Type initialization */
void npcl_staticinit(void);

/* NPC type functions */
int npcl_ctor(lua_State* state);
int npcl_init(lua_State* state, NPC* nd, int dataref, void* param);
int npcl_getattr(lua_State* state, NPC* nd, char* attrname);
int npcl_setattr(lua_State* state, NPC* nd, char* attrname);

/* NPC methods */
int npcl_move(lua_State* state, NPC* nd);
int npcl_warp(lua_State* state, NPC* nd);
int npcl_getequippeditem(lua_State* state, NPC* nd);

#endif /* _SL_NPC_H_ */
