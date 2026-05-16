/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

#include "ScriptMgr.h"
#include "Player.h"
#include "Config.h"
#include "Chat.h"
#include "GameObject.h"
#include "Map.h"
#include "Bag.h"
#include "Item.h"
#include "LootMgr.h"
#include "ObjectMgr.h"
#include "SpellMgr.h"
#include "SpellInfo.h"
#include "DBCStores.h"
#include "GameObjectAI.h"
#include "GameTime.h"
#include <unordered_map>

// GameObject entry for the loot bag
#define LOOT_BAG_ENTRY 400002

// Despawn time in seconds (2 minutes for testing)
#define LOOT_BAG_DESPAWN_TIME 900

// Structure to store original item data for copying enchantments and gems
struct OriginalItemData
{
    ObjectGuid itemGUID;
    uint32 entry;
    uint32 count;
    int32 randomPropertyId;
    uint32 randomSuffix;
    
    // Enchantments (PERM_ENCHANTMENT_SLOT to TEMP_ENCHANTMENT_SLOT)
    struct EnchantmentData
    {
        uint32 id;
        uint32 duration;
        uint32 charges;
    };
    EnchantmentData enchantments[MAX_ENCHANTMENT_SLOT];
    
    // Durability
    uint32 durability;
    uint32 maxDurability;
    
    // Gems (stored as enchantments in SOCK_ENCHANTMENT_SLOT to SOCK_ENCHANTMENT_SLOT + MAX_GEM_SOCKETS)
    // Already included in enchantments array
    
    OriginalItemData() : itemGUID(), entry(0), count(0), randomPropertyId(0), randomSuffix(0), 
                         durability(0), maxDurability(0)
    {
        for (uint8 i = 0; i < MAX_ENCHANTMENT_SLOT; ++i)
        {
            enchantments[i].id = 0;
            enchantments[i].duration = 0;
            enchantments[i].charges = 0;
        }
    }
};

// Map to store original items by GameObject GUID and item index
static std::unordered_map<ObjectGuid, std::unordered_map<uint32, OriginalItemData>> s_originalItemsMap;

// Script to handle player death and spawn loot bag
class ConquestLootDropPlayer : public PlayerScript
{
public:
    ConquestLootDropPlayer() : PlayerScript("ConquestLootDropPlayer") { }

    void OnPlayerJustDied(Player* player) override
    {
        if (!sConfigMgr->GetOption<bool>("ConquestLootDrop.Enable", true))
            return;

        // Don't spawn loot bag in battlegrounds or arenas
        if (player->InBattleground() || player->InArena())
            return;

        // Spawn the loot bag gameobject at player position
        // Use Create() directly like mod-gvg-build does for better visibility
        Map* map = player->GetMap();
        if (!map)
        {
            LOG_ERROR("module", "ConquestLootDrop: Player {} is not in a valid map", player->GetName());
            return;
        }

        float x = player->GetPositionX();
        float y = player->GetPositionY();
        float z = player->GetPositionZ();
        float o = player->GetOrientation();
        
        // Create the GameObject directly (like mod-gvg-build does)
        GameObject* lootBag = new GameObject();
        ObjectGuid::LowType guidLow = map->GenerateLowGuid<HighGuid::GameObject>();
        G3D::Quat rotation(0, 0, 0, 1); // Identity quaternion for no rotation
        
        if (!lootBag->Create(guidLow, LOOT_BAG_ENTRY, map, player->GetPhaseMask(), 
                            x, y, z, o, rotation, 100, GO_STATE_READY))
        {
            delete lootBag;
            LOG_ERROR("module", "ConquestLootDrop: Failed to create loot bag for player {} at position ({}, {}, {})", player->GetName(), x, y, z);
            return;
        }

        // Set as temporary GameObject (will despawn after time)
        lootBag->SetSpawnedByDefault(false); // Temporary GameObject
        lootBag->SetRespawnTime(LOOT_BAG_DESPAWN_TIME); // Set despawn time in seconds
        
        // Add to map (this makes it visible)
        if (!map->AddToMap(lootBag))
        {
            delete lootBag;
            LOG_ERROR("module", "ConquestLootDrop: Failed to add loot bag to map for player {}", player->GetName());
            return;
        }

        // Fill the loot with all player items
        FillLootBagWithPlayerItems(player, lootBag);
        
        // Force visibility update after filling loot
        lootBag->UpdateObjectVisibility(true);
    }

private:
    void FillLootBagWithPlayerItems(Player* deadPlayer, GameObject* lootBag)
    {
        if (!deadPlayer || !lootBag)
            return;

        // Clear any existing loot
        lootBag->loot.clear();

        // Set loot owner to the dead player (this is the player whose items we're looting)
        lootBag->loot.lootOwnerGUID = deadPlayer->GetGUID();
        
        // IMPORTANT: Set sourceWorldObjectGUID to the GameObject GUID
        // This is used by AllowedForPlayer to check if items can be looted
        lootBag->loot.sourceWorldObjectGUID = lootBag->GetGUID();
        
        // IMPORTANT: Set containerGUID to the GameObject GUID for FillFFALoot
        // FillFFALoot uses containerGUID instead of sourceWorldObjectGUID
        lootBag->loot.containerGUID = lootBag->GetGUID();

        // Conquest patch: l'équipement (slots 0-18) NE drop PAS.
        // Le joueur garde son gear R14/T3 à la mort. Seuls les bags + contenu droppent.
        // Cette boucle a été retirée volontairement.

        // Copy all inventory items (backpack) from the DEAD player
        for (uint8 i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
        {
            if (Item* item = deadPlayer->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            {
                AddItemToLoot(item, lootBag->loot, lootBag, i);
            }
        }

        // Copy all items from bags (but not the bags themselves) from the DEAD player
        for (uint8 i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
        {
            if (Bag* bag = deadPlayer->GetBagByPos(i))
            {
                for (uint32 j = 0; j < bag->GetBagSize(); ++j)
                {
                    if (Item* item = bag->GetItemByPos(j))
                    {
                        AddItemToLoot(item, lootBag->loot, lootBag, j);
                    }
                }
            }
        }

        // Update unlootedCount for proper display
        lootBag->loot.unlootedCount = lootBag->loot.items.size();
        
        // Set loot recipient to allow anyone to loot (free for all)
        // This is important for the loot system to allow players to see the loot
        lootBag->SetLootRecipient(deadPlayer->GetMap());
        
        // Add all players in range as allowed looters (free for all loot)
        // This ensures that any player can loot the bag
        Map::PlayerList const& players = deadPlayer->GetMap()->GetPlayers();
        for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
        {
            if (Player* nearbyPlayer = itr->GetSource())
            {
                if (nearbyPlayer->IsWithinDistInMap(lootBag, INTERACTION_DISTANCE * 2))
                {
                    lootBag->AddAllowedLooter(nearbyPlayer->GetGUID());
                }
            }
        }

        // Set loot generation time (important for the system to recognize the loot)
        lootBag->SetLootGenerationTime();

        // Set the loot state to ready
        lootBag->SetLootState(GO_READY);
    }

    void AddItemToLoot(Item* item, Loot& loot, GameObject* lootBag, uint8 /*slot*/)
    {
        if (!item || !lootBag)
            return;

        ItemTemplate const* itemTemplate = item->GetTemplate();
        if (!itemTemplate)
            return;

        // Skip bags - equipped bags should not be lootable
        if (item->IsBag())
            return;

        // Create a new loot item with default constructor
        LootItem lootItem;
        
        // Set item properties
        lootItem.itemid = item->GetEntry();
        lootItem.count = item->GetCount();
        lootItem.randomPropertyId = item->GetItemRandomPropertyId();
        lootItem.randomSuffix = item->GetItemSuffixFactor();
        lootItem.itemIndex = loot.items.size();
        
        // Set loot flags
        lootItem.is_looted = false;
        lootItem.is_blocked = false;
        lootItem.is_underthreshold = false;
        lootItem.is_counted = true;
        lootItem.needs_quest = false;
        lootItem.follow_loot_rules = false;
        lootItem.freeforall = false;  // Shared loot - when one player loots, it disappears for others
        lootItem.groupid = 0;
        
        // Clear conditions - no special conditions needed
        // IMPORTANT: Empty conditions are required for FFA items to be visible
        lootItem.conditions.clear();
        
        // Clear allowed looters - free for all
        lootItem.allowedGUIDs.clear();
        
        // IMPORTANT: For FFA items, we need to ensure they are counted in unlootedCount
        // This is done automatically by the loot system when conditions are empty
        
        // Add to loot items list
        loot.items.push_back(lootItem);
        
        // Store original item data for copying enchantments and gems
        OriginalItemData originalData;
        originalData.itemGUID = item->GetGUID();
        originalData.entry = item->GetEntry();
        originalData.count = item->GetCount();
        originalData.randomPropertyId = item->GetItemRandomPropertyId();
        originalData.randomSuffix = item->GetItemSuffixFactor();
        originalData.durability = item->GetUInt32Value(ITEM_FIELD_DURABILITY);
        originalData.maxDurability = item->GetUInt32Value(ITEM_FIELD_MAXDURABILITY);
        
        // Copy all enchantments (including gems which are stored as enchantments)
        uint32 enchantmentsFound = 0;
        uint32 gemsFound = 0;
        for (uint8 i = 0; i < MAX_ENCHANTMENT_SLOT; ++i)
        {
            originalData.enchantments[i].id = item->GetEnchantmentId(EnchantmentSlot(i));
            originalData.enchantments[i].duration = item->GetEnchantmentDuration(EnchantmentSlot(i));
            originalData.enchantments[i].charges = item->GetEnchantmentCharges(EnchantmentSlot(i));
            
            if (originalData.enchantments[i].id > 0)
            {
                enchantmentsFound++;
                // Check if this is a gem slot (SOCK_ENCHANTMENT_SLOT, SOCK_ENCHANTMENT_SLOT_2, SOCK_ENCHANTMENT_SLOT_3)
                if (i >= SOCK_ENCHANTMENT_SLOT && i <= SOCK_ENCHANTMENT_SLOT_3)
                    gemsFound++;
            }
        }
        
        // Store in map for later retrieval
        s_originalItemsMap[lootBag->GetGUID()][lootItem.itemIndex] = originalData;
    }

};

// AI to handle despawn timer for temporary GameObjects
class ConquestLootDropGameObjectAI : public GameObjectAI
{
public:
    explicit ConquestLootDropGameObjectAI(GameObject* go) : GameObjectAI(go) { }

    void UpdateAI(uint32 /*diff*/) override
    {
        // Check if this is a temporary GameObject that should despawn
        if (!me->isSpawnedByDefault() && me->GetRespawnDelay() > 0)
        {
            time_t respawnTime = me->GetRespawnTime();
            if (respawnTime > 0)
            {
                time_t now = GameTime::GetGameTime().count();
                if (now >= respawnTime)
                {
                    // Despawn time has passed, delete the GameObject
                    s_originalItemsMap.erase(me->GetGUID());
                    me->SetRespawnTime(0);
                    me->Delete();
                }
            }
        }
    }
};

// Script to handle gameobject loot state changes and opening
class ConquestLootDropGameObject : public GameObjectScript
{
public:
    ConquestLootDropGameObject() : GameObjectScript("ConquestLootDropGameObject") { }

    GameObjectAI* GetAI(GameObject* go) const override
    {
        return new ConquestLootDropGameObjectAI(go);
    }

    bool OnGossipHello(Player* lootingPlayer, GameObject* go) override
    {
        if (!go || go->GetEntry() != LOOT_BAG_ENTRY)
            return false;

        // For CHEST type GameObjects, we need to manually call SendLoot
        // The core doesn't automatically open loot for CHEST when OnGossipHello returns false
        // Allow opening if loot is READY or ACTIVATED (so it can be reopened)
        if ((go->getLootState() == GO_READY || go->getLootState() == GO_ACTIVATED) && !go->loot.isLooted())
        {
            // Check if player is within interaction distance
            if (!go->IsWithinDistInMap(lootingPlayer, INTERACTION_DISTANCE))
                return false;
            
            // Manually open the loot window
            // Use LOOT_SKINNING instead of LOOT_CORPSE to avoid the respawn time check
            // LOOT_SKINNING doesn't have the same restrictions as LOOT_CORPSE for GameObjects
            // Since items are not FFA, the normal loot system will handle shared items correctly
            lootingPlayer->SendLoot(go->GetGUID(), LOOT_SKINNING);
            
            // Return true to prevent default behavior (we've handled it)
            return true;
        }
        
        // Return false to allow default behavior if loot is not ready
        return false;
    }

    void OnLootStateChanged(GameObject* go, uint32 state, Unit* /*unit*/) override
    {
        if (!go || go->GetEntry() != LOOT_BAG_ENTRY)
            return;

        // Check if loot is completely empty when the loot window is closed
        // GO_JUST_DEACTIVATED is set when player closes the loot window
        if (state == GO_JUST_DEACTIVATED && go->loot.isLooted())
        {
            // Clean up stored original items data
            s_originalItemsMap.erase(go->GetGUID());
            
            // Despawn the gameobject immediately when all loot is taken
            go->SetRespawnTime(0);
            go->Delete();
        }
    }
};

// Script to handle copying item properties when looted
class ConquestLootDropPlayerLoot : public PlayerScript
{
public:
    ConquestLootDropPlayerLoot() : PlayerScript("ConquestLootDropPlayerLoot") { }

    void OnPlayerLootItem(Player* player, Item* newItem, uint32 /*count*/, ObjectGuid lootguid) override
    {
        if (!newItem || !lootguid.IsGameObject())
            return;

        // Check if this is our loot bag
        GameObject* go = player->GetMap()->GetGameObject(lootguid);
        if (!go || go->GetEntry() != LOOT_BAG_ENTRY)
            return;

        // Find the loot item that corresponds to this item
        Loot* loot = &go->loot;
        uint32 itemIndex = 0;
        bool found = false;
        
        for (size_t i = 0; i < loot->items.size(); ++i)
        {
            if (loot->items[i].itemid == newItem->GetEntry() && 
                loot->items[i].randomPropertyId == newItem->GetItemRandomPropertyId() &&
                loot->items[i].randomSuffix == newItem->GetItemSuffixFactor())
            {
                itemIndex = loot->items[i].itemIndex;
                found = true;
                break;
            }
        }

        if (!found)
            return;

        // Get original item data
        auto goItr = s_originalItemsMap.find(go->GetGUID());
        if (goItr == s_originalItemsMap.end())
        {
            LOG_WARN("module", "ConquestLootDrop: OnPlayerLootItem - no original items data found for GameObject {}", go->GetGUID().ToString());
            return;
        }

        auto itemItr = goItr->second.find(itemIndex);
        if (itemItr == goItr->second.end())
        {
            LOG_WARN("module", "ConquestLootDrop: OnPlayerLootItem - no original item data found for index {}", itemIndex);
            return;
        }

        OriginalItemData const& originalData = itemItr->second;

        // Copy all enchantments (including gems)
        for (uint8 i = 0; i < MAX_ENCHANTMENT_SLOT; ++i)
        {
            if (originalData.enchantments[i].id > 0)
            {
                newItem->SetEnchantment(
                    EnchantmentSlot(i),
                    originalData.enchantments[i].id,
                    originalData.enchantments[i].duration,
                    originalData.enchantments[i].charges
                );
            }
        }

        // Copy durability
        if (originalData.maxDurability > 0)
        {
            uint32 newDurability = originalData.durability;
            if (newDurability > originalData.maxDurability)
                newDurability = originalData.maxDurability;
            newItem->SetUInt32Value(ITEM_FIELD_DURABILITY, newDurability);
        }
    }
};

// Add all scripts
void AddConquestLootDropScripts()
{
    new ConquestLootDropPlayer();
    new ConquestLootDropGameObject();
    new ConquestLootDropPlayerLoot();
}
