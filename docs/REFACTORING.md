# RetroTK Server Refactoring Progress

This document tracks the refactoring work completed on the RetroTK Server codebase.

---

## Summary

| Phase | Status | Description |
|-------|--------|-------------|
| Phase 1 | COMPLETE | Split Monolithic C Files |
| Phase 2 | COMPLETE | Eliminate Lua Code Duplication |
| Phase 3 | COMPLETE | Architecture Improvements |
| Phase 4 | COMPLETE | Code Quality |

---

## Phase 1: Split Monolithic C Files

> **Target**: Break down massive files into focused, maintainable modules
> **Result**: client.c and sl.c successfully modularized

### 1.1 client.c Refactoring (15,731 lines)

**Status**: COMPLETE

Split into 8 focused modules:

| New File | Lines | Description |
|----------|-------|-------------|
| `client_crypto.c` | ~80 | Encryption/decryption |
| `client_visual.c` | ~500 | Animations, look packets, movement display |
| `client_combat.c` | ~900 | Health bars, damage, attacks |
| `client_inventory.c` | ~1,000 | Items, equipment, durability |
| `client_npc.c` | ~1,835 | NPC dialogs, menus, shops |
| `client_chat.c` | ~900 | Chat, whispers, broadcasts |
| `client_player.c` | ~750 | Status, groups, exchange, movement |
| `client.c` | ~2,000 | Core packet routing, parse loop |

#### Function Mapping

**client_crypto.c**
- `encrypt()`, `decrypt()`, `isKey()`, `isKey2()`

**client_visual.c**
- Animation: `client_sendanimation()`, `client_animation()`, `client_sendanimations()`
- Actions: `client_sendaction()`, `client_sendmob_action()`, `client_playsound()`
- Movement: `client_mob_move()`, `client_npc_move()`
- Look: `client_charlook_sub()`, `client_mob_look_*()`, `client_object_look_*()`

**client_combat.c**
- Damage: `client_pc_damage()`, `client_mob_damage()`
- Health bars: `client_send_pc_health()`, `client_send_mob_health()`, `client_send_selfbar()`
- Combat: `client_parseattack()`, `client_calc_critical()`, `client_mob_kill()`
- Durability: `client_deductweapon()`, `client_deductarmor()`, `client_deductdura()`

**client_inventory.c**
- Items: `client_sendadditem()`, `client_senddelitem()`
- Equipment: `client_sendequip()`, `client_equipit()`, `client_unequipit()`
- Actions: `client_parsedropitem()`, `client_parsegetitem()`, `client_parseuseitem()`
- Throwing: `client_throwitem_script()`, `client_parsethrow()`
- Gold: `client_dropgold()`, `client_handgold()`, `client_handitem()`

**client_npc.c**
- Dialogs: `client_scriptmes()`, `client_scriptmenu()`, `client_scriptmenuseq()`
- Menus: `client_parsemenu()`, `client_parsenpcdialog()`
- Shops: `client_buydialog()`, `client_parsebuy()`, `client_selldialog()`, `client_parsesell()`
- Input: `client_input()`, `client_inputseq()`, `client_parseinput()`

**client_chat.c**
- Chat: `client_sendsay()`, `client_parsesay()`
- Whisper: `client_sendwisp()`, `client_parsewisp()`
- Broadcast: `client_broadcast()`, `client_gmbroadcast()`
- NPC speech: `client_sendnpcsay()`, `client_sendmobsay()`
- Popups: `client_popup()`, `client_paperpopup()`

**client_player.c**
- Status: `client_sendstatus()`, `client_sendupdatestatus()`
- Movement: `client_parsewalk()`, `client_canmove()`, `client_blockmovement()`
- Groups: `client_groupstatus()`, `client_addgroup()`, `client_leavegroup()`
- Exchange: `client_parse_exchange()`, `client_startexchange()`, `client_exchange_*()`

---

### 1.2 sl.c Refactoring (11,344 lines)

**Status**: COMPLETE (~8,135 lines extracted)

Split into 7 modules:

| New File | Lines | Description |
|----------|-------|-------------|
| `sl_types.c` | ~170 | Type metaclass system (typel_*) |
| `sl_blocklist.c` | ~850 | Block list bindings (bll_*) |
| `sl_player.c` | ~5,250 | Player bindings (pcl_*) |
| `sl_registry.c` | ~200 | Registry types |
| `sl_item.c` | ~430 | Item types |
| `sl_npc.c` | ~265 | NPC type (npcl_*) |
| `sl_mob.c` | ~970 | Mob type (mobl_*) |
| `sl.c` | ~3,000 | Core Lua init, async |

#### pcl_* Functions (Player Lua API)

- **Core**: pcl_ctor, pcl_init, pcl_getattr, pcl_setattr
- **Health/Combat**: addhealth, removehealth, sendhealth, die, resurrect
- **Durations/Aethers**: setduration, hasduration, getduration, setaether, hasaether
- **Inventory**: additem, hasitem, removeinventoryitem, hasspace
- **Equipment**: equip, forceequip, takeoff, stripequip, hasequipped
- **Spells**: addspell, removespell, hasspell, getspells
- **Banking**: bankdeposit, bankwithdraw, clanbankdeposit, subpathbankdeposit
- **Dialog/Menu**: dialog, menu, menuseq, input, inputseq, popup
- **Legend/Status**: addlegend, haslegend, removelegend, sendstatus
- **Movement**: warp, move, respawn, refresh
- **Timer**: settimer, addtime, removetime
- **Kan System**: addKan, removeKan, setKan, checkKan, claimKan

---

### 1.3 Other Large C Files

**Status**: REVIEWED (No extraction needed)

- **map.c (3,010 lines)** - Core map infrastructure - well-organized
- **pc.c (2,996 lines)** - Player character functions - coherent domain
- **mob.c (2,411 lines)** - Mob AI, spawning, combat - focused responsibility
- **command.c (1,694 lines)** - ~90 GM commands - manageable as-is

---

## Phase 2: Eliminate Lua Code Duplication

> **Target**: Reduce Lua codebase by 25-35%
> **Result**: Achieved 70% reduction (~8,860 lines saved)

### 2.1 Crafting System Consolidation

| File | Original | Refactored | Reduction |
|------|----------|------------|-----------|
| gemcutting.lua | 1,934 | 60 | 97% |
| jewelrymaking.lua | 1,002 | 150 | 85% |
| metalworking.lua | 793 | 120 | 85% |
| tailoring.lua | 395 | 130 | 67% |
| woodworking.lua | 905 | 180 | 80% |
| **Total** | **5,029** | **640** | **87%** |

**Infrastructure**: `crafting_base.lua` (580 lines)
- `GEMCUTTING_THRESHOLDS` - Skill level probability tables
- `rollResult()`, `giveCraftResult()` - Generic crafting functions
- `craftGem()`, `craftRing()`, `craftMetalWeapon()` - Type-specific functions

---

### 2.2 Trainer NPC Consolidation

| File | Original | Refactored | Reduction |
|------|----------|------------|-----------|
| mage_trainer.lua | ~1,329 | 250 | 81% |
| warrior_trainer.lua | ~1,100 | 200 | 82% |
| rogue_trainer.lua | ~1,000 | 280 | 72% |
| poet_trainer.lua | ~1,000 | 220 | 78% |
| **Total** | **~4,429** | **950** | **79%** |

**Infrastructure**: `trainer_base.lua` (230 lines)
- `CLASS_DATA`, `QUEST_STAGES` - Class/quest configurations
- `initDialog()`, `buildStandardOptions()`, `addQuestOptions()`
- `handleCommonChoice()`, `handleClassEnrollment()`

---

### 2.3 Player.lua Cleanup

| Function Group | Original | Refactored | Reduction |
|----------------|----------|------------|-----------|
| Repair functions | ~1,333 | 135 | 90% |
| Bank functions | ~569 | 150 | 74% |
| Health functions | ~109 | 55 | 50% |
| **Total** | **~2,011** | **519** | **74%** |

**Infrastructure**: `player_base.lua` (268 lines)
- Equipment slot constants
- `calculateRepairCost()`, `needsRepair()`, `repairItems()`
- `buildInventoryItemList()`, `buildBankBuyText()`
- Health modifier helpers

---

### 2.4 General NPC Functions

| Function | Original | Refactored | Reduction |
|----------|----------|------------|-----------|
| haircut | 393 | 50 | 87% |
| hairdye | 162 | 40 | 75% |
| changeFace | 153 | 35 | 77% |
| changeGender | 93 | 25 | 73% |
| changeEyes | 91 | 30 | 67% |
| beard | 146 | 35 | 76% |
| shave | 39 | 15 | 62% |
| scalpMassage | 43 | 15 | 65% |
| **Total** | **~1,120** | **300** | **73%** |

**Infrastructure**: `npc_base.lua` (242 lines)
- `HAIR_STYLES`, `HAIR_COLORS`, `FACES`, `EYE_COLORS`, `BEARD_STYLES`
- `initDialog()`, `selectionLoop()`, `checkCost()`

---

## Phase 3: Architecture Improvements

### 3.1 Database Abstraction

**Status**: Reviewed - existing db_mysql.c already provides solid SQL abstraction

### 3.2 Global State Management

**Status**: COMPLETE

Created centralized game state:
- `game_state.h` (275 lines) - State structures
- `game_state.c` (270 lines) - State initialization and accessors

Structures:
- `network_state_t` - Network connections
- `server_identity_t` - Server ID and name
- `time_state_t` - Game time tracking
- `spawn_state_t` - Mob/NPC spawning
- `stats_state_t` - Online counts
- `db_state_t` - Database handle

### 3.3 Configuration Externalization

**Status**: COMPLETE

- `server_config.h` (200 lines) - Config structures
- `server_config.c` (450 lines) - Config loading
- `crypto.conf` (30 lines) - Externalized encryption keys

---

## Phase 4: Code Quality

### 4.1 Error Handling

**Status**: COMPLETE

Created `rtk_error.h` (280 lines):
- `rtk_result_t` - Standard result type
- Success codes: `RTK_SUCCESS`, `RTK_SUCCESS_PARTIAL`, `RTK_SUCCESS_NOOP`
- Error categories: General, Memory, Network, Database, File, Game, Player
- Helper macros: `RTK_CHECK_NULL`, `RTK_TRY`, `RTK_RETURN_ERROR`

### 4.2 Memory Management

**Status**: COMPLETE

Created `rtk_memory.h` (160 lines):
- CALLOC/REALLOC/FREE macro guidelines
- nullpo_ret/nullpo_chk null pointer checking
- Memory ownership patterns
- Safety macros: `SAFE_STRCPY`, `SAFE_STRCAT`, `ZERO_STRUCT`

### 4.3 Documentation

**Status**: COMPLETE

- `docs/PROTOCOL.md` - Packet protocol documentation
- `docs/LUA_API.md` - Lua API reference (150+ player methods)
- `docs/OPCODES.md` - Packet opcode tables

---

## Build System Fixes

- Renamed `crypt()`/`crypt2()` to `rtk_crypt()`/`rtk_crypt2()` (libc conflict)
- Changed `my_bool` to `bool` in db_mysql.c (MySQL 8.0 compatibility)
- Fixed global variable declarations (added `extern` keyword)
- Fixed duplicate definitions across source files

---

## Files Created

### Phase 1 - C Modules

| File | Lines | Purpose |
|------|-------|---------|
| client_crypto.h/c | 45/80 | Encryption |
| client_chat.h/c | 70/900 | Chat system |
| client_visual.h/c | 55/500 | Visual effects |
| client_combat.h/c | 50/900 | Combat |
| client_inventory.h/c | 45/1000 | Inventory |
| client_npc.h/c | 45/1835 | NPC dialogs |
| client_player.h/c | 65/750 | Player status |
| sl_types.h/c | 75/170 | Type system |
| sl_blocklist.h/c | 85/850 | Block lists |
| sl_player.h/c | 270/5250 | Player Lua API |
| sl_registry.h/c | 75/200 | Registries |
| sl_item.h/c | 65/430 | Items |
| sl_npc.h/c | 35/265 | NPCs |
| sl_mob.h/c | 55/970 | Mobs |

### Phase 2 - Lua Infrastructure

| File | Lines | Purpose |
|------|-------|---------|
| crafting_base.lua | 580 | Crafting system |
| trainer_base.lua | 230 | Trainer NPCs |
| player_base.lua | 268 | Player utilities |
| npc_base.lua | 242 | NPC utilities |

### Phase 3-4 - Architecture

| File | Lines | Purpose |
|------|-------|---------|
| game_state.h/c | 275/270 | Runtime state |
| server_config.h/c | 200/450 | Configuration |
| rtk_error.h | 280 | Error handling |
| rtk_memory.h | 160 | Memory conventions |

---

## Total Impact

| Metric | Before | After | Change |
|--------|--------|-------|--------|
| client.c | 15,731 | ~2,000 | -87% |
| sl.c | 11,344 | ~3,000 | -73% |
| Lua (refactored) | ~12,589 | ~3,729 | -70% |

**Net reduction**: ~8,860 Lua lines, 21K+ C lines reorganized into focused modules.

---

*Last Updated: 2025-12-18*
