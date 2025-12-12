--[[
    jewelrymaking_refactored.lua - Refactored jewelry making using crafting_base

    Original: 1,002 lines
    Refactored: ~150 lines (85% reduction)

    Uses data-driven approach from crafting_base.lua

    Part of RTK Server refactoring - Phase 2.1
]]

jewelrymaking = {
    craft = function(player, npc, speech)
        local t = {
            graphic = convertGraphic(npc.look, "monster"),
            color = npc.lookColor
        }
        player.npcGraphic = t.graphic
        player.npcColor = t.color
        player.dialogType = 0
        player.lastClick = npc.ID

        if speech == "jewel" then
            local jewelskill = crafting.getSkillValue(player, "jewelry making")

            -- Build craft type options based on skill
            local choices = { "Ring" }
            if jewelskill >= 6 then
                table.insert(choices, "Bracelet")
            end
            if jewelskill >= 9 then
                table.insert(choices, "Headgear")
            end

            local choice = player:menuSeq("What would you like to craft?", choices, {})

            if choices[choice] == "Ring" then
                jewelrymaking.ring(player, npc, jewelskill)
            elseif choices[choice] == "Bracelet" then
                jewelrymaking.bracelet(player, npc, jewelskill)
            elseif choices[choice] == "Headgear" then
                jewelrymaking.headgear(player, npc, jewelskill)
            end
        end
    end,

    ring = function(player, npc, skillValue)
        local cost = crafting_base.JEWELRY_COSTS.ring

        -- Get available amber options
        local amberOptions = crafting_base.getJewelryAmberOptions(player, cost.materials)
        if #amberOptions == 0 then
            player:dialogSeq({ { graphic = 0, color = 0 }, "You need 2 crafted ambers to make a ring." }, 0)
            return
        end

        -- Show amber selection menu
        local choices = {}
        for _, amber in ipairs(amberOptions) do
            table.insert(choices, amber.display)
        end

        local choice = player:menuSeq("What type of ring?", choices, {})
        local selectedAmber = amberOptions[choice]

        -- Check gold
        if player.money < cost.gold then
            player:dialogSeq({ { graphic = 0, color = 0 }, "You need " .. cost.gold .. " gold to craft a ring." }, 0)
            return
        end

        -- Find and consume materials
        local materialUsed = crafting_base.hasJewelryMaterials(player, selectedAmber, cost.materials)
        if not materialUsed then
            player:dialogSeq({ { graphic = 0, color = 0 }, "You need 2 crafted ambers." }, 0)
            return
        end

        -- Consume resources
        player.money = player.money - cost.gold
        player:sendStatus()
        player:removeItem(materialUsed, cost.materials)

        -- Craft the ring
        crafting_base.craftRing(player, npc, selectedAmber, materialUsed)
    end,

    bracelet = function(player, npc, skillValue)
        local cost = crafting_base.JEWELRY_COSTS.bracelet

        -- Get available amber options
        local amberOptions = crafting_base.getJewelryAmberOptions(player, cost.materials)
        if #amberOptions == 0 then
            player:dialogSeq({ { graphic = 0, color = 0 }, "You need 4 crafted ambers to make a bracelet." }, 0)
            return
        end

        -- Show amber selection menu
        local choices = {}
        for _, amber in ipairs(amberOptions) do
            table.insert(choices, amber.display)
        end

        local choice = player:menuSeq("What type of bracelet?", choices, {})
        local selectedAmber = amberOptions[choice]

        -- Check gold
        if player.money < cost.gold then
            player:dialogSeq({ { graphic = 0, color = 0 }, "You need " .. cost.gold .. " gold to craft a bracelet." }, 0)
            return
        end

        -- Find and consume materials
        local materialUsed = crafting_base.hasJewelryMaterials(player, selectedAmber, cost.materials)
        if not materialUsed then
            player:dialogSeq({ { graphic = 0, color = 0 }, "You need 4 crafted ambers." }, 0)
            return
        end

        -- Consume resources
        player.money = player.money - cost.gold
        player:sendStatus()
        player:removeItem(materialUsed, cost.materials)

        -- Skill increase
        crafting.skillChanceIncrease(player, npc, "jewelry making")

        -- Roll for bracelet quality
        local isWellCrafted = materialUsed:match("well_crafted")
        local rand = math.random(1, 100)
        local threshold = isWellCrafted and (6 * skillValue) or (3 * skillValue)

        local size = (rand <= threshold) and "luxury" or "plain"
        local itemName = size .. "_" .. selectedAmber.key .. "_bracelet"

        player:addItem(itemName, 1)
        local msg = (size == "luxury") and "You have succeeded masterfully!" or "You managed to make a bracelet."
        crafting_base.showItemDialog(player, itemName, msg)
    end,

    headgear = function(player, npc, skillValue)
        local cost = crafting_base.JEWELRY_COSTS.headgear

        -- Get available amber options
        local amberOptions = crafting_base.getJewelryAmberOptions(player, cost.materials)
        if #amberOptions == 0 then
            player:dialogSeq({ { graphic = 0, color = 0 }, "You need 8 crafted ambers to make headgear." }, 0)
            return
        end

        -- Show amber selection menu
        local choices = {}
        for _, amber in ipairs(amberOptions) do
            table.insert(choices, amber.display)
        end

        local choice = player:menuSeq("What type of headgear?", choices, {})
        local selectedAmber = amberOptions[choice]

        -- Check gold
        if player.money < cost.gold then
            player:dialogSeq({ { graphic = 0, color = 0 }, "You need " .. cost.gold .. " gold to craft headgear." }, 0)
            return
        end

        -- Find and consume materials
        local materialUsed = crafting_base.hasJewelryMaterials(player, selectedAmber, cost.materials)
        if not materialUsed then
            player:dialogSeq({ { graphic = 0, color = 0 }, "You need 8 crafted ambers." }, 0)
            return
        end

        -- Consume resources
        player.money = player.money - cost.gold
        player:sendStatus()
        player:removeItem(materialUsed, cost.materials)

        -- Skill increase
        crafting.skillChanceIncrease(player, npc, "jewelry making")

        -- Check for diadem (rare)
        if math.random(1, 111) == 1 and os.time() > (player.registry["diadem_crafted"] or 0) then
            player.registry["diadem_crafted"] = os.time() + 259200
            player:addItem("diadem", 1)
            crafting_base.showItemDialog(player, "diadem", "All of your efforts have paid off, you have crafted a Diadem!")
            return
        end

        -- Check for totem headgear
        local totem = crafting_base.checkTotemChance(player, "totem_helm_crafted")
        if totem then
            local itemName = totem .. "_casque"
            player:addItem(itemName, 1)
            crafting_base.showItemDialog(player, itemName, "The totems have blessed your work!")
            return
        end

        -- Roll for headgear quality
        local isWellCrafted = materialUsed:match("well_crafted")
        local rand = math.random(1, 100)
        local casqueThreshold = isWellCrafted and (4 * skillValue) or (2 * skillValue)

        local itemType = (rand <= casqueThreshold) and "casque" or "circlet"
        local itemName = selectedAmber.key .. "_" .. itemType

        player:addItem(itemName, 1)
        local msg = (itemType == "casque") and "You have succeeded masterfully!" or "You have managed to make a circlet."
        crafting_base.showItemDialog(player, itemName, msg)
    end
}
