/**
 * @file lua_npc.h
 * @brief NPC type Lua bindings
 *
 * Renamed from sl_npc.h as part of the RTK naming refactor.
 */

#ifndef _LUA_NPC_H_
#define _LUA_NPC_H_

#include <lua.h>
#include <lauxlib.h>
#include "map.h"
#include "lua_types.h"

/* NPC type class */
extern lua_type_class lua_npc_type;

/* Type initialization */
void lua_npc_staticinit(void);

/* NPC type functions */
int lua_npc_ctor(lua_State* state);
int lua_npc_init(lua_State* state, NPC* nd, int dataref, void* param);
int lua_npc_getattr(lua_State* state, NPC* nd, char* attrname);
int lua_npc_setattr(lua_State* state, NPC* nd, char* attrname);

/* NPC methods */
int lua_npc_move(lua_State* state, NPC* nd);
int lua_npc_warp(lua_State* state, NPC* nd);
int lua_npc_getequippeditem(lua_State* state, NPC* nd);

/* Backward compatibility */
#define npcl_type lua_npc_type
#define npcl_staticinit lua_npc_staticinit
#define npcl_ctor lua_npc_ctor
#define npcl_init lua_npc_init
#define npcl_getattr lua_npc_getattr
#define npcl_setattr lua_npc_setattr
#define npcl_move lua_npc_move
#define npcl_warp lua_npc_warp
#define npcl_getequippeditem lua_npc_getequippeditem

#endif /* _LUA_NPC_H_ */
