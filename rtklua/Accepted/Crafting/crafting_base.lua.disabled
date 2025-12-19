--[[
    crafting_base.lua - Data-driven crafting infrastructure

    This module provides shared crafting functions and threshold tables
    to eliminate code duplication across crafting files.

    Usage:
        local result = crafting_base.rollCraft(player, "gemcutting", thresholds)
        crafting_base.giveCraftResult(player, result, itemDefs)

    Part of RTK Server refactoring - Phase 2.1
]]

crafting_base = {}

-- Skill levels in order (for reference)
crafting_base.SKILL_LEVELS = {
    "novice", "apprentice", "accomplished", "adept", "talented",
    "skilled", "expert", "master", "grandmaster", "champion", "legendary"
}

-- Standard crafting messages
crafting_base.MESSAGES = {
    masterful = "You have succeeded masterfully!",
    success = "Your efforts are successful!",
    failure = "You have failed in your attempt.",
    destroyed = "Your feeble efforts have destroyed that which you meant to enhance."
}

--[[
    Gemcutting thresholds extracted from gemcutting.lua

    Format: { masterful_max, success_max, failure_max }
    - Roll 1-masterful_max = masterful (well_crafted)
    - Roll masterful_max+1 to success_max = success (crafted)
    - Roll success_max to failure_max = failure (tarnished)
    - Roll > failure_max = destroyed (nothing)

    Note: When failure_max >= 100, no destruction is possible
]]
crafting_base.GEMCUTTING_THRESHOLDS = {
    -- Basic amber (available from novice)
    amber = {
        novice      = { masterful = 5,  success = 15, failure = 90 },
        apprentice  = { masterful = 5,  success = 20, failure = 90 },
        accomplished = { masterful = 5, success = 25, failure = 90 },
        adept       = { masterful = 5,  success = 30, failure = 100 },
        talented    = { masterful = 5,  success = 35, failure = 100 },
        skilled     = { masterful = 10, success = 40, failure = 100 },
        expert      = { masterful = 10, success = 45, failure = 100 },
        master      = { masterful = 10, success = 50, failure = 100 },
        grandmaster = { masterful = 13, success = 60, failure = 100 },
        champion    = { masterful = 16, success = 65, failure = 100 },
        legendary   = { masterful = 20, success = 70, failure = 100 },
        default     = { masterful = 1,  success = 20, failure = 90 }
    },
    -- Dark amber (available from adept)
    dark_amber = {
        adept       = { masterful = 5,  success = 15, failure = 90 },
        talented    = { masterful = 5,  success = 20, failure = 100 },
        skilled     = { masterful = 10, success = 25, failure = 100 },
        expert      = { masterful = 10, success = 30, failure = 100 },
        master      = { masterful = 10, success = 35, failure = 100 },
        grandmaster = { masterful = 12, success = 40, failure = 100 },
        champion    = { masterful = 14, success = 45, failure = 100 },
        legendary   = { masterful = 16, success = 50, failure = 100 },
        default     = { masterful = 1,  success = 20, failure = 90 }
    },
    -- White amber (available from adept)
    white_amber = {
        adept       = { masterful = 1,  success = 15, failure = 90 },
        talented    = { masterful = 2,  success = 20, failure = 90 },
        skilled     = { masterful = 3,  success = 25, failure = 90 },
        expert      = { masterful = 4,  success = 30, failure = 100 },
        master      = { masterful = 5,  success = 35, failure = 100 },
        grandmaster = { masterful = 6,  success = 38, failure = 100 },
        champion    = { masterful = 7,  success = 41, failure = 100 },
        legendary   = { masterful = 8,  success = 45, failure = 100 },
        default     = { masterful = 1,  success = 20, failure = 90 }
    },
    -- Yellow amber (available from skilled)
    yellow_amber = {
        skilled     = { masterful = 10, success = 30, failure = 100 },
        expert      = { masterful = 10, success = 40, failure = 100 },
        master      = { masterful = 10, success = 50, failure = 100 },
        grandmaster = { masterful = 12, success = 55, failure = 100 },
        champion    = { masterful = 14, success = 60, failure = 100 },
        legendary   = { masterful = 16, success = 65, failure = 100 },
        default     = { masterful = 10, success = 30, failure = 100 }
    }
}

-- Item name patterns for gemcutting
crafting_base.GEMCUTTING_ITEMS = {
    amber = {
        masterful = "well_crafted_amber",
        success = "crafted_amber",
        failure = "tarnished_amber",
        base = "amber"
    },
    dark_amber = {
        masterful = "well_crafted_dark_amber",
        success = "crafted_dark_amber",
        failure = "tarnished_dark_amber",
        base = "dark_amber"
    },
    white_amber = {
        masterful = "well_crafted_white_amber",
        success = "crafted_white_amber",
        failure = "tarnished_white_amber",
        base = "white_amber"
    },
    yellow_amber = {
        masterful = "well_crafted_yellow_amber",
        success = "crafted_yellow_amber",
        failure = "tarnished_yellow_amber",
        base = "yellow_amber"
    }
}

--[[
    Roll for crafting result based on skill thresholds

    @param skill - Player's skill level string (e.g. "novice", "master")
    @param thresholds - Table of thresholds by skill level
    @return "masterful", "success", "failure", or "destroyed"
]]
function crafting_base.rollResult(skill, thresholds)
    local t = thresholds[skill] or thresholds.default or thresholds.novice
    if not t then
        return "failure"
    end

    local rand = math.random(1, 100)

    if rand <= t.masterful then
        return "masterful"
    elseif rand <= t.success then
        return "success"
    elseif rand <= t.failure then
        return "failure"
    else
        return "destroyed"
    end
end

--[[
    Give crafting result to player with appropriate dialog

    @param player - Player object
    @param result - Result string from rollResult()
    @param items - Item definition table with masterful/success/failure/base keys
    @param messages - Optional custom messages table
]]
function crafting_base.giveCraftResult(player, result, items, messages)
    local msgs = messages or crafting_base.MESSAGES

    if result == "masterful" and items.masterful then
        player:addItem(items.masterful, 1)
        player:dialogSeq({
            {
                graphic = Item(items.masterful).icon,
                color = Item(items.masterful).iconC
            },
            msgs.masterful
        }, 0)
    elseif result == "success" and items.success then
        player:addItem(items.success, 1)
        player:dialogSeq({
            {
                graphic = Item(items.success).icon,
                color = Item(items.success).iconC
            },
            msgs.success
        }, 0)
    elseif result == "failure" and items.failure then
        player:addItem(items.failure, 1)
        player:dialogSeq({
            {
                graphic = Item(items.failure).icon,
                color = Item(items.failure).iconC
            },
            msgs.failure
        }, 0)
    else
        -- Destroyed - show base item icon with destroyed message
        local icon = items.base and Item(items.base).icon or 0
        local iconC = items.base and Item(items.base).iconC or 0
        player:dialogSeq({
            {
                graphic = icon,
                color = iconC
            },
            msgs.destroyed
        }, 0)
    end
end

--[[
    Complete gemcutting craft - combines rollResult and giveCraftResult

    @param player - Player object
    @param gemType - Gem type key (e.g. "amber", "dark_amber")
    @return true if something was given (not destroyed), false otherwise
]]
function crafting_base.craftGem(player, gemType)
    local skill = crafting.getSkillLevel(player, "gemcutting")
    local thresholds = crafting_base.GEMCUTTING_THRESHOLDS[gemType]
    local items = crafting_base.GEMCUTTING_ITEMS[gemType]

    if not thresholds or not items then
        return false
    end

    local result = crafting_base.rollResult(skill, thresholds)
    crafting_base.giveCraftResult(player, result, items)

    return result ~= "destroyed"
end

--[[
    Get available gem options based on skill level

    @param skill - Player's skill level string
    @return table of {display_name, gem_key} pairs
]]
function crafting_base.getGemOptions(skill)
    local options = {}

    -- Amber always available
    table.insert(options, { display = "Amber", key = "amber" })

    -- Dark/White Amber from adept
    local advancedSkills = { "adept", "talented", "skilled", "expert",
                            "master", "grandmaster", "champion", "legendary" }
    for _, s in ipairs(advancedSkills) do
        if skill == s then
            table.insert(options, { display = "Dark Amber", key = "dark_amber" })
            table.insert(options, { display = "White Amber", key = "white_amber" })
            break
        end
    end

    -- Yellow Amber from skilled
    local yellowSkills = { "skilled", "expert", "master", "grandmaster",
                          "champion", "legendary" }
    for _, s in ipairs(yellowSkills) do
        if skill == s then
            table.insert(options, { display = "Yellow Amber", key = "yellow_amber" })
            break
        end
    end

    return options
end

-- ============================================================================
-- JEWELRY MAKING SUPPORT
-- ============================================================================

-- Amber types for jewelry making
crafting_base.JEWELRY_AMBERS = {
    { key = "amber", display = "Amber", material = "crafted_amber", wellMaterial = "well_crafted_amber" },
    { key = "dark", display = "Dark amber", material = "crafted_dark_amber", wellMaterial = "well_crafted_dark_amber" },
    { key = "white", display = "White amber", material = "crafted_white_amber", wellMaterial = "well_crafted_white_amber" },
    { key = "yellow", display = "Yellow amber", material = "crafted_yellow_amber", wellMaterial = "well_crafted_yellow_amber" }
}

-- Totem types for special ring/headgear crafts
crafting_base.TOTEMS = { "hyun_moo", "ju_jak", "chung_ryong", "baekho" }

-- Ring size thresholds (multiplied by skill value)
-- well_crafted materials get 2x multiplier
crafting_base.RING_THRESHOLDS = {
    normal = { large = 1, medium = 3, small = 6 },   -- multiplied by skill
    well = { large = 2, medium = 6, small = 12 }     -- multiplied by skill
}

-- Jewelry costs
crafting_base.JEWELRY_COSTS = {
    ring = { gold = 100, materials = 2 },
    bracelet = { gold = 5000, materials = 4 },
    headgear = { gold = 10000, materials = 8 }
}

--[[
    Check if player has required jewelry materials

    @param player - Player object
    @param amberType - Amber type table from JEWELRY_AMBERS
    @param amount - Amount needed
    @return material key if found, nil otherwise
]]
function crafting_base.hasJewelryMaterials(player, amberType, amount)
    if player:hasItem(amberType.material, amount) == true then
        return amberType.material
    elseif player:hasItem(amberType.wellMaterial, amount) == true then
        return amberType.wellMaterial
    end
    return nil
end

--[[
    Get available jewelry amber options based on player inventory

    @param player - Player object
    @param materialAmount - Amount of materials needed
    @return table of available amber options
]]
function crafting_base.getJewelryAmberOptions(player, materialAmount)
    local options = {}
    for _, amber in ipairs(crafting_base.JEWELRY_AMBERS) do
        if crafting_base.hasJewelryMaterials(player, amber, materialAmount) then
            table.insert(options, amber)
        end
    end
    return options
end

--[[
    Check for totem ring chance (1% with 3-day cooldown)

    @param player - Player object
    @param registryKey - Registry key for cooldown tracking
    @return totem name if lucky, nil otherwise
]]
function crafting_base.checkTotemChance(player, registryKey)
    local totemRoll = math.random(1, 100)
    if totemRoll == 1 then
        if os.time() > (player.registry[registryKey] or 0) then
            player.registry[registryKey] = os.time() + 259200  -- 3 day cooldown
            return crafting_base.TOTEMS[math.random(1, 4)]
        end
    end
    return nil
end

--[[
    Roll for ring size based on skill value

    @param skillValue - Numeric skill value (1-11)
    @param isWellCrafted - Whether using well_crafted materials
    @return "large", "medium", "small", or nil (destroyed)
]]
function crafting_base.rollRingSize(skillValue, isWellCrafted)
    local rand = math.random(1, 100)
    local thresholds = isWellCrafted and crafting_base.RING_THRESHOLDS.well or crafting_base.RING_THRESHOLDS.normal

    if rand <= thresholds.large * skillValue then
        return "large"
    elseif rand <= thresholds.medium * skillValue then
        return "medium"
    elseif rand <= thresholds.small * skillValue then
        return "small"
    else
        return nil  -- destroyed
    end
end

--[[
    Craft a ring with jewelry making

    @param player - Player object
    @param npc - NPC object for skill increase
    @param amberType - Amber type table from JEWELRY_AMBERS
    @param materialUsed - Material key that was consumed
    @return true if successful
]]
function crafting_base.craftRing(player, npc, amberType, materialUsed)
    local skillValue = crafting.getSkillValue(player, "jewelry making")
    local isWellCrafted = materialUsed:match("well_crafted")

    -- Skill increase (2x for rings)
    crafting.skillChanceIncrease(player, npc, "jewelry making")
    crafting.skillChanceIncrease(player, npc, "jewelry making")

    -- Check for totem ring
    local totem = crafting_base.checkTotemChance(player, "totemring")
    if totem then
        local size = crafting_base.rollRingSize(skillValue, isWellCrafted) or "small"
        local itemName = size .. "_" .. totem .. "_ring"
        player:addItem(itemName, 1)
        player:dialogSeq({
            { graphic = Item(itemName).icon, color = Item(itemName).iconC },
            "The totems have blessed your work!"
        }, 0)
        return true
    end

    -- Normal ring craft
    local size = crafting_base.rollRingSize(skillValue, isWellCrafted)
    if size then
        local itemName = size .. "_" .. amberType.key .. "_ring"
        player:addItem(itemName, 1)
        player:dialogSeq({
            { graphic = Item(itemName).icon, color = Item(itemName).iconC },
            "Your efforts were successful!"
        }, 0)
        return true
    else
        player:dialogSeq({
            { graphic = Item(materialUsed).icon, color = Item(materialUsed).iconC },
            "Your feeble efforts have destroyed that which you meant to enhance."
        }, 0)
        return false
    end
end

--[[
    Show dialog with item icon

    @param player - Player object
    @param itemName - Item name for icon lookup
    @param message - Message to display
]]
function crafting_base.showItemDialog(player, itemName, message)
    player:dialogSeq({
        { graphic = Item(itemName).icon, color = Item(itemName).iconC },
        message
    }, 0)
end

-- ============================================================================
-- METALWORKING SUPPORT
-- ============================================================================

-- Weapon definitions: { skillReq, metalCost, successMultiplier }
crafting_base.METAL_WEAPONS = {
    { name = "Steel dagger", item = "steel_dagger", fineItem = "fine_steel_dagger",
      skillReq = 0, metalCost = 2, successMult = 10 },
    { name = "Steel saber", item = "steel_saber", fineItem = "fine_steel_saber",
      skillReq = 2, metalCost = 3, successMult = 10 },
    { name = "Steel sword", item = "steel_sword", fineItem = "fine_steel_sword",
      skillReq = 4, metalCost = 4, successMult = 10 },
    { name = "Steel blade", item = "steel_blade", fineItem = "fine_steel_blade",
      skillReq = 6, metalCost = 5, successMult = 7 }
}

-- Armor types by gender
crafting_base.METAL_ARMOR_TYPES = {
    male = {
        { name = "Scale mail", prefix = "", suffix = "_scale_mail",
          items = {"jade_scale_mail", "royal_scale_mail", "sky_scale_mail",
                   "ancient_scale_mail", "blood_scale_mail", "earth_scale_mail", "star_scale_mail"} },
        { name = "Armor", prefix = "", suffix = "_armor",
          items = {"farmer_armor", "royal_armor", "sky_armor",
                   "ancient_armor", "blood_armor", "earth_armor", "star_armor"} }
    },
    female = {
        { name = "Mail dress", prefix = "", suffix = "_mail_dress",
          items = {"summer_mail_dress", "autumn_mail_dress", "winter_mail_dress",
                   "ancient_mail_dress", "blood_mail_dress", "earth_mail_dress", "star_mail_dress"} },
        { name = "War dress", prefix = "", suffix = "_war_dress",
          items = {"summer_war_dress", "autumn_war_dress", "winter_war_dress",
                   "ancient_war_dress", "blood_war_dress", "earth_war_dress", "star_war_dress"} },
        { name = "Armor dress", prefix = "", suffix = "_armor_dress",
          items = {"summer_armor_dress", "autumn_armor_dress", "winter_armor_dress",
                   "ancient_armor_dress", "blood_armor_dress", "earth_armor_dress", "star_armor_dress"} }
    }
}

-- Skill level thresholds for armor tiers (0-indexed for array)
crafting_base.ARMOR_SKILL_TIERS = {
    { min = 0, max = 2, tier = 1 },
    { min = 2, max = 4, tier = 2 },
    { min = 3, max = 5, tier = 3 },
    { min = 4, max = 7, tier = 4 },
    { min = 6, max = 9, tier = 5 },
    { min = 8, max = 10, tier = 6 },
    { min = 9, max = 99, tier = 7 }
}

--[[
    Get available weapon options based on skill

    @param skillValue - Numeric metalworking skill value
    @return table of available weapon definitions
]]
function crafting_base.getMetalWeaponOptions(skillValue)
    local options = {}
    for _, weapon in ipairs(crafting_base.METAL_WEAPONS) do
        if skillValue >= weapon.skillReq then
            table.insert(options, weapon)
        end
    end
    return options
end

--[[
    Craft a metal weapon

    @param player - Player object
    @param npc - NPC object
    @param weapon - Weapon definition from METAL_WEAPONS
    @return true if successful
]]
function crafting_base.craftMetalWeapon(player, npc, weapon)
    local skillValue = crafting.getSkillValue(player, "metalworking")
    local usingFine = false

    -- Check materials
    if player:hasItem("fine_metal", weapon.metalCost) == true then
        usingFine = true
    elseif player:hasItem("metal", weapon.metalCost) ~= true then
        crafting_base.showItemDialog(player, "metal",
            "You need " .. weapon.metalCost .. " units of metal to craft a " .. weapon.name .. ".")
        return false
    end

    -- Consume materials
    local material = usingFine and "fine_metal" or "metal"
    player:removeItem(material, weapon.metalCost)

    -- Give skill increases (one per metal used)
    for i = 1, weapon.metalCost do
        crafting.skillChanceIncrease(player, npc, "metalworking")
    end

    -- Roll for success
    local rand = math.random(1, 100)
    local successThreshold = 10 + (skillValue * weapon.successMult)
    local fineChance = math.random(1, 15 - skillValue)

    -- Fine metal always succeeds
    if usingFine then
        local itemName = (fineChance == 1) and weapon.fineItem or weapon.item
        local msg = (fineChance == 1) and "You have succeeded masterfully!" or "Your efforts are successful!"
        player:addItem(itemName, 1)
        crafting_base.showItemDialog(player, itemName, msg)
        return true
    end

    -- Normal metal crafting
    if rand <= successThreshold then
        local itemName = (fineChance == 1) and weapon.fineItem or weapon.item
        local msg = (fineChance == 1) and "You have succeeded masterfully!" or "Your efforts are successful!"
        player:addItem(itemName, 1)
        crafting_base.showItemDialog(player, itemName, msg)
        return true
    else
        -- Second chance mechanic
        local secondChance = math.random(1, 2)
        if secondChance == 1 and player:hasItem("metal", 1) == true then
            local choice = player:menuString(
                "Your work shows some progress, but you need more materials. Continue your efforts?",
                {"Yes", "No"}
            )
            if choice == "Yes" then
                player:removeItem("metal", 1)
                crafting.skillChanceIncrease(player, npc, "metalworking")
                if math.random(1, 2) == 1 then
                    player:addItem(weapon.item, 1)
                    crafting_base.showItemDialog(player, weapon.item, "Your efforts are successful!")
                    return true
                end
            end
        end

        -- Failed
        player:addItem("spent_metal", 1)
        crafting_base.showItemDialog(player, "metal",
            "Your feeble efforts have destroyed that which you meant to enhance.")
        return false
    end
end

--[[
    Get armor tier index based on skill value

    @param skillValue - Numeric metalworking skill value
    @return tier index (1-7)
]]
function crafting_base.getArmorTier(skillValue)
    for _, tier in ipairs(crafting_base.ARMOR_SKILL_TIERS) do
        if skillValue >= tier.min and skillValue <= tier.max then
            return tier.tier
        end
    end
    return 1
end

return crafting_base
