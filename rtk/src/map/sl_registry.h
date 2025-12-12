/**
 * @file sl_registry.h
 * @brief Registry type Lua bindings
 *
 * Extracted from sl.c as part of the RTK refactoring project.
 * Contains registry types for storing persistent data:
 * - Player registries (int and string values)
 * - NPC registries
 * - Mob registries
 * - Quest registries
 * - Map registries
 * - Game-wide registries
 */

#ifndef _SL_REGISTRY_H_
#define _SL_REGISTRY_H_

#include <lua.h>
#include <lauxlib.h>
#include "map.h"
#include "sl_types.h"

/* Registry type classes */
extern typel_class regl_type;
extern typel_class reglstring_type;
extern typel_class npcintregl_type;
extern typel_class questregl_type;
extern typel_class npcregl_type;
extern typel_class mobregl_type;
extern typel_class mapregl_type;
extern typel_class gameregl_type;

/* Type initialization functions */
void regl_staticinit(void);
void reglstring_staticinit(void);
void npcintregl_staticinit(void);
void questregl_staticinit(void);
void npcregl_staticinit(void);
void mobregl_staticinit(void);
void mapregl_staticinit(void);
void gameregl_staticinit(void);

/* Player registry (int) - stores integer values per player */
int regl_getattr(lua_State* state, USER* sd, char* attrname);
int regl_setattr(lua_State* state, USER* sd, char* attrname);

/* Player registry (string) - stores string values per player */
int reglstring_getattr(lua_State* state, USER* sd, char* attrname);
int reglstring_setattr(lua_State* state, USER* sd, char* attrname);

/* NPC int registry - stores integer values for NPC interactions */
int npcintregl_getattr(lua_State* state, USER* sd, char* attrname);
int npcintregl_setattr(lua_State* state, USER* sd, char* attrname);

/* Quest registry - stores quest progress */
int questregl_getattr(lua_State* state, USER* sd, char* attrname);
int questregl_setattr(lua_State* state, USER* sd, char* attrname);

/* NPC registry - stores values per NPC */
int npcregl_getattr(lua_State* state, NPC* nd, char* attrname);
int npcregl_setattr(lua_State* state, NPC* nd, char* attrname);

/* Mob registry - stores values per mob */
int mobregl_getattr(lua_State* state, MOB* mob, char* attrname);
int mobregl_setattr(lua_State* state, MOB* mob, char* attrname);

/* Map registry - stores values per map */
int mapregl_getattr(lua_State* state, USER* sd, char* attrname);
int mapregl_setattr(lua_State* state, USER* sd, char* attrname);

/* Game registry - stores game-wide values */
int gameregl_getattr(lua_State* state, USER* sd, char* attrname);
int gameregl_setattr(lua_State* state, USER* sd, char* attrname);

#endif /* _SL_REGISTRY_H_ */
