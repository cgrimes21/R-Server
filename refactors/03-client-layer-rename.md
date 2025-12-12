# Refactor 03: Client Layer Rename (clif_* → client_*)

**Date:** December 12, 2025
**Status:** Complete

## Overview

Renamed cryptic `clif_` prefix to readable `client_` prefix throughout the client interface layer.

## Rationale

- `clif` is unclear abbreviation ("client interface")
- `client_` is self-documenting
- Improves code readability

## File Renames

### Map Server (`rtk/src/map/`)

| Old | New |
|-----|-----|
| `clif.c` | `client.c` |
| `clif.h` | `client.h` |
| `clif_crypto.c/h` | `client_crypto.c/h` |
| `clif_chat.c/h` | `client_chat.c/h` |
| `clif_visual.c/h` | `client_visual.c/h` |
| `clif_combat.c/h` | `client_combat.c/h` |
| `clif_inventory.c/h` | `client_inventory.c/h` |
| `clif_npc.c/h` | `client_npc.c/h` |
| `clif_player.c/h` | `client_player.c/h` |

### Login Server (`rtk/src/login/`)

| Old | New |
|-----|-----|
| `clif.c` | `login_client.c` |
| `clif.h` | `login_client.h` |

## Function Renames (~180 functions)

Examples:
```c
clif_send()          → client_send()
clif_parse()         → client_parse()
clif_broadcast()     → client_broadcast()
clif_sendminitext()  → client_send_minitext()
clif_mob_damage()    → client_mob_damage()
clif_Hacker()        → client_hacker()
```

## Files Updated

Cross-references updated in:
- `map.c`, `pc.c`, `mob.c`, `npc.c`
- `command.c`, `intif.c`, `creation.c`, `script.c`
- `sl.c`, `sl_blocklist.c`, `sl_mob.c`, `sl_player.c`
- `Makefile` (map and login)

## Makefile Changes

```makefile
# Old
CLIF_OBJ = clif.o clif_crypto.o clif_chat.o ...

# New
CLIENT_OBJ = client.o client_crypto.o client_chat.o ...
```

## Verification

- Syntax check passes
- Zero `clif_` references remaining in map directory
- Old files removed
