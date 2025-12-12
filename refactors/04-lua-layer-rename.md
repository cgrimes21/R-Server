# Refactor 04: Lua Layer Rename (sl_* → lua_*)

**Date:** December 12, 2025
**Status:** Complete

## Overview

Renamed cryptic Lua binding prefixes to readable `lua_*` names throughout the scripting layer.

## Rationale

- `sl_` is unclear ("scripting layer"?)
- `pcl_`, `mobl_`, `npcl_` are cryptic
- `lua_*` prefixes are self-documenting

## File Renames (16 files)

| Old | New |
|-----|-----|
| `sl.c/h` | `lua_core.c/h` |
| `sl_types.c/h` | `lua_types.c/h` |
| `sl_blocklist.c/h` | `lua_blocklist.c/h` |
| `sl_player.c/h` | `lua_player.c/h` |
| `sl_mob.c/h` | `lua_mob.c/h` |
| `sl_npc.c/h` | `lua_npc.c/h` |
| `sl_item.c/h` | `lua_item.c/h` |
| `sl_registry.c/h` | `lua_registry.c/h` |

## Function Prefix Renames (~350 functions)

| Old Prefix | New Prefix | Count |
|------------|------------|-------|
| `sl_*` | `lua_*` | ~20 |
| `typel_*` | `lua_type_*` | ~15 |
| `pcl_*` | `lua_player_*` | ~160 |
| `mobl_*` | `lua_mob_*` | ~35 |
| `npcl_*` | `lua_npc_*` | ~10 |
| `bll_*` | `lua_blocklist_*` | ~45 |
| `iteml_*` | `lua_item_*` | ~10 |
| `biteml_*` | `lua_bound_item_*` | ~5 |
| `bankiteml_*` | `lua_bank_item_*` | ~5 |
| `recipel_*` | `lua_recipe_*` | ~5 |
| `parcell_*` | `lua_parcel_*` | ~3 |
| `fll_*` | `lua_floor_item_*` | ~6 |
| `regl_*` | `lua_registry_*` | ~5 |
| `questregl_*` | `lua_quest_registry_*` | ~3 |
| `npcregl_*` | `lua_npc_registry_*` | ~3 |
| `mobregl_*` | `lua_mob_registry_*` | ~3 |
| `mapregl_*` | `lua_map_registry_*` | ~3 |
| `gameregl_*` | `lua_game_registry_*` | ~3 |

## Examples

```c
sl_init()           → lua_binding_init()
sl_doscript_blargs() → lua_do_script_blargs()
pcl_warp()          → lua_player_warp()
pcl_additem()       → lua_player_additem()
mobl_attack()       → lua_mob_attack()
bll_spawn()         → lua_blocklist_spawn()
typel_new()         → lua_type_new()
```

## Backward Compatibility

All headers include `#define` macros for backward compatibility:
```c
#define pcl_warp lua_player_warp
#define pcl_additem lua_player_additem
// ... etc
```

Existing code using old names will continue to work.

## Makefile Changes

```makefile
# Old
SL_OBJ = sl.o sl_types.o

# New
LUA_OBJ = lua_core.o lua_types.o lua_blocklist.o lua_player.o \
          lua_mob.o lua_npc.o lua_item.o lua_registry.o
```

## Verification

- Syntax check passes for all renamed files
- Old sl_* files removed
- Backward compat macros ensure existing code works
