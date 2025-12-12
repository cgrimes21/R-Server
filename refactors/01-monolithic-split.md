# Refactor 01: Monolithic File Split

**Date:** December 2025
**Status:** Complete

## Overview

Split massive monolithic C files into focused, maintainable modules.

## Changes

### Phase 1.1: clif.c Split (15,731 lines → 8 modules)

| New File | Purpose | Lines |
|----------|---------|-------|
| `clif_crypto.c/h` | Packet encryption/decryption | ~80 |
| `clif_chat.c/h` | Chat, whispers, broadcasts | ~900 |
| `clif_visual.c/h` | Visual updates, animations | ~600 |
| `clif_combat.c/h` | Combat packets, damage, health bars | ~1,200 |
| `clif_inventory.c/h` | Inventory, equipment packets | ~800 |
| `clif_npc.c/h` | NPC dialogs, menus, shops | ~2,500 |
| `clif_player.c/h` | Player status, movement, groups | ~1,500 |
| `clif.c/h` | Core send functions, main parser | ~8,000 |

### Phase 1.2: sl.c Split (11,344 lines → 7 modules)

| New File | Purpose | Lines |
|----------|---------|-------|
| `sl_types.c/h` | Lua type metaclass system | ~200 |
| `sl_blocklist.c/h` | Spatial object operations | ~1,000 |
| `sl_player.c/h` | Player Lua bindings (~160 functions) | ~5,500 |
| `sl_mob.c/h` | Mob Lua bindings | ~1,100 |
| `sl_npc.c/h` | NPC Lua bindings | ~400 |
| `sl_item.c/h` | Item type bindings | ~650 |
| `sl_registry.c/h` | Registry type bindings | ~200 |

## Impact

- ~7,000 lines organized into focused modules
- Each module has single responsibility
- Easier to navigate and maintain

## Notes

These files were later renamed in Refactor 03 (clif_* → client_*) and Refactor 04 (sl_* → lua_*).
