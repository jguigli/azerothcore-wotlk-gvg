/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

#include "Config.h"
#include "Log.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "Item.h"
#include "Bag.h"
#include "Creature.h"

namespace
{
constexpr uint32 HEARTHSTONE_ENTRY = 6948;

bool IsEnabled()
{
    return sConfigMgr->GetOption<bool>("ConquestCore.Enable", true);
}

void ClearInventoryExceptHearthstone(Player* player)
{
    if (!player || !player->IsInWorld())
        return;
    
    // Don't manipulate inventory if player is being teleported
    if (player->IsBeingTeleported())
        return;

    // Store hearthstone if player has one and move it to main inventory if needed
    Item* hearthstone = player->GetItemByEntry(HEARTHSTONE_ENTRY);
    
    if (hearthstone)
    {
        uint8 hearthstoneBag = hearthstone->GetBagSlot();
        uint8 hearthstoneSlot = hearthstone->GetSlot();
        
        // If hearthstone is in a bag, move it to main inventory first
        if (hearthstoneBag != NULL_BAG)
        {
            ItemPosCountVec dest;
            InventoryResult msg = player->CanStoreItem(NULL_BAG, NULL_SLOT, dest, hearthstone, false);
            if (msg == EQUIP_ERR_OK)
            {
                player->RemoveItem(hearthstoneBag, hearthstoneSlot, true);
                player->StoreItem(dest, hearthstone, true);
                LOG_INFO("module", "ConquestPlayerDeath: Moved hearthstone from bag {} to main inventory for player {}", hearthstoneBag, player->GetName());
            }
        }
    }

    // Clear inventory slots (main backpack)
    for (uint8 i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
    {
        Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (item && item->GetEntry() != HEARTHSTONE_ENTRY)
        {
            player->DestroyItem(INVENTORY_SLOT_BAG_0, i, true);
        }
    }

    // Clear bags and their contents
    for (uint8 i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
    {
        Bag* bag = player->GetBagByPos(i);
        if (bag)
        {
            // Clear items inside the bag (hearthstone should already be moved to main inventory)
            for (uint32 j = 0; j < bag->GetBagSize(); ++j)
            {
                Item* item = bag->GetItemByPos(j);
                if (item && item->GetEntry() != HEARTHSTONE_ENTRY)
                {
                    player->DestroyItem(i, j, true);
                }
            }
            
            // Destroy the bag itself
            player->DestroyItem(INVENTORY_SLOT_BAG_0, i, true);
        }
    }

    // Clear keyring slots
    for (uint8 i = KEYRING_SLOT_START; i < CURRENCYTOKEN_SLOT_END; ++i)
    {
        Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (item && item->GetEntry() != HEARTHSTONE_ENTRY)
        {
            player->DestroyItem(INVENTORY_SLOT_BAG_0, i, true);
        }
    }

    // Clear equipment slots (unequip everything)
    for (uint8 i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; ++i)
    {
        Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (item && item->GetEntry() != HEARTHSTONE_ENTRY)
        {
            player->DestroyItem(INVENTORY_SLOT_BAG_0, i, true);
        }
    }

    // Ensure hearthstone is in inventory
    Item* remainingHearthstone = player->GetItemByEntry(HEARTHSTONE_ENTRY);
    if (!remainingHearthstone)
    {
        // Hearthstone was lost or player didn't have one, create a new one
        Item* newHearthstone = Item::CreateItem(HEARTHSTONE_ENTRY, 1, player);
        if (newHearthstone)
        {
            // Explicitly remove binding
            newHearthstone->SetBinding(false);
            
            ItemPosCountVec dest;
            InventoryResult msg = player->CanStoreItem(NULL_BAG, NULL_SLOT, dest, newHearthstone, false);
            if (msg == EQUIP_ERR_OK)
            {
                player->StoreItem(dest, newHearthstone, true);
                LOG_INFO("module", "ConquestPlayerDeath: Created hearthstone for player {} after inventory clear", player->GetName());
            }
            else
            {
                delete newHearthstone;
                LOG_WARN("module", "ConquestPlayerDeath: Could not store hearthstone for player {} after inventory clear", player->GetName());
            }
        }
    }

    LOG_INFO("module", "ConquestPlayerDeath: Cleared inventory for player {}, kept only hearthstone", player->GetName());
}

} // namespace

class ConquestPlayerDeath : public PlayerScript
{
public:
    ConquestPlayerDeath() : PlayerScript("ConquestPlayerDeath") { }

    void OnPlayerJustDied(Player* player) override
    {
        if (!IsEnabled())
            return;

        if (player->IsGameMaster())
            return;

        // Schedule inventory clearing after a short delay (1 second)
        // This allows the loot drop system to copy items before they are destroyed
        player->m_Events.AddEventAtOffset([player]()
        {
            if (!player || !player->IsInWorld())
                return;

            ClearInventoryExceptHearthstone(player);
            LOG_INFO("module", "ConquestPlayerDeath: Player {} died - inventory cleared after delay", player->GetName());
        }, 1s);
    }
};

void AddConquestPlayerDeathScripts()
{
    new ConquestPlayerDeath();
}

