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
#include "sl.h"
#include "sl_types.h"
#include "sl_registry.h"

/*==============================================================================
 * Registry Type Declarations
 *============================================================================*/

typel_class regl_type;
typel_class reglstring_type;
typel_class npcintregl_type;
typel_class questregl_type;
typel_class npcregl_type;
typel_class mobregl_type;
typel_class mapregl_type;
typel_class gameregl_type;

/*==============================================================================
 * Player Registry (Integer Values)
 *============================================================================*/

void regl_staticinit() {
	regl_type = typel_new("Registry", 0);
	regl_type.getattr = regl_getattr;
	regl_type.setattr = regl_setattr;
}

int regl_getattr(lua_State* state, USER* sd, char* attrname) {
	lua_pushnumber(state, pc_readglobalreg(sd, attrname));
	return 1;
}

int regl_setattr(lua_State* state, USER* sd, char* attrname) {
	pc_setglobalreg(sd, attrname, lua_tonumber(state, -1));
	return 1;
}

/*==============================================================================
 * Player Registry (String Values)
 *============================================================================*/

void reglstring_staticinit() {
	reglstring_type = typel_new("RegistryString", 0);
	reglstring_type.getattr = reglstring_getattr;
	reglstring_type.setattr = reglstring_setattr;
}

int reglstring_getattr(lua_State* state, USER* sd, char* attrname) {
	lua_pushstring(state, pc_readglobalregstring(sd, attrname));
	return 1;
}

int reglstring_setattr(lua_State* state, USER* sd, char* attrname) {
	pc_setglobalregstring(sd, attrname, lua_tostring(state, -1));
	return 1;
}

/*==============================================================================
 * NPC Int Registry
 *============================================================================*/

void npcintregl_staticinit() {
	npcintregl_type = typel_new("NpcInt", 0);
	npcintregl_type.getattr = npcintregl_getattr;
	npcintregl_type.setattr = npcintregl_setattr;
}

int npcintregl_getattr(lua_State* state, USER* sd, char* attrname) {
	lua_pushnumber(state, pc_readnpcintreg(sd, attrname));
	return 1;
}

int npcintregl_setattr(lua_State* state, USER* sd, char* attrname) {
	pc_setnpcintreg(sd, attrname, lua_tonumber(state, -1));
	return 1;
}

/*==============================================================================
 * Quest Registry
 *============================================================================*/

void questregl_staticinit() {
	questregl_type = typel_new("Quest", 0);
	questregl_type.getattr = questregl_getattr;
	questregl_type.setattr = questregl_setattr;
}

int questregl_getattr(lua_State* state, USER* sd, char* attrname) {
	lua_pushnumber(state, pc_readquestreg(sd, attrname));
	return 1;
}

int questregl_setattr(lua_State* state, USER* sd, char* attrname) {
	pc_setquestreg(sd, attrname, lua_tonumber(state, -1));
	return 1;
}

/*==============================================================================
 * NPC Registry
 *============================================================================*/

void npcregl_staticinit() {
	npcregl_type = typel_new("Registry", 0);
	npcregl_type.getattr = npcregl_getattr;
	npcregl_type.setattr = npcregl_setattr;
}

int npcregl_getattr(lua_State* state, NPC* nd, char* attrname) {
	lua_pushnumber(state, npc_readglobalreg(nd, attrname));
	return 1;
}

int npcregl_setattr(lua_State* state, NPC* nd, char* attrname) {
	npc_setglobalreg(nd, attrname, lua_tonumber(state, -1));
	return 0;
}

/*==============================================================================
 * Mob Registry
 *============================================================================*/

void mobregl_staticinit() {
	mobregl_type = typel_new("Registry", 0);
	mobregl_type.getattr = mobregl_getattr;
	mobregl_type.setattr = mobregl_setattr;
}

int mobregl_getattr(lua_State* state, MOB* mob, char* attrname) {
	lua_pushnumber(state, mob_readglobalreg(mob, attrname));
	return 1;
}

int mobregl_setattr(lua_State* state, MOB* mob, char* attrname) {
	mob_setglobalreg(mob, attrname, lua_tonumber(state, -1));
	return 0;
}

/*==============================================================================
 * Map Registry
 *============================================================================*/

void mapregl_staticinit() {
	mapregl_type = typel_new("Mapregistry", 0);
	mapregl_type.getattr = mapregl_getattr;
	mapregl_type.setattr = mapregl_setattr;
}

int mapregl_setattr(lua_State* state, USER* sd, char* attrname) {
	map_setglobalreg(sd->bl.m, attrname, lua_tonumber(state, -1));
	return 0;
}

int mapregl_getattr(lua_State* state, USER* sd, char* attrname) {
	lua_pushnumber(state, map_readglobalreg(sd->bl.m, attrname));
	return 1;
}

/*==============================================================================
 * Game Registry
 *============================================================================*/

void gameregl_staticinit() {
	gameregl_type = typel_new("Gameregistry", 0);
	gameregl_type.getattr = gameregl_getattr;
	gameregl_type.setattr = gameregl_setattr;
}

int gameregl_setattr(lua_State* state, USER* sd, char* attrname) {
	map_setglobalgamereg(attrname, lua_tonumber(state, -1));
	return 0;
}

int gameregl_getattr(lua_State* state, USER* sd, char* attrname) {
	lua_pushnumber(state, map_readglobalgamereg(attrname));
	return 1;
}

/*==============================================================================
 * NOTE: All registry type implementations have been extracted from sl.c.
 *
 * To complete the extraction:
 * 1. Remove registry functions from sl.c (they are duplicated there)
 * 2. Add sl_registry.o to SL_OBJ in Makefile
 * 3. Add #include "sl_registry.h" to sl.h
 * 4. Rebuild and test
 *============================================================================*/
