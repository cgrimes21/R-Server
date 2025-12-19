--[[
    npc_base.lua - Shared NPC function infrastructure

    Provides common utilities for NPC interactions:
    - Dialog setup initialization
    - Selection loop for browsing options
    - Cost check patterns
    - Salon data tables (hair, face, colors)

    Part of RTK Server refactoring - Phase 2.4
]]

npc_base = {}

-- Common NPC dialog setup (replaces ~8 lines repeated in every NPC function)
function npc_base.initDialog(player, npc)
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

-- Setup for preview mode (clone display)
function npc_base.initPreviewMode(player)
    player.dialogType = 2
    player.lastClick = player.ID
    clone.equip(player, player)
end

-- Check if player can afford cost, show message if not
function npc_base.checkCost(player, t, cost, message)
    if player.money < cost then
        local msg = message or ("You need " .. cost .. " coins. Come back when you have enough.")
        player:dialogSeq({t, msg}, 0)
        return false
    end
    return true
end

-- Generic selection loop for browsing options (face, hair, eyes, etc.)
-- Returns: selected index, or nil if cancelled
function npc_base.selectionLoop(player, t, items, previewFunc, confirmFunc, options)
    options = options or {}
    local index = options.startIndex or 1
    local selectText = options.selectText or "I want this one"
    local cancelText = options.cancelText or "Nevermind"
    local nextText = options.nextText or "Next"
    local prevText = options.prevText or "Previous"
    local promptText = options.promptText or "Do you like this one?"

    local menu = {selectText, cancelText, nextText, prevText}

    while true do
        -- Apply preview
        if previewFunc then
            previewFunc(player, items[index], index)
        end

        local menuChoice = player:menuString(promptText, menu)

        if menuChoice == selectText then
            -- Confirm selection
            if confirmFunc then
                local success = confirmFunc(player, items[index], index)
                if success then
                    return index
                end
            else
                return index
            end
        elseif menuChoice == nextText then
            index = index + 1
            if index > #items then
                index = #items
            end
        elseif menuChoice == prevText then
            index = index - 1
            if index < 1 then
                index = 1
            end
        elseif menuChoice == cancelText then
            return nil
        end
    end
end

-- Salon data tables

-- Hair styles by location and gender
npc_base.HAIR_STYLES = {
    ["Kugnae Salon"] = {
        male = {
            names = {"Long tied back", "Medium razorback", "Short razor cut", "Long tied slick hair",
                     "Tarzan", "Sumo's do", "The Surf", "Floor sweeper", "Chopsticks"},
            ids = {7, 22, 23, 52, 55, 58, 85, 53, 95}
        },
        female = {
            names = {"Long, loose hair", "Pulled up bunches", "The Updo", "Long pig tails",
                     "Long stylish pony tail", "Medium blunt cut", "Whale back", "Long horse tail",
                     "Medium V cut", "The Flippy", "Pig tails", "Short cute bob", "Sweet rolls",
                     "Two little braids", "Innocent with headband", "Long low ribbon",
                     "The Floor sweeper", "Jane", "Kimono hair", "The Twisler",
                     "Long elegant ponytail", "Long pinned weavey locks", "Teddy bear ears",
                     "Left shoulder braid", "Medium under curls", "Walnut whip", "Tinker toys",
                     "Medium ponytail", "Chopsticks", "Twisted medley", "Long lovely waves",
                     "Long pocahontas braid"},
            ids = {4, 6, 12, 13, 16, 22, 23, 25, 28, 31, 35, 38, 41, 45, 50, 52, 53, 55, 58, 63,
                   64, 65, 66, 67, 72, 83, 84, 88, 95, 96, 98, 99}
        }
    },
    ["Buya Salon"] = {
        male = {
            names = {"Mullet with bandana", "Chieftain", "Leather weave", "Split curtain",
                     "The Pineapple", "Widge cut with bandana", "Masked Bandit", "Mushroom cut",
                     "Wing tips", "Buddha long top knot", "The Jester", "The ramp", "Afro",
                     "Dread locks", "Retro cut", "Ice cream whip", "Fluff cut", "The Wooly hair",
                     "Dual horns", "Swept out", "The Clown", "The Elvis", "Short snake tail"},
            ids = {8, 9, 11, 14, 15, 20, 24, 33, 37, 57, 61, 73, 74, 75, 76, 77, 78, 81, 86, 87, 93, 94, 100}
        },
        female = {
            names = {"Amazon", "Eggbeater", "Dread locks", "Lady Dread locks", "Half shaved punk",
                     "Curly ball", "Spout", "The Mermaid", "The Cat", "Poof ball", "Geisha",
                     "Flapper", "Medusa", "Ribbon with curls", "The Afro", "The Spiral",
                     "Curled ponytail", "Cheerleader curls", "Perm wave", "Headband with waves"},
            ids = {8, 9, 14, 15, 18, 19, 26, 33, 39, 40, 42, 43, 44, 47, 74, 75, 76, 89, 90, 91}
        }
    },
    ["Nagnang Salon"] = {
        male = {
            names = {"Buzz cut", "Spiky", "Side swept spiky", "Slick back", "Long slick",
                     "Top knot warrior", "Samurai", "Ronin", "Monk shaved", "Battle cut"},
            ids = {1, 2, 3, 4, 5, 6, 10, 12, 13, 16}
        },
        female = {
            names = {"Short bob", "Medium waves", "Long straight", "High ponytail",
                     "Twin buns", "Side swept", "Braided crown", "Warrior braid",
                     "Noble updo", "Battle ready"},
            ids = {1, 2, 3, 5, 7, 10, 11, 17, 21, 24}
        }
    }
}

-- Hair colors by location
npc_base.HAIR_COLORS = {
    ["Kugnae Salon"] = {
        names = {"Black", "Silver", "Brown", "Sky blue", "Dark blue", "Royal blue",
                 "Orange", "Red", "Green", "Scarlet"},
        ids = {0, 1, 2, 8, 7, 24, 10, 11, 22, 21}
    },
    ["Buya Salon"] = {
        names = {"Black", "Silver", "Brown", "Light brown", "Tan", "Blonde",
                 "Orange", "Red", "Green", "Scarlet"},
        ids = {0, 1, 2, 27, 25, 20, 10, 11, 22, 21}
    },
    ["Nagnang Salon"] = {
        names = {"Black", "Silver", "Brown", "Orchid", "Purple", "Indigo",
                 "Orange", "Red", "Green", "Scarlet"},
        ids = {0, 1, 2, 4, 3, 6, 10, 11, 22, 21}
    }
}

-- Face options
npc_base.FACES = {200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216}

-- Eye colors
npc_base.EYE_COLORS = {
    names = {"Normal", "Hazel", "Light brown", "Tan", "Copper", "Brown", "Dark brown",
             "Black", "Light blue", "Blue", "Gray", "Dark blue", "Deep blue",
             "Light green", "Green", "Dark green", "Emerald", "Pink", "Red",
             "Crimson", "Maroon", "Violet", "Purple", "Magenta", "Yellow",
             "Gold", "Orange"},
    ids = {0, 27, 25, 17, 10, 2, 19, 18, 8, 24, 1, 7, 6, 28, 22, 29, 30, 4, 11, 21, 23, 3, 26, 5, 20, 9, 16}
}

-- Beard styles (male only)
npc_base.BEARD_STYLES = {
    names = {"None", "Stubble", "Short beard", "Medium beard", "Long beard",
             "Goatee", "Mustache", "Fu Manchu", "Sideburns", "Full beard"},
    ids = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}
}

-- Service costs
npc_base.COSTS = {
    haircut = 2000,
    hairdye = 2000,
    changeFace = 3000,
    changeGender = 100000,
    changeEyes = 1000,
    beard = 1000,
    shave = 500,
    scalpMassage = 500
}

-- Location-specific messages
npc_base.MESSAGES = {
    ["Kugnae Salon"] = {
        haircut = "A hair cut will cost 2,000 coins.",
        hairdye = "A new dye can breathe life into your spirit!"
    },
    ["Buya Salon"] = {
        haircut = "2,000 clams will buy you some really far out styles, man..",
        hairdye = "A new dye can breathe life into your spirit!"
    },
    ["Nagnang Salon"] = {
        haircut = "A hair cut will cost 2,000 coins.",
        hairdye = "Hair dye costs 2,000 coins. Pay it or get out."
    }
}

-- Get hair styles for location and gender
function npc_base.getHairStyles(location, gender)
    local genderKey = gender == 0 and "male" or "female"
    local locationData = npc_base.HAIR_STYLES[location]
    if locationData and locationData[genderKey] then
        return locationData[genderKey].names, locationData[genderKey].ids
    end
    return {}, {}
end

-- Get hair colors for location
function npc_base.getHairColors(location)
    local locationData = npc_base.HAIR_COLORS[location]
    if locationData then
        return locationData.names, locationData.ids
    end
    return {}, {}
end

-- Get message for location and service
function npc_base.getMessage(location, service)
    local locationData = npc_base.MESSAGES[location]
    if locationData and locationData[service] then
        return locationData[service]
    end
    return nil
end

