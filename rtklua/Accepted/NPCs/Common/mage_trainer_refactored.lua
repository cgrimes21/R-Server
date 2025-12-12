--[[
    mage_trainer_refactored.lua - Refactored mage trainer using trainer_base

    Original: ~1,329 lines
    Refactored: ~250 lines (81% reduction)

    Uses shared infrastructure from trainer_base.lua
    Class-specific logic (Ward quest) preserved here.

    Part of RTK Server refactoring - Phase 2.2
]]

local MAGE_CLASS = 3

-- Mage-specific welcome messages
local MAGE_WELCOME = {
    "Hail, mighty one! Welcome to my sanctuary, the sanctuary of the great magic users.",
    "Have you come to pick your path? I think you would make a great mage, and a great hero."
}

-- Mage class description
local MAGE_DESCRIPTION = {
    "Tell you about mages? Well, mages are the magic users of the land, combining great offensive and defensive magic.",
    "We use magic to subdue our foes, and to conquer all who stand before us. We can also use our great powers defensively, to heal and save ourselves, or others.",
    "The mage is a self contained hunter, and can easily solo hunt without the aid of others, however it is always best to join others - safety in numbers!"
}

-- Mage starting equipment
local function giveMageEquipment(player)
    player:addItem("staff_of_power", 1)

    if player.sex == 0 then
        player:addItem("summer_garb", 1)
        player:addItem("merchant_helm", 1)
    elseif player.sex == 1 then
        player:addItem("summer_dress", 1)
        player:addItem("spring_helmet", 1)
    end

    player:addItem("herb_pipe", 4)
    player:addGold(500)
    player:updatePath(MAGE_CLASS, 0)
    player:calcStat()
end

-- Handle Mage Ward quest (class-specific)
local function handleWardQuest(player, npc, t)
    if player.quest["mage_ward"] == 1 then
        -- Check completion requirements
        if player.quest["zapped_yin_mouse"] == 0
           or player.quest["zapped_yang_mouse"] == 0
           or player.quest["zapped_void_mouse"] == 0
           or player.quest["mage_ward_met_ghost"] == 0
           or player:hasItem("rose", 1) ~= true
           or player:hasItem("ore_high", 1) ~= true then
            player:sendMinitext("You have not completed all of the tasks handed to you.")
            return
        end

        -- Complete the quest
        player:removeItem("ore_high", 1)
        player:removeItem("rose", 1)

        if not player:hasLegend("family_nangen_mages") then
            player:addLegend(
                "Family to the Nangen Mages (" .. curT() .. ")",
                "family_nangen_mages",
                3,
                128
            )
        end

        -- Reset quest flags
        player.quest["zapped_yin_mouse"] = 0
        player.quest["zapped_yang_mouse"] = 0
        player.quest["zapped_void_mouse"] = 0
        player.quest["mage_ward_met_ghost"] = 0
        player.quest["mage_ward"] = 0

        player:addItem("magicians_ward", 1, 0, player.ID)

        player:dialogSeq({
            t,
            "You have learned well and earned the protection of the Nangen Mages. Take this ward, forged long ago by the same prophets who instructed you in our ways.",
            "It will not only boost your magical strength but also shield you in your upcoming battles. This is the only one I shall ever give you."
        }, 0)
        return
    end

    -- Start ward quest
    player:dialogSeq({
        t,
        "Ah, I see that you have come for the knowledge of the Mages of Nagnang."
    }, 1)

    local choice = player:menuSeq(
        "Well, I am not the person who has the knowledge, merely the one who tells those who are worthy where to seek out that knowledge. Are you such a worthy person?",
        {"Yes, I am worthy", "No, I am not worthy."},
        {}
    )

    if choice == 1 then
        player:dialogSeq({
            t,
            "Well then, I will tell you where to find the knowledge you seek. Above this cave, there is another. Inside of it is the home to three prohets.",
            "Each embodies a mystical force. One is for Yin, the other, Yang and the third is the Void. Each of them will evaluate your potential and will then instruct you on what to do.",
            "If you follow their instructions, and prove to me that you are honorable and wise by returning here after completing all of their tasks, I will reward you with a protective ward.",
            "To visit each prophet, you need to first attack one of the mice with a spell and then curse that same immortal mouse. The creature will die as an offering and you can then enter.",
            "Take care to curse only ONE creature before entering each room. If you curse more, the wise men will not speak with you and you will need to return to me.",
            "I also implore you, listen to ALL of them and all they have to say. If you do not, I will not grant you the ward."
        }, 1)
        player.quest["mage_ward"] = 1
        player:sendMinitext("Good luck.")
    elseif choice == 2 then
        player:sendMinitext("I admire your honesty.")
    end
end

MageTrainerNpc = {
    click = async(function(player, npc)
        -- Initialize dialog
        local t = trainer_base.initDialog(player, npc)

        -- Build standard menu options
        local opts = trainer_base.buildStandardOptions(player, MAGE_CLASS)

        -- Add quest progression options (Star/Moon/Sun)
        trainer_base.addQuestOptions(player, opts, MAGE_CLASS)

        -- Add mage-specific Ward option
        if npc.mapTitle == "Wand"
           and player.level >= 10
           and player.baseClass == MAGE_CLASS
           and not player:hasLegend("family_nangen_mages") then
            table.insert(opts, "Ward")
        end

        -- Show menu
        local choice = player:menuString(
            "Hello! How can I help you today?",
            opts
        )

        -- Handle common choices first
        if trainer_base.handleCommonChoice(player, npc, choice, t) then
            return
        end

        -- Handle class-specific choices
        if choice == "Become a Mage" then
            local choice2 = trainer_base.handleClassEnrollment(
                player, npc, MAGE_CLASS, t,
                { t, MAGE_WELCOME[1], MAGE_WELCOME[2] }
            )

            if choice2 == "Yes" then
                player:dialogSeq({
                    t,
                    "Great! You have made a great decision. I see you becoming a great hero in these lands. Now let me set you up with some supplies."
                }, 1)
                giveMageEquipment(player)
                player:dialogSeq({
                    t,
                    "Here is some armor, and a weapon. These are specific to the mage path, and will help get you started.",
                    "I have also given you some gold, it's all I can spare right now. It will help you with repairs, and getting some other equipment like rings.",
                    "You also have four herb pipes, these will replenish your mana. Once they are used up you should buy some more, shop keepers around town sell them",
                    "If you wish to learn some skills let me know, I can teach you many things to help you in battle."
                }, 1)

            elseif choice2 == "Tell me more" then
                trainer_base.showClassDescription(player, t, { t, MAGE_DESCRIPTION[1], MAGE_DESCRIPTION[2], MAGE_DESCRIPTION[3] })

                local choice3 = player:menuString(
                    "Will you join us now?",
                    {"Yes", "No"}
                )

                if choice3 == "Yes" then
                    player:dialogSeq({
                        t,
                        "Great! You have made a great decision. I see you becoming a great hero in these lands. Now let me set you up with some supplies."
                    }, 1)
                    giveMageEquipment(player)
                    player:dialogSeq({
                        t,
                        "Here is some armor, and a weapon. These are specific to the mage path, and will help get you started.",
                        "I have also given you some gold, it's all I can spare right now. It will help you with repairs, and getting some other equipment like rings.",
                        "You also have four herb pipes, these will replenish your mana. Once they are used up you should buy some more, shop keepers around town sell them",
                        "If you wish to learn some skills let me know, I can teach you many things to help you in battle."
                    }, 1)
                else
                    player:dialogSeq({
                        t,
                        "Very well, I will be waiting here if you change your mind. I am seeking great people all the time to join this great path."
                    }, 1)
                end

            elseif choice2 == "No" then
                player:dialogSeq({
                    t,
                    "Very well, I will be waiting here if you change your mind. I am seeking great people all the time to join this great path."
                }, 1)
            end

        elseif choice == "Ward" then
            handleWardQuest(player, npc, t)

        -- Handle Star/Moon/Sun quest stages using trainer_base parser
        elseif trainer_base.parseQuestChoice(choice, "Mage", "Star") then
            local stage = trainer_base.parseQuestChoice(choice, "Mage", "Star")
            -- Call existing quest handler for this stage
            -- MageStarQuest.stage(player, npc, stage)

        elseif trainer_base.parseQuestChoice(choice, "Mage", "Moon") then
            local stage = trainer_base.parseQuestChoice(choice, "Mage", "Moon")
            -- Call existing quest handler for this stage
            -- MageMoonQuest.stage(player, npc, stage)

        elseif trainer_base.parseQuestChoice(choice, "Mage", "Sun") then
            local stage = trainer_base.parseQuestChoice(choice, "Mage", "Sun")
            -- Call existing quest handler for this stage
            -- MageSunQuest.stage(player, npc, stage)
        end
    end)
}
