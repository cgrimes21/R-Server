/**
 * @file sl_item.c
 * @brief Item type Lua bindings implementation
 *
 * Extracted from sl.c as part of the RTK refactoring project.
 * Contains item-related Lua type implementations.
 */

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "map.h"
#include "itemdb.h"
#include "recipedb.h"
#include "class_db.h"
#include "lua_core.h"
#include "lua_types.h"
#include "lua_blocklist.h"
#include "lua_item.h"

/*==============================================================================
 * Item Type Declarations
 *============================================================================*/

lua_type_class lua_item_type;
lua_type_class lua_bound_item_type;
lua_type_class lua_bank_item_type;
lua_type_class lua_recipe_type;
lua_type_class lua_parcel_type;
lua_type_class lua_floor_item_type;

/* Push instance macros - defined here for use in this module */
#define iteml_pushinst(state, item) lua_type_pushinst(state, &lua_item_type, item, 0)
#define recipel_pushinst(state, recipe) lua_type_pushinst(state, &lua_recipe_type, recipe, 0)
#define fll_pushinst(state, fl) lua_type_pushinst(state, &lua_floor_item_type, fl, 0)

/*==============================================================================
 * Item Database Type (read-only item definitions)
 *============================================================================*/

void lua_item_staticinit() {
	lua_item_type = lua_type_new("Item", lua_item_ctor);
	lua_item_type.getattr = lua_item_getattr;
}

int lua_item_ctor(lua_State* state) {
	struct item_data* item = 0;
	if (lua_isnumber(state, 2))
		item = itemdb_search(lua_tonumber(state, 2));
	else if (lua_isstring(state, 2))
		item = itemdb_searchname(lua_tostring(state, 2));
	else luaL_argerror(state, 1, "expected string or number");
	if (!item) {
		lua_pushnil(state);
		return 1;
	}
	iteml_pushinst(state, item);
	return 1;
}

int lua_item_getattr(lua_State* state, struct item_data* item, char* attrname) {
	if (!strcmp(attrname, "vita")) lua_pushnumber(state, item->vita);
	else if (!strcmp(attrname, "mana")) lua_pushnumber(state, item->mana);
	else if (!strcmp(attrname, "dam")) lua_pushnumber(state, item->dam);
	else if (!strcmp(attrname, "price")) lua_pushnumber(state, item->price);
	else if (!strcmp(attrname, "sell")) lua_pushnumber(state, item->sell);
	else if (!strcmp(attrname, "name")) lua_pushstring(state, item->name);
	else if (!strcmp(attrname, "yname")) lua_pushstring(state, item->yname);
	else if (!strcmp(attrname, "armor")) lua_pushnumber(state, item->ac);
	else if (!strcmp(attrname, "icon")) lua_pushnumber(state, item->icon);
	else if (!strcmp(attrname, "iconC")) lua_pushnumber(state, item->icon_color);
	else if (!strcmp(attrname, "look")) lua_pushnumber(state, item->look);
	else if (!strcmp(attrname, "lookC")) lua_pushnumber(state, item->look_color);
	else if (!strcmp(attrname, "id")) lua_pushnumber(state, item->id);
	else if (!strcmp(attrname, "amount")) lua_pushnumber(state, item->amount);
	else if (!strcmp(attrname, "stackAmount")) lua_pushnumber(state, item->stack_amount);
	else if (!strcmp(attrname, "maxDura")) lua_pushnumber(state, item->dura);
	else if (!strcmp(attrname, "type")) lua_pushnumber(state, item->type);
	else if (!strcmp(attrname, "depositable")) lua_pushboolean(state, item->depositable);
	else if (!strcmp(attrname, "exchangeable")) lua_pushboolean(state, item->exchangeable);
	else if (!strcmp(attrname, "droppable")) lua_pushboolean(state, item->droppable);
	else if (!strcmp(attrname, "sound")) lua_pushnumber(state, item->sound);
	else if (!strcmp(attrname, "minSDmg")) lua_pushnumber(state, item->minSdam);
	else if (!strcmp(attrname, "maxSDmg")) lua_pushnumber(state, item->maxSdam);
	else if (!strcmp(attrname, "minLDmg")) lua_pushnumber(state, item->minLdam);
	else if (!strcmp(attrname, "maxLDmg")) lua_pushnumber(state, item->maxLdam);
	else if (!strcmp(attrname, "wisdom")) lua_pushnumber(state, item->wisdom);
	else if (!strcmp(attrname, "thrown")) lua_pushboolean(state, item->thrown);
	else if (!strcmp(attrname, "con")) lua_pushnumber(state, item->con);
	else if (!strcmp(attrname, "level")) lua_pushnumber(state, item->level);
	else if (!strcmp(attrname, "might")) lua_pushnumber(state, item->might);
	else if (!strcmp(attrname, "grace")) lua_pushnumber(state, item->grace);
	else if (!strcmp(attrname, "will")) lua_pushnumber(state, item->will);
	else if (!strcmp(attrname, "sex")) lua_pushnumber(state, item->sex);
	else if (!strcmp(attrname, "ac")) lua_pushnumber(state, item->ac);
	else if (!strcmp(attrname, "hit")) lua_pushnumber(state, item->hit);
	else if (!strcmp(attrname, "rank")) lua_pushstring(state, classdb_name(classdb_path(item->class), item->rank));
	else if (!strcmp(attrname, "maxAmount")) lua_pushnumber(state, item->max_amount);
	else if (!strcmp(attrname, "healing")) lua_pushnumber(state, item->healing);
	else if (!strcmp(attrname, "ethereal")) lua_pushboolean(state, item->ethereal);
	else if (!strcmp(attrname, "soundHit")) lua_pushnumber(state, item->sound_hit);
	else if (!strcmp(attrname, "class")) lua_pushnumber(state, item->class);
	else if (!strcmp(attrname, "baseClass")) lua_pushnumber(state, classdb_path(item->class));
	else if (!strcmp(attrname, "className")) lua_pushstring(state, classdb_name(item->class, item->rank));
	else if (!strcmp(attrname, "protection")) lua_pushnumber(state, item->protection);
	else if (!strcmp(attrname, "reqMight")) lua_pushnumber(state, item->mightreq);
	else if (!strcmp(attrname, "time")) lua_pushnumber(state, item->time);
	else if (!strcmp(attrname, "skinnable")) lua_pushnumber(state, item->skinnable);
	else if (!strcmp(attrname, "BoD")) lua_pushnumber(state, item->bod);
	else if (!strcmp(attrname, "repairable")) lua_pushnumber(state, item->repairable);
	else return 0;
	return 1;
}

/*==============================================================================
 * Bound Item Type (inventory/equipped items)
 *============================================================================*/

void lua_bound_item_staticinit() {
	lua_bound_item_type = lua_type_new("BoundItem", 0);
	lua_bound_item_type.getattr = lua_bound_item_getattr;
	lua_bound_item_type.setattr = lua_bound_item_setattr;
}

int lua_bound_item_setattr(lua_State* state, struct item* bitem, char* attrname) {
	if (!strcmp(attrname, "id")) bitem->id = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "amount")) bitem->amount = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "dura")) bitem->dura = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "protected")) bitem->protected = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "owner")) bitem->owner = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "realName")) strcpy(bitem->real_name, lua_tostring(state, -1));
	else if (!strcmp(attrname, "time")) bitem->time = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "repairCheck")) bitem->repair = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "custom")) bitem->custom = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "customLook")) bitem->customLook = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "customLookColor")) bitem->customLookColor = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "customIcon")) bitem->customIcon = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "customIconColor")) bitem->customIconColor = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "note")) strcpy(bitem->note, lua_tostring(state, -1));
	return 1;
}

int lua_bound_item_getattr(lua_State* state, struct item* bitem, char* attrname) {
	if (!strcmp(attrname, "amount")) lua_pushnumber(state, bitem->amount);
	else if (!strcmp(attrname, "dura")) lua_pushnumber(state, bitem->dura);
	else if (!strcmp(attrname, "protected")) lua_pushnumber(state, bitem->protected);
	else if (!strcmp(attrname, "owner")) lua_pushnumber(state, bitem->owner);
	else if (!strcmp(attrname, "realName")) lua_pushstring(state, bitem->real_name);
	else if (!strcmp(attrname, "time")) lua_pushnumber(state, bitem->time);
	else if (!strcmp(attrname, "repairCheck")) lua_pushnumber(state, bitem->repair);
	else if (!strcmp(attrname, "custom")) lua_pushnumber(state, bitem->custom);
	else if (!strcmp(attrname, "customLook")) lua_pushnumber(state, bitem->customLook);
	else if (!strcmp(attrname, "customLookColor")) lua_pushnumber(state, bitem->customLookColor);
	else if (!strcmp(attrname, "customIcon")) lua_pushnumber(state, bitem->customIcon);
	else if (!strcmp(attrname, "customIconColor")) lua_pushnumber(state, bitem->customIconColor);
	else if (!strcmp(attrname, "note")) lua_pushstring(state, bitem->note);
	else {
		struct item_data* item = itemdb_search(bitem->id);
		assert(item);
		return lua_item_getattr(state, item, attrname);
	}
	return 1;
}

/*==============================================================================
 * Bank Item Type
 *============================================================================*/

void lua_bank_item_staticinit() {
	lua_bank_item_type = lua_type_new("BankItem", 0);
	lua_bank_item_type.getattr = lua_bank_item_getattr;
	lua_bank_item_type.setattr = lua_bank_item_setattr;
}

int lua_bank_item_setattr(lua_State* state, struct bank_data* bitem, char* attrname) {
	if (!strcmp(attrname, "id")) bitem->item_id = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "amount")) bitem->amount = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "protected")) bitem->protected = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "owner")) bitem->owner = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "realName")) strcpy(bitem->real_name, lua_tostring(state, -1));
	else if (!strcmp(attrname, "time")) bitem->time = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "customLook")) bitem->customLook = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "customLookColor")) bitem->customLookColor = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "customIcon")) bitem->customIcon = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "customIconColor")) bitem->customIconColor = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "note")) strcpy(bitem->note, lua_tostring(state, -1));
	return 1;
}

int lua_bank_item_getattr(lua_State* state, struct bank_data* bitem, char* attrname) {
	if (!strcmp(attrname, "id")) lua_pushnumber(state, bitem->item_id);
	else if (!strcmp(attrname, "amount")) lua_pushnumber(state, bitem->amount);
	else if (!strcmp(attrname, "protected")) lua_pushnumber(state, bitem->protected);
	else if (!strcmp(attrname, "owner")) lua_pushnumber(state, bitem->owner);
	else if (!strcmp(attrname, "realName")) lua_pushstring(state, bitem->real_name);
	else if (!strcmp(attrname, "time")) lua_pushnumber(state, bitem->time);
	else if (!strcmp(attrname, "customLook")) lua_pushnumber(state, bitem->customLook);
	else if (!strcmp(attrname, "customLookColor")) lua_pushnumber(state, bitem->customLookColor);
	else if (!strcmp(attrname, "customIcon")) lua_pushnumber(state, bitem->customIcon);
	else if (!strcmp(attrname, "customIconColor")) lua_pushnumber(state, bitem->customIconColor);
	else if (!strcmp(attrname, "note")) lua_pushstring(state, bitem->note);
	else {
		struct item_data* item = itemdb_search(bitem->item_id);
		assert(item);
		return lua_item_getattr(state, item, attrname);
	}
	return 1;
}

/*==============================================================================
 * Recipe Type
 *============================================================================*/

void lua_recipe_staticinit() {
	lua_recipe_type = lua_type_new("Recipe", lua_recipe_ctor);
	lua_recipe_type.getattr = lua_recipe_getattr;
}

int lua_recipe_ctor(lua_State* state) {
	struct recipe_data* recipe = NULL;
	if (lua_isnumber(state, 2))
		recipe = recipedb_search(lua_tonumber(state, 2));
	else if (lua_isstring(state, 2))
		recipe = recipedb_searchname(lua_tostring(state, 2));
	else luaL_argerror(state, 1, "expected string or number");
	if (!recipe) {
		lua_pushnil(state);
		return 1;
	}
	recipel_pushinst(state, recipe);
	return 1;
}

int lua_recipe_getattr(lua_State* state, struct recipe_data* recipe, char* attrname) {
	if (!strcmp(attrname, "id")) lua_pushnumber(state, recipe->id);
	else if (!strcmp(attrname, "identifier")) lua_pushstring(state, recipe->identifier);
	else if (!strcmp(attrname, "description")) lua_pushstring(state, recipe->description);
	else if (!strcmp(attrname, "critIdentifier")) lua_pushstring(state, recipe->critIdentifier);
	else if (!strcmp(attrname, "critDescription")) lua_pushstring(state, recipe->critDescription);
	else if (!strcmp(attrname, "craftTime")) lua_pushnumber(state, recipe->craftTime);
	else if (!strcmp(attrname, "successRate")) lua_pushnumber(state, recipe->successRate);
	else if (!strcmp(attrname, "skillAdvance")) lua_pushnumber(state, recipe->skillAdvance);
	else if (!strcmp(attrname, "critRate")) lua_pushnumber(state, recipe->critRate);
	else if (!strcmp(attrname, "bonus")) lua_pushnumber(state, recipe->bonus);
	else if (!strcmp(attrname, "skillRequired")) lua_pushnumber(state, recipe->skillRequired);
	else if (!strcmp(attrname, "tokensRequired")) lua_pushnumber(state, recipe->tokensRequired);
	else if (!strcmp(attrname, "materials")) {
		int i = 0;
		lua_newtable(state);
		for (i = 0; i < 10; i++) {
			lua_pushnumber(state, recipe->materials[i]);
			lua_rawseti(state, -2, i + 1);
		}
	}
	else if (!strcmp(attrname, "superiorMaterials")) {
		int i = 0;
		lua_newtable(state);
		for (i = 0; i < 2; i++) {
			lua_pushnumber(state, recipe->superiorMaterials[i]);
			lua_rawseti(state, -2, i + 1);
		}
	}
	else return 0;
	return 1;
}

/*==============================================================================
 * Parcel Type
 *============================================================================*/

void lua_parcel_staticinit() {
	lua_parcel_type = lua_type_new("Parcel", 0);
	lua_parcel_type.getattr = lua_parcel_getattr;
}

int lua_parcel_getattr(lua_State* state, struct parcel* parcel, char* attrname) {
	if (!strcmp(attrname, "id")) lua_pushnumber(state, parcel->data.id);
	else if (!strcmp(attrname, "amount")) lua_pushnumber(state, parcel->data.amount);
	else if (!strcmp(attrname, "dura")) lua_pushnumber(state, parcel->data.dura);
	else if (!strcmp(attrname, "protected")) lua_pushnumber(state, parcel->data.protected);
	else if (!strcmp(attrname, "owner")) lua_pushnumber(state, parcel->data.owner);
	else if (!strcmp(attrname, "realName")) lua_pushstring(state, parcel->data.real_name);
	else if (!strcmp(attrname, "time")) lua_pushnumber(state, parcel->data.time);
	else if (!strcmp(attrname, "sender")) lua_pushnumber(state, parcel->sender);
	else if (!strcmp(attrname, "pos")) lua_pushnumber(state, parcel->pos);
	else if (!strcmp(attrname, "npcFlag")) lua_pushnumber(state, parcel->npcflag);
	else {
		struct item_data* item = itemdb_search(parcel->data.id);
		assert(item);
		return lua_item_getattr(state, item, attrname);
	}
	return 1;
}

/*==============================================================================
 * Floor Item Type
 *============================================================================*/

void lua_floor_item_staticinit() {
	lua_floor_item_type = lua_type_new("FloorItem", lua_floor_item_ctor);
	lua_floor_item_type.getattr = lua_floor_item_getattr;
	lua_floor_item_type.setattr = lua_floor_item_setattr;
	lua_floor_item_type.init = lua_floor_item_init;

	lua_type_extendproto(&lua_floor_item_type, "getTrapSpotters", lua_floor_item_getTrapSpotters);
	lua_type_extendproto(&lua_floor_item_type, "addTrapSpotters", lua_floor_item_addTrapSpotters);

	bll_extendproto(&lua_floor_item_type);
}

int lua_floor_item_ctor(lua_State* state) {
	FLOORITEM* fl = NULL;
	if (lua_isnumber(state, 2)) {
		unsigned int id = lua_tonumber(state, 2);
		fl = map_id2fl(id);
		if (!fl) {
			return luaL_error(state, "invalid floor item id (%d)", id);
		}
	}
	else {
		luaL_argerror(state, 1, "expected number");
	}
	fll_pushinst(state, fl);
	return 1;
}

int lua_floor_item_init(lua_State* state, FLOORITEM* fl, int dataref, void* param) {
	sl_pushref(state, dataref);
	lua_pop(state, 1);
	return 0;
}

int lua_floor_item_getattr(lua_State* state, FLOORITEM* fl, char* attrname) {
	if (bll_getattr(state, &fl->bl, attrname)) return 1;
	if (!strcmp(attrname, "id")) lua_pushnumber(state, fl->data.id);
	else if (!strcmp(attrname, "amount")) lua_pushnumber(state, fl->data.amount);
	else if (!strcmp(attrname, "lastAmount")) lua_pushnumber(state, fl->lastamount);
	else if (!strcmp(attrname, "owner")) lua_pushnumber(state, fl->data.owner);
	else if (!strcmp(attrname, "realName")) lua_pushstring(state, fl->data.real_name);
	else if (!strcmp(attrname, "dura")) lua_pushnumber(state, fl->data.dura);
	else if (!strcmp(attrname, "protected")) lua_pushnumber(state, fl->data.protected);
	else if (!strcmp(attrname, "custom")) lua_pushnumber(state, fl->data.custom);
	else if (!strcmp(attrname, "customIcon")) lua_pushnumber(state, fl->data.customIcon);
	else if (!strcmp(attrname, "customIconC")) lua_pushnumber(state, fl->data.customIconColor);
	else if (!strcmp(attrname, "customLook")) lua_pushnumber(state, fl->data.customLook);
	else if (!strcmp(attrname, "customLookC")) lua_pushnumber(state, fl->data.customLookColor);
	else if (!strcmp(attrname, "note")) lua_pushstring(state, fl->data.note);
	else if (!strcmp(attrname, "timer")) lua_pushnumber(state, fl->timer);
	else if (!strcmp(attrname, "looters")) {
		lua_newtable(state);
		for (int i = 0; i < MAX_GROUP_MEMBERS; i++) {
			lua_pushnumber(state, fl->looters[i]);
			lua_rawseti(state, -2, i + 1);
		}
	}
	else {
		struct item_data* item = itemdb_search(fl->data.id);
		assert(item);
		return lua_item_getattr(state, item, attrname);
	}
	return 1;
}

int lua_floor_item_setattr(lua_State* state, FLOORITEM* fl, char* attrname) {
	if (bll_setattr(state, &fl->bl, attrname)) return 1;
	if (!strcmp(attrname, "amount")) fl->data.amount = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "owner")) fl->data.owner = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "dura")) fl->data.dura = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "realName")) strcpy(fl->data.real_name, lua_tostring(state, -1));
	else if (!strcmp(attrname, "protected")) fl->data.protected = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "custom")) fl->data.custom = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "customIcon")) fl->data.customIcon = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "customIconC")) fl->data.customIconColor = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "customLook")) fl->data.customLook = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "customLookC")) fl->data.customLookColor = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "note")) strcpy(fl->data.note, lua_tostring(state, -1));
	else if (!strcmp(attrname, "timer")) fl->timer = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "looters")) {
		lua_pushnil(state);
		while (lua_next(state, sl_memberarg(2)) != 0) {
			int index = lua_tonumber(state, -2);
			fl->looters[index - 1] = lua_tonumber(state, -1);
			lua_pop(state, 1);
		}
	}
	else return 0;
	return 1;
}

int lua_floor_item_addTrapSpotters(lua_State* state, FLOORITEM* fl) {
	unsigned int playerid = lua_tonumber(state, sl_memberarg(1));

	for (int i = 0; i < 100; i++) {
		if (fl->data.trapsTable[i] == 0) {
			fl->data.trapsTable[i] = playerid;
			break;
		}
	}

	return 1;
}

int lua_floor_item_getTrapSpotters(lua_State* state, FLOORITEM* fl) {
	lua_newtable(state);

	for (int i = 0; i < 100; i++) {
		if (fl->data.trapsTable[i] == 0) continue;
		lua_pushnumber(state, fl->data.trapsTable[i]);
		lua_rawseti(state, -2, i + 1);
	}

	return 1;
}

/*==============================================================================
 * NOTE: All item type implementations have been extracted from sl.c.
 *
 * To complete the extraction:
 * 1. Remove item type functions from sl.c (they are duplicated there)
 * 2. Add sl_item.o to SL_OBJ in Makefile
 * 3. Add #include "lua_item.h" to sl.h
 * 4. Rebuild and test
 *============================================================================*/
