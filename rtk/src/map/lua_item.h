/**
 * @file lua_item.h
 * @brief Item type Lua bindings
 *
 * Renamed from sl_item.h as part of the RTK naming refactor.
 */

#ifndef _LUA_ITEM_H_
#define _LUA_ITEM_H_

#include <lua.h>
#include <lauxlib.h>
#include "map.h"
#include "lua_types.h"

/* Item type classes */
extern lua_type_class lua_item_type;
extern lua_type_class lua_bound_item_type;
extern lua_type_class lua_bank_item_type;
extern lua_type_class lua_recipe_type;
extern lua_type_class lua_parcel_type;
extern lua_type_class lua_floor_item_type;

/* Type initialization functions */
void lua_item_staticinit(void);
void lua_bound_item_staticinit(void);
void lua_bank_item_staticinit(void);
void lua_recipe_staticinit(void);
void lua_parcel_staticinit(void);
void lua_floor_item_staticinit(void);

/* Item database type */
int lua_item_ctor(lua_State* state);
int lua_item_getattr(lua_State* state, struct item_data* item, char* attrname);

/* Bound item type */
int lua_bound_item_getattr(lua_State* state, struct item* bitem, char* attrname);
int lua_bound_item_setattr(lua_State* state, struct item* bitem, char* attrname);

/* Bank item type */
int lua_bank_item_getattr(lua_State* state, struct bank_data* bitem, char* attrname);
int lua_bank_item_setattr(lua_State* state, struct bank_data* bitem, char* attrname);

/* Recipe type */
int lua_recipe_ctor(lua_State* state);
int lua_recipe_getattr(lua_State* state, struct recipe_data* recipe, char* attrname);

/* Parcel type */
int lua_parcel_getattr(lua_State* state, struct parcel* parcel, char* attrname);

/* Floor item type */
int lua_floor_item_ctor(lua_State* state);
int lua_floor_item_init(lua_State* state, FLOORITEM* fl, int dataref, void* param);
int lua_floor_item_getattr(lua_State* state, FLOORITEM* fl, char* attrname);
int lua_floor_item_setattr(lua_State* state, FLOORITEM* fl, char* attrname);
int lua_floor_item_getTrapSpotters(lua_State* state, FLOORITEM* fl);
int lua_floor_item_addTrapSpotters(lua_State* state, FLOORITEM* fl);

/* Backward compatibility */
#define iteml_type lua_item_type
#define biteml_type lua_bound_item_type
#define bankiteml_type lua_bank_item_type
#define recipel_type lua_recipe_type
#define parcell_type lua_parcel_type
#define fll_type lua_floor_item_type

#define iteml_staticinit lua_item_staticinit
#define biteml_staticinit lua_bound_item_staticinit
#define bankiteml_staticinit lua_bank_item_staticinit
#define recipel_staticinit lua_recipe_staticinit
#define parcell_staticinit lua_parcel_staticinit
#define fll_staticinit lua_floor_item_staticinit

#define iteml_ctor lua_item_ctor
#define iteml_getattr lua_item_getattr
#define biteml_getattr lua_bound_item_getattr
#define biteml_setattr lua_bound_item_setattr
#define bankiteml_getattr lua_bank_item_getattr
#define bankiteml_setattr lua_bank_item_setattr
#define recipel_ctor lua_recipe_ctor
#define recipel_getattr lua_recipe_getattr
#define parcell_getattr lua_parcel_getattr
#define fll_ctor lua_floor_item_ctor
#define fll_init lua_floor_item_init
#define fll_getattr lua_floor_item_getattr
#define fll_setattr lua_floor_item_setattr
#define fll_getTrapSpotters lua_floor_item_getTrapSpotters
#define fll_addTrapSpotters lua_floor_item_addTrapSpotters

#endif /* _LUA_ITEM_H_ */
