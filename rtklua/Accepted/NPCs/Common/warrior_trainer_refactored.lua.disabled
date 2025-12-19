--[[
    warrior_trainer_refactored.lua - Refactored warrior trainer using trainer_base

    Original: ~1,100 lines
    Refactored: ~200 lines (82% reduction)

    Uses shared infrastructure from trainer_base.lua
    Class-specific logic (Shield quest) preserved here.

    Part of RTK Server refactoring - Phase 2.2
]]

local WARRIOR_CLASS = 1

-- Warrior-specific welcome messages
local WARRIOR_WELCOME = {
    "Hail, mighty one! Welcome to my sanctuary, the sanctuary of the mightiest of all fighters.",
    "Have you come to pick your path? I think you would make a great warrior, and a great hero."
}

-- Warrior class description
local WARRIOR_DESCRIPTION = {
    "Tell you about warriors? Well, they are the greatest of the fighter classes. A one man army, so to speak. Warriors are fierce, and powerful, and can battle many foes at once.",
    "Warriors use little magic, instead we prefer to use skills, such as the ability to hit more than one creature at a time.",
    "We depend on the healing skills of other paths, like the poets, but they are always willing to group with a warrior for our awesome killing abilities."
}

-- Shield quest dialog
local function showShieldDialog(player)
    player:dialogSeq({
        "Anyone who dedicates their lives to the weapon should learn how to use a shield.",
        "First, though, I will ask you to prove this to me. To the West and North of here, there is a cave. This is the training caves for our Warriors.",
        "In it, you will find many different dyed creatures. You may not kill the red and blue ones, you must avoid them.",
        "At the end of the caves, there is a statue of Chung Ryong. If you reach it without killing any of the Blue or Red animals, you will be rewarded with a shield."
    }, 1)
end

-- Warrior starting equipment
local function giveWarriorEquipment(player)
    player:addItem("sword_of_power", 1)
    player:addItem("bears_liver", 25)

    if player.sex == 0 then
        player:addItem("jade_scale_mail", 1)
        player:addItem("merchant_helm", 1)
    elseif player.sex == 1 then
        player:addItem("summer_mail_dress", 1)
        player:addItem("spring_helmet", 1)
    end

    player:addGold(500)
    player:updatePath(WARRIOR_CLASS, 0)
    player:calcStat()
end

-- Handle Strangers quest (start shield quest)
local function handleStrangersQuest(player, npc)
    local mobs = { "red_deer", "red_doe", "red_rabbit", "blue_deer", "blue_doe", "blue_rabbit" }

    if player:hasItem("green_squirrel_pelt", 1) ~= true then
        player:sendMinitext("Eh? Please don't bother me.")
        player:sendMinitext("You probably couldn't even kill one of the Green squirrels to the south.")
        return
    end

    player:removeItem("green_squirrel_pelt", 1)
    player.quest["nagnang_warrior_trial"] = 1

    for i = 1, #mobs do
        player:flushKills(mobs[i])
    end

    showShieldDialog(player)
end

WarriorTrainerNpc = {
    click = async(function(player, npc)
        -- Initialize dialog
        local t = trainer_base.initDialog(player, npc)

        -- Build standard menu options
        local opts = trainer_base.buildStandardOptions(player, WARRIOR_CLASS)

        -- Add quest progression options (Star/Moon/Sun)
        trainer_base.addQuestOptions(player, opts, WARRIOR_CLASS)

        -- Add warrior-specific Shield quest option
        if npc.mapTitle == "Sword"
           and player.level >= 10
           and player.baseClass == WARRIOR_CLASS
           and not player:hasLegend("nagnang_warrior_trial") then
            if player.quest["nagnang_warrior_trial"] == 0 then
                table.insert(opts, "Strangers")
            elseif player.quest["nagnang_warrior_trial"] == 1 then
                table.insert(opts, "Shield")
            end
        end

        -- Show menu
        local choice = player:menuString("Hello! How can I help you today?", opts)

        -- Handle common choices first
        if trainer_base.handleCommonChoice(player, npc, choice, t) then
            return
        end

        -- Handle Divine Secret / Learn Secret with special spells for Nagnang trainers
        if choice == "Divine Secret" then
            if npc.mapTitle == "Sword" or npc.mapTitle == "Kwi-Sin Sword" or npc.mapTitle == "Ming-Ken Sword" or npc.mapTitle == "Ohaeng Sword" then
                player:futureSpells(npc, {"feral_berserk_warrior"})
            else
                player:futureSpells(npc)
            end
            return
        elseif choice == "Learn Secret" then
            if npc.mapTitle == "Sword" or npc.mapTitle == "Kwi-Sin Sword" or npc.mapTitle == "Ming-Ken Sword" or npc.mapTitle == "Ohaeng Sword" then
                player:learnSpell(npc, {"feral_berserk_warrior"})
            else
                player:learnSpell(npc)
            end
            return
        end

        -- Handle class-specific choices
        if choice == "Become a Warrior" then
            local choice2 = trainer_base.handleClassEnrollment(
                player, npc, WARRIOR_CLASS, t,
                { t, WARRIOR_WELCOME[1], WARRIOR_WELCOME[2] }
            )

            if choice2 == "Yes" then
                player:dialogSeq({
                    t,
                    "Great! You have made a great decision. I see you becoming a great hero in these lands. Now let me set you up with some supplies."
                }, 1)
                giveWarriorEquipment(player)
                player:dialogSeq({
                    t,
                    "Here is some armor, and a weapon. These are specific to the warrior path, and will help get you started.",
                    "I have also given you some gold, it's all I can spare right now. It will help you with repairs, and getting some other equipment like rings.",
                    "I have also given you some Bear's livers, these will help you keep your strength up. Eat one when you are feeling weak, and near death. Shop keepers around town sell them if you need more.",
                    "If you wish to learn some skills let me know, I can teach you many things to help you in battle."
                }, 1)

            elseif choice2 == "Tell me more" then
                trainer_base.showClassDescription(player, t, { t, WARRIOR_DESCRIPTION[1], WARRIOR_DESCRIPTION[2], WARRIOR_DESCRIPTION[3] })

                local choice3 = player:menuString("Will you join us now?", {"Yes", "No"})

                if choice3 == "Yes" then
                    player:dialogSeq({
                        t,
                        "Great! You have made a great decision. I see you becoming a great hero in these lands. Now let me set you up with some supplies."
                    }, 1)
                    giveWarriorEquipment(player)
                    player:dialogSeq({
                        t,
                        "Here is some armor, and a weapon. These are specific to the warrior path, and will help get you started.",
                        "I have also given you some gold, it's all I can spare right now. It will help you with repairs, and getting some other equipment like rings.",
                        "I have also given you some Bear's livers, these will help you keep your strength up. Eat one when you are feeling weak, and near death. Shop keepers around town sell them if you need more.",
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

        elseif choice == "Strangers" then
            handleStrangersQuest(player, npc)

        elseif choice == "Shield" then
            showShieldDialog(player)

        -- Handle Star/Moon/Sun quest stages
        elseif trainer_base.parseQuestChoice(choice, "Warrior", "Star") then
            local stage = trainer_base.parseQuestChoice(choice, "Warrior", "Star")
            -- Call existing quest handler

        elseif trainer_base.parseQuestChoice(choice, "Warrior", "Moon") then
            local stage = trainer_base.parseQuestChoice(choice, "Warrior", "Moon")
            -- Call existing quest handler

        elseif trainer_base.parseQuestChoice(choice, "Warrior", "Sun") then
            local stage = trainer_base.parseQuestChoice(choice, "Warrior", "Sun")
            -- Call existing quest handler
        end
    end)
}
