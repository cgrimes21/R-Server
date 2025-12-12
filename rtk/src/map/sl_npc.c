/**
 * @file sl_npc.c
 * @brief NPC type Lua bindings implementation
 *
 * Extracted from sl.c as part of the RTK refactoring project.
 * Contains NPC-related Lua type implementations.
 */

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <string.h>
#include <stdlib.h>

#include "map.h"
#include "npc.h"
#include "sl.h"
#include "sl_types.h"
#include "sl_blocklist.h"
#include "sl_registry.h"
#include "sl_npc.h"

/*==============================================================================
 * NPC Type Declaration
 *============================================================================*/

typel_class npcl_type;

/* Push instance macros */
#define npcl_pushinst(state, nd) typel_pushinst(state, &npcl_type, nd, 0)
#define npcregl_pushinst(state, nd) typel_pushinst(state, &npcregl_type, nd, 0)
#define mapregl_pushinst(state, nd) typel_pushinst(state, &mapregl_type, nd, 0)
#define gameregl_pushinst(state, nd) typel_pushinst(state, &gameregl_type, nd, 0)

/*==============================================================================
 * NPC Type Implementation
 *============================================================================*/

void npcl_staticinit() {
	npcl_type = typel_new("NPC", npcl_ctor);
	npcl_type.init = npcl_init;
	npcl_type.getattr = npcl_getattr;
	npcl_type.setattr = npcl_setattr;
	typel_extendproto(&npcl_type, "move", npcl_move);
	typel_extendproto(&npcl_type, "warp", npcl_warp);
	typel_extendproto(&npcl_type, "getEquippedItem", npcl_getequippeditem);
	bll_extendproto(&npcl_type);
}

int npcl_ctor(lua_State* state) {
	NPC* nd = NULL;
	if (lua_isnumber(state, 2)) {
		unsigned int id = lua_tonumber(state, 2);
		nd = map_id2npc(id);

		if (!nd) {
			lua_pushnil(state);
			return 1; //luaL_error(state, "invalid NPC id (%d)", id);
		}
	}
	else if (lua_isstring(state, 2)) {
		nd = map_name2npc(lua_tostring(state, 2));

		if (!nd) {
			lua_pushnil(state);
			return 1; //luaL_error(state, "invalid NPC id (%d)", id);
		}
	}
	else {
		luaL_argerror(state, 1, "expected number");
	}

	npcl_pushinst(state, nd);
	return 1;
}

int npcl_init(lua_State* state, NPC* nd, int dataref, void* param) {
	sl_pushref(state, dataref);

	npcregl_pushinst(state, nd);
	lua_setfield(state, -2, "registry");

	mapregl_pushinst(state, nd);
	lua_setfield(state, -2, "mapRegistry");

	gameregl_pushinst(state, nd);
	lua_setfield(state, -2, "gameRegistry");

	lua_pop(state, 1); // pop the data table
	return 0;
}

int npcl_getattr(lua_State* state, NPC* nd, char* attrname) {
	if (bll_getattr(state, &nd->bl, attrname)) return 1;
	if (!strcmp(attrname, "id")) lua_pushnumber(state, nd->id);
	else if (!strcmp(attrname, "look")) lua_pushnumber(state, nd->bl.graphic_id);
	else if (!strcmp(attrname, "lookColor")) lua_pushnumber(state, nd->bl.graphic_color);
	else if (!strcmp(attrname, "name")) lua_pushstring(state, nd->npc_name);
	else if (!strcmp(attrname, "yname")) lua_pushstring(state, nd->name);
	else if (!strcmp(attrname, "subType")) lua_pushnumber(state, nd->bl.subtype);
	else if (!strcmp(attrname, "npcType")) lua_pushnumber(state, nd->npctype);
	else if (!strcmp(attrname, "side")) lua_pushnumber(state, nd->side);
	else if (!strcmp(attrname, "state")) lua_pushnumber(state, nd->state);
	else if (!strcmp(attrname, "sex")) lua_pushnumber(state, nd->sex);
	else if (!strcmp(attrname, "face")) lua_pushnumber(state, nd->face);
	else if (!strcmp(attrname, "faceColor")) lua_pushnumber(state, nd->face_color);
	else if (!strcmp(attrname, "hair")) lua_pushnumber(state, nd->hair);
	else if (!strcmp(attrname, "hairColor")) lua_pushnumber(state, nd->hair_color);
	else if (!strcmp(attrname, "skinColor")) lua_pushnumber(state, nd->skin_color);
	else if (!strcmp(attrname, "armorColor")) lua_pushnumber(state, nd->armor_color);
	else if (!strcmp(attrname, "lastAction")) lua_pushnumber(state, nd->lastaction);
	else if (!strcmp(attrname, "actionTime")) lua_pushnumber(state, nd->actiontime);
	else if (!strcmp(attrname, "duration")) lua_pushnumber(state, nd->duration);
	else if (!strcmp(attrname, "owner")) lua_pushnumber(state, nd->owner);
	else if (!strcmp(attrname, "startM")) lua_pushnumber(state, nd->startm);
	else if (!strcmp(attrname, "startX")) lua_pushnumber(state, nd->startx);
	else if (!strcmp(attrname, "startY")) lua_pushnumber(state, nd->starty);
	else if (!strcmp(attrname, "shopNPC")) lua_pushnumber(state, nd->shopNPC);
	else if (!strcmp(attrname, "repairNPC")) lua_pushnumber(state, nd->repairNPC);
	else if (!strcmp(attrname, "retDist")) lua_pushnumber(state, nd->retdist);
	else if (!strcmp(attrname, "returning")) lua_pushboolean(state, nd->returning);
	else if (!strcmp(attrname, "bankNPC")) lua_pushnumber(state, nd->bankNPC);
	else if (!strcmp(attrname, "gfxFace")) lua_pushnumber(state, nd->gfx.face);
	else if (!strcmp(attrname, "gfxHair")) lua_pushnumber(state, nd->gfx.hair);
	else if (!strcmp(attrname, "gfxHairC")) lua_pushnumber(state, nd->gfx.chair);
	else if (!strcmp(attrname, "gfxFaceC")) lua_pushnumber(state, nd->gfx.cface);
	else if (!strcmp(attrname, "gfxSkinC")) lua_pushnumber(state, nd->gfx.cskin);
	else if (!strcmp(attrname, "gfxDye")) lua_pushnumber(state, nd->gfx.dye);
	else if (!strcmp(attrname, "gfxTitleColor")) lua_pushnumber(state, nd->gfx.titleColor);
	else if (!strcmp(attrname, "gfxWeap")) lua_pushnumber(state, nd->gfx.weapon);
	else if (!strcmp(attrname, "gfxWeapC")) lua_pushnumber(state, nd->gfx.cweapon);
	else if (!strcmp(attrname, "gfxArmor")) lua_pushnumber(state, nd->gfx.armor);
	else if (!strcmp(attrname, "gfxArmorC")) lua_pushnumber(state, nd->gfx.carmor);
	else if (!strcmp(attrname, "gfxShield")) lua_pushnumber(state, nd->gfx.shield);
	else if (!strcmp(attrname, "gfxShiedlC")) lua_pushnumber(state, nd->gfx.cshield);
	else if (!strcmp(attrname, "gfxHelm")) lua_pushnumber(state, nd->gfx.helm);
	else if (!strcmp(attrname, "gfxHelmC")) lua_pushnumber(state, nd->gfx.chelm);
	else if (!strcmp(attrname, "gfxMantle")) lua_pushnumber(state, nd->gfx.mantle);
	else if (!strcmp(attrname, "gfxMantleC")) lua_pushnumber(state, nd->gfx.cmantle);
	else if (!strcmp(attrname, "gfxCrown")) lua_pushnumber(state, nd->gfx.crown);
	else if (!strcmp(attrname, "gfxCrownC")) lua_pushnumber(state, nd->gfx.ccrown);
	else if (!strcmp(attrname, "gfxFaceA")) lua_pushnumber(state, nd->gfx.faceAcc);
	else if (!strcmp(attrname, "gfxFaceAC")) lua_pushnumber(state, nd->gfx.cfaceAcc);
	else if (!strcmp(attrname, "gfxFaceAT")) lua_pushnumber(state, nd->gfx.faceAccT);
	else if (!strcmp(attrname, "gfxFaceATC")) lua_pushnumber(state, nd->gfx.cfaceAccT);
	else if (!strcmp(attrname, "gfxBoots")) lua_pushnumber(state, nd->gfx.boots);
	else if (!strcmp(attrname, "gfxBootsC")) lua_pushnumber(state, nd->gfx.cboots);
	else if (!strcmp(attrname, "gfxNeck")) lua_pushnumber(state, nd->gfx.necklace);
	else if (!strcmp(attrname, "gfxNeckC")) lua_pushnumber(state, nd->gfx.cnecklace);
	else if (!strcmp(attrname, "gfxName")) lua_pushstring(state, nd->gfx.name);
	else if (!strcmp(attrname, "gfxClone")) lua_pushnumber(state, nd->clone);
	else return 0;
	return 1;
}

int npcl_setattr(lua_State* state, NPC* nd, char* attrname) {
	if (bll_setattr(state, &nd->bl, attrname)) return 1;
	if (!strcmp(attrname, "side")) nd->side = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "subType")) nd->bl.subtype = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "look")) nd->bl.graphic_id = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "lookColor")) nd->bl.graphic_color = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "state")) nd->state = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "sex")) nd->sex = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "face")) nd->face = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "faceColor")) nd->face_color = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "hair")) nd->hair = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "hairColor")) nd->hair_color = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "skinColor")) nd->skin_color = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "armorColor")) nd->armor_color = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "lastAction")) nd->lastaction = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "actionTime")) nd->actiontime = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "duration")) nd->duration = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "owner")) nd->owner = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "shopNPC")) nd->shopNPC = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "repairNPC")) nd->repairNPC = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "bankNPC")) nd->bankNPC = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "retDist")) nd->retdist = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "returning")) nd->returning = lua_toboolean(state, -1);
	else if (!strcmp(attrname, "gfxFace")) nd->gfx.face = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxHair")) nd->gfx.hair = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxHairC")) nd->gfx.chair = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxFaceC")) nd->gfx.cface = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxSkinC")) nd->gfx.cskin = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxDye")) nd->gfx.dye = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxTitleColor")) nd->gfx.titleColor = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxWeap")) nd->gfx.weapon = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxWeapC")) nd->gfx.cweapon = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxArmor")) nd->gfx.armor = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxArmorC")) nd->gfx.carmor = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxShield")) nd->gfx.shield = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxShieldC")) nd->gfx.cshield = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxHelm")) nd->gfx.helm = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxHelmC")) nd->gfx.chelm = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxMantle")) nd->gfx.mantle = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxMantleC")) nd->gfx.cmantle = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxCrown")) nd->gfx.crown = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxCrownC")) nd->gfx.ccrown = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxFaceA")) nd->gfx.faceAcc = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxFaceAC")) nd->gfx.cfaceAcc = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxFaceAT")) nd->gfx.faceAccT = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxFaceATC")) nd->gfx.cfaceAccT = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxBoots")) nd->gfx.boots = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxBootsC")) nd->gfx.cboots = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxNeck")) nd->gfx.necklace = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxNeckC")) nd->gfx.cnecklace = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "gfxName")) strcpy(nd->gfx.name, lua_tostring(state, -1));
	else if (!strcmp(attrname, "gfxClone")) nd->clone = lua_tonumber(state, -1);
	else return 0;
	return 1;
}

int npcl_move(lua_State* state, NPC* nd) {
	lua_pushnumber(state, npc_move(nd));
	return 1;
}

int npcl_warp(lua_State* state, NPC* nd) {
	sl_checkargs(state, "nnn");
	unsigned short m, x, y;

	m = lua_tonumber(state, sl_memberarg(1));
	x = lua_tonumber(state, sl_memberarg(2));
	y = lua_tonumber(state, sl_memberarg(3));

	npc_warp(nd, m, x, y);
	return 0;
}

int npcl_getequippeditem(lua_State* state, NPC* nd) {
	sl_checkargs(state, "n");
	int num = lua_tonumber(state, sl_memberarg(1));
	lua_newtable(state);

	if (nd->equip[num].id != 0) {
		lua_pushnumber(state, nd->equip[num].id);
		lua_rawseti(state, -2, 1);
		lua_pushnumber(state, nd->equip[num].custom);
		lua_rawseti(state, -2, 2);
	}
	else {
		lua_pushnil(state);
	}

	return 1;
}

/*==============================================================================
 * NOTE: All NPC type implementations have been extracted from sl.c.
 *
 * To complete the extraction:
 * 1. Remove NPC type functions from sl.c (they are duplicated there)
 * 2. Add sl_npc.o to SL_OBJ in Makefile
 * 3. Add #include "sl_npc.h" to sl.h
 * 4. Rebuild and test
 *============================================================================*/
