# RetroTK Server - Claude Code Instructions

## Project Overview

RetroTK Server is a complete MMO game server implementation:
- **Core Server**: C (performance-critical)
- **Game Logic**: Lua (scripting)
- **Database**: MySQL
- **Client**: `/client/RetroTK`

### Server Components

| Server | Location | Purpose |
|--------|----------|---------|
| Login | `rtk/src/login/` | Authentication |
| Character | `rtk/src/char/` | Character management |
| Map | `rtk/src/map/` | Game world, NPCs, mobs |
| Metadata | `rtk/src/metan/` | Item/class metadata |

---

## Documentation

| Document | Description |
|----------|-------------|
| [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) | File structure, dependencies, data structures |
| [docs/BUILDING.md](docs/BUILDING.md) | Build instructions and prerequisites |
| [docs/REFACTORING.md](docs/REFACTORING.md) | Refactoring progress and history |
| [docs/PROTOCOL.md](docs/PROTOCOL.md) | Packet protocol documentation |
| [docs/LUA_API.md](docs/LUA_API.md) | Lua scripting API reference |
| [docs/OPCODES.md](docs/OPCODES.md) | Packet opcode tables |

---

## Build Commands

```bash
# Prerequisites (WSL/Linux)
sudo apt install liblua5.1-dev libmysqlclient-dev zlib1g-dev

# Build
cd rtk
make all      # Build all servers
make map      # Map server only
make clean    # Clean build
```

---

## Key Files

### C Source (Map Server)

| File | Purpose |
|------|---------|
| `client.c` | Core packet routing |
| `client_*.c` | Modular client handlers (chat, combat, inventory, etc.) |
| `sl.c` | Lua scripting core |
| `sl_*.c` | Modular Lua bindings (player, mob, npc, etc.) |
| `map.c` | World management |
| `pc.c` | Player character |
| `mob.c` | Mob AI |

### Lua Scripts

| File | Purpose |
|------|---------|
| `rtklua/Accepted/player.lua` | Player mechanics |
| `rtklua/Accepted/Crafting/` | Crafting systems |
| `rtklua/Accepted/NPCs/` | NPC scripts |

---

## Known TODOs

From sl.c:
- Line 1: `// TODO: pcl functions like dialog and menu should accept an npc parameter!`
- Line 59: `// TODO: do we even need this anymore?`
- Line 64: `// TODO: actually use this!`

---

## Conventions

### Headers
- Headers include `map.h` for USER/MOB/block_list definitions
- Use `extern` in headers, define in one .c file

### Error Handling
- Use `rtk_error.h` codes (RTK_SUCCESS, RTK_ERR_*, etc.)
- Check null pointers with nullpo_ret/nullpo_chk

### Memory
- Use CALLOC/REALLOC/FREE macros
- See `rtk_memory.h` for conventions

---

*Refactoring Status: ALL PHASES COMPLETE*
*See [docs/REFACTORING.md](docs/REFACTORING.md) for details*
