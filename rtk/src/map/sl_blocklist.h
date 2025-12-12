/**
 * @file sl_blocklist.h
 * @brief Block list Lua bindings - spatial object operations
 *
 * Extracted from sl.c as part of the RTK refactoring project.
 * Provides Lua bindings for spatial operations on block_list objects.
 */

#ifndef _SL_BLOCKLIST_H_
#define _SL_BLOCKLIST_H_

#include <lua.h>
#include <lauxlib.h>
#include <stdarg.h>
#include "map.h"
#include "sl_types.h"

/* Block list instance macros */
#define bll_pushinst_macro(state, bl) bll_pushinst(state, bl, 0)

/* Core bll functions */
void bll_pushinst(lua_State* state, struct block_list* bl, void* param);
void bll_extendproto(typel_class* class);
int bll_getattr(lua_State* state, struct block_list* bl, char* attrname);
int bll_setattr(lua_State* state, struct block_list* bl, char* attrname);

/* Object spawning/management */
int bll_spawn(lua_State* state, struct block_list* bl);
int bll_delete(lua_State* state, struct block_list* bl);
int bll_deliddb(lua_State* state, struct block_list* bl);
int bll_addnpc(lua_State* state, struct block_list* bl);
int bll_respawn(lua_State* state, struct block_list* bl);
int bll_permspawn(lua_State* state, struct block_list* bl);

/* Object queries */
int bll_getblock(lua_State* state, void* self);
int bll_getobjects(lua_State* state, void* self, int area);
int bll_getaliveobjects(lua_State* state, void* self, int area);
int bll_getobjects_area(lua_State* state, void* self);
int bll_getaliveobjects_area(lua_State* state, void* self);
int bll_getobjects_samemap(lua_State* state, void* self);
int bll_getaliveobjects_samemap(lua_State* state, void* self);
int bll_getobjects_map(lua_State* state, void* self);
int bll_getaliveobjects_map(lua_State* state, void* self);
int bll_getobjects_cell(lua_State* state, void* self);
int bll_getaliveobjects_cell(lua_State* state, void* self);
int bll_getobjects_cell_traps(lua_State* state, void* self);
int bll_getaliveobjects_cell_traps(lua_State* state, void* self);
int bll_getusers(lua_State* state, void* self);
int bll_getfriends(lua_State* state, struct block_list* bl);

/* Object query helpers */
int bll_getobjects_helper(struct block_list* bl, va_list ap);
int bll_getaliveobjects_helper(struct block_list* bl, va_list ap);
int bll_handle_mob(struct block_list* bl, va_list ap);

/* Animation/visual */
int bll_sendanim(lua_State* state, void* self);
int bll_sendanimxy(lua_State* state, void* self);
int bll_selfanimation(lua_State* state, struct block_list* bl);
int bll_selfanimationxy(lua_State* state, struct block_list* bl);
int bll_repeatanimation(lua_State* state, struct block_list* bl);
int bll_sendaction(lua_State* state, void* self);
int bll_sendside(lua_State* state, struct block_list* bl);
int bll_removesprite(lua_State* state, struct block_list* bl);
int bll_updatestate(lua_State* state, struct block_list* bl);

/* Item operations */
int bll_throw(lua_State* state, void* self);
int bll_dropitem(lua_State* state, struct block_list* bl);
int bll_dropitemxy(lua_State* state, struct block_list* bl);
int bll_sendparcel(lua_State* state, struct block_list* bl);

/* Movement */
int bll_objectcanmove(lua_State* state, struct block_list* bl);
int bll_objectcanmovefrom(lua_State* state, struct block_list* bl);

/* Communication */
int bll_talk(lua_State* state, struct block_list* bl);
int bll_talkcolor(lua_State* state, struct block_list* bl);
int bll_playsound(lua_State* state, struct block_list* bl);

/* Helper functions */
int sl_sendanimation(USER* sd, ...);
int sl_sendanimationxy(USER* sd, ...);

#endif /* _SL_BLOCKLIST_H_ */
