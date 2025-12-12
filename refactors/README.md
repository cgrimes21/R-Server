# RTK Server Refactoring History

This folder tracks all major refactoring efforts on the RTK Server codebase.

## Refactors

| # | Name | Date | Status | Description |
|---|------|------|--------|-------------|
| 01 | [Monolithic Split](01-monolithic-split.md) | Dec 2025 | Complete | All split files integrated, ~9,600 lines removed from monolithic files |
| 02 | [Lua Consolidation](02-lua-consolidation.md) | Dec 2025 | Complete | 70% reduction in Lua code through data-driven patterns |
| 03 | [Client Layer Rename](03-client-layer-rename.md) | Dec 2025 | Complete | clif_* → client_* (headers renamed, compat macros added) |
| 04 | [Lua Layer Rename](04-lua-layer-rename.md) | Dec 2025 | Complete | sl_* → lua_* (headers renamed, compat macros added) |

## Summary Statistics

| Metric | Value |
|--------|-------|
| C files reorganized | 32 |
| C functions renamed | ~530 |
| Lua lines saved | ~8,860 |
| C lines modularized | ~9,600 |
| Documentation added | ~1,100 lines |

## Infrastructure Added

- `server_config.c/h` - Configuration management
- `game_state.c/h` - Global state management
- `rtk_error.h` - Error handling macros
- `rtk_memory.h` - Memory safety macros
- `conf/crypto.conf` - Externalized crypto keys
- `doc/PROTOCOL.md` - Packet protocol documentation
- `doc/LUA_API.md` - Lua API reference

## Current Build State

The build uses fully modular split files:

### Client Layer (8 files)
- `client.c` - Core client functions (7,248 lines, reduced from ~15,700)
- `client_crypto.c` - Encryption/decryption
- `client_chat.c` - Chat, whispers, broadcasts
- `client_visual.c` - Animations, look packets, movement display
- `client_combat.c` - Health bars, damage, attacks
- `client_inventory.c` - Items, equipment packets
- `client_npc.c` - NPC dialogs, menus, shops
- `client_player.c` - Player status, groups, exchange

### Lua Layer (8 files)
- `lua_core.c` - Core Lua bindings (10,050 lines, reduced from ~11,210)
- `lua_types.c` - Type system
- `lua_blocklist.c` - Spatial operations
- `lua_player.c` - Player Lua bindings
- `lua_mob.c` - Mob Lua bindings
- `lua_npc.c` - NPC Lua bindings
- `lua_item.c` - Item/recipe/parcel/floor item bindings
- `lua_registry.c` - Registry bindings

### Compatibility
- Backward compatibility macros in headers map old names to new
- Old function names (`clif_*`, `sl_*`) work via `#define` macros

## Naming Conventions (Current)

### C Files
- `client_*.c/h` - Client packet handling
- `lua_*.c/h` - Lua binding layer
- `*_db.c/h` - Database/data files

### Function Prefixes
- `client_*` - Client interface functions
- `lua_*` - Lua binding core
- `lua_player_*` - Player Lua bindings
- `lua_mob_*` - Mob Lua bindings
- `lua_npc_*` - NPC Lua bindings
- `lua_blocklist_*` - Spatial operations
- `lua_type_*` - Type system
