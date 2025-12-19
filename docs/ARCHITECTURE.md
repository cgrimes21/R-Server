# RetroTK Server Architecture

This document describes the architecture, file structure, and key components of the RetroTK Server.

## Project Overview

RetroTK Server is a complete MMO game server implementation featuring:
- **Core Server**: C (performance-critical components)
- **Game Logic**: Lua (scripting and dynamic content)
- **Database**: MySQL (persistence)
- **Client**: Custom client in `/client/RetroTK`

### Server Components

| Server | Directory | Purpose |
|--------|-----------|---------|
| Login Server | `rtk/src/login/` | Authentication |
| Character Server | `rtk/src/char/` | Character management |
| Map Server | `rtk/src/map/` | Game world, NPCs, mobs, players |
| Metadata Server | `rtk/src/metan/` | Item/class metadata |

### Codebase Statistics

| Metric | Count |
|--------|-------|
| C Source Lines | 63,829 |
| Lua Script Lines | 73,346 |
| C Source Files | 42 |
| Lua Script Files | 907 |
| Header Files | 47 |

---

## File Structure

### C Source Layout

```
rtk/src/
├── common/          # Shared utilities
│   ├── db.c         # Database abstraction (2,680 lines)
│   ├── db_mysql.c   # MySQL implementation (859 lines)
│   ├── socket.c     # Network I/O (1,008 lines)
│   ├── strlib.c     # String utilities (1,062 lines)
│   ├── timer.c      # Timer system (356 lines)
│   ├── crypt.c      # Encryption (242 lines)
│   ├── malloc.c     # Memory management (134 lines)
│   ├── game_state.c # Runtime state management (270 lines)
│   ├── server_config.c # Configuration loading (450 lines)
│   ├── rtk_error.h  # Error codes and macros (280 lines)
│   └── rtk_memory.h # Memory conventions (160 lines)
├── login/           # Login server
│   ├── login.c      # Main login logic
│   ├── client.c     # Login client interface
│   └── intif.c      # Inter-server communication
├── char/            # Character server
│   ├── char.c       # Main char logic
│   ├── char_db.c    # Character database (1,675 lines)
│   ├── mapif.c      # Map server interface
│   └── saveif.c     # Save interface
├── map/             # Map server
│   ├── client.c     # Client interface (core packet routing)
│   ├── client_crypto.c   # Encryption/decryption (~80 lines)
│   ├── client_chat.c     # Chat, whispers, broadcasts (~900 lines)
│   ├── client_visual.c   # Animations, movement display (~500 lines)
│   ├── client_combat.c   # Health bars, damage, attacks (~900 lines)
│   ├── client_inventory.c # Items, equipment (~1,000 lines)
│   ├── client_npc.c      # NPC dialogs, menus, shops (~1,835 lines)
│   ├── client_player.c   # Status, groups, exchange (~750 lines)
│   ├── sl.c         # Lua scripting core
│   ├── sl_types.c   # Type metaclass system (~170 lines)
│   ├── sl_player.c  # Player Lua bindings (~5,250 lines)
│   ├── sl_mob.c     # Mob Lua bindings (~970 lines)
│   ├── sl_npc.c     # NPC Lua bindings (~265 lines)
│   ├── sl_item.c    # Item Lua bindings (~430 lines)
│   ├── sl_blocklist.c # BlockList bindings (~850 lines)
│   ├── sl_registry.c  # Registry bindings (~200 lines)
│   ├── map.c        # World management (3,006 lines)
│   ├── pc.c         # Player character (2,998 lines)
│   ├── mob.c        # Mob AI (2,411 lines)
│   ├── command.c    # GM commands (1,694 lines)
│   ├── npc.c        # NPC system (817 lines)
│   └── intif.c      # Inter-server (864 lines)
└── metan/           # Metadata server
```

### Lua Script Layout

```
rtklua/
├── Accepted/
│   ├── player.lua           # Player mechanics (5,356 lines)
│   ├── speech.lua           # NPC dialogue (2,316 lines)
│   ├── crafting.lua         # Crafting orchestrator (1,474 lines)
│   ├── Crafting/            # Individual crafting systems
│   │   ├── gemcutting.lua
│   │   ├── jewelrymaking.lua
│   │   ├── metalworking.lua
│   │   ├── tailoring.lua
│   │   └── woodworking.lua
│   ├── NPCs/
│   │   ├── Common/
│   │   │   └── general_npc_funcs.lua  # (2,898 lines)
│   │   ├── Trainers/
│   │   │   ├── mage_trainer.lua
│   │   │   ├── warrior_trainer.lua
│   │   │   ├── rogue_trainer.lua
│   │   │   └── poet_trainer.lua
│   │   └── mobSpawnHandler.lua        # (3,103 lines)
│   ├── Mobs/
│   │   └── MobDrops.lua     # Drop tables (3,654 lines)
│   ├── Items/
│   ├── Quests/
│   └── Scripts/
```

---

## Dependency Flow

```
client.c ──────────────┐
                       ├──> map.c ──┐
sl.c ──────────────────┤            ├──> pc.c ────┐
                       ├──> mob.c   │             ├──> db.c
command.c ─────────────┤            ├──> npc.c    │
                       └──> itemdb  └──> intif.c  │
                                                   │
socket.c <─────────────────────────────────────────┘
```

---

## Key Data Structures

### C Structures

| Structure | Description | Location |
|-----------|-------------|----------|
| `DBMap` | Generic hash map (int, uint, string keys) | db.h |
| `block_list` | Spatial entity management | map.h |
| `map_session_data` (USER) | Player session | map.h |
| `mob_data` (MOB) | Mob instance | mob.h |
| `npc_data` (NPC) | NPC instance | npc.h |

### Lua Type System

| Type | Description | Source File |
|------|-------------|-------------|
| `typel_class` | Type definitions | sl_types.c |
| `typel_inst` | Type instances | sl_types.c |
| `pcl_type` | Player character class | sl_player.c |
| `mobl_type` | Mob class | sl_mob.c |
| `npcl_type` | NPC class | sl_npc.c |
| `regl_type` | Global registry | sl_registry.c |
| `npcregl_type` | NPC registry | sl_registry.c |
| `questregl_type` | Quest registry | sl_registry.c |
| `bll_*` | Block list types | sl_blocklist.c |

---

## Configuration Files

| File | Purpose |
|------|---------|
| `conf/map.conf` | Map server settings |
| `conf/char.conf` | Character server settings |
| `conf/login.conf` | Login server settings |
| `conf/crypto.conf` | Packet encryption keys |
| `conf/inter.conf` | Inter-server communication |

---

## Runtime State

The `game_state_t` structure (game_state.h) centralizes runtime state:

- **network_state_t**: Server connections (char_fd, map_fd, log_fd)
- **server_identity_t**: Server ID and name
- **time_state_t**: Game time and cron tracking
- **spawn_state_t**: Mob/NPC spawning counters
- **stats_state_t**: Online counts, peak tracking
- **db_state_t**: Database connection handle

---

## Related Documentation

- [PROTOCOL.md](PROTOCOL.md) - Packet protocol documentation
- [LUA_API.md](LUA_API.md) - Lua scripting API reference
- [OPCODES.md](OPCODES.md) - Packet opcode tables
- [REFACTORING.md](REFACTORING.md) - Refactoring progress and history
- [BUILDING.md](BUILDING.md) - Build instructions
