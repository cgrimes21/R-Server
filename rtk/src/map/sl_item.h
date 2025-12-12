/**
 * @file sl_item.h
 * @brief Item type Lua bindings
 *
 * Extracted from sl.c as part of the RTK refactoring project.
 * Contains item-related types:
 * - Item database entries
 * - Bound items (inventory/equipped)
 * - Bank items
 * - Recipes
 * - Parcels
 * - Floor items
 */

#ifndef _SL_ITEM_H_
#define _SL_ITEM_H_

#include <lua.h>
#include <lauxlib.h>
#include "map.h"
#include "sl_types.h"

/* Item type classes */
extern typel_class iteml_type;
extern typel_class biteml_type;
extern typel_class bankiteml_type;
extern typel_class recipel_type;
extern typel_class parcell_type;
extern typel_class fll_type;

/* Type initialization functions */
void iteml_staticinit(void);
void biteml_staticinit(void);
void bankiteml_staticinit(void);
void recipel_staticinit(void);
void parcell_staticinit(void);
void fll_staticinit(void);

/* Item database type - read-only item definitions */
int iteml_ctor(lua_State* state);
int iteml_getattr(lua_State* state, struct item_data* item, char* attrname);

/* Bound item type - inventory/equipped items with durability, owner, etc */
int biteml_getattr(lua_State* state, struct item* bitem, char* attrname);
int biteml_setattr(lua_State* state, struct item* bitem, char* attrname);

/* Bank item type - items stored in bank */
int bankiteml_getattr(lua_State* state, struct bank_data* bitem, char* attrname);
int bankiteml_setattr(lua_State* state, struct bank_data* bitem, char* attrname);

/* Recipe type - crafting recipes */
int recipel_ctor(lua_State* state);
int recipel_getattr(lua_State* state, struct recipe_data* recipe, char* attrname);

/* Parcel type - mailed items */
int parcell_getattr(lua_State* state, struct parcel* parcel, char* attrname);

/* Floor item type - items on the ground */
int fll_ctor(lua_State* state);
int fll_init(lua_State* state, FLOORITEM* fl, int dataref, void* param);
int fll_getattr(lua_State* state, FLOORITEM* fl, char* attrname);
int fll_setattr(lua_State* state, FLOORITEM* fl, char* attrname);
int fll_getTrapSpotters(lua_State* state, FLOORITEM* fl);
int fll_addTrapSpotters(lua_State* state, FLOORITEM* fl);

#endif /* _SL_ITEM_H_ */
