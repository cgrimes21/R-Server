/**
 * @file lua_blocklist.h
 * @brief Block list Lua bindings - spatial object operations
 *
 * Provides Lua bindings for spatial operations on block_list objects.
 * Renamed from sl_blocklist.h as part of the RTK naming refactor.
 */

#ifndef _LUA_BLOCKLIST_H_
#define _LUA_BLOCKLIST_H_

#include <lua.h>
#include <lauxlib.h>
#include <stdarg.h>
#include "map.h"
#include "lua_types.h"

/* Block list instance macros */
#define lua_blocklist_pushinst_macro(state, bl) lua_blocklist_pushinst(state, bl, 0)

/* Core functions */
void lua_blocklist_pushinst(lua_State* state, struct block_list* bl, void* param);
void lua_blocklist_extendproto(lua_type_class* class);
int lua_blocklist_getattr(lua_State* state, struct block_list* bl, char* attrname);
int lua_blocklist_setattr(lua_State* state, struct block_list* bl, char* attrname);

/* Object spawning/management */
int lua_blocklist_spawn(lua_State* state, struct block_list* bl);
int lua_blocklist_delete(lua_State* state, struct block_list* bl);
int lua_blocklist_deliddb(lua_State* state, struct block_list* bl);
int lua_blocklist_addnpc(lua_State* state, struct block_list* bl);
int lua_blocklist_respawn(lua_State* state, struct block_list* bl);
int lua_blocklist_permspawn(lua_State* state, struct block_list* bl);

/* Object queries */
int lua_blocklist_getblock(lua_State* state, void* self);
int lua_blocklist_getobjects(lua_State* state, void* self, int area);
int lua_blocklist_getaliveobjects(lua_State* state, void* self, int area);
int lua_blocklist_getobjects_area(lua_State* state, void* self);
int lua_blocklist_getaliveobjects_area(lua_State* state, void* self);
int lua_blocklist_getobjects_samemap(lua_State* state, void* self);
int lua_blocklist_getaliveobjects_samemap(lua_State* state, void* self);
int lua_blocklist_getobjects_map(lua_State* state, void* self);
int lua_blocklist_getaliveobjects_map(lua_State* state, void* self);
int lua_blocklist_getobjects_cell(lua_State* state, void* self);
int lua_blocklist_getaliveobjects_cell(lua_State* state, void* self);
int lua_blocklist_getobjects_cell_traps(lua_State* state, void* self);
int lua_blocklist_getaliveobjects_cell_traps(lua_State* state, void* self);
int lua_blocklist_getusers(lua_State* state, void* self);
int lua_blocklist_getfriends(lua_State* state, struct block_list* bl);

/* Object query helpers */
int lua_blocklist_getobjects_helper(struct block_list* bl, va_list ap);
int lua_blocklist_getaliveobjects_helper(struct block_list* bl, va_list ap);
int lua_blocklist_handle_mob(struct block_list* bl, va_list ap);

/* Animation/visual */
int lua_blocklist_sendanim(lua_State* state, void* self);
int lua_blocklist_sendanimxy(lua_State* state, void* self);
int lua_blocklist_selfanimation(lua_State* state, struct block_list* bl);
int lua_blocklist_selfanimationxy(lua_State* state, struct block_list* bl);
int lua_blocklist_repeatanimation(lua_State* state, struct block_list* bl);
int lua_blocklist_sendaction(lua_State* state, void* self);
int lua_blocklist_sendside(lua_State* state, struct block_list* bl);
int lua_blocklist_removesprite(lua_State* state, struct block_list* bl);
int lua_blocklist_updatestate(lua_State* state, struct block_list* bl);

/* Item operations */
int lua_blocklist_throw(lua_State* state, void* self);
int lua_blocklist_dropitem(lua_State* state, struct block_list* bl);
int lua_blocklist_dropitemxy(lua_State* state, struct block_list* bl);
int lua_blocklist_sendparcel(lua_State* state, struct block_list* bl);

/* Movement */
int lua_blocklist_objectcanmove(lua_State* state, struct block_list* bl);
int lua_blocklist_objectcanmovefrom(lua_State* state, struct block_list* bl);

/* Communication */
int lua_blocklist_talk(lua_State* state, struct block_list* bl);
int lua_blocklist_talkcolor(lua_State* state, struct block_list* bl);
int lua_blocklist_playsound(lua_State* state, struct block_list* bl);

/* Helper functions */
int lua_sendanimation(USER* sd, ...);
int lua_sendanimationxy(USER* sd, ...);

/* Backward compatibility macros */
#define bll_pushinst_macro lua_blocklist_pushinst_macro
#define bll_pushinst lua_blocklist_pushinst
#define bll_extendproto lua_blocklist_extendproto
#define bll_getattr lua_blocklist_getattr
#define bll_setattr lua_blocklist_setattr
#define bll_spawn lua_blocklist_spawn
#define bll_delete lua_blocklist_delete
#define bll_deliddb lua_blocklist_deliddb
#define bll_addnpc lua_blocklist_addnpc
#define bll_respawn lua_blocklist_respawn
#define bll_permspawn lua_blocklist_permspawn
#define bll_getblock lua_blocklist_getblock
#define bll_getobjects lua_blocklist_getobjects
#define bll_getaliveobjects lua_blocklist_getaliveobjects
#define bll_getobjects_area lua_blocklist_getobjects_area
#define bll_getaliveobjects_area lua_blocklist_getaliveobjects_area
#define bll_getobjects_samemap lua_blocklist_getobjects_samemap
#define bll_getaliveobjects_samemap lua_blocklist_getaliveobjects_samemap
#define bll_getobjects_map lua_blocklist_getobjects_map
#define bll_getaliveobjects_map lua_blocklist_getaliveobjects_map
#define bll_getobjects_cell lua_blocklist_getobjects_cell
#define bll_getaliveobjects_cell lua_blocklist_getaliveobjects_cell
#define bll_getobjects_cell_traps lua_blocklist_getobjects_cell_traps
#define bll_getaliveobjects_cell_traps lua_blocklist_getaliveobjects_cell_traps
#define bll_getusers lua_blocklist_getusers
#define bll_getfriends lua_blocklist_getfriends
#define bll_getobjects_helper lua_blocklist_getobjects_helper
#define bll_getaliveobjects_helper lua_blocklist_getaliveobjects_helper
#define bll_handle_mob lua_blocklist_handle_mob
#define bll_sendanim lua_blocklist_sendanim
#define bll_sendanimxy lua_blocklist_sendanimxy
#define bll_selfanimation lua_blocklist_selfanimation
#define bll_selfanimationxy lua_blocklist_selfanimationxy
#define bll_repeatanimation lua_blocklist_repeatanimation
#define bll_sendaction lua_blocklist_sendaction
#define bll_sendside lua_blocklist_sendside
#define bll_removesprite lua_blocklist_removesprite
#define bll_updatestate lua_blocklist_updatestate
#define bll_throw lua_blocklist_throw
#define bll_dropitem lua_blocklist_dropitem
#define bll_dropitemxy lua_blocklist_dropitemxy
#define bll_sendparcel lua_blocklist_sendparcel
#define bll_objectcanmove lua_blocklist_objectcanmove
#define bll_objectcanmovefrom lua_blocklist_objectcanmovefrom
#define bll_talk lua_blocklist_talk
#define bll_talkcolor lua_blocklist_talkcolor
#define bll_playsound lua_blocklist_playsound
#define sl_sendanimation lua_sendanimation
#define sl_sendanimationxy lua_sendanimationxy

#endif /* _LUA_BLOCKLIST_H_ */
