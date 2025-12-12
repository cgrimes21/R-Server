/**
 * @file lua_registry.h
 * @brief Registry type Lua bindings
 *
 * Renamed from sl_registry.h as part of the RTK naming refactor.
 */

#ifndef _LUA_REGISTRY_H_
#define _LUA_REGISTRY_H_

#include <lua.h>
#include <lauxlib.h>
#include "map.h"
#include "lua_types.h"

/* Registry type classes */
extern lua_type_class lua_registry_type;
extern lua_type_class lua_registry_string_type;
extern lua_type_class lua_npc_int_registry_type;
extern lua_type_class lua_quest_registry_type;
extern lua_type_class lua_npc_registry_type;
extern lua_type_class lua_mob_registry_type;
extern lua_type_class lua_map_registry_type;
extern lua_type_class lua_game_registry_type;

/* Type initialization functions */
void lua_registry_staticinit(void);
void lua_registry_string_staticinit(void);
void lua_npc_int_registry_staticinit(void);
void lua_quest_registry_staticinit(void);
void lua_npc_registry_staticinit(void);
void lua_mob_registry_staticinit(void);
void lua_map_registry_staticinit(void);
void lua_game_registry_staticinit(void);

/* Player registry (int) */
int lua_registry_getattr(lua_State* state, USER* sd, char* attrname);
int lua_registry_setattr(lua_State* state, USER* sd, char* attrname);

/* Player registry (string) */
int lua_registry_string_getattr(lua_State* state, USER* sd, char* attrname);
int lua_registry_string_setattr(lua_State* state, USER* sd, char* attrname);

/* NPC int registry */
int lua_npc_int_registry_getattr(lua_State* state, USER* sd, char* attrname);
int lua_npc_int_registry_setattr(lua_State* state, USER* sd, char* attrname);

/* Quest registry */
int lua_quest_registry_getattr(lua_State* state, USER* sd, char* attrname);
int lua_quest_registry_setattr(lua_State* state, USER* sd, char* attrname);

/* NPC registry */
int lua_npc_registry_getattr(lua_State* state, NPC* nd, char* attrname);
int lua_npc_registry_setattr(lua_State* state, NPC* nd, char* attrname);

/* Mob registry */
int lua_mob_registry_getattr(lua_State* state, MOB* mob, char* attrname);
int lua_mob_registry_setattr(lua_State* state, MOB* mob, char* attrname);

/* Map registry */
int lua_map_registry_getattr(lua_State* state, USER* sd, char* attrname);
int lua_map_registry_setattr(lua_State* state, USER* sd, char* attrname);

/* Game registry */
int lua_game_registry_getattr(lua_State* state, USER* sd, char* attrname);
int lua_game_registry_setattr(lua_State* state, USER* sd, char* attrname);

/* Backward compatibility */
#define regl_type lua_registry_type
#define reglstring_type lua_registry_string_type
#define npcintregl_type lua_npc_int_registry_type
#define questregl_type lua_quest_registry_type
#define npcregl_type lua_npc_registry_type
#define mobregl_type lua_mob_registry_type
#define mapregl_type lua_map_registry_type
#define gameregl_type lua_game_registry_type

#define regl_staticinit lua_registry_staticinit
#define reglstring_staticinit lua_registry_string_staticinit
#define npcintregl_staticinit lua_npc_int_registry_staticinit
#define questregl_staticinit lua_quest_registry_staticinit
#define npcregl_staticinit lua_npc_registry_staticinit
#define mobregl_staticinit lua_mob_registry_staticinit
#define mapregl_staticinit lua_map_registry_staticinit
#define gameregl_staticinit lua_game_registry_staticinit

#define regl_getattr lua_registry_getattr
#define regl_setattr lua_registry_setattr
#define reglstring_getattr lua_registry_string_getattr
#define reglstring_setattr lua_registry_string_setattr
#define npcintregl_getattr lua_npc_int_registry_getattr
#define npcintregl_setattr lua_npc_int_registry_setattr
#define questregl_getattr lua_quest_registry_getattr
#define questregl_setattr lua_quest_registry_setattr
#define npcregl_getattr lua_npc_registry_getattr
#define npcregl_setattr lua_npc_registry_setattr
#define mobregl_getattr lua_mob_registry_getattr
#define mobregl_setattr lua_mob_registry_setattr
#define mapregl_getattr lua_map_registry_getattr
#define mapregl_setattr lua_map_registry_setattr
#define gameregl_getattr lua_game_registry_getattr
#define gameregl_setattr lua_game_registry_setattr

#endif /* _LUA_REGISTRY_H_ */
