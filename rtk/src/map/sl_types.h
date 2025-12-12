/**
 * @file sl_types.h
 * @brief Lua type metaclass system for C-Lua bindings
 *
 * Extracted from sl.c as part of the RTK refactoring project.
 * Provides the core type system infrastructure for exposing C objects to Lua.
 */

#ifndef _SL_TYPES_H_
#define _SL_TYPES_H_

#include <lua.h>
#include <lauxlib.h>

/* Forward declarations */
struct block_list;

/* Type accessor function signatures */
typedef int (*typel_getattrfunc)(lua_State*, void* /* self */, char* /* attrname */);
typedef int (*typel_setattrfunc)(lua_State*, void* /* self */, char* /* attrname */);
typedef int (*typel_initfunc)(lua_State*, void* /* self */, int /* dataref */, void* /* param */);
typedef int (*typel_func)(lua_State*, void* /* self */);

/**
 * Type class definition
 * Represents a Lua-accessible type with getattr/setattr accessors
 */
typedef struct typel_class_ {
	/* luaL_ref reference to the prototype table for this class */
	int protoref;
	char* name;
	typel_getattrfunc getattr;
	typel_setattrfunc setattr;
	typel_initfunc init;
	/* Constructor function - first argument is always the class */
	lua_CFunction ctor;
} typel_class;

/**
 * Type instance
 * Represents a single instance of a type class
 */
typedef struct typel_inst_ {
	typel_class* type;
	int dataref;
	void* self;
} typel_inst;

/* Global metatable reference */
extern int typel_mtref;

/* Utility macros */
#define typel_topointer(state, index) (((typel_inst *)lua_touserdata(state, index))->self)
#define typel_check(state, index, type) (((typel_inst *)lua_touserdata(state, index))->type == type)
#define sl_memberarg(x) (x + 1)
#define sl_memberself 1
#define sl_pushref(state, ref) lua_rawgeti(state, LUA_REGISTRYINDEX, ref)

/* Type system initialization */
void typel_staticinit(void);
void typel_staticdestroy(void);

/* Type class creation */
typel_class typel_new(char* name, lua_CFunction ctor);
void typel_extendproto(typel_class* type, char* name, typel_func func);

/* Instance management */
void typel_pushinst(lua_State* state, typel_class* type, void* self, void* param);

/* Metatable handlers */
int typel_mtindex(lua_State* state);
int typel_mtnewindex(lua_State* state);
int typel_mtgc(lua_State* state);
int typel_boundfunc(lua_State* state);

#endif /* _SL_TYPES_H_ */
