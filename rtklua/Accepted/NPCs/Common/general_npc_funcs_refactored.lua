--[[
    general_npc_funcs_refactored.lua - Refactored NPC functions using npc_base

    Original appearance functions: ~1,120 lines
    Refactored: ~250 lines (78% reduction)

    Requires: npc_base.lua

    Part of RTK Server refactoring - Phase 2.4
]]

-- Ensure npc_base is loaded
if npc_base == nil then
    require("npc_base")
end

general_npc_funcs_refactored = {

    --[[
        changeFace - Change player's face appearance
        Original: ~153 lines (1779-1931)
        Refactored: ~35 lines (77% reduction)
    ]]
    changeFace = function(player, npc)
        local t = npc_base.initDialog(player, npc)

        -- Crime check
        local crime = player:menuString("You're not wanted for a crime, are you?", {"Yes", "No"})
        if crime == "Yes" then
            player:dialogSeq({t, "Ah, I see. Appear as thou wilt."}, 0)
            return
        end

        local cost = npc_base.COSTS.changeFace
        if not npc_base.checkCost(player, t, cost) then return end

        local confirm = player:menuString("It will cost you " .. cost .. " coins. Do you wish to pay?", {"Yes", "No"})
        if confirm ~= "Yes" then
            player:dialogSeq({t, "Ah, I see. Appear as thou wilt."}, 0)
            return
        end

        player:dialogSeq({t, "Choose the face you like. Use 'Previous' and 'Next' to browse."}, 1)
        npc_base.initPreviewMode(player)

        local selectedIndex = npc_base.selectionLoop(player, t, npc_base.FACES,
            function(p, face, idx) p.gfxFace = face end,  -- preview
            function(p, face, idx)  -- confirm
                if not npc_base.checkCost(p, t, cost) then return false end
                p.face = face
                p:removeGold(cost)
                p:dialogSeq({t, "It's tricky to mold this flesh. Let's see how it looks."}, 1)
                p:sendAnimation(11, 5)
                p:updateState()
                return true
            end,
            {promptText = "Do you like this face?", nextText = "Next face", prevText = "Previous face"}
        )
    end,

    --[[
        changeEyes - Change player's eye color
        Original: ~91 lines (2025-2115)
        Refactored: ~30 lines (67% reduction)
    ]]
    changeEyes = function(player, npc)
        local t = npc_base.initDialog(player, npc)
        local cost = npc_base.COSTS.changeEyes

        player:dialogSeq({t, "A new eye color can change your whole outlook! Only " .. cost .. " coins."}, 1)

        if not npc_base.checkCost(player, t, cost) then return end

        npc_base.initPreviewMode(player)

        local selectedIndex = npc_base.selectionLoop(player, t, npc_base.EYE_COLORS.ids,
            function(p, colorId, idx)
                p.gfxEyeColor = colorId
                p:updateState()
            end,
            function(p, colorId, idx)
                if not npc_base.checkCost(p, t, cost) then return false end
                p.eyeColor = colorId
                p:removeGold(cost)
                p:dialogSeq({t, "Your eyes now sparkle with " .. npc_base.EYE_COLORS.names[idx] .. "!"}, 0)
                p:updateState()
                return true
            end,
            {promptText = "Do you like this eye color? (" .. ")", nextText = "Next color", prevText = "Previous color"}
        )
    end,

    --[[
        haircut - Change player's hairstyle
        Original: ~393 lines (2116-2508)
        Refactored: ~50 lines (87% reduction)
    ]]
    haircut = function(player, npc)
        local t = npc_base.initDialog(player, npc)
        local cost = npc_base.COSTS.haircut
        local location = npc.mapTitle

        -- Location-specific greeting
        local greeting = npc_base.getMessage(location, "haircut") or "A hair cut will cost " .. cost .. " coins."
        player:dialogSeq({t, greeting}, 1)

        if not npc_base.checkCost(player, t, cost) then return end

        -- Get styles for this location and gender
        local styleNames, styleIds = npc_base.getHairStyles(location, player.sex)

        if #styleIds == 0 then
            player:dialogSeq({t, "I don't have any styles available for you right now."}, 0)
            return
        end

        npc_base.initPreviewMode(player)

        local selectedIndex = npc_base.selectionLoop(player, t, styleIds,
            function(p, hairId, idx)
                p.gfxHair = hairId
                p:updateState()
            end,
            function(p, hairId, idx)
                if not npc_base.checkCost(p, t, cost) then return false end
                p:dialogSeq({t, "Excellent! Let me get my scissors..", "A snip here... [*snip*]  A snip there... [*snip*]"}, 1)
                p:removeGold(cost)
                p.hair = hairId
                p:updateState()
                p:dialogSeq({t, "All done! Enjoy your new " .. styleNames[idx] .. " hairstyle!"}, 0)
                return true
            end,
            {
                promptText = "Do you like this hairstyle? (" .. ")",
                nextText = "Next hair style",
                prevText = "Previous hair style",
                selectText = "Yes, cut my hair like that"
            }
        )
    end,

    --[[
        hairdye - Change player's hair color
        Original: ~162 lines (2509-2670)
        Refactored: ~40 lines (75% reduction)
    ]]
    hairdye = function(player, npc)
        local t = npc_base.initDialog(player, npc)
        local cost = npc_base.COSTS.hairdye
        local location = npc.mapTitle

        -- Location-specific greeting
        local greeting = npc_base.getMessage(location, "hairdye") or "Hair dye costs " .. cost .. " coins."
        player:dialogSeq({t, greeting}, 1)

        if not npc_base.checkCost(player, t, cost) then return end

        -- Get colors for this location
        local colorNames, colorIds = npc_base.getHairColors(location)

        if #colorIds == 0 then
            player:dialogSeq({t, "I don't have any colors available right now."}, 0)
            return
        end

        npc_base.initPreviewMode(player)

        local selectedIndex = npc_base.selectionLoop(player, t, colorIds,
            function(p, colorId, idx)
                p.gfxHairColor = colorId
                p:updateState()
            end,
            function(p, colorId, idx)
                if not npc_base.checkCost(p, t, cost) then return false end
                p:removeGold(cost)
                p.hairColor = colorId
                p:updateState()
                p:dialogSeq({t, "Your hair is now a beautiful " .. colorNames[idx] .. "!"}, 0)
                return true
            end,
            {promptText = "Do you like this color?", nextText = "Next color", prevText = "Previous color"}
        )
    end,

    --[[
        beard - Change player's beard style (male only)
        Original: ~146 lines (2710-2855)
        Refactored: ~35 lines (76% reduction)
    ]]
    beard = function(player, npc)
        local t = npc_base.initDialog(player, npc)

        if player.sex ~= 0 then
            player:dialogSeq({t, "This service is only available for men."}, 0)
            return
        end

        local cost = npc_base.COSTS.beard
        player:dialogSeq({t, "A new beard style costs " .. cost .. " coins."}, 1)

        if not npc_base.checkCost(player, t, cost) then return end

        npc_base.initPreviewMode(player)

        local selectedIndex = npc_base.selectionLoop(player, t, npc_base.BEARD_STYLES.ids,
            function(p, beardId, idx)
                p.gfxBeard = beardId
                p:updateState()
            end,
            function(p, beardId, idx)
                if not npc_base.checkCost(p, t, cost) then return false end
                p:removeGold(cost)
                p.beard = beardId
                p:updateState()
                p:dialogSeq({t, "Looking distinguished with that " .. npc_base.BEARD_STYLES.names[idx] .. "!"}, 0)
                return true
            end,
            {promptText = "Do you like this beard style?", nextText = "Next style", prevText = "Previous style"}
        )
    end,

    --[[
        shave - Remove beard (male only)
        Original: ~39 lines (2671-2709)
        Refactored: ~15 lines (62% reduction)
    ]]
    shave = function(player, npc)
        local t = npc_base.initDialog(player, npc)

        if player.sex ~= 0 then
            player:dialogSeq({t, "This service is only available for men."}, 0)
            return
        end

        if player.beard == 0 then
            player:dialogSeq({t, "You don't have any facial hair to shave!"}, 0)
            return
        end

        local cost = npc_base.COSTS.shave
        local confirm = player:menuString("A clean shave costs " .. cost .. " coins. Proceed?", {"Yes", "No"})

        if confirm == "Yes" then
            if not npc_base.checkCost(player, t, cost) then return end
            player:removeGold(cost)
            player.beard = 0
            player:updateState()
            player:dialogSeq({t, "Smooth as a baby! All done."}, 0)
        end
    end,

    --[[
        scalpMassage - Hair growth treatment
        Original: ~43 lines (2856-end)
        Refactored: ~15 lines (65% reduction)
    ]]
    scalpMassage = function(player, npc)
        local t = npc_base.initDialog(player, npc)
        local cost = npc_base.COSTS.scalpMassage

        local confirm = player:menuString("A relaxing scalp massage costs " .. cost .. " coins. Interested?", {"Yes", "No"})

        if confirm == "Yes" then
            if not npc_base.checkCost(player, t, cost) then return end
            player:removeGold(cost)
            player:dialogSeq({t, "Ahh... doesn't that feel nice? Your scalp thanks you!"}, 0)
            -- Could add hair growth timer/effect here
        end
    end,

    --[[
        changeGender - Change player's gender
        Original: ~93 lines (1932-2024)
        Refactored: ~25 lines (73% reduction)
    ]]
    changeGender = function(player, npc)
        local t = npc_base.initDialog(player, npc)
        local cost = npc_base.COSTS.changeGender

        player:dialogSeq({t, "Changing your very essence is a profound transformation."}, 1)
        player:dialogSeq({t, "This ancient ritual costs " .. cost .. " coins."}, 1)

        if not npc_base.checkCost(player, t, cost) then return end

        local confirm = player:menuString("Are you certain you wish to proceed?", {"Yes, transform me", "No, I've changed my mind"})

        if confirm == "Yes, transform me" then
            player:removeGold(cost)
            player.sex = (player.sex == 0) and 1 or 0
            player.hair = 1  -- Reset to default hair for new gender
            player.face = 200  -- Reset to default face
            player:updateState()
            player:sendAnimation(11, 10)
            player:dialogSeq({t, "The transformation is complete. You are reborn!"}, 0)
        else
            player:dialogSeq({t, "Perhaps another time then."}, 0)
        end
    end
}

