/**
 * @file lua_types.h
 * @brief Lua type metaclass system for C-Lua bindings
 *
 * Provides the core type system infrastructure for exposing C objects to Lua.
 * Renamed from sl_types.h as part of the RTK naming refactor.
 */

#ifndef _LUA_TYPES_H_
#define _LUA_TYPES_H_

#include <lua.h>
#include <lauxlib.h>

/* Forward declarations */
struct block_list;

/* Type accessor function signatures */
typedef int (*lua_type_getattrfunc)(lua_State*, void* /* self */, char* /* attrname */);
typedef int (*lua_type_setattrfunc)(lua_State*, void* /* self */, char* /* attrname */);
typedef int (*lua_type_initfunc)(lua_State*, void* /* self */, int /* dataref */, void* /* param */);
typedef int (*lua_type_func)(lua_State*, void* /* self */);

/**
 * Type class definition
 * Represents a Lua-accessible type with getattr/setattr accessors
 */
typedef struct lua_type_class_ {
	/* luaL_ref reference to the prototype table for this class */
	int protoref;
	char* name;
	lua_type_getattrfunc getattr;
	lua_type_setattrfunc setattr;
	lua_type_initfunc init;
	/* Constructor function - first argument is always the class */
	lua_CFunction ctor;
} lua_type_class;

/**
 * Type instance
 * Represents a single instance of a type class
 */
typedef struct lua_type_inst_ {
	lua_type_class* type;
	int dataref;
	void* self;
} lua_type_inst;

/* Global metatable reference */
extern int lua_type_mtref;

/* Utility macros */
#define lua_type_topointer(state, index) (((lua_type_inst *)lua_touserdata(state, index))->self)
#define lua_type_check(state, index, type) (((lua_type_inst *)lua_touserdata(state, index))->type == type)
#define lua_memberarg(x) (x + 1)
#define lua_memberself 1
#define lua_pushref(state, ref) lua_rawgeti(state, LUA_REGISTRYINDEX, ref)

/* Type system initialization */
void lua_type_staticinit(void);
void lua_type_staticdestroy(void);

/* Type class creation */
lua_type_class lua_type_new(char* name, lua_CFunction ctor);
void lua_type_extendproto(lua_type_class* type, char* name, lua_type_func func);

/* Instance management */
void lua_type_pushinst(lua_State* state, lua_type_class* type, void* self, void* param);

/* Metatable handlers */
int lua_type_mtindex(lua_State* state);
int lua_type_mtnewindex(lua_State* state);
int lua_type_mtgc(lua_State* state);
int lua_type_boundfunc(lua_State* state);

/* Backward compatibility typedefs - to ease transition */
typedef lua_type_getattrfunc typel_getattrfunc;
typedef lua_type_setattrfunc typel_setattrfunc;
typedef lua_type_initfunc typel_initfunc;
typedef lua_type_func typel_func;
typedef lua_type_class typel_class;
typedef lua_type_inst typel_inst;

/* Backward compatibility macros */
#define typel_mtref lua_type_mtref
#define typel_topointer lua_type_topointer
#define typel_check lua_type_check
#define sl_memberarg lua_memberarg
#define sl_memberself lua_memberself
#define sl_pushref lua_pushref

/* Backward compatibility functions */
#define typel_staticinit lua_type_staticinit
#define typel_staticdestroy lua_type_staticdestroy
#define typel_new lua_type_new
#define typel_extendproto lua_type_extendproto
#define typel_pushinst lua_type_pushinst
#define typel_mtindex lua_type_mtindex
#define typel_mtnewindex lua_type_mtnewindex
#define typel_mtgc lua_type_mtgc
#define typel_boundfunc lua_type_boundfunc

#endif /* _LUA_TYPES_H_ */
