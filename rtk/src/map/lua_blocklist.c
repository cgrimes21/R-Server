/**
 * @file sl_blocklist.c
 * @brief Block list Lua bindings - spatial object operations
 *
 * Extracted from sl.c as part of the RTK refactoring project.
 * Provides Lua bindings for spatial operations on block_list objects.
 */

#include "lua_blocklist.h"
#include "lua_core.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "client.h"
#include "pc.h"
#include "mob.h"
#include "itemdb.h"
#include "npc.h"
#include "socket.h"
#include "malloc.h"
#include "db_mysql.h"

/* External type references from sl.c */
extern lua_type_class pcl_type;
extern lua_type_class mobl_type;
extern lua_type_class npcl_type;
extern lua_type_class fll_type;

/* External function references */
extern void sl_checkargs(lua_State*, char* fmt);
extern int sl_doscript_blargs(char*, char*, int, ...);
extern unsigned int* mobspawn_onetime(int, int, int, int, unsigned int, int, int, int, unsigned int);
extern void mob_respawn(MOB*);
extern int mobdb_id(const char*);
extern void mobdb_dropitem(unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, USER*);
extern int client_object_can_move_from(int, int, int, int);

/* Macros from sl.c */
#define sl_memberself 1
#define sl_memberarg(i) (sl_memberself + i)

/*============================================================
 * Object Spawning
 *============================================================*/

int lua_blocklist_spawn(lua_State* state, struct block_list* bl) {
	int i = 0;
	int mob = 0;

	if (lua_isnumber(state, sl_memberarg(1))) {
		sl_checkargs(state, "nnnn");
		mob = lua_tonumber(state, sl_memberarg(1));
	}
	else {
		sl_checkargs(state, "snnn");
		mob = mobdb_id(lua_tostring(state, sl_memberarg(1)));
	}

	int x = lua_tonumber(state, sl_memberarg(2));
	int y = lua_tonumber(state, sl_memberarg(3));
	unsigned int amount = lua_tonumber(state, sl_memberarg(4));
	int m = lua_tonumber(state, sl_memberarg(5));
	unsigned int owner = lua_tonumber(state, sl_memberarg(6));

	unsigned int* spawnedmobs = NULL;
	lua_newtable(state);

	if (m != 0) {
		spawnedmobs = mobspawn_onetime(mob, m, x, y, amount, 0, 0, 0, owner);

		for (i = 1; i <= amount; i++) {
			lua_blocklist_pushinst(state, map_id2bl(spawnedmobs[i - 1]), 0);
			lua_rawseti(state, -2, i);
		}
	}
	else {
		spawnedmobs = mobspawn_onetime(mob, bl->m, x, y, amount, 0, 0, 0, owner);

		for (i = 1; i <= amount; i++) {
			lua_blocklist_pushinst(state, map_id2bl(spawnedmobs[i - 1]), 0);
			lua_rawseti(state, -2, i);
		}
	}

	FREE(spawnedmobs);
	return 1;
}

int lua_blocklist_delete(lua_State* state, struct block_list* bl) {
	nullpo_ret(0, bl);

	if (bl->type == BL_PC) {
		return 0;
	}

	map_delblock(bl);
	map_deliddb(bl);

	if (bl->id > 0) {
		client_look_gone(bl);
		FREE(bl);
	}

	return 0;
}

int lua_blocklist_deliddb(lua_State* state, struct block_list* bl) {
	map_deliddb(bl);
	return 0;
}

int lua_blocklist_addnpc(lua_State* state, struct block_list* bl) {
	sl_checkargs(state, "snnn");
	struct npc_data* nd = NULL;
	int timer = 0;
	int duration = 0;
	int owner = 0;
	int movetime = 0;

	timer = lua_tonumber(state, sl_memberarg(6));
	duration = lua_tonumber(state, sl_memberarg(7));
	owner = lua_tonumber(state, sl_memberarg(8));
	movetime = lua_tonumber(state, sl_memberarg(9));
	CALLOC(nd, struct npc_data, 1);

	strcpy(nd->name, lua_tostring(state, sl_memberarg(1)));
	if (lua_isstring(state, sl_memberarg(9))) {
		strcpy(nd->npc_name, lua_tostring(state, sl_memberarg(9)));
	}
	else {
		strcpy(nd->npc_name, "nothing");
	}
	nd->bl.type = BL_NPC;
	nd->bl.subtype = lua_tonumber(state, sl_memberarg(5));
	nd->bl.m = lua_tonumber(state, sl_memberarg(2));
	nd->bl.x = lua_tonumber(state, sl_memberarg(3));
	nd->bl.y = lua_tonumber(state, sl_memberarg(4));
	nd->bl.graphic_id = 0;
	nd->bl.graphic_color = 0;
	nd->bl.id = npc_get_new_npctempid();
	nd->bl.next = NULL;
	nd->bl.prev = NULL;
	nd->actiontime = timer;
	nd->duration = duration;
	nd->owner = owner;
	nd->movetime = movetime;
	map_addblock(&nd->bl);
	map_addiddb(&nd->bl);

	sl_doscript_blargs(nd->name, "on_spawn", 1, &nd->bl);
	return 0;
}

int lua_blocklist_respawn(lua_State* state, struct block_list* bl) {
	int m = bl->m;

	map_foreachinarea(lua_blocklist_handle_mob, m, 1, 1, SAMEMAP, BL_MOB);
	return 0;
}

int lua_blocklist_permspawn(lua_State* state, struct block_list* bl) {
	return 0;
}

/*============================================================
 * Object Query Helpers
 *============================================================*/

int lua_blocklist_getobjects_helper(struct block_list* bl, va_list ap) {
	lua_State* state = va_arg(ap, lua_State*);
	MOB* mob = NULL;
	USER* ptr = NULL;
	int* def = va_arg(ap, int*);
	nullpo_ret(0, bl);
	if (bl->type == BL_MOB) {
		mob = (MOB*)bl;
		if (mob->state == MOB_DEAD) {
			return 0;
		}
	}
	if (bl->type == BL_PC) {
		ptr = (USER*)bl;
		if (ptr && (ptr->optFlags & optFlag_stealth))
			return 0;
	}

	def[0]++;
	lua_blocklist_pushinst(state, bl, 0);
	lua_rawseti(state, -2, def[0]);
	return 0;
}

int lua_blocklist_getaliveobjects_helper(struct block_list* bl, va_list ap) {
	lua_State* state = va_arg(ap, lua_State*);
	MOB* mob = NULL;
	USER* ptr = NULL;
	int* def = va_arg(ap, int*);
	nullpo_ret(0, bl);
	if (bl->type == BL_MOB) {
		mob = (MOB*)bl;
		if (mob->state == MOB_DEAD) {
			return 0;
		}
	}
	if (bl->type == BL_PC) {
		ptr = (USER*)bl;
		if (ptr && (ptr->optFlags & optFlag_stealth) || ptr->status.state == 1)
			return 0;
	}

	def[0]++;
	lua_blocklist_pushinst(state, bl, 0);
	lua_rawseti(state, -2, def[0]);
	return 0;
}

int lua_blocklist_handle_mob(struct block_list* bl, va_list ap) {
	MOB* mob = (MOB*)bl;

	if (mob && mob->state == MOB_DEAD && mob->onetime == 0) {
		mob_respawn(mob);
	}
	return 0;
}

/*============================================================
 * Object Queries
 *============================================================*/

int lua_blocklist_getaliveobjects(lua_State* state, void* self, int area) {
	sl_checkargs(state, "n");
	struct block_list* bl = (struct block_list*)self;
	int def[1];
	def[0] = 0;

	lua_newtable(state);
	int type = lua_tonumber(state, sl_memberarg(1));
	map_foreachinarea(lua_blocklist_getaliveobjects_helper, bl->m, bl->x, bl->y, area, type, state, def);
	return 1;
}

int lua_blocklist_getobjects(lua_State* state, void* self, int area) {
	sl_checkargs(state, "n");
	struct block_list* bl = (struct block_list*)self;
	int def[1];
	def[0] = 0;

	lua_newtable(state);
	int type = lua_tonumber(state, sl_memberarg(1));
	map_foreachinarea(lua_blocklist_getobjects_helper, bl->m, bl->x, bl->y, area, type, state, def);
	return 1;
}

int lua_blocklist_getobjects_cell(lua_State* state, void* self) {
	sl_checkargs(state, "nnnn");
	int m = lua_tonumber(state, sl_memberarg(1)),
		x = lua_tonumber(state, sl_memberarg(2)),
		y = lua_tonumber(state, sl_memberarg(3)),
		type = lua_tonumber(state, sl_memberarg(4));
	int def[1];
	def[0] = 0;
	lua_newtable(state);
	map_foreachincell(lua_blocklist_getobjects_helper, m, x, y, type, state, def);
	return 1;
}

int lua_blocklist_getaliveobjects_cell(lua_State* state, void* self) {
	sl_checkargs(state, "nnnn");
	int m = lua_tonumber(state, sl_memberarg(1)),
		x = lua_tonumber(state, sl_memberarg(2)),
		y = lua_tonumber(state, sl_memberarg(3)),
		type = lua_tonumber(state, sl_memberarg(4));
	int def[1];
	def[0] = 0;
	lua_newtable(state);
	map_foreachincell(lua_blocklist_getaliveobjects_helper, m, x, y, type, state, def);
	return 1;
}

int lua_blocklist_getobjects_cell_traps(lua_State* state, void* self) {
	sl_checkargs(state, "nnnn");
	int m = lua_tonumber(state, sl_memberarg(1)),
		x = lua_tonumber(state, sl_memberarg(2)),
		y = lua_tonumber(state, sl_memberarg(3)),
		type = lua_tonumber(state, sl_memberarg(4));
	int def[1];
	def[0] = 0;
	lua_newtable(state);
	map_foreachincellwithtraps(lua_blocklist_getobjects_helper, m, x, y, type, state, def);
	return 1;
}

int lua_blocklist_getaliveobjects_cell_traps(lua_State* state, void* self) {
	sl_checkargs(state, "nnnn");
	int m = lua_tonumber(state, sl_memberarg(1)),
		x = lua_tonumber(state, sl_memberarg(2)),
		y = lua_tonumber(state, sl_memberarg(3)),
		type = lua_tonumber(state, sl_memberarg(4));
	int def[1];
	def[0] = 0;
	lua_newtable(state);
	map_foreachincellwithtraps(lua_blocklist_getaliveobjects_helper, m, x, y, type, state, def);
	return 1;
}

int lua_blocklist_getobjects_area(lua_State* state, void* self) {
	return lua_blocklist_getobjects(state, self, AREA);
}

int lua_blocklist_getaliveobjects_area(lua_State* state, void* self) {
	return lua_blocklist_getaliveobjects(state, self, AREA);
}

int lua_blocklist_getobjects_samemap(lua_State* state, void* self) {
	return lua_blocklist_getobjects(state, self, SAMEMAP);
}

int lua_blocklist_getaliveobjects_samemap(lua_State* state, void* self) {
	return lua_blocklist_getaliveobjects(state, self, SAMEMAP);
}

int lua_blocklist_getobjects_map(lua_State* state, void* self) {
	sl_checkargs(state, "nn");
	struct block_list* bl = (struct block_list*)self;
	int def[1];
	def[0] = 0;

	lua_newtable(state);
	int m = lua_tonumber(state, sl_memberarg(1));
	int type = lua_tonumber(state, sl_memberarg(2));
	map_foreachinarea(lua_blocklist_getobjects_helper, m, bl->x, bl->y, SAMEMAP, type, state, def);

	return 1;
}

int lua_blocklist_getaliveobjects_map(lua_State* state, void* self) {
	sl_checkargs(state, "nn");
	struct block_list* bl = (struct block_list*)self;
	int def[1];
	def[0] = 0;

	lua_newtable(state);
	int m = lua_tonumber(state, sl_memberarg(1));
	int type = lua_tonumber(state, sl_memberarg(2));
	map_foreachinarea(lua_blocklist_getaliveobjects_helper, m, bl->x, bl->y, SAMEMAP, type, state, def);

	return 1;
}

int lua_blocklist_getblock(lua_State* state, void* self) {
	sl_checkargs(state, "n");
	struct block_list* bl = map_id2bl(lua_tonumber(state, sl_memberarg(1)));

	if (bl) {
		lua_blocklist_pushinst(state, bl, 0);
		return 1;
	}
	else {
		lua_pushnil(state);
		return 1;
	}
}

int lua_blocklist_getusers(lua_State* state, void* self) {
	USER* tsd = NULL;
	struct socket_data* p;
	int i;
	lua_newtable(state);
	for (i = 0; i < fd_max; i++) {
		p = session[i];
		if (p && (tsd = p->session_data)) {
			lua_blocklist_pushinst(state, &tsd->bl, 0);
			lua_rawseti(state, -2, lua_objlen(state, -2) + 1);
		}
	}
	return 1;
}

int lua_blocklist_getfriends(lua_State* state, struct block_list* bl) {
	int i, id, numFriends;
	USER* sd = (USER*)bl;
	USER* tsd = NULL;
	lua_newtable(state);
	SqlStmt* stmt = SqlStmt_Malloc(sql_handle);

	if (stmt == NULL)
	{
		SqlStmt_ShowDebug(stmt);
		return 0;
	}

	if (bl->type != BL_PC)
		return 0;

	if (SQL_ERROR == SqlStmt_Prepare(stmt, "SELECT `FndChaId` FROM `Friends` JOIN `Character` ON `FndChaId` = `ChaId` AND `ChaOnline` = '1' WHERE `FndChaName1` = '%s' OR `FndChaName2` = '%s' OR `FndChaName3` = '%s' OR `FndChaName4` = '%s'"
		"OR `FndChaName5` = '%s' OR `FndChaName6` = '%s' OR `FndChaName7` = '%s' OR `FndChaName8` = '%s' OR `FndChaName9` = '%s' OR `FndChaName10` = '%s' OR `FndChaName11` = '%s' OR `FndChaName12` = '%s' OR `FndChaName13` = '%s'"
		"OR `FndChaName14` = '%s' OR `FndChaName15` = '%s' OR `FndChaName16` = '%s' OR `FndChaName17` = '%s' OR `FndChaName18` = '%s' OR `FndChaName19` = '%s' OR `FndChaName20` = '%s'", sd->status.name, sd->status.name, sd->status.name,
		sd->status.name, sd->status.name, sd->status.name, sd->status.name, sd->status.name, sd->status.name, sd->status.name, sd->status.name, sd->status.name, sd->status.name, sd->status.name, sd->status.name, sd->status.name, sd->status.name,
		sd->status.name, sd->status.name, sd->status.name)
		|| SQL_ERROR == SqlStmt_Execute(stmt)
		|| SQL_ERROR == SqlStmt_BindColumn(stmt, 0, SQLDT_UINT, &id, 0, NULL, NULL))
	{
		SqlStmt_ShowDebug(stmt);
		SqlStmt_Free(stmt);
		return 0;
	}

	numFriends = SqlStmt_NumRows(stmt);

	for (i = 1; i <= numFriends && SQL_SUCCESS == SqlStmt_NextRow(stmt); i++)
	{
		tsd = map_id2sd(id);

		if (tsd)
		{
			lua_blocklist_pushinst(state, &tsd->bl, 0);
			lua_rawseti(state, -2, i);
		}
	}

	SqlStmt_Free(stmt);
	return 1;
}

/*============================================================
 * Animation/Visual
 *============================================================*/

int lua_blocklist_sendanimxy(lua_State* state, void* self) {
	sl_checkargs(state, "nnn");
	struct block_list* bl = (struct block_list*)self;
	int anim = lua_tonumber(state, sl_memberarg(1));
	int x = lua_tonumber(state, sl_memberarg(2));
	int y = lua_tonumber(state, sl_memberarg(3));
	int times = lua_tonumber(state, sl_memberarg(4));

	map_foreachinarea(client_send_animation_xy, bl->m, bl->x, bl->y, AREA, BL_PC, anim, times, x, y);
	return 0;
}

int lua_blocklist_sendanim(lua_State* state, void* self) {
	sl_checkargs(state, "n");
	struct block_list* bl = (struct block_list*)self;
	int anim = lua_tonumber(state, sl_memberarg(1));
	int times = lua_tonumber(state, sl_memberarg(2));

	map_foreachinarea(client_send_animation, bl->m, bl->x, bl->y, AREA, BL_PC, anim, bl, times);
	return 0;
}

int lua_sendanimationxy(USER* sd, ...) {
	va_list ap;

	va_start(ap, sd);
	client_send_animation_xy(&sd->bl, ap);
	va_end(ap);
	return 0;
}

int lua_sendanimation(USER* sd, ...) {
	va_list ap;

	va_start(ap, sd);
	client_send_animation(&sd->bl, ap);
	va_end(ap);
	return 0;
}

int lua_blocklist_selfanimationxy(lua_State* state, struct block_list* bl) {
	USER* sd = NULL;
	int anim = lua_tonumber(state, sl_memberarg(2));
	int x = lua_tonumber(state, sl_memberarg(3));
	int y = lua_tonumber(state, sl_memberarg(4));
	int times = lua_tonumber(state, sl_memberarg(5));

	if (lua_isnumber(state, sl_memberarg(1))) {
		sl_checkargs(state, "nnnn");
		sd = map_id2sd(lua_tonumber(state, sl_memberarg(1)));
	}
	else {
		sl_checkargs(state, "snnn");
		sd = map_name2sd(lua_tostring(state, sl_memberarg(1)));
	}

	lua_sendanimationxy(&sd->bl, anim, times, x, y);
	return 0;
}

int lua_blocklist_selfanimation(lua_State* state, struct block_list* bl) {
	USER* sd = NULL;
	int anim = lua_tonumber(state, sl_memberarg(2));
	int times = lua_tonumber(state, sl_memberarg(3));

	if (lua_isnumber(state, sl_memberarg(1))) {
		sl_checkargs(state, "nn");
		sd = map_id2sd(lua_tonumber(state, sl_memberarg(1)));
	}
	else {
		sl_checkargs(state, "sn");
		sd = map_name2sd(lua_tostring(state, sl_memberarg(1)));
	}

	lua_sendanimation(&sd->bl, anim, bl, times);
	return 0;
}

int lua_blocklist_repeatanimation(lua_State* state, struct block_list* bl) {
	sl_checkargs(state, "nn");
	int anim = lua_tonumber(state, sl_memberarg(1));
	int duration = lua_tonumber(state, sl_memberarg(2));
	if (duration > 0) {
		duration /= 1000;
	}
	map_foreachinarea(client_send_animation, bl->m, bl->x, bl->y, AREA, BL_PC, anim, bl, duration);
	return 0;
}

int lua_blocklist_sendaction(lua_State* state, void* self) {
	sl_checkargs(state, "nn");
	struct block_list* bl = (struct block_list*)self;
	int action = lua_tonumber(state, sl_memberarg(1));
	int time = lua_tonumber(state, sl_memberarg(2));

	client_send_action(bl, action, time, 0);
	return 0;
}

int lua_blocklist_sendside(lua_State* state, struct block_list* bl) {
	client_send_side(bl);
	return 0;
}

int lua_blocklist_removesprite(lua_State* state, struct block_list* bl) {
	client_look_gone(bl);
	return 1;
}

int lua_blocklist_updatestate(lua_State* state, struct block_list* bl) {
	USER* sd = NULL;
	MOB* mob = NULL;
	NPC* npc = NULL;

	if (bl->type == BL_PC) {
		sd = (USER*)bl;
		map_foreachinarea(client_update_state, sd->bl.m, sd->bl.x, sd->bl.y, AREA, BL_PC, sd);
	}
	else if (bl->type == BL_MOB) {
		mob = (MOB*)bl;

		if (mob->data->subtype == 1) {
			map_foreachinarea(client_mob_look_sub, mob->bl.m, mob->bl.x, mob->bl.y, AREA, BL_PC, LOOK_SEND, mob);
		}
		else {
			map_foreachinarea(client_object_look_sub2, mob->bl.m, mob->bl.x, mob->bl.y, AREA, BL_PC, LOOK_SEND, mob);
		}
	}
	else if (bl->type == BL_NPC) {
		npc = (NPC*)bl;

		if (npc->npctype == 1) {
			map_foreachinarea(client_npc_look_sub, npc->bl.m, npc->bl.x, npc->bl.y, AREA, BL_PC, LOOK_SEND, npc);
		}
		else {
			map_foreachinarea(client_object_look_sub2, npc->bl.m, npc->bl.x, npc->bl.y, AREA, BL_PC, LOOK_SEND, npc);
		}
	}
	else {
		lua_pushboolean(state, 0);
		return 0;
	}

	lua_pushboolean(state, 1);
	return 1;
}

/*============================================================
 * Item Operations
 *============================================================*/

int lua_blocklist_throw(lua_State* state, void* self) {
	sl_checkargs(state, "nnnnn");
	int x, y, icon, color, action;
	char buf[255];
	x = lua_tonumber(state, sl_memberarg(1));
	y = lua_tonumber(state, sl_memberarg(2));
	icon = lua_tonumber(state, sl_memberarg(3));
	color = lua_tonumber(state, sl_memberarg(4));
	action = lua_tonumber(state, sl_memberarg(5));
	struct block_list* bl = (struct block_list*)self;

	WBUFB(buf, 0) = 0xAA;
	WBUFW(buf, 1) = SWAP16(0x1B);
	WBUFB(buf, 3) = 0x16;
	WBUFB(buf, 4) = 0x03;
	WBUFL(buf, 5) = SWAP32(bl->id);
	WBUFW(buf, 9) = SWAP16(icon + 49152);
	WBUFB(buf, 11) = color;
	WBUFL(buf, 12) = 0;
	WBUFW(buf, 16) = SWAP16(bl->x);
	WBUFW(buf, 18) = SWAP16(bl->y);
	WBUFW(buf, 20) = SWAP16(x);
	WBUFW(buf, 22) = SWAP16(y);
	WBUFL(buf, 24) = 0;
	WBUFB(buf, 28) = action;
	WBUFB(buf, 29) = 0x00;
	client_send(buf, 30, bl, SAMEAREA);
	return 0;
}

int lua_blocklist_dropitem(lua_State* state, struct block_list* bl) {
	unsigned int id;
	unsigned int amount;
	int dura;
	int protected;

	USER* sd = NULL;
	if (lua_gettop(state) == 4)
		sd = map_id2sd(lua_tonumber(state, 4));

	if (lua_isnumber(state, sl_memberarg(1))) {
		sl_checkargs(state, "nn");
		id = lua_tonumber(state, sl_memberarg(1));
	}
	else {
		sl_checkargs(state, "sn");
		id = itemdb_id(lua_tostring(state, sl_memberarg(1)));
	}

	amount = lua_tonumber(state, sl_memberarg(2));
	dura = lua_tonumber(state, sl_memberarg(3));
	protected = lua_tonumber(state, sl_memberarg(4));

	if (dura == 0) dura = itemdb_dura(id);
	if (protected == 0) protected = itemdb_protected(id);

	mobdb_dropitem(bl->id, id, amount, dura, protected, 0, bl->m, bl->x, bl->y, sd);
	return 0;
}

int lua_blocklist_dropitemxy(lua_State* state, struct block_list* bl) {
	unsigned int id;
	unsigned int amount;
	int dura;
	int map;
	int protected;

	int x;
	int y;
	USER* sd = NULL;
	if (lua_gettop(state) == 7)
		sd = map_id2sd(lua_tonumber(state, 7));

	if (lua_isnumber(state, sl_memberarg(1))) {
		sl_checkargs(state, "nn");
		id = lua_tonumber(state, sl_memberarg(1));
	}
	else {
		sl_checkargs(state, "sn");
		id = itemdb_id(lua_tostring(state, sl_memberarg(1)));
	}

	amount = lua_tonumber(state, sl_memberarg(2));
	dura = lua_tonumber(state, sl_memberarg(3));
	protected = lua_tonumber(state, sl_memberarg(4));

	if (dura == 0) dura = itemdb_dura(id);
	if (protected == 0) protected = itemdb_protected(id);

	map = lua_tonumber(state, sl_memberarg(5));
	x = lua_tonumber(state, sl_memberarg(6));
	y = lua_tonumber(state, sl_memberarg(7));

	mobdb_dropitem(bl->id, id, amount, dura, protected, 0, map, x, y, sd);
	return 0;
}

int lua_blocklist_sendparcel(lua_State* state, struct block_list* bl) {
	sl_checkargs(state, "nnn");
	int x;
	int pos = -1;
	int newest = -1;
	char escape[255];

	unsigned int customLookColor = 0;
	unsigned int customIconColor = 0;

	int customLook = 0;
	int customIcon = 0;

	unsigned int protected = 0;
	unsigned int dura = 0;

	int receiver = lua_tonumber(state, sl_memberarg(1));
	int sender = lua_tonumber(state, sl_memberarg(2));
	unsigned int item = lua_tonumber(state, sl_memberarg(3));
	unsigned int amount = lua_tonumber(state, sl_memberarg(4));
	int owner = lua_tonumber(state, sl_memberarg(5));
	char* engrave = lua_tostring(state, sl_memberarg(6));
	char npcflag = lua_tonumber(state, sl_memberarg(7));
	customLook = lua_tonumber(state, sl_memberarg(8));
	customLookColor = lua_tonumber(state, sl_memberarg(9));
	customIcon = lua_tonumber(state, sl_memberarg(10));
	customIconColor = lua_tonumber(state, sl_memberarg(11));
	protected = lua_tonumber(state, sl_memberarg(12));
	dura = lua_tonumber(state, sl_memberarg(13));

	SqlStmt* stmt = SqlStmt_Malloc(sql_handle);

	if (stmt == NULL) {
		SqlStmt_ShowDebug(stmt);
		lua_pushboolean(state, 0);
		return 1;
	}

	if (SQL_ERROR == SqlStmt_Prepare(stmt, "SELECT `ParPosition` FROM `Parcels` WHERE `ParChaIdDestination` = '%u'", receiver)
		|| SQL_ERROR == SqlStmt_Execute(stmt)
		|| SQL_ERROR == SqlStmt_BindColumn(stmt, 0, SQLDT_INT, &pos, 0, NULL, NULL)) {
		SqlStmt_ShowDebug(stmt);
		SqlStmt_Free(stmt);
		lua_pushboolean(state, 0);
		return 1;
	}

	if (SqlStmt_NumRows(stmt) > 0) {
		for (x = 0; x < SqlStmt_NumRows(stmt) && SQL_SUCCESS == SqlStmt_NextRow(stmt); x++) {
			if (pos > newest) {
				newest = pos;
			}
		}
	}

	newest += 1;
	SqlStmt_Free(stmt);
	Sql_EscapeString(sql_handle, escape, engrave);

	if (SQL_ERROR == Sql_Query(sql_handle, "INSERT INTO `Parcels` (`ParChaIdDestination`, `ParSender`, `ParItmId`, `ParAmount`, `ParChaIdOwner`, `ParEngrave`, `ParPosition`, `ParNpc`, `ParCustomLook`, `ParCustomLookColor`, `ParCustomIcon`, `ParCustomIconColor`, `ParProtected`, `ParItmDura`) VALUES ('%u', '%u', '%u', '%u', '%u', '%s', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%u')",
		receiver, sender, item, amount, owner, escape, newest, npcflag, customLook, customLookColor, customIcon, customIconColor, protected, dura)) {
		Sql_ShowDebug(sql_handle);
		lua_pushboolean(state, 0);
		return 1;
	}

	lua_pushboolean(state, 1);
	return 1;
}

/*============================================================
 * Movement
 *============================================================*/

int lua_blocklist_objectcanmove(lua_State* state, struct block_list* bl) {
	sl_checkargs(state, "nnn");
	int x = lua_tonumber(state, sl_memberarg(1));
	int y = lua_tonumber(state, sl_memberarg(2));
	int side = lua_tonumber(state, sl_memberarg(3));
	int canmove = client_object_can_move(bl->m, x, y, side);

	if (canmove) {
		lua_pushboolean(state, 0);
	}
	else {
		lua_pushboolean(state, 1);
	}
	return 1;
}

int lua_blocklist_objectcanmovefrom(lua_State* state, struct block_list* bl) {
	sl_checkargs(state, "nnn");
	int x = lua_tonumber(state, sl_memberarg(1));
	int y = lua_tonumber(state, sl_memberarg(2));
	int side = lua_tonumber(state, sl_memberarg(3));
	int canmove = client_object_can_move_from(bl->m, x, y, side);

	if (canmove) {
		lua_pushboolean(state, 0);
	}
	else {
		lua_pushboolean(state, 1);
	}
	return 1;
}

/*============================================================
 * Communication
 *============================================================*/

int lua_blocklist_playsound(lua_State* state, struct block_list* bl) {
	sl_checkargs(state, "n");
	int sound = lua_tonumber(state, sl_memberarg(1));

	client_play_sound(bl, sound);
	return 0;
}

int lua_blocklist_talk(lua_State* state, struct block_list* bl) {
	sl_checkargs(state, "ns");
	int type = lua_tonumber(state, sl_memberarg(1));
	char* msg = lua_tostring(state, sl_memberarg(2));

	map_foreachinarea(client_speak, bl->m, bl->x, bl->y, AREA, BL_PC, msg, bl, type);

	return 0;
}

int lua_blocklist_talkcolor(lua_State* state, struct block_list* bl) {
	sl_checkargs(state, "ns");
	int color = lua_tonumber(state, sl_memberarg(1));
	char* msg = lua_tostring(state, sl_memberarg(2));
	USER* tsd = NULL;

	if (lua_isnumber(state, sl_memberarg(3))) {
		sl_checkargs(state, "nsn");
		tsd = map_id2sd(lua_tonumber(state, sl_memberarg(3)));
	}
	else {
		sl_checkargs(state, "nss");
		tsd = map_name2sd(lua_tostring(state, sl_memberarg(3)));
	}

	if (lua_tonumber(state, sl_memberarg(3)) == 0) {
		//map_foreachinarea(client_send_msg, bl->m, bl->x, bl->y, AREA, BL_PC, msg
	}
	else if (!tsd) {
		return 0;
	}
	else {
		client_send_msg(tsd, color, msg);
	}
	return 0;
}

/*============================================================
 * Core Type Functions
 *============================================================*/

void lua_blocklist_extendproto(lua_type_class* class) {
	lua_type_extendproto(class, "getObjectsInCell", lua_blocklist_getobjects_cell);
	lua_type_extendproto(class, "getAliveObjectsInCell", lua_blocklist_getaliveobjects_cell);
	lua_type_extendproto(class, "getObjectsInCellWithTraps", lua_blocklist_getobjects_cell_traps);
	lua_type_extendproto(class, "getAliveObjectsInCellWithTraps", lua_blocklist_getaliveobjects_cell_traps);
	lua_type_extendproto(class, "getObjectsInArea", lua_blocklist_getobjects_area);
	lua_type_extendproto(class, "getAliveObjectsInArea", lua_blocklist_getaliveobjects_area);
	lua_type_extendproto(class, "getObjectsInSameMap", lua_blocklist_getobjects_samemap);
	lua_type_extendproto(class, "getAliveObjectsInSameMap", lua_blocklist_getaliveobjects_samemap);
	lua_type_extendproto(class, "getObjectsInMap", lua_blocklist_getobjects_map);
	lua_type_extendproto(class, "getAliveObjectsInMap", lua_blocklist_getobjects_map);
	lua_type_extendproto(class, "getBlock", lua_blocklist_getblock);

	lua_type_extendproto(class, "sendAnimation", lua_blocklist_sendanim);
	lua_type_extendproto(class, "sendAnimationXY", lua_blocklist_sendanimxy);
	lua_type_extendproto(class, "getUsers", lua_blocklist_getusers);
	lua_type_extendproto(class, "delFromIDDB", lua_blocklist_deliddb);
	lua_type_extendproto(class, "playSound", lua_blocklist_playsound);
	lua_type_extendproto(class, "sendAction", lua_blocklist_sendaction);
	lua_type_extendproto(class, "talk", lua_blocklist_talk);
	lua_type_extendproto(class, "msg", lua_blocklist_talkcolor);
	lua_type_extendproto(class, "spawn", lua_blocklist_spawn);
	lua_type_extendproto(class, "addPermanentSpawn", lua_blocklist_permspawn);
	lua_type_extendproto(class, "sendSide", lua_blocklist_sendside);
	lua_type_extendproto(class, "delete", lua_blocklist_delete);
	lua_type_extendproto(class, "addNPC", lua_blocklist_addnpc);
	lua_type_extendproto(class, "dropItem", lua_blocklist_dropitem);
	lua_type_extendproto(class, "dropItemXY", lua_blocklist_dropitemxy);
	lua_type_extendproto(class, "respawn", lua_blocklist_respawn);
	lua_type_extendproto(class, "objectCanMove", lua_blocklist_objectcanmove);
	lua_type_extendproto(class, "objectCanMoveFrom", lua_blocklist_objectcanmovefrom);
	lua_type_extendproto(class, "repeatAnimation", lua_blocklist_repeatanimation);
	lua_type_extendproto(class, "throw", lua_blocklist_throw);
	lua_type_extendproto(class, "selfAnimation", lua_blocklist_selfanimation);
	lua_type_extendproto(class, "selfAnimationXY", lua_blocklist_selfanimationxy);
	lua_type_extendproto(class, "sendParcel", lua_blocklist_sendparcel);
	lua_type_extendproto(class, "updateState", lua_blocklist_updatestate);
	lua_type_extendproto(class, "removeSprite", lua_blocklist_removesprite);
	lua_type_extendproto(class, "getFriends", lua_blocklist_getfriends);
}

void lua_blocklist_pushinst(lua_State* state, struct block_list* bl, void* param) {
	if (!bl) {
		lua_pushnil(state);
		return;
	}
	if (bl->type == BL_PC) {
		USER* sd = map_id2sd(bl->id);
		lua_type_pushinst(state, &pcl_type, sd, param);
	}
	else if (bl->type == BL_MOB) {
		MOB* mob = (MOB*)bl;
		lua_type_pushinst(state, &mobl_type, mob, param);
	}
	else if (bl->type == BL_NPC) {
		struct npc_data* nd = (struct npc_data*)bl;
		lua_type_pushinst(state, &npcl_type, nd, param);
	}
	else if (bl->type == BL_ITEM) {
		FLOORITEM* fl = (FLOORITEM*)bl;
		lua_type_pushinst(state, &fll_type, fl, param);
	}
	else lua_pushnil(state);
}

int lua_blocklist_getattr(lua_State* state, struct block_list* bl, char* attrname) {
	if (!bl) return 0;
	if (!strcmp(attrname, "x")) lua_pushnumber(state, bl->x);
	else if (!strcmp(attrname, "y")) lua_pushnumber(state, bl->y);
	else if (!strcmp(attrname, "m")) lua_pushnumber(state, bl->m);
	else if (!strcmp(attrname, "xmax")) lua_pushnumber(state, map[bl->m].xs - 1);
	else if (!strcmp(attrname, "ymax")) lua_pushnumber(state, map[bl->m].ys - 1);
	else if (!strcmp(attrname, "blType")) lua_pushnumber(state, bl->type);
	else if (!strcmp(attrname, "ID")) lua_pushnumber(state, bl->id);
	else if (!strcmp(attrname, "mapId")) lua_pushnumber(state, map[bl->m].id);
	else if (!strcmp(attrname, "mapTitle")) lua_pushstring(state, map[bl->m].title);
	else if (!strcmp(attrname, "mapFile")) lua_pushstring(state, map[bl->m].mapfile);
	else if (!strcmp(attrname, "bgm")) lua_pushnumber(state, map[bl->m].bgm);
	else if (!strcmp(attrname, "bgmType")) lua_pushnumber(state, map[bl->m].bgmtype);
	else if (!strcmp(attrname, "pvp")) lua_pushnumber(state, map[bl->m].pvp);
	else if (!strcmp(attrname, "spell")) lua_pushnumber(state, map[bl->m].spell);
	else if (!strcmp(attrname, "light")) lua_pushnumber(state, map[bl->m].light);
	else if (!strcmp(attrname, "weather")) lua_pushnumber(state, map[bl->m].weather);
	else if (!strcmp(attrname, "sweepTime")) lua_pushnumber(state, map[bl->m].sweeptime);
	else if (!strcmp(attrname, "canTalk")) lua_pushnumber(state, map[bl->m].cantalk);
	else if (!strcmp(attrname, "showGhosts")) lua_pushnumber(state, map[bl->m].show_ghosts);
	else if (!strcmp(attrname, "region")) lua_pushnumber(state, map[bl->m].region);
	else if (!strcmp(attrname, "indoor")) lua_pushnumber(state, map[bl->m].indoor);
	else if (!strcmp(attrname, "warpOut")) lua_pushnumber(state, map[bl->m].warpout);
	else if (!strcmp(attrname, "bind")) lua_pushnumber(state, map[bl->m].bind);
	else if (!strcmp(attrname, "reqLvl")) lua_pushnumber(state, map[bl->m].reqlvl);
	else if (!strcmp(attrname, "reqVita")) lua_pushnumber(state, map[bl->m].reqvita);
	else if (!strcmp(attrname, "reqMana")) lua_pushnumber(state, map[bl->m].reqmana);
	else if (!strcmp(attrname, "reqPath")) lua_pushnumber(state, map[bl->m].reqpath);
	else if (!strcmp(attrname, "reqMark")) lua_pushnumber(state, map[bl->m].reqmark);
	else if (!strcmp(attrname, "maxLvl")) lua_pushnumber(state, map[bl->m].lvlmax);
	else if (!strcmp(attrname, "maxVita")) lua_pushnumber(state, map[bl->m].vitamax);
	else if (!strcmp(attrname, "maxMana")) lua_pushnumber(state, map[bl->m].manamax);
	else if (!strcmp(attrname, "canSummon")) lua_pushnumber(state, map[bl->m].summon);
	else if (!strcmp(attrname, "canUse")) lua_pushnumber(state, map[bl->m].canUse);
	else if (!strcmp(attrname, "canEat")) lua_pushnumber(state, map[bl->m].canEat);
	else if (!strcmp(attrname, "canSmoke")) lua_pushnumber(state, map[bl->m].canSmoke);
	else if (!strcmp(attrname, "canMount")) lua_pushnumber(state, map[bl->m].canMount);
	else if (!strcmp(attrname, "canGroup")) lua_pushnumber(state, map[bl->m].canGroup);

	else return 0;
	return 1;
}

int lua_blocklist_setattr(lua_State* state, struct block_list* bl, char* attrname) {
	if (!bl) return 0;
	if (!strcmp(attrname, "mapTitle")) strcpy(map[bl->m].title, lua_tostring(state, -1));
	else if (!strcmp(attrname, "mapFile")) strcpy(map[bl->m].mapfile, lua_tostring(state, -1));
	else if (!strcmp(attrname, "bgm")) map[bl->m].bgm = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "bgmType")) map[bl->m].bgmtype = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "pvp")) map[bl->m].pvp = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "spell")) map[bl->m].spell = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "light")) map[bl->m].light = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "weather")) map[bl->m].weather = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "sweepTime")) map[bl->m].sweeptime = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "canTalk")) map[bl->m].cantalk = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "showGhosts")) map[bl->m].show_ghosts = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "region")) map[bl->m].region = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "indoor")) map[bl->m].indoor = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "warpOut")) map[bl->m].warpout = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "bind")) map[bl->m].bind = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "reqLvl")) map[bl->m].reqlvl = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "reqVita")) map[bl->m].reqvita = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "reqMana")) map[bl->m].reqmana = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "reqPath")) map[bl->m].reqpath = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "reqMark")) map[bl->m].reqmark = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "maxLvl")) map[bl->m].lvlmax = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "maxVita")) map[bl->m].vitamax = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "maxMana")) map[bl->m].manamax = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "canSummon")) map[bl->m].summon = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "canUse")) map[bl->m].canUse = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "canEat")) map[bl->m].canEat = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "canSmoke")) map[bl->m].canSmoke = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "canMount")) map[bl->m].canMount = lua_tonumber(state, -1);
	else if (!strcmp(attrname, "canGroup")) map[bl->m].canGroup = lua_tonumber(state, -1);

	else return 0;
	return 1;
}
