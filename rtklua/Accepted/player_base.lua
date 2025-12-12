--[[
    player_base.lua - Shared player function infrastructure

    Provides data-driven utilities for player.lua functions:
    - Equipment slot definitions
    - Repair cost calculation
    - Bank operation helpers
    - Health modification utilities

    Part of RTK Server refactoring - Phase 2.3
]]

-- Equipment slot constants for repair functions
player_base = {}

player_base.EQUIPMENT_SLOTS = {
    EQ_FACEACC, EQ_HELM, EQ_CROWN, EQ_WEAP, EQ_ARMOR, EQ_SHIELD,
    EQ_LEFT, EQ_RIGHT, EQ_MANTLE, EQ_SUBLEFT, EQ_SUBRIGHT,
    EQ_NECKLACE, EQ_BOOTS, EQ_COAT
}

-- Calculate repair cost for an item
-- Formula: ((price / maxDura) * (maxDura - dura)) * 0.5
function player_base.calculateRepairCost(item)
    if item == nil or item.maxDura == nil or item.maxDura == 0 then
        return 0
    end
    return math.ceil(((item.price / item.maxDura) * (item.maxDura - item.dura)) * 0.5)
end

-- Check if item needs repair
function player_base.needsRepair(item)
    return item ~= nil
        and item.repairable == 1
        and item.dura < item.maxDura
end

-- Check if item is equipment type (for inventory scanning)
function player_base.isEquipmentType(item)
    return item ~= nil
        and item.type > 2
        and item.type < 17
end

-- Get all equipped items that need repair
function player_base.getEquippedItemsNeedingRepair(player)
    local items = {}
    local totalCost = 0

    for _, slot in ipairs(player_base.EQUIPMENT_SLOTS) do
        local item = player:getEquippedItem(slot)
        if item then
            if player_base.needsRepair(item) then
                local cost = player_base.calculateRepairCost(item)
                totalCost = totalCost + cost
                table.insert(items, { item = item, cost = cost, slot = slot })
            elseif item.repairable ~= 1 and item.dura < item.maxDura then
                player:sendMinitext(item.name .. " is not a repairable item.")
            end
        end
    end

    return items, totalCost
end

-- Get all inventory items that need repair (equipment type only)
function player_base.getInventoryItemsNeedingRepair(player)
    local items = {}
    local totalCost = 0

    for i = 0, player.maxInv or 52 do
        local item = player:getInventoryItem(i)
        if item and player_base.isEquipmentType(item) then
            if player_base.needsRepair(item) then
                local cost = player_base.calculateRepairCost(item)
                totalCost = totalCost + cost
                table.insert(items, { item = item, cost = cost, slot = i })
            elseif item.repairable ~= 1 and item.dura < item.maxDura then
                player:sendMinitext(item.name .. " is not a repairable item.")
            end
        end
    end

    return items, totalCost
end

-- Get all items needing repair (equipped + inventory)
function player_base.getAllItemsNeedingRepair(player)
    local equippedItems, equippedCost = player_base.getEquippedItemsNeedingRepair(player)
    local inventoryItems, inventoryCost = player_base.getInventoryItemsNeedingRepair(player)

    -- Combine lists
    for _, itemData in ipairs(inventoryItems) do
        table.insert(equippedItems, itemData)
    end

    return equippedItems, equippedCost + inventoryCost
end

-- Repair all items in a list
function player_base.repairItems(items)
    for _, itemData in ipairs(items) do
        if itemData.item then
            itemData.item.dura = itemData.item.maxDura
            itemData.item.repairCheck = 0
        end
    end
end

-- Bank operation helpers

-- Build unique item list from inventory (for deposit dialogs)
function player_base.buildInventoryItemList(player)
    local itemTable = {}
    local seen = {}

    for i = 0, player.maxInv or 52 do
        local item = player:getInventoryItem(i)
        if item and item.id > 0 and not seen[item.id] then
            table.insert(itemTable, item.id)
            seen[item.id] = true
        end
    end

    return itemTable
end

-- Build buytext with owner info for bank display
function player_base.buildBankBuyText(bankOwnerTable)
    local buytext = {}

    for i = 1, #bankOwnerTable do
        local text = ""
        if bankOwnerTable[i] ~= 0 then
            text = "Owner: " .. getOfflineID(bankOwnerTable[i])
        end
        table.insert(buytext, text)
    end

    return buytext
end

-- Check max amount constraint for item
function player_base.checkMaxAmountConstraint(player, itemId, requestedAmount)
    local maxAmount = Item(itemId).maxAmount
    if maxAmount <= 0 then
        return true, nil
    end

    local hasAmount = 0
    for i = 0, 52 do
        local item = player:getInventoryItem(i)
        if item and item.id == itemId then
            hasAmount = item.amount
            break
        end
    end

    if hasAmount >= maxAmount or hasAmount + requestedAmount > maxAmount then
        return false, "(" .. Item(itemId).name .. "). You can't have more than (" .. maxAmount .. ")."
    end

    return true, nil
end

-- Get stackable amount from player input
function player_base.getStackableAmount(player, maxAvailable, itemId)
    if Item(itemId).stackAmount > 1 then
        local amount = player:inputNumberCheck(player:input("How many shall you withdraw?"))
        amount = math.ceil(math.abs(amount))
        if amount > maxAvailable then
            amount = maxAvailable
        end
        return amount
    else
        return 1
    end
end

-- Health/Magic utility functions

-- Apply sleep multiplier to amount
function player_base.applySleepModifier(player, amount, sleepMode)
    if sleepMode > 0 then
        amount = amount * player.sleep
        if sleepMode == 1 then
            player.sleep = 1
            player:updateState()
        end
    end
    return amount
end

-- Apply deduction multiplier to amount
function player_base.applyDeductionModifier(player, amount, useDeduction)
    if useDeduction == 1 then
        if player.deduction < 0 then
            return 0
        elseif player.deduction > 0 then
            return amount * player.deduction
        end
    end
    return amount
end

-- Apply armor class modifier to amount (with optional caps)
function player_base.applyArmorModifier(player, amount, useArmor, useCaps)
    if useArmor ~= 1 then
        return amount
    end

    local armorAmount = player.armor

    if useCaps then
        if armorAmount < -80 then armorAmount = -80 end
        if armorAmount > 100 then armorAmount = 100 end
    end

    if armorAmount ~= 0 then
        return amount * (1 + (armorAmount / 100))
    end

    return amount
end

-- Apply damage shield to amount
-- Returns: modifiedAmount, newShieldValue
function player_base.applyDamageShield(player, amount, shieldMode)
    local shield = player.dmgShield

    if shieldMode == 1 then
        if shield > 0 then
            if shield > amount then
                player.dmgShield = shield - amount
                return 0, player.dmgShield
            else
                amount = amount - shield
                player.dmgShield = 0
                return amount, 0
            end
        else
            amount = amount - shield
            player.dmgShield = 0
            return amount, 0
        end
    elseif shieldMode == 2 then
        player.dmgShield = shield - amount
        return amount, player.dmgShield
    end

    -- shieldMode > 0 but not 1 or 2 (preview mode)
    if shieldMode > 0 then
        return amount - shield, shield
    end

    return amount, shield
end

-- Get attacker reference (Mob or Player)
function player_base.getAttacker(player)
    if player.attacker >= 1073741823 then
        return Mob(player.attacker)
    elseif player.attacker > 0 then
        return Player(player.attacker)
    end
    return nil
end

