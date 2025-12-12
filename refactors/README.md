# RTK Server Refactoring History

This folder tracks all major refactoring efforts on the RTK Server codebase.

## Refactors

| # | Name | Date | Status | Description |
|---|------|------|--------|-------------|
| 01 | [Monolithic Split](01-monolithic-split.md) | Dec 2025 | Complete | Split clif.c (15K lines) and sl.c (11K lines) into focused modules |
| 02 | [Lua Consolidation](02-lua-consolidation.md) | Dec 2025 | Complete | 70% reduction in Lua code through data-driven patterns |
| 03 | [Client Layer Rename](03-client-layer-rename.md) | Dec 2025 | Complete | clif_* → client_* (16 files, ~180 functions) |
| 04 | [Lua Layer Rename](04-lua-layer-rename.md) | Dec 2025 | Complete | sl_* → lua_* (16 files, ~350 functions) |

## Summary Statistics

| Metric | Value |
|--------|-------|
| C files reorganized | 32 |
| C functions renamed | ~530 |
| Lua lines saved | ~8,860 |
| Documentation added | ~1,100 lines |

## Infrastructure Added

- `server_config.c/h` - Configuration management
- `game_state.c/h` - Global state management
- `rtk_error.h` - Error handling macros
- `rtk_memory.h` - Memory safety macros
- `conf/crypto.conf` - Externalized crypto keys
- `doc/PROTOCOL.md` - Packet protocol documentation
- `doc/LUA_API.md` - Lua API reference

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
