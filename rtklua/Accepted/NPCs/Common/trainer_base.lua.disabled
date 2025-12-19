--[[
    trainer_base.lua - Shared trainer NPC infrastructure

    This module provides common functions for class trainer NPCs
    to eliminate code duplication across trainer files.

    Usage:
        local opts = trainer_base.buildMenuOptions(player, npc, classData)
        trainer_base.handleCommonChoice(player, npc, choice, classData)

    Part of RTK Server refactoring - Phase 2.2
]]

trainer_base = {}

-- Class definitions
trainer_base.CLASS_DATA = {
    [1] = { name = "Warrior", prefix = "Warrior", action = "Become a Warrior" },
    [2] = { name = "Rogue", prefix = "Rogue", action = "Become a Rogue" },
    [3] = { name = "Mage", prefix = "Mage", action = "Become a Mage" },
    [4] = { name = "Poet", prefix = "Poet", action = "Become a Poet" }
}

-- Quest stage configurations
trainer_base.QUEST_STAGES = {
    star = {
        levelReq = 66,
        requiredLegend = "blessed_by_the_stars",
        blockedLegend = "mastered_the_stars",
        questKey = "star_armor",
        maxStage = 3
    },
    moon = {
        levelReq = 76,
        requiredLegend = "mastered_the_stars",
        blockedLegend = "understood_the_moon",
        alsoBlocked = "survived_the_sun",
        questKey = "moon_armor",
        maxStage = 5  -- Varies by class, this is default
    },
    sun = {
        levelReq = 86,
        requiredLegend = "mastered_the_stars",
        requiredLegend2 = "understood_the_moon",
        blockedLegend = "survived_the_sun",
        questKey = "sun_armor",
        maxStage = 8  -- Varies by class
    }
}

-- Class-specific max stages for quests
trainer_base.CLASS_MAX_STAGES = {
    [1] = { moon = 4, sun = 6 },  -- Warrior
    [2] = { moon = 4, sun = 5 },  -- Rogue
    [3] = { moon = 5, sun = 8 },  -- Mage
    [4] = { moon = 4, sun = 6 }   -- Poet
}

--[[
    Initialize NPC dialog state

    @param player - Player object
    @param npc - NPC object
    @return table with graphic/color for dialogs
]]
function trainer_base.initDialog(player, npc)
    local t = {
        graphic = convertGraphic(npc.look, "monster"),
        color = npc.lookColor
    }
    player.npcGraphic = t.graphic
    player.npcColor = t.color
    player.dialogType = 0
    player.lastClick = npc.ID
    return t
end

--[[
    Build standard trainer menu options

    @param player - Player object
    @param baseClass - The class ID this trainer teaches (1-4)
    @return table of menu option strings
]]
function trainer_base.buildStandardOptions(player, baseClass)
    local opts = {}
    local classData = trainer_base.CLASS_DATA[baseClass]

    if not classData then
        return opts
    end

    -- Class change option for classless players
    if player.class == 0 then
        table.insert(opts, classData.action)
    elseif player.baseClass == baseClass then
        -- Spell options for this class
        if player.level < 99 then
            table.insert(opts, "Divine Secret")
        end
        table.insert(opts, "Learn Secret")
    end

    table.insert(opts, "Forget Secret")
    table.insert(opts, "Become Noble")
    table.insert(opts, "Minor Quest")

    if player.registryString["minor_quest"] ~= "" then
        table.insert(opts, "Complete Minor Quest")
    end

    return opts
end

--[[
    Add quest progression options (Star/Moon/Sun armor)

    @param player - Player object
    @param opts - Menu options table to modify
    @param baseClass - The class ID (1-4)
]]
function trainer_base.addQuestOptions(player, opts, baseClass)
    if player.baseClass ~= baseClass then
        return
    end

    local classData = trainer_base.CLASS_DATA[baseClass]
    local classStages = trainer_base.CLASS_MAX_STAGES[baseClass] or {}

    -- Star armor quest (level 66+)
    local star = trainer_base.QUEST_STAGES.star
    if player.level >= star.levelReq
       and player:hasLegend(star.requiredLegend)
       and not player:hasLegend(star.blockedLegend) then
        local stage = player.quest[star.questKey] or 0
        if stage == 0 or stage == 1 then
            table.insert(opts, classData.prefix .. " Star 1")
        elseif stage == 2 then
            table.insert(opts, classData.prefix .. " Star 2")
        elseif stage == 3 then
            table.insert(opts, classData.prefix .. " Star 3")
        end
    end

    -- Moon armor quest (level 76+)
    local moon = trainer_base.QUEST_STAGES.moon
    if player.level >= moon.levelReq
       and player:hasLegend(moon.requiredLegend)
       and not player:hasLegend(moon.blockedLegend)
       and not player:hasLegend(moon.alsoBlocked) then
        local stage = player.quest[moon.questKey] or 0
        local maxStage = classStages.moon or moon.maxStage
        for i = 1, maxStage do
            if stage == 0 or stage == i then
                table.insert(opts, classData.prefix .. " Moon " .. i)
                break
            end
        end
    end

    -- Sun armor quest (level 86+)
    local sun = trainer_base.QUEST_STAGES.sun
    if player.level >= sun.levelReq
       and player:hasLegend(sun.requiredLegend)
       and player:hasLegend(sun.requiredLegend2)
       and not player:hasLegend(sun.blockedLegend) then
        local stage = player.quest[sun.questKey] or 0
        local maxStage = classStages.sun or sun.maxStage
        for i = 1, maxStage do
            if stage == 0 or stage == i then
                table.insert(opts, classData.prefix .. " Sun " .. i)
                break
            end
        end
    end
end

--[[
    Handle common menu choices

    @param player - Player object
    @param npc - NPC object
    @param choice - Menu choice string
    @param t - Dialog graphic table from initDialog()
    @return true if choice was handled, false otherwise
]]
function trainer_base.handleCommonChoice(player, npc, choice, t)
    if choice == "Become Noble" then
        if player.level < 75 then
            player:dialogSeq({
                t,
                "You are still young, and not ready for this yet. Return when you have gained your 75th level."
            }, 1)
            return true
        else
            general_npc_funcs.setTitle(player, npc)
            return true
        end
    elseif choice == "Minor Quest" then
        MinorQuest.quest(player, npc)
        return true
    elseif choice == "Complete Minor Quest" then
        MinorQuest.complete(player, npc)
        return true
    elseif choice == "Divine Secret" then
        player:futureSpells(npc)
        return true
    elseif choice == "Learn Secret" then
        player:learnSpell(npc)
        return true
    elseif choice == "Forget Secret" then
        player:forgetSpell(npc)
        return true
    end

    return false
end

--[[
    Handle class enrollment (Become a Warrior/Rogue/Mage/Poet)

    @param player - Player object
    @param npc - NPC object
    @param baseClass - The class ID (1-4)
    @param t - Dialog graphic table from initDialog()
    @param welcomeMessages - Table of welcome dialog strings
    @return true if player should continue, false to end dialog
]]
function trainer_base.handleClassEnrollment(player, npc, baseClass, t, welcomeMessages)
    local classData = trainer_base.CLASS_DATA[baseClass]

    if player.level < 5 then
        player:dialogSeq({
            t,
            "Hail, little one! Please return to me when you have reached the 5th insight."
        }, 0)
        return false
    end

    -- Show welcome messages
    player:dialogSeq(welcomeMessages, 1)

    local choice2 = player:menuString(
        "Will you join the path of the " .. string.lower(classData.name) .. "?",
        {"Yes", "Tell me more", "No"}
    )

    return choice2
end

--[[
    Show the "Tell me more" class description

    @param player - Player object
    @param t - Dialog graphic table
    @param classDescription - Table of description dialog strings
]]
function trainer_base.showClassDescription(player, t, classDescription)
    player:dialogSeq(classDescription, 1)
end

--[[
    Check if a choice matches a quest stage pattern

    @param choice - Menu choice string
    @param classPrefix - Class prefix (e.g. "Mage")
    @param questType - Quest type ("Star", "Moon", or "Sun")
    @return stage number if match, nil otherwise
]]
function trainer_base.parseQuestChoice(choice, classPrefix, questType)
    local pattern = classPrefix .. " " .. questType .. " (%d+)"
    local stage = choice:match(pattern)
    if stage then
        return tonumber(stage)
    end
    return nil
end

return trainer_base
