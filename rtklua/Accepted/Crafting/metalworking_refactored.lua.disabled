--[[
    metalworking_refactored.lua - Refactored metalworking using crafting_base

    Original: 793 lines
    Refactored: ~120 lines (85% reduction)

    Uses data-driven approach from crafting_base.lua

    Part of RTK Server refactoring - Phase 2.1
]]

metalworking = {
    craft = function(player, npc, speech)
        local t = {
            graphic = convertGraphic(npc.look, "monster"),
            color = npc.lookColor
        }
        player.npcGraphic = t.graphic
        player.npcColor = t.color
        player.dialogType = 0
        player.lastClick = npc.ID

        if speech == "metal" then
            local skillValue = crafting.getSkillValue(player, "metalworking")

            -- Build weapon options based on skill
            local weapons = crafting_base.getMetalWeaponOptions(skillValue)
            local opts = {}
            for _, weapon in ipairs(weapons) do
                table.insert(opts, weapon.name)
            end

            local choice = player:menuString("Which type of weapon would you like to craft?", opts)

            -- Find selected weapon
            for _, weapon in ipairs(weapons) do
                if weapon.name == choice then
                    crafting_base.craftMetalWeapon(player, npc, weapon)
                    return
                end
            end
        end

        if speech == "armor" then
            metalworking.craftArmor(player, npc)
        end
    end,

    craftArmor = function(player, npc)
        local skillValue = crafting.getSkillValue(player, "metalworking")
        local tier = crafting_base.getArmorTier(skillValue)

        -- Get armor types based on gender
        local armorTypes = player.sex == 0
            and crafting_base.METAL_ARMOR_TYPES.male
            or crafting_base.METAL_ARMOR_TYPES.female

        -- Build armor type menu
        local opts = {}
        for _, armorType in ipairs(armorTypes) do
            table.insert(opts, armorType.name)
        end

        local choice = player:menuString("What type of armor would you like to craft?", opts)

        -- Find selected armor type
        local selectedType = nil
        for _, armorType in ipairs(armorTypes) do
            if armorType.name == choice then
                selectedType = armorType
                break
            end
        end

        if not selectedType then return end

        -- Get available armors for this tier
        local availableArmors = {}
        for i = 1, tier do
            if selectedType.items[i] then
                table.insert(availableArmors, selectedType.items[i])
            end
        end

        if #availableArmors == 0 then
            player:sendMinitext("You don't have the skill to craft this type of armor.")
            return
        end

        -- Build armor selection menu
        local armorOpts = {}
        for _, armor in ipairs(availableArmors) do
            table.insert(armorOpts, Item(armor).name or armor)
        end

        local armorChoice = player:menuSeq("Which armor would you like to craft?", armorOpts, {})
        local selectedArmor = availableArmors[armorChoice]

        if not selectedArmor then return end

        -- Check materials (5 metal for armor)
        local metalCost = 5
        if player:hasItem("metal", metalCost) ~= true and player:hasItem("fine_metal", metalCost) ~= true then
            crafting_base.showItemDialog(player, "metal",
                "You need " .. metalCost .. " units of metal to craft armor.")
            return
        end

        local usingFine = player:hasItem("fine_metal", metalCost) == true
        local material = usingFine and "fine_metal" or "metal"

        player:removeItem(material, metalCost)

        -- Skill increases
        for i = 1, metalCost do
            crafting.skillChanceIncrease(player, npc, "metalworking")
        end

        -- Roll for success
        local rand = math.random(1, 100)
        local successThreshold = 10 + (skillValue * 8)

        if usingFine or rand <= successThreshold then
            player:addItem(selectedArmor, 1)
            crafting_base.showItemDialog(player, selectedArmor, "Your efforts are successful!")
        else
            player:addItem("spent_metal", 1)
            crafting_base.showItemDialog(player, "metal",
                "Your feeble efforts have destroyed that which you meant to enhance.")
        end
    end
}
