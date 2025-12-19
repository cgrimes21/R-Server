/**
 * @file sl_types.c
 * @brief Lua type metaclass system implementation
 *
 * Extracted from sl.c as part of the RTK refactoring project.
 * Implements the core type system for C-Lua bindings.
 */

#include "lua_types.h"
#include "lua_core.h"
#include <stdio.h>
#include <string.h>

/* Global metatable reference */
int lua_type_mtref;

/**
 * Initialize the type system
 * Creates the global metatable used by all type instances
 */
void lua_type_staticinit() {
	lua_newtable(sl_gstate); /* the global metatable */
	lua_pushcfunction(sl_gstate, lua_type_mtindex);
	lua_setfield(sl_gstate, -2, "__index");
	lua_pushcfunction(sl_gstate, lua_type_mtnewindex);
	lua_setfield(sl_gstate, -2, "__newindex");
	lua_pushcfunction(sl_gstate, lua_type_mtgc);
	lua_setfield(sl_gstate, -2, "__gc");
	lua_type_mtref = luaL_ref(sl_gstate, LUA_REGISTRYINDEX);
	printf("old pause: %d\n", lua_gc(sl_gstate, LUA_GCSETPAUSE, 100));
	printf("old mult: %d\n", lua_gc(sl_gstate, LUA_GCSETSTEPMUL, 1000));
}

/**
 * Create a new type class
 * @param name The name of the type (exposed to Lua)
 * @param ctor Constructor function (can be NULL)
 * @return The new type class
 */
lua_type_class lua_type_new(char* name, lua_CFunction ctor) {
	lua_type_class type;
	memset(&type, 0, sizeof(lua_type_class));
	/* TODO: expose the type object, not the prototype, to lua */
	lua_newtable(sl_gstate); /* the prototype table */
	if (ctor) {
		lua_newtable(sl_gstate);
		lua_pushcfunction(sl_gstate, ctor);
		lua_setfield(sl_gstate, -2, "__call");
		lua_setmetatable(sl_gstate, -2);
		type.ctor = ctor;
	}
	type.protoref = luaL_ref(sl_gstate, LUA_REGISTRYINDEX);
	sl_pushref(sl_gstate, type.protoref); /* expose the prototype table to lua */
	lua_setglobal(sl_gstate, name);
	type.name = name;
	return type;
}

/**
 * Garbage collector callback for type instances
 */
int lua_type_mtgc(lua_State* state) {
	lua_type_inst* inst = lua_touserdata(state, 1);
	struct block_list* bl = (struct block_list*)inst->self;
	luaL_unref(state, LUA_REGISTRYINDEX, inst->dataref);
	return 0;
}

/**
 * Push a new type instance onto the Lua stack
 * @param state Lua state
 * @param type The type class
 * @param self Pointer to the C object
 * @param param Optional parameter for init
 */
void lua_type_pushinst(lua_State* state, lua_type_class* type, void* self, void* param) {
	printf("[DEBUG] lua_type_pushinst: type=%p self=%p param=%p\n", (void*)type, self, param); fflush(stdout);
	printf("[DEBUG] lua_type_pushinst: type->name=%s type->protoref=%d\n",
		type ? (type->name ? type->name : "NULL") : "NULL_TYPE",
		type ? type->protoref : -999); fflush(stdout);
	lua_type_inst* inst = lua_newuserdata(state, sizeof(lua_type_inst));
	printf("[DEBUG] lua_type_pushinst: lua_newuserdata returned inst=%p\n", (void*)inst); fflush(stdout);
	inst->self = self;
	inst->type = type;
	printf("[DEBUG] lua_type_pushinst: calling sl_pushref with mtref=%d\n", lua_type_mtref); fflush(stdout);
	sl_pushref(state, lua_type_mtref); /* set the metatable */
	printf("[DEBUG] lua_type_pushinst: sl_pushref done, calling lua_setmetatable\n"); fflush(stdout);
	lua_setmetatable(state, -2);
	printf("[DEBUG] lua_type_pushinst: creating data table\n"); fflush(stdout);
	lua_newtable(state); /* create a new data table and store a ref */
	inst->dataref = luaL_ref(state, LUA_REGISTRYINDEX);
	printf("[DEBUG] lua_type_pushinst: dataref=%d, checking init\n", inst->dataref); fflush(stdout);
	if (type->init && inst->self) {
		printf("[DEBUG] lua_type_pushinst: calling type->init\n"); fflush(stdout);
		type->init(state, inst->self, inst->dataref, param);
		printf("[DEBUG] lua_type_pushinst: type->init returned\n"); fflush(stdout);
	}
	printf("[DEBUG] lua_type_pushinst: done\n"); fflush(stdout);
}

/**
 * Extend a type's prototype with a new method
 * @param type The type class to extend
 * @param name Method name
 * @param func The method implementation
 */
void lua_type_extendproto(lua_type_class* type, char* name, lua_type_func func) {
	sl_pushref(sl_gstate, type->protoref);
	lua_pushlightuserdata(sl_gstate, func);
	lua_pushcclosure(sl_gstate, lua_type_boundfunc, 1);
	lua_setfield(sl_gstate, -2, name);
	lua_pop(sl_gstate, 1); /* pop the prototype table */
}

/**
 * Bound function wrapper - calls the wrapped function with self
 */
int lua_type_boundfunc(lua_State* state) {
	lua_type_func wrapped = lua_touserdata(state, lua_upvalueindex(1));
	lua_type_inst* inst = lua_touserdata(state, 1); /* the first parameter is the userdata object */

	if (inst && inst->self) {
		return wrapped(state, inst->self);
	}
	return 0;
}

/**
 * __index metamethod for type instances
 * Handles attribute access: first checks getattr, then prototype, then data table
 */
int lua_type_mtindex(lua_State* state) {
	lua_type_inst* inst = lua_touserdata(state, 1);
	lua_type_class* type = inst->type;
	char* attrname = lua_tostring(state, 2);
	int result = 0;

	if (!inst->self) {
		lua_pushnil(state);
		return 1;
	}

	if (type->getattr)
		result = type->getattr(state, inst->self, attrname);

	if (!result) {
		/* the attribute was not handled by the getattr method */
		/* next, try the prototype table */
		sl_pushref(state, type->protoref);
		lua_getfield(state, -1, attrname);
		if (lua_isnil(state, -1)) {
			lua_pop(state, 2); /* clean up the stack */
			/* then try the data table associated with the instance */
			sl_pushref(state, inst->dataref);
			lua_getfield(state, -1, attrname);
			if (lua_isnil(state, -1)) {
				/* there is no such attribute; push nil */
				lua_pop(state, 1);
				lua_pushnil(state);
			}
		}
		lua_replace(state, -2);
	}
	return 1;
}

/**
 * __newindex metamethod for type instances
 * Handles attribute assignment via setattr
 */
int lua_type_mtnewindex(lua_State* state) {
	lua_type_inst* inst = lua_touserdata(state, 1);
	lua_type_class* type = inst->type;
	char* attrname = lua_tostring(state, 2);

	if (!inst->self) {
		return 0;
	}

	int result = 0;
	if (type->setattr)
		result = type->setattr(state, inst->self, attrname);

	/* TODO: error if not handled */
	return 0;
}
