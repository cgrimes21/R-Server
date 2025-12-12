/**
 * @file lua_core.h
 * @brief Core Lua scripting layer interface
 *
 * Central header for the Lua binding layer.
 * Renamed from sl.h as part of the RTK naming refactor.
 */

#ifndef _LUA_CORE_H_
#define _LUA_CORE_H_

#include <lua.h>
#include "map.h"

/* Include sub-module headers */
#include "lua_types.h"
#include "lua_blocklist.h"

/* Global Lua state */
extern lua_State* lua_gstate;

/* Argument checking - defined in lua_core.c */
void sl_checkargs(lua_State* state, char* fmt);

/* Initialization */
void lua_binding_init();

/* Script execution */
void lua_runfunc(char*, struct block_list*);
int lua_do_script_blargs(char*, char*, int, ...);
int lua_do_script_stackargs(char*, char*, int);
int lua_updatepeople(struct block_list*, va_list);

/* Simple script execution macro */
#define lua_do_script_simple(root, method, bl) \
	lua_do_script_blargs(root, method, 1, bl)

/* Menu/dialog resume */
void lua_resume_menu(unsigned int, USER*);
void lua_resume_dialog(unsigned int, USER*);
void lua_resume_buy(char*, USER*);
void lua_resume_input(char*, char*, USER*);
void lua_resume_sell(unsigned int, USER*);
void lua_resume_menuseq(unsigned int selection, int choice, USER* sd);
void lua_resume_inputseq(unsigned int choice, char* input, USER* sd);

/* Direct Lua execution */
void lua_exec(USER*, char*);

/* Coroutine cleanup */
void lua_async_freeco(USER*);

/* Script reload */
int lua_reload();

/* Memory management */
int lua_luasize(USER*);
void lua_fixmem();

/* Backward compatibility */
#define sl_gstate lua_gstate
#define sl_init lua_binding_init
#define sl_runfunc lua_runfunc
#define sl_doscript_blargs lua_do_script_blargs
#define sl_doscript_stackargs lua_do_script_stackargs
#define sl_updatepeople lua_updatepeople
#define sl_doscript_simple lua_do_script_simple
#define sl_resumemenu lua_resume_menu
#define sl_resumedialog lua_resume_dialog
#define sl_resumebuy lua_resume_buy
#define sl_resumeinput lua_resume_input
#define sl_resumesell lua_resume_sell
#define sl_resumemenuseq lua_resume_menuseq
#define sl_resumeinputseq lua_resume_inputseq
#define sl_exec lua_exec
#define sl_async_freeco lua_async_freeco
#define sl_reload lua_reload
#define sl_luasize lua_luasize
#define sl_fixmem lua_fixmem

#endif /* _LUA_CORE_H_ */
