/**
 * @file sl_registry.c
 * @brief Registry type Lua bindings implementation
 *
 * Extracted from sl.c as part of the RTK refactoring project.
 * Contains registry types for storing persistent data.
 */

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <string.h>
#include <stdlib.h>

#include "map.h"
#include "pc.h"
#include "npc.h"
#include "mob.h"
#include "lua_core.h"
#include "lua_types.h"
#include "lua_registry.h"

/*==============================================================================
 * Registry Type Declarations
 *============================================================================*/

lua_type_class lua_registry_type;
lua_type_class lua_registry_string_type;
lua_type_class lua_npc_int_registry_type;
lua_type_class lua_quest_registry_type;
lua_type_class lua_npc_registry_type;
lua_type_class lua_mob_registry_type;
lua_type_class lua_map_registry_type;
lua_type_class lua_game_registry_type;

/*==============================================================================
 * Player Registry (Integer Values)
 *============================================================================*/

void lua_registry_staticinit() {
	lua_registry_type = lua_type_new("Registry", 0);
	lua_registry_type.getattr = lua_registry_getattr;
	lua_registry_type.setattr = lua_registry_setattr;
}

int lua_registry_getattr(lua_State* state, USER* sd, char* attrname) {
	lua_pushnumber(state, pc_readglobalreg(sd, attrname));
	return 1;
}

int lua_registry_setattr(lua_State* state, USER* sd, char* attrname) {
	pc_setglobalreg(sd, attrname, lua_tonumber(state, -1));
	return 1;
}

/*==============================================================================
 * Player Registry (String Values)
 *============================================================================*/

void lua_registry_string_staticinit() {
	lua_registry_string_type = lua_type_new("RegistryString", 0);
	lua_registry_string_type.getattr = lua_registry_string_getattr;
	lua_registry_string_type.setattr = lua_registry_string_setattr;
}

int lua_registry_string_getattr(lua_State* state, USER* sd, char* attrname) {
	lua_pushstring(state, pc_readglobalregstring(sd, attrname));
	return 1;
}

int lua_registry_string_setattr(lua_State* state, USER* sd, char* attrname) {
	pc_setglobalregstring(sd, attrname, lua_tostring(state, -1));
	return 1;
}

/*==============================================================================
 * NPC Int Registry
 *============================================================================*/

void lua_npc_int_registry_staticinit() {
	lua_npc_int_registry_type = lua_type_new("NpcInt", 0);
	lua_npc_int_registry_type.getattr = lua_npc_int_registry_getattr;
	lua_npc_int_registry_type.setattr = lua_npc_int_registry_setattr;
}

int lua_npc_int_registry_getattr(lua_State* state, USER* sd, char* attrname) {
	lua_pushnumber(state, pc_readnpcintreg(sd, attrname));
	return 1;
}

int lua_npc_int_registry_setattr(lua_State* state, USER* sd, char* attrname) {
	pc_setnpcintreg(sd, attrname, lua_tonumber(state, -1));
	return 1;
}

/*==============================================================================
 * Quest Registry
 *============================================================================*/

void lua_quest_registry_staticinit() {
	lua_quest_registry_type = lua_type_new("Quest", 0);
	lua_quest_registry_type.getattr = lua_quest_registry_getattr;
	lua_quest_registry_type.setattr = lua_quest_registry_setattr;
}

int lua_quest_registry_getattr(lua_State* state, USER* sd, char* attrname) {
	lua_pushnumber(state, pc_readquestreg(sd, attrname));
	return 1;
}

int lua_quest_registry_setattr(lua_State* state, USER* sd, char* attrname) {
	pc_setquestreg(sd, attrname, lua_tonumber(state, -1));
	return 1;
}

/*==============================================================================
 * NPC Registry
 *============================================================================*/

void lua_npc_registry_staticinit() {
	lua_npc_registry_type = lua_type_new("Registry", 0);
	lua_npc_registry_type.getattr = lua_npc_registry_getattr;
	lua_npc_registry_type.setattr = lua_npc_registry_setattr;
}

int lua_npc_registry_getattr(lua_State* state, NPC* nd, char* attrname) {
	lua_pushnumber(state, npc_readglobalreg(nd, attrname));
	return 1;
}

int lua_npc_registry_setattr(lua_State* state, NPC* nd, char* attrname) {
	npc_setglobalreg(nd, attrname, lua_tonumber(state, -1));
	return 0;
}

/*==============================================================================
 * Mob Registry
 *============================================================================*/

void lua_mob_registry_staticinit() {
	lua_mob_registry_type = lua_type_new("Registry", 0);
	lua_mob_registry_type.getattr = lua_mob_registry_getattr;
	lua_mob_registry_type.setattr = lua_mob_registry_setattr;
}

int lua_mob_registry_getattr(lua_State* state, MOB* mob, char* attrname) {
	lua_pushnumber(state, mob_readglobalreg(mob, attrname));
	return 1;
}

int lua_mob_registry_setattr(lua_State* state, MOB* mob, char* attrname) {
	mob_setglobalreg(mob, attrname, lua_tonumber(state, -1));
	return 0;
}

/*==============================================================================
 * Map Registry
 *============================================================================*/

void lua_map_registry_staticinit() {
	lua_map_registry_type = lua_type_new("Mapregistry", 0);
	lua_map_registry_type.getattr = lua_map_registry_getattr;
	lua_map_registry_type.setattr = lua_map_registry_setattr;
}

int lua_map_registry_setattr(lua_State* state, USER* sd, char* attrname) {
	map_setglobalreg(sd->bl.m, attrname, lua_tonumber(state, -1));
	return 0;
}

int lua_map_registry_getattr(lua_State* state, USER* sd, char* attrname) {
	lua_pushnumber(state, map_readglobalreg(sd->bl.m, attrname));
	return 1;
}

/*==============================================================================
 * Game Registry
 *============================================================================*/

void lua_game_registry_staticinit() {
	lua_game_registry_type = lua_type_new("Gameregistry", 0);
	lua_game_registry_type.getattr = lua_game_registry_getattr;
	lua_game_registry_type.setattr = lua_game_registry_setattr;
}

int lua_game_registry_setattr(lua_State* state, USER* sd, char* attrname) {
	map_setglobalgamereg(attrname, lua_tonumber(state, -1));
	return 0;
}

int lua_game_registry_getattr(lua_State* state, USER* sd, char* attrname) {
	lua_pushnumber(state, map_readglobalgamereg(attrname));
	return 1;
}

/*==============================================================================
 * NOTE: All registry type implementations have been extracted from sl.c.
 *
 * To complete the extraction:
 * 1. Remove registry functions from sl.c (they are duplicated there)
 * 2. Add sl_registry.o to SL_OBJ in Makefile
 * 3. Add #include "lua_registry.h" to sl.h
 * 4. Rebuild and test
 *============================================================================*/
