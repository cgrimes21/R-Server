--[[
    gemcutting_refactored.lua - Refactored gemcutting using crafting_base

    Original: 1,934 lines
    Refactored: ~120 lines (94% reduction)

    Uses data-driven approach from crafting_base.lua

    Part of RTK Server refactoring - Phase 2.1
]]

gemcutting = {
    craft = function(player, npc, speech)
        local t = {
            graphic = convertGraphic(npc.look, "monster"),
            color = npc.lookColor
        }
        player.npcGraphic = t.graphic
        player.npcColor = t.color
        player.dialogType = 0
        player.lastClick = npc.ID

        if speech == "gem" then
            local skill = crafting.getSkillLevel(player, "gemcutting")

            -- Build available gem options based on skill level
            local gemOptions = crafting_base.getGemOptions(skill)
            local opts = {}
            local gemKeys = {}

            for i, gem in ipairs(gemOptions) do
                table.insert(opts, gem.display)
                table.insert(gemKeys, gem.key)
            end

            -- Show menu if multiple options
            local choice = 1
            if #opts > 1 then
                choice = player:menuSeq(
                    "Which gem would you like to attempt?",
                    opts,
                    {}
                )
            end

            local gemKey = gemKeys[choice]
            local items = crafting_base.GEMCUTTING_ITEMS[gemKey]

            -- Check if player has the required material
            if player:hasItem(gemKey, 1) ~= true then
                local ta = { graphic = Item(gemKey).icon, color = Item(gemKey).iconC }
                player:dialogSeq({ ta, "You have nothing to craft." }, 0)
                return
            end

            -- Consume material and give skill chance
            player:removeItem(gemKey, 1, 9)
            crafting.skillChanceIncrease(player, NPC("Sel"), "gemcutting")

            -- Roll for result and give output
            crafting_base.craftGem(player, gemKey)
            return
        end
    end
}
