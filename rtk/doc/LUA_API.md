# RTK Server Lua API Documentation

This document describes the Lua scripting API available for NPC scripts, spells, and game events.

## Type System

The Lua API uses a custom type system with the following base types:

| Type | Description | Source File |
|------|-------------|-------------|
| `Player` | Player character | sl_player.c |
| `Mob` | Monster/creature | sl_mob.c |
| `NPC` | Non-player character | sl_npc.c |
| `Item` | Inventory item | sl_item.c |
| `BlockList` | Spatial entity (base) | sl_blocklist.c |
| `Registry` | Data storage | sl_registry.c |

## Player API

### Attributes (Read)

Access via `player.attribute`:

```lua
player.name          -- Character name
player.level         -- Current level
player.health        -- Current HP
player.maxhealth     -- Maximum HP
player.mana          -- Current MP
player.maxmana       -- Maximum MP
player.gold          -- Gold amount
player.x             -- X coordinate
player.y             -- Y coordinate
player.map           -- Current map name
player.class         -- Class ID (1=Warrior, 2=Rogue, 3=Mage, 4=Poet)
player.path          -- Path/subpath ID
player.country       -- Country (0=Buya, 1=Kugnae, 2=Nagnang)
player.gender        -- Gender (0=Male, 1=Female)
player.strength      -- STR stat
player.dexterity     -- DEX stat
player.intelligence  -- INT stat
player.willpower     -- WIL stat
player.constitution  -- CON stat
player.ac            -- Armor class
player.damage        -- Damage range
player.hit           -- Hit rate
player.state         -- Player state (0=normal, 1=dead, etc.)
player.clan          -- Clan ID
player.clanrank      -- Clan rank
player.alignment     -- Alignment value
```

### Attributes (Read/Write)

```lua
player.health = 100       -- Set health
player.mana = 50          -- Set mana
player.gold = 1000        -- Set gold
player.level = 99         -- Set level
player.experience = 1000  -- Set experience
player.strength = 50      -- Set STR
-- etc. for other stats
```

### Health Methods

```lua
player:addHealth(amount)              -- Add HP
player:addHealth(amount, attacker)    -- Add HP with attacker reference
player:removeHealth(amount)           -- Remove HP
player:removeHealth(amount, attacker) -- Remove HP with attacker
player:sendHealth()                   -- Update health display
player:showHealth(target)             -- Show health to target
player:die()                          -- Kill player
player:resurrect()                    -- Resurrect player
```

### Movement Methods

```lua
player:warp(map, x, y)          -- Teleport to location
player:move(x, y)               -- Move to coordinates
player:respawn()                -- Return to respawn point
player:refresh()                -- Refresh player display
```

### Duration/Buff System

```lua
player:setDuration(name, value, time, icon)  -- Apply buff
player:hasDuration(name)                      -- Check if has buff
player:getDuration(name)                      -- Get buff value
player:getDurationId(name)                    -- Get buff ID
player:flushDuration()                        -- Remove all buffs
player:flushDuration(name)                    -- Remove specific buff
player:durationAmount()                       -- Count active buffs
player:getAllDurations()                      -- Get all buffs as table
```

### Aether System

```lua
player:setAether(name, value, icon)   -- Apply aether
player:hasAether(name)                -- Check if has aether
player:getAether(name)                -- Get aether value
player:flushAether()                  -- Remove all aethers
player:getAllAethers()                -- Get all aethers as table
```

### Legend Methods

```lua
player:addLegend(text, color, icon)   -- Add legend mark
player:hasLegend(text)                -- Check legend exists
player:getLegend(text)                -- Get legend data
player:removeLegendByName(text)       -- Remove by text
player:removeLegendByColor(color)     -- Remove by color
```

### Spell Methods

```lua
player:addSpell(name)                 -- Learn spell
player:removeSpell(name)              -- Forget spell
player:hasSpell(name)                 -- Check if knows spell
player:getSpells()                    -- Get all known spells
player:getUnknownSpells()             -- Get learnable spells
player:getAllClassSpells()            -- Get class spell list
```

### Inventory Methods

```lua
player:addItem(name, amount)          -- Add item
player:addItem(name, amount, dura)    -- Add with durability
player:hasItem(name)                  -- Check item count
player:hasItem(name, amount)          -- Check specific amount
player:hasItemDura(name, dura)        -- Check with durability
player:hasSpace(amount)               -- Check inventory space
player:removeInventoryItem(name)      -- Remove item
player:removeInventoryItem(name, amt) -- Remove amount
player:removeItemSlot(slot)           -- Remove from slot
player:getInventoryItem(slot)         -- Get item at slot
player:getEquippedItem(slot)          -- Get equipped item
player:updateInv()                    -- Refresh inventory
player:refreshInventory()             -- Full inventory refresh
```

### Equipment Methods

```lua
player:equip(name)                    -- Equip item
player:forceEquip(name, slot)         -- Force equip to slot
player:takeOff(slot)                  -- Unequip from slot
player:stripEquip()                   -- Remove all equipment
player:hasEquipped(name)              -- Check if item equipped
player:deductDura(slot, amount)       -- Reduce durability
player:deductArmor(amount)            -- Reduce armor dura
player:deductWeapon(amount)           -- Reduce weapon dura
```

### Item Actions

```lua
player:useItem(slot)                  -- Use item
player:forceDrop(slot)                -- Force drop item
player:throwItem(name, x, y)          -- Throw item to coords
player:pickup()                       -- Pick up ground item
player:expireItem(slot)               -- Expire item
```

### Banking Methods

```lua
player:getBankItems()                 -- Get bank items
player:bankDeposit(name, amount)      -- Deposit to bank
player:bankWithdraw(name, amount)     -- Withdraw from bank

player:getClanBankItems()             -- Clan bank items
player:clanBankDeposit(name, amount)  -- Clan deposit
player:clanBankWithdraw(name, amount) -- Clan withdraw

player:getSubpathBankItems()          -- Subpath bank items
player:subpathBankDeposit(name, amt)  -- Subpath deposit
player:subpathBankWithdraw(name, amt) -- Subpath withdraw
```

### Dialog Methods

```lua
player:dialog(npc, text)              -- Show NPC dialog
player:menu(npc, title, options)      -- Show menu
player:menuSeq(npc, title, options)   -- Sequential menu
player:input(npc, prompt)             -- Text input
player:inputSeq(npc, prompt)          -- Sequential input
player:popup(text)                    -- Show popup
player:paperPopup(text, w, h)         -- Paper popup
player:paperPopupWrite(text, w, h)    -- Writable paper
```

### Shop Methods

```lua
player:buy(npc, items)                -- Open buy dialog
player:sell(npc, items)               -- Open sell dialog
player:logBuySell(type, item, amt)    -- Log transaction
```

### Communication Methods

```lua
player:sendMiniText(text)             -- Show minitext
player:speak(text)                    -- Character speech
player:talkSelf(text)                 -- Talk to self
player:guiText(text, x, y)            -- GUI text display
player:sendMail(to, subject, body)    -- Send mail
player:sendUrl(url, type)             -- Open URL
```

### Board Methods

```lua
player:showBoard(board)               -- Show board
player:showPost(board, post)          -- Show specific post
player:sendBoardQuestions(board)      -- Get board questions
player:powerBoard(board)              -- Power board access
```

### Kill Tracking

```lua
player:killCount(mobname)             -- Get kill count
player:setKillCount(mobname, count)   -- Set kill count
player:flushKills(mobname)            -- Reset mob kills
player:flushAllKills()                -- Reset all kills
```

### Miscellaneous

```lua
player:addClan(clanid)                -- Join clan
player:addGuide(guide)                -- Add guide entry
player:delGuide(guide)                -- Remove guide entry
player:forceSave()                    -- Save character
player:lock()                         -- Lock for async
player:unlock()                       -- Unlock from async
player:freeAsync()                    -- Free async state
player:sendStatus()                   -- Update status
player:calcStat()                     -- Recalculate stats
player:status()                       -- Send status packet
player:level()                        -- Level up
player:addEventXP(amount)             -- Add event experience
```

## Mob API

### Attributes

```lua
mob.name              -- Mob name
mob.id                -- Mob ID
mob.health            -- Current HP
mob.maxhealth         -- Maximum HP
mob.x                 -- X coordinate
mob.y                 -- Y coordinate
mob.map               -- Current map
mob.state             -- State (0=alive, 1=dead)
mob.level             -- Level
mob.damage            -- Damage
mob.ac                -- Armor class
mob.experience        -- XP reward
```

### Health Methods

```lua
mob:addHealth(amount)                 -- Add HP
mob:removeHealth(amount)              -- Remove HP
mob:removeHealth(amount, attacker)    -- With attacker ref
mob:sendHealth()                      -- Update health display
```

### Movement Methods

```lua
mob:move(x, y)                        -- Move to coords
mob:moveIgnoreObject(x, y)            -- Move ignoring collision
mob:warp(map, x, y)                   -- Teleport
mob:moveGhost(x, y)                   -- Ghost movement
mob:moveIntent(x, y)                  -- Set move intent
mob:checkMove(x, y)                   -- Check if can move
mob:callBase()                        -- Return to spawn
```

### Duration System

```lua
mob:setDuration(name, value, time)    -- Apply effect
mob:hasDuration(name)                 -- Check effect
mob:getDuration(name)                 -- Get effect value
mob:flushDuration()                   -- Remove effects
mob:durationAmount()                  -- Count effects
```

### Combat Methods

```lua
mob:attack(target)                    -- Attack target
mob:checkThreat()                     -- Get threat info
mob:setIndDmg(player, amount)         -- Set individual damage
mob:setGrpDmg(group, amount)          -- Set group damage
mob:getIndDmg(player)                 -- Get individual damage
mob:getGrpDmg(group)                  -- Get group damage
```

### Miscellaneous

```lua
mob:getEquippedItem(slot)             -- Get equipment
mob:calcStat()                        -- Recalculate stats
mob:sendStatus()                      -- Send status
mob:sendMiniText(text)                -- Show minitext
```

## NPC API

### Attributes

```lua
npc.name              -- NPC name
npc.id                -- NPC ID
npc.x                 -- X coordinate
npc.y                 -- Y coordinate
npc.map               -- Current map
npc.look              -- Appearance ID
```

### Methods

```lua
npc:move(x, y)                        -- Move NPC
npc:warp(map, x, y)                   -- Teleport NPC
npc:getEquippedItem(slot)             -- Get NPC equipment
```

## BlockList API (Base Type)

All entities inherit from BlockList:

### Attributes

```lua
bl.id                 -- Block ID
bl.type               -- Type (PLAYER, MOB, NPC, etc.)
bl.x                  -- X coordinate
bl.y                  -- Y coordinate
bl.map                -- Map reference
```

### Methods

```lua
bl:look()                             -- Get appearance
bl:pos()                              -- Get position {x, y, map}
bl:distance(other)                    -- Distance to other
bl:inRange(other, range)              -- Check if in range
bl:getObjects(range, type)            -- Get nearby objects
```

## Registry API

### Global Registry

```lua
Registry[key]         -- Get value
Registry[key] = value -- Set value
```

### Player Registry

```lua
player.npc_reg[key]   -- NPC-specific registry
player.quest_reg[key] -- Quest registry
```

### Item Registry

```lua
item_db[name]         -- Get item definition
class_db[id]          -- Get class definition
```

## Global Functions

### Object Finding

```lua
GetPlayer(name)                       -- Find player by name
GetPlayerById(id)                     -- Find player by ID
GetMob(id)                            -- Find mob by ID
GetNPC(name)                          -- Find NPC by name
GetNPCById(id)                        -- Find NPC by ID
```

### Area Functions

```lua
GetPlayersInArea(map, x, y, range)    -- Get players in area
GetMobsInArea(map, x, y, range)       -- Get mobs in area
GetNPCsInArea(map, x, y, range)       -- Get NPCs in area
```

### Spawning

```lua
SpawnMob(name, map, x, y)             -- Spawn mob
SpawnNPC(name, map, x, y)             -- Spawn NPC
```

### Broadcasting

```lua
Broadcast(message)                    -- Server-wide message
BroadcastMap(map, message)            -- Map-wide message
BroadcastArea(map, x, y, r, msg)      -- Area message
```

### Utility

```lua
Random(min, max)                      -- Random number
Time()                                -- Current time
Date()                                -- Current date
Log(message)                          -- Server log
```

## Script Structure

### NPC Script Example

```lua
-- npc_shopkeeper.lua
function OnClick(player, npc)
    local t = {}  -- Dialog state

    player:dialog(npc, "Welcome to my shop!")

    local choice = player:menu(npc, "What would you like?", {
        "Buy items",
        "Sell items",
        "Leave"
    })

    if choice == 1 then
        player:buy(npc, {"Potion", "Scroll"})
    elseif choice == 2 then
        player:sell(npc, {})
    else
        player:dialog(npc, "Come again!")
    end
end
```

### Spell Script Example

```lua
-- spell_heal.lua
function OnCast(caster, target, spell)
    local healAmount = caster.intelligence * 2 + 50

    if target.health >= target.maxhealth then
        caster:sendMiniText("Target already at full health!")
        return false
    end

    target:addHealth(healAmount)
    target:sendHealth()

    caster:sendMiniText("Healed " .. target.name .. " for " .. healAmount)
    return true
end
```

### Event Script Example

```lua
-- event_login.lua
function OnLogin(player)
    player:sendMiniText("Welcome back, " .. player.name .. "!")

    -- Check for daily reward
    if not player:hasLegend("daily_" .. Date()) then
        player:addItem("Daily Reward Box", 1)
        player:addLegend("daily_" .. Date(), "white", 0)
        player:dialog(nil, "You received your daily reward!")
    end
end
```

## Files Reference

| File | Description |
|------|-------------|
| `sl.c` | Main Lua state management |
| `sl_types.c` | Type system implementation |
| `sl_player.c` | Player type bindings |
| `sl_mob.c` | Mob type bindings |
| `sl_npc.c` | NPC type bindings |
| `sl_item.c` | Item type bindings |
| `sl_blocklist.c` | BlockList base bindings |
| `sl_registry.c` | Registry type bindings |
