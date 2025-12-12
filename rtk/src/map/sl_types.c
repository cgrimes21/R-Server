/**
 * @file sl_types.c
 * @brief Lua type metaclass system implementation
 *
 * Extracted from sl.c as part of the RTK refactoring project.
 * Implements the core type system for C-Lua bindings.
 */

#include "sl_types.h"
#include "sl.h"
#include <stdio.h>
#include <string.h>

/* Global metatable reference */
int typel_mtref;

/**
 * Initialize the type system
 * Creates the global metatable used by all type instances
 */
void typel_staticinit() {
	lua_newtable(sl_gstate); /* the global metatable */
	lua_pushcfunction(sl_gstate, typel_mtindex);
	lua_setfield(sl_gstate, -2, "__index");
	lua_pushcfunction(sl_gstate, typel_mtnewindex);
	lua_setfield(sl_gstate, -2, "__newindex");
	lua_pushcfunction(sl_gstate, typel_mtgc);
	lua_setfield(sl_gstate, -2, "__gc");
	typel_mtref = luaL_ref(sl_gstate, LUA_REGISTRYINDEX);
	printf("old pause: %d\n", lua_gc(sl_gstate, LUA_GCSETPAUSE, 100));
	printf("old mult: %d\n", lua_gc(sl_gstate, LUA_GCSETSTEPMUL, 1000));
}

/**
 * Create a new type class
 * @param name The name of the type (exposed to Lua)
 * @param ctor Constructor function (can be NULL)
 * @return The new type class
 */
typel_class typel_new(char* name, lua_CFunction ctor) {
	typel_class type;
	memset(&type, 0, sizeof(typel_class));
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
int typel_mtgc(lua_State* state) {
	typel_inst* inst = lua_touserdata(state, 1);
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
void typel_pushinst(lua_State* state, typel_class* type, void* self, void* param) {
	typel_inst* inst = lua_newuserdata(state, sizeof(typel_inst));
	inst->self = self;
	inst->type = type;
	sl_pushref(state, typel_mtref); /* set the metatable */
	lua_setmetatable(state, -2);
	lua_newtable(state); /* create a new data table and store a ref */
	inst->dataref = luaL_ref(state, LUA_REGISTRYINDEX);
	if (type->init && inst->self) {
		type->init(state, inst->self, inst->dataref, param);
	}
}

/**
 * Extend a type's prototype with a new method
 * @param type The type class to extend
 * @param name Method name
 * @param func The method implementation
 */
void typel_extendproto(typel_class* type, char* name, typel_func func) {
	sl_pushref(sl_gstate, type->protoref);
	lua_pushlightuserdata(sl_gstate, func);
	lua_pushcclosure(sl_gstate, typel_boundfunc, 1);
	lua_setfield(sl_gstate, -2, name);
	lua_pop(sl_gstate, 1); /* pop the prototype table */
}

/**
 * Bound function wrapper - calls the wrapped function with self
 */
int typel_boundfunc(lua_State* state) {
	typel_func wrapped = lua_touserdata(state, lua_upvalueindex(1));
	typel_inst* inst = lua_touserdata(state, 1); /* the first parameter is the userdata object */

	if (inst && inst->self) {
		return wrapped(state, inst->self);
	}
	return 0;
}

/**
 * __index metamethod for type instances
 * Handles attribute access: first checks getattr, then prototype, then data table
 */
int typel_mtindex(lua_State* state) {
	typel_inst* inst = lua_touserdata(state, 1);
	typel_class* type = inst->type;
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
int typel_mtnewindex(lua_State* state) {
	typel_inst* inst = lua_touserdata(state, 1);
	typel_class* type = inst->type;
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
