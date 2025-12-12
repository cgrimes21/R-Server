# Refactor 01: Monolithic File Split

**Date:** December 2025
**Status:** Reverted

## Overview

Attempted split of massive monolithic C files into focused modules.

## Outcome

Split files were created but the original monolithic files (`client.c`, `lua_core.c`) were never pruned. This resulted in duplicate function definitions. The split files have been **removed** as of December 12, 2025.

### Why Reverted
1. Split files had duplicate functions with the originals
2. Naming conventions differed between split files and originals
3. Simpler to maintain single monolithic files with backward compatibility macros

### Current State
- `client.c` (~15,700 lines) - Monolithic client layer
- `lua_core.c` (~11,200 lines) - Monolithic Lua binding layer
- `lua_types.c` (~200 lines) - Separate module (kept, not duplicated)
- Headers contain backward compatibility `#define` macros

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
