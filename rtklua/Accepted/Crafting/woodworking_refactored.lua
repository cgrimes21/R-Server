--[[
    woodworking_refactored.lua - Refactored woodworking using crafting_base

    Original: 905 lines
    Refactored: ~180 lines (80% reduction)

    Uses data-driven approach from crafting_base.lua

    Part of RTK Server refactoring - Phase 2.1
]]

-- Weapon definitions: { name, item, fineItem, skillReq, woodCost }
local WOOD_WEAPONS = {
    { name = "Wooden sword", item = "wooden_sword", fineItem = "supple_wooden_sword",
      skillReq = "novice", woodCost = 2 },
    { name = "Viperhead woodsaber", item = "viperhead_woodsaber", fineItem = "supple_viperhead_woodsaber",
      skillReq = "apprentice", woodCost = 2 },
    { name = "Viperhead woodsword", item = "viperhead_woodsword", fineItem = "supple_viperhead_woodsword",
      skillReq = "accomplished", woodCost = 3 },
    { name = "Oaken sword", item = "oaken_sword", fineItem = "supple_oaken_sword",
      skillReq = "adept", woodCost = 3 },
    { name = "Oaken blade", item = "oaken_blade", fineItem = "supple_oaken_blade",
      skillReq = "talented", woodCost = 4 },
    { name = "Juk-do", item = "juk_do", fineItem = "supple_juk_do",
      skillReq = "expert", woodCost = 4 },
    { name = "Wooden blade", item = "wooden_blade", fineItem = "supple_wooden_blade",
      skillReq = "grandmaster", woodCost = 5 }
}

-- Quiver definitions by skill
local QUIVERS = {
    { item = "spring_quiver", skillReq = "novice" },
    { item = "summer_quiver", skillReq = "apprentice" },
    { item = "autumn_quiver", skillReq = "accomplished" },
    { item = "winter_quiver", skillReq = "adept" },
    { item = "ancient_quiver", skillReq = "talented" },
    { item = "blood_quiver", skillReq = "skilled" },
    { item = "earth_quiver", skillReq = "expert" },
    { item = "star_quiver", skillReq = "master" }
}

-- Weaving tools success thresholds by skill
local WEAVING_SUCCESS = {
    novice = 50, apprentice = 45, accomplished = 40, adept = 35,
    talented = 30, skilled = 25, expert = 20, master = 18,
    grandmaster = 15, champion = 15, legendary = 12
}

-- Skill level order for comparison
local SKILL_ORDER = {
    novice = 1, apprentice = 2, accomplished = 3, adept = 4,
    talented = 5, skilled = 6, expert = 7, master = 8,
    grandmaster = 9, champion = 10, legendary = 11
}

local function hasSkillLevel(playerSkill, requiredSkill)
    return (SKILL_ORDER[playerSkill] or 0) >= (SKILL_ORDER[requiredSkill] or 99)
end

woodworking = {
    craft = function(player, npc, speech)
        local t = {
            graphic = convertGraphic(npc.look, "monster"),
            color = npc.lookColor
        }
        player.npcGraphic = t.graphic
        player.npcColor = t.color
        player.dialogType = 0
        player.lastClick = npc.ID

        if speech == "wood" then
            local titem = { graphic = convertGraphic(723, "item"), color = 0 }
            local choices = { "Melee Weapons", "Arrows", "Weaving Tools", "Water jug" }

            local choice = player:menuSeq("What would you like to craft?", choices, {})
            local woodlevel = crafting.getSkillLevel(player, "woodworking")

            if choice == 1 then
                woodworking.craftWeapon(player, npc, woodlevel, titem)
            elseif choice == 2 then
                woodworking.craftArrows(player, npc, woodlevel, titem)
            elseif choice == 3 then
                woodworking.craftWeavingTools(player, npc, woodlevel, titem)
            elseif choice == 4 then
                woodworking.craftWaterJug(player, npc, titem)
            end
        end
    end,

    craftWeapon = function(player, npc, woodlevel, titem)
        -- Build available weapon list
        local craftable = {}
        for _, weapon in ipairs(WOOD_WEAPONS) do
            if hasSkillLevel(woodlevel, weapon.skillReq) then
                table.insert(craftable, weapon)
            end
        end

        if #craftable == 0 then
            player:sendMinitext("You don't have the skill to craft weapons yet.")
            return
        end

        local opts = {}
        for _, weapon in ipairs(craftable) do
            table.insert(opts, weapon.name)
        end

        local weaponChoice = player:menuSeq("What would you like to craft?", opts, {})
        local selectedWeapon = craftable[weaponChoice]

        if not selectedWeapon then return end

        -- Check materials
        if player:hasItem("ginko_wood", selectedWeapon.woodCost) ~= true then
            player:dialogSeq({ titem, "You need " .. selectedWeapon.woodCost .. " units of Ginko Wood to make this." }, 0)
            return
        end

        -- Consume materials and give skill
        player:removeItem("ginko_wood", selectedWeapon.woodCost)
        for i = 1, selectedWeapon.woodCost do
            crafting.skillChanceIncrease(player, NPC("Splinter"), "woodworking")
        end

        -- Roll for result (10% fine, 10% fail, 80% normal)
        local rand = math.random(1, 10)
        if rand == 1 then
            player:addItem(selectedWeapon.fineItem, 1)
            crafting_base.showItemDialog(player, selectedWeapon.fineItem, "Your efforts were successful!")
        elseif rand == 2 then
            player:addItem("wood_scraps", 1)
            crafting_base.showItemDialog(player, "wood_scraps", "Your efforts were unsuccessful.")
        else
            player:addItem(selectedWeapon.item, 1)
            crafting_base.showItemDialog(player, selectedWeapon.item, "Your efforts were successful!")
        end
    end,

    craftArrows = function(player, npc, woodlevel, titem)
        -- Build available quiver list
        local craftable = {}
        for _, quiver in ipairs(QUIVERS) do
            if hasSkillLevel(woodlevel, quiver.skillReq) then
                table.insert(craftable, quiver.item)
            end
        end

        if #craftable == 0 then
            player:sendMinitext("You don't have the skill to craft quivers yet.")
            return
        end

        -- Check materials (5 wood for quivers)
        if player:hasItem("ginko_wood", 5) ~= true then
            player:dialogSeq({ titem, "You need 5 units of Ginko Wood to make arrows." }, 0)
            return
        end

        player:removeItem("ginko_wood", 5)
        for i = 1, 5 do
            crafting.skillChanceIncrease(player, NPC("Splinter"), "woodworking")
        end

        -- Roll for quality (10% fail, random tier from available)
        local rand = math.random(1, 10)
        if rand == 1 then
            player:addItem("wood_scraps", 1)
            crafting_base.showItemDialog(player, "wood_scraps", "Your efforts were unsuccessful.")
        else
            local quiverIndex = math.random(1, #craftable)
            local quiver = craftable[quiverIndex]
            player:addItem(quiver, 1)
            crafting_base.showItemDialog(player, quiver, "Your efforts were successful!")
        end
    end,

    craftWeavingTools = function(player, npc, woodlevel, titem)
        local successThreshold = WEAVING_SUCCESS[woodlevel] or 50

        if player:hasItem("ginko_wood", 1) ~= true then
            player:dialogSeq({ titem, "You need 1 unit of Ginko Wood to make weaving tools." }, 0)
            return
        end

        player:removeItem("ginko_wood", 1)
        crafting.skillChanceIncrease(player, NPC("Splinter"), "woodworking")

        local rand = math.random(1, 100)
        if rand > successThreshold then
            local fineRoll = math.random(1, 10)
            local item = (fineRoll == 1) and "fine_weaving_tools" or "weaving_tools"
            player:addItem(item, 1)
            crafting_base.showItemDialog(player, item, "Your efforts were successful!")
        else
            player:addItem("wood_scraps", 1)
            crafting_base.showItemDialog(player, "wood_scraps", "Your efforts were unsuccessful.")
        end
    end,

    craftWaterJug = function(player, npc, titem)
        if player:hasItem("ginko_wood", 2) ~= true then
            player:dialogSeq({ titem, "You need 2 units of Ginko Wood to make a water jug." }, 0)
            return
        end

        player:removeItem("ginko_wood", 2)
        crafting.skillChanceIncrease(player, NPC("Splinter"), "woodworking")
        crafting.skillChanceIncrease(player, NPC("Splinter"), "woodworking")

        player:addItem("water_jug", 1)
        crafting_base.showItemDialog(player, "water_jug", "Your efforts were successful!")
    end
}
