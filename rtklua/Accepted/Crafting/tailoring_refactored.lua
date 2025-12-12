--[[
    tailoring_refactored.lua - Refactored tailoring using crafting_base

    Original: 395 lines
    Refactored: ~130 lines (67% reduction)

    Uses data-driven approach from crafting_base.lua

    Part of RTK Server refactoring - Phase 2.1
]]

-- Tailoring skill level data: { itemRangeMin, itemRangeMax, regFail, starChance, starFail, canMakeBandanna }
local TAILORING_SKILLS = {
    novice      = { rangeMin = 1, rangeMax = 2, regFail = 35, starChance = 0, starFail = 0, bandanna = false },
    apprentice  = { rangeMin = 1, rangeMax = 3, regFail = 37.5, starChance = 0, starFail = 0, bandanna = false },
    accomplished = { rangeMin = 2, rangeMax = 4, regFail = 40, starChance = 0, starFail = 0, bandanna = false },
    adept       = { rangeMin = 2, rangeMax = 5, regFail = 42.5, starChance = 0, starFail = 0, bandanna = false },
    talented    = { rangeMin = 3, rangeMax = 5, regFail = 45, starChance = 0, starFail = 0, bandanna = false },
    skilled     = { rangeMin = 3, rangeMax = 7, regFail = 50, starChance = 0, starFail = 0, bandanna = false },
    expert      = { rangeMin = 3, rangeMax = 7, regFail = 60, starChance = 0, starFail = 0, bandanna = false },
    master      = { rangeMin = 4, rangeMax = 7, regFail = 35, starChance = 100, starFail = 70, bandanna = true },
    grandmaster = { rangeMin = 4, rangeMax = 7, regFail = 32, starChance = 80, starFail = 70, bandanna = true },
    champion    = { rangeMin = 5, rangeMax = 7, regFail = 29, starChance = 70, starFail = 60, bandanna = true },
    legendary   = { rangeMin = 5, rangeMax = 7, regFail = 25, starChance = 50, starFail = 50, bandanna = true }
}

-- Clothing types: { name, clothCost, qualities }
local CLOTHING_TYPES = {
    { name = "Waistcoat", cost = 2, qualities = {"novice", "farmer", "royal", "sky", "ancient", "blood", "earth"} },
    { name = "Blouse", cost = 2, qualities = {"spring", "summer", "autumn", "winter", "leather", "ancient", "earth"} },
    { name = "Garb", cost = 3, qualities = {"spring", "summer", "royal", "sky", "leather", "blood", "earth"} },
    { name = "Clothes", cost = 3, qualities = {"peasant", "farmer", "royal", "sky", "ancient", "family", "earth"} },
    { name = "Dress", cost = 2, qualities = {"spring", "summer", "autumn", "winter", "ancient", "leather", "earth"} },
    { name = "Skirt", cost = 3, qualities = {"spring", "summer", "autumn", "winter", "leather", "heart", "earth"} },
    { name = "Robes", cost = 2, qualities = {"spring", "summer", "autumn", "winter", "ancient", "blood", "earth"} },
    { name = "Mantle", cost = 3, qualities = {"spring", "summer", "autumn", "winter", "ancient", "blood", "earth"} },
    { name = "Gown", cost = 2, qualities = {"spring", "summer", "autumn", "winter", "leather", "ancient", "earth"} },
    { name = "Drapery", cost = 3, qualities = {"spring", "summer", "autumn", "winter", "leather", "ancient", "earth"} },
    { name = "Bandanna", cost = 3, qualities = {"black", "purple", "white", "brown", "green", "pink", "teal",
                                                 "navy", "darkpurple", "orange", "babyblue", "lava", "star"} }
}

tailoring = {
    craft = function(player, npc, speech)
        local t = {
            graphic = convertGraphic(npc.look, "monster"),
            color = npc.lookColor
        }
        player.npcGraphic = t.graphic
        player.npcColor = t.color
        player.dialogType = 0
        player.lastClick = npc.ID

        if speech == "tailor" then
            local tcloth = { graphic = convertGraphic(1632, "item"), color = 0 }
            player.npcGraphic = tcloth.graphic
            player.npcColor = tcloth.color

            local tailorskill = crafting.getSkillLevel(player, "tailoring")
            local skillData = TAILORING_SKILLS[tailorskill] or TAILORING_SKILLS.novice

            -- Build clothing options (bandanna only for master+)
            local opts = {}
            for i, clothing in ipairs(CLOTHING_TYPES) do
                if clothing.name ~= "Bandanna" or skillData.bandanna then
                    table.insert(opts, clothing.name)
                end
            end

            local choice = player:menuSeq("What type of clothing are you trying to tailor?", opts, {})
            local selectedClothing = nil

            for _, clothing in ipairs(CLOTHING_TYPES) do
                if clothing.name == opts[choice] then
                    selectedClothing = clothing
                    break
                end
            end

            if not selectedClothing then return end

            -- Check materials
            local matsAmts = selectedClothing.cost
            if player:hasItem("cloth", matsAmts) ~= true and player:hasItem("fine_cloth", matsAmts) ~= true then
                player:dialogSeq({ tcloth, "You need " .. matsAmts .. " bolts of cloth to tailor." }, 0)
                return
            end

            -- Consume materials
            local clothtaken = "cloth"
            if player:hasItem("cloth", matsAmts) == true then
                player:removeItem("cloth", matsAmts, 9)
            else
                clothtaken = "fine_cloth"
                player:removeItem("fine_cloth", matsAmts, 9)
            end

            -- Skill increases
            for z = 1, matsAmts do
                crafting.skillChanceIncrease(player, NPC("Lin"), "tailoring")
            end

            -- Determine quality tier
            local itemRange
            if selectedClothing.name == "Bandanna" then
                itemRange = math.random(1, #selectedClothing.qualities)
            else
                itemRange = math.random(skillData.rangeMin, skillData.rangeMax)
            end

            -- Build item name
            local chosenItem = string.lower(selectedClothing.name)
            local quality = selectedClothing.qualities[itemRange] or selectedClothing.qualities[1]
            local itemName = quality .. "_" .. chosenItem

            -- Roll for success
            local rand = math.random(1, 100)
            local failThreshold = skillData.regFail

            -- Star cloth chance for master+
            if skillData.bandanna and selectedClothing.name ~= "Bandanna" then
                local starRoll = math.random(1, skillData.starChance)
                if starRoll == 1 then
                    failThreshold = skillData.starFail
                    itemName = "star_" .. chosenItem
                end
            end

            if rand <= failThreshold then
                player:addItem(itemName, 1)
                crafting_base.showItemDialog(player, itemName, "Your efforts are successful!")
            else
                player:addItem("spent_cloth", 1)
                crafting_base.showItemDialog(player, "cloth",
                    "Your feeble efforts have destroyed that which you meant to enhance.")
            end
        end
    end
}
