--[[
    player_refactored.lua - Refactored player functions using player_base

    Original functions: ~1,620 lines (repairAll, repairAllNoConfirm, repairItemNoConfirm)
    Refactored: ~150 lines (91% reduction)

    Requires: player_base.lua

    Part of RTK Server refactoring - Phase 2.3
]]

-- Ensure player_base is loaded
if player_base == nil then
    require("player_base")
end

--[[
    repairAll - Repair all equipped and inventory items with confirmation
    Original: ~350 lines (Player.repairAll lines 1582-1933)
]]
function Player.repairAll_refactored(player, npc)
    local items, totalCost = player_base.getAllItemsNeedingRepair(player)

    if #items == 0 then
        player:menuSeq("I can't make any of your items better than they already are.", {"Go Back"}, {})
        return
    end

    -- Show confirmation dialog
    local message
    if totalCost > 0 then
        message = "Let's see... That will be about " .. totalCost .. " gold for everything, you willing to pay that?"
    else
        message = "Let's see... That will be no charge for everything, continue?"
    end

    local confirm = player:menuString(message, {"Yes", "No"})

    if confirm == "Yes" then
        if player:removeGold(totalCost) == true then
            player_base.repairItems(items)
            player:sendStatus()
            player:updateState()
            player:updateInv()

            local costText = totalCost > 0 and (totalCost .. " Coins") or "free"
            player:menuSeq("Klank  Klank...\nEverything's done.\n\nTotal Cost: " .. costText, {"Go back"}, {})
        else
            player:menuSeq(
                "This is my job, and it's hard work. I will need to be paid for it. Come back when you have the gold to pay for my services.",
                {"Go back"}, {}
            )
        end
    end
end

--[[
    repairAllNoConfirm - Repair all items without confirmation dialog
    Original: ~445 lines (Player.repairAllNoConfirm lines 1936-2380)
]]
function Player.repairAllNoConfirm_refactored(player, npc)
    player:freeAsync()

    local items, totalCost = player_base.getAllItemsNeedingRepair(player)

    if #items == 0 then
        npc:talk(0, npc.name .. ": I can't make any of your items better than they already are.")
        return false
    end

    if player.money < totalCost then
        npc:talk(0, npc.name .. ": I'll need at least " .. totalCost .. " gold to repair everything.")
        return false
    end

    -- Deduct gold and repair
    player.money = player.money - totalCost
    player_base.repairItems(items)
    player:sendStatus()
    player:updateState()
    player:updateInv()

    local costText = totalCost > 0 and (totalCost .. " gold") or "nothing"
    npc:talk(0, npc.name .. ": I patched up all your equipment for " .. costText .. ".")
    return true
end

--[[
    repairItemNoConfirm - Repair a specific item by name without confirmation
    Original: ~473 lines (Player.repairItemNoConfirm lines 2380-2853)
]]
function Player.repairItemNoConfirm_refactored(player, npc, itemName)
    player:freeAsync()

    local targetItem = Item(itemName)
    if targetItem == nil then
        npc:talk(0, npc.name .. ": Well, where is it? I can't see it any place.")
        return false
    end

    -- Check equipped items
    for _, slot in ipairs(player_base.EQUIPMENT_SLOTS) do
        local equippedItem = player:getEquippedItem(slot)
        if equippedItem and equippedItem.name == targetItem.name then
            return player_base.repairSingleItem(player, npc, equippedItem)
        end
    end

    -- Check inventory items
    for i = 0, player.maxInv or 52 do
        local invItem = player:getInventoryItem(i)
        if invItem and player_base.isEquipmentType(invItem) and invItem.name == targetItem.name then
            return player_base.repairSingleItem(player, npc, invItem)
        end
    end

    npc:talk(0, npc.name .. ": You don't seem to have a " .. targetItem.name .. " that needs repair.")
    return false
end

-- Helper for repairItemNoConfirm - repairs a single item
function player_base.repairSingleItem(player, npc, item)
    if item.repairable ~= 1 then
        player:sendMinitext(item.name .. " is not a repairable item.")
        return false
    end

    if item.dura >= item.maxDura then
        npc:talk(0, npc.name .. ": Your " .. item.name .. " doesn't need any repairs.")
        return false
    end

    local cost = player_base.calculateRepairCost(item)

    if player.money < cost then
        npc:talk(0, npc.name .. ": Your " .. item.name .. " is pretty worn out, I'll need at least " .. cost .. " gold to repair it.")
        return false
    end

    -- Perform repair
    item.dura = item.maxDura
    item.repairCheck = 0
    player.money = player.money - cost
    player:sendStatus()

    npc:talk(0, npc.name .. ": I patched up your " .. item.name .. " for " .. cost .. " gold.")
    return true
end

--[[
    repairExtend - Interactive repair with item selection
    Original: ~65 lines (Player.repairExtend lines 1517-1580)
]]
function Player.repairExtend_refactored(player)
    -- Build list of repairable items
    local list = {}
    for i = 0, player.maxInv or 52 do
        local item = player:getInventoryItem(i)
        if item and item.type >= 3 and item.type <= 15 and item.dura < item.maxDura then
            table.insert(list, item.id)
        end
    end

    if #list == 0 then
        player:menuSeq("I can't make any of your items better than they already are.", {"Go Back"}, {})
        return
    end

    local ask = player:sell("What equipment do you need fixed?", list)
    local choice = player:getInventoryItem(ask - 1)

    if choice == nil or choice.dura >= choice.maxDura then
        return
    end

    local icon = { graphic = choice.icon, color = choice.iconC }

    if choice.repairable == 0 then
        player:dialogSeq({icon, "Sorry, but this item cannot be repaired!"})
        return
    end

    local cost = player_base.calculateRepairCost(choice)
    local chosenItemId = choice.id
    local chosenItemDura = choice.dura

    local confirm = player:menuString("I'll need atleast " .. cost .. " gold to fix that, Ok?", {"Yes", "No"})

    if confirm == "Yes" then
        -- Verify item hasn't changed
        if choice.id == chosenItemId and choice.dura == chosenItemDura then
            if player:removeGold(cost) == true or cost == 0 then
                choice.dura = choice.maxDura
                player:updateInv()
                player:menuSeq("Klank  Klank...\nEverything's done.\n\nTotal Cost: " .. cost .. " Coins", {"Go back"}, {})
            else
                player:menuSeq(
                    "This is my job, and it's hard work. I will need to be paid for it. Come back when you have the gold to pay for my services.",
                    {"Go back"}, {}
                )
            end
        else
            player:dialogSeq({icon, "Wait, that isn't right. Come back later."})
        end
    end
end

--[[
    Bank functions - refactored withdraw/deposit
    Original showBankWithdraw + showClanBankWithdraw: ~300 lines
    Refactored: ~80 lines (73% reduction)
]]

-- Generic bank withdraw function
function Player.performBankWithdraw(player, npc, bankType, bankOwner, emptyMessage, successPrefix)
    local bankItemTable, bankCountTable, bankOwnerTable, bankEngraveTable, bankTimerTable, bankItemTableNames =
        bankOwner:bankItemsList(bankType)

    if #bankItemTable == 0 then
        player:dialogSeq({emptyMessage})
        return false
    end

    local buytext = player_base.buildBankBuyText(bankOwnerTable)

    local temp = player:buy(
        successPrefix .. "\nWhat do you want back?",
        bankItemTable, bankCountTable, bankEngraveTable, bankOwnerTable, buytext, {}, {}
    )

    -- Find selected item
    local found = 0
    for i = 1, #bankItemTable do
        if bankItemTableNames[i] == temp then
            found = i
            break
        end
    end

    if found == 0 then
        return nil
    end

    -- Get amount for stackable items
    local amount = player_base.getStackableAmount(player, bankCountTable[found], bankItemTable[found])

    if amount <= 0 then
        return false
    end

    -- Check max amount constraint
    local canWithdraw, errorMsg = player_base.checkMaxAmountConstraint(player, bankItemTable[found], amount)
    if not canWithdraw then
        player:sendMinitext(errorMsg)
        return false
    end

    -- Check inventory space
    if player:hasSpace(bankItemTable[found], amount, bankOwnerTable[found], bankEngraveTable[found]) ~= true then
        player:menu("You don't have enough hands to carry all that, free up some space in your inventory then come back to me.", {}, {})
        return false
    end

    local itemName = Item(bankItemTable[found]).name
    local engrave = bankEngraveTable[found]
    local owner = bankOwnerTable[found]

    -- Perform withdraw
    local worked = player:addItem(bankItemTable[found], amount, 0, bankOwnerTable[found], bankTimerTable[found], bankEngraveTable[found])

    if worked == true then
        if bankType == 0 then
            bankOwner:bankWithdraw(bankItemTable[found], amount, bankOwnerTable[found], bankTimerTable[found], bankEngraveTable[found])
        else
            player:clanBankWithdraw(bankItemTable[found], amount, bankOwnerTable[found], bankTimerTable[found], bankEngraveTable[found])
        end
    else
        player:sendMinitext("Cannot withdraw " .. amount .. " " .. itemName .. "(s).")
        return false
    end

    characterLog.withdrawItemWrite(bankOwner, itemName, engrave, amount, owner)

    local amountText = amount > 1 and (" (" .. amount .. ")") or ""
    npc:talk(0, npc.name .. ": Here's your " .. itemName .. amountText .. ".")

    return true
end

-- Wrapper for personal bank withdraw
function Player.showBankWithdraw_refactored(player, npc, bankOwnerName)
    if bankOwnerName == nil then
        bankOwnerName = player.name
    end

    local bankOwner = bank.get_bankOwner(player, bankOwnerName)
    if bankOwner == nil then
        return false
    end

    -- Check bank lock
    if bank.is_bank_locked(player, bankOwner) then
        bank.show_bank_locked(player)
        return false
    end

    return Player.performBankWithdraw(
        player, npc, 0, bankOwner,
        bankOwner.name .. "... I don't see anything for that account.",
        bankOwner.name .. "... Here's what I have."
    )
end

-- Wrapper for clan bank withdraw
function Player.showClanBankWithdraw_refactored(player, npc)
    return Player.performBankWithdraw(
        player, npc, 2, player,
        "Your clan bank is currently empty.",
        "Here's what I've been holding for your clan."
    )
end

--[[
    Bank deposit functions - refactored
    Original showBankDeposit + showClanBankDeposit: ~265 lines
    Refactored: ~70 lines (74% reduction)
]]

-- Generic bank deposit function
function Player.performBankDeposit(player, npc, bankType, bankOwner, promptMessage)
    local itemTable = player_base.buildInventoryItemList(player)

    if #itemTable == 0 then
        player:dialogSeq({"You don't have anything to deposit."})
        return false
    end

    local choice = player:sell(promptMessage, itemTable)
    local dItem = player:getInventoryItem(choice - 1)

    if dItem == nil or dItem.name == "" then
        player:sendMinitext("You cannot deposit that item.")
        return false
    end

    if dItem.depositable then
        player:sendMinitext("You cannot deposit that item.")
        return false
    end

    if dItem.realName ~= "" then
        player:sendMinitext("You cannot deposit engraved items.")
        return false
    end

    -- Get amount for stackable items
    local amount = 1
    if dItem.stackAmount > 1 and dItem.amount > 1 then
        amount = player:inputNumberCheck(player:input("How many would you like to deposit?"))
        local hasAmount = player:hasItem(dItem.id, amount)
        if hasAmount ~= true then
            amount = hasAmount
        end
    end

    if amount == 0 then
        player:dialogSeq({"You cannot deposit zero of something."})
        return false
    end

    -- Check durability (must be fully repaired)
    if dItem.maxDura > 0 and dItem.dura ~= dItem.maxDura then
        player:sendMinitext("I don't want your junk. Ask a smith to fix it.")
        return false
    end

    -- Calculate deposit fee
    local moneyAmount = math.ceil(dItem.sell * 0.10 * amount)

    -- Verify player has item and money
    if player:hasItem(dItem.id, amount, dItem.dura, dItem.owner, dItem.realName) ~= true then
        player:dialogSeq({"You do not have what you asked me to hold for you."})
        return false
    end

    if player.money < moneyAmount then
        player:dialogSeq({"You do not have enough money to cover my safe keeping fees."})
        return false
    end

    -- Check bank lock (personal bank only)
    if bankType == 0 then
        local updatedBankOwner = bank.get_bankOwner(player, bankOwner.name)
        if updatedBankOwner == nil then
            return false
        end
        if bank.is_bank_locked(player, updatedBankOwner) then
            bank.show_bank_locked(player)
            return false
        end
        bankOwner = updatedBankOwner
    end

    -- Perform deposit
    local itemName = dItem.name
    local engrave = dItem.realName
    local owner = dItem.owner

    player.money = player.money - moneyAmount

    if bankType == 0 then
        bankOwner:bankDeposit(dItem.id, amount, dItem.owner, dItem.time, dItem.realName)
    else
        player:clanBankDeposit(dItem.id, amount, dItem.owner, dItem.time, dItem.realName)
    end

    if amount == 1 then
        player:removeItemSlot(choice - 1, amount, 9)
    else
        player:removeItem(dItem.id, amount, 9)
    end

    player:sendStatus()
    characterLog.depositItemWrite(bankOwner, itemName, engrave, amount, owner)

    npc:talk(0, npc.name .. ": I'll take your " .. itemName .. ". " .. amount .. " of them.")
    if moneyAmount > 0 then
        npc:talk(0, npc.name .. ": The fee is " .. moneyAmount .. " coins.")
    end

    return true
end

-- Wrapper for personal bank deposit
function Player.showBankDeposit_refactored(player, npc, bankOwnerName)
    if bankOwnerName == nil then
        bankOwnerName = player.name
    end

    local bankOwner = bank.get_bankOwner(player, bankOwnerName)
    if bankOwner == nil then
        return false
    end

    return Player.performBankDeposit(
        player, npc, 0, bankOwner,
        "Deposit for " .. bankOwner.name .. ". What would you like me to hold onto?"
    )
end

-- Wrapper for clan bank deposit
function Player.showClanBankDeposit_refactored(player, npc)
    return Player.performBankDeposit(
        player, npc, 2, player,
        "What would you like me to deposit in your clan's bank?"
    )
end

--[[
    Health modification functions - refactored to use shared calculateNetDamage
    Original addHealthExtend: ~109 lines
    Refactored: ~50 lines (55% reduction)
]]

function Player.addHealthExtend_refactored(player, amount, sleep, deduction, ac, ds, print)
    if player.state == 1 then
        return
    end

    local healer = player_base.getAttacker(player)

    -- Apply blossom duration bonus
    if healer and healer:hasDuration("blossom") then
        amount = amount * 2
    end

    -- Apply modifiers using shared logic
    amount = player_base.applySleepModifier(player, amount, sleep)
    amount = player_base.applyDeductionModifier(player, amount, deduction)
    amount = player_base.applyArmorModifier(player, amount, ac, false)  -- no caps for healing
    amount, _ = player_base.applyDamageShield(player, amount, ds)

    -- Negate amount (this function actually applies damage despite the name)
    amount = -amount

    -- PvP map protection
    if healer and player.gfxDye ~= healer.gfxDye then
        if player.m == 33 or player.m == 3011 or player.m == 3017 then
            amount = 0
        end
    end

    -- Set damage/crit for display
    if healer then
        healer.damage = amount
        healer.critChance = 0
    else
        player.damage = amount
        player.critChance = 0
    end

    -- Handle print modes
    if print == 1 then
        if player.health - amount > player.maxHealth then
            player.health = player.maxHealth
            player:updateState()
        else
            player.health = player.health - amount
            player:sendStatus()
        end
    elseif print == 2 then
        return amount
    else
        player.attacker = healer.ID
        player:sendHealth(math.floor(healer.damage), healer.critChance)
        player:sendStatus()
    end
end

