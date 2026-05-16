/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

#include "ScriptMgr.h"
#include "Player.h"
#include "Config.h"
#include "Log.h"
#include "Map.h"
#include "MapMgr.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "ItemTemplate.h"
#include "DBCStores.h"
#include "Battleground.h"
#include "BattlegroundMgr.h"
#include "SharedDefines.h"
#include "Item.h"

// Map IDs to block
#define MAP_OUTLAND 530  // Outreterre
#define MAP_NORTHREND 571  // Norfendre

// Default teleport location (GM Island)
#define DEFAULT_MAP_ID 1
#define DEFAULT_X 16201.588f
#define DEFAULT_Y 16211.277f
#define DEFAULT_Z 1.1370115f
#define DEFAULT_O 1.1048489f

namespace
{
void TeleportPlayerToHomebind(Player* player)
{
    if (!player)
        return;

    uint32 mapId = player->m_homebindMapId;
    float x = player->m_homebindX;
    float y = player->m_homebindY;
    float z = player->m_homebindZ;

    if (MapMgr::IsValidMapCoord(mapId, x, y, z))
    {
        player->TeleportTo(mapId, x, y, z, player->GetOrientation());
    }
    else
    {
        LOG_WARN("module", "ConquestRestrictions: Invalid homebind location for player {}. Using default fallback.", player->GetName());
        player->TeleportTo(DEFAULT_MAP_ID, DEFAULT_X, DEFAULT_Y, DEFAULT_Z, DEFAULT_O);
    }
}
} // namespace

// Script to prevent mount item usage
class ConquestRestrictionsPlayer : public PlayerScript
{
public:
    ConquestRestrictionsPlayer() : PlayerScript("ConquestRestrictionsPlayer") { }

    bool OnPlayerCanUseItem(Player* player, ItemTemplate const* proto, InventoryResult& result) override
    {
        if (!sConfigMgr->GetOption<bool>("ConquestCore.Enable", true))
            return true;

        if (player->IsGameMaster())
            return true;

        // Check if item is a mount item
        if (IsMountItem(proto))
        {
            result = EQUIP_ERR_CANT_DO_RIGHT_NOW;
            player->SendEquipError(result, nullptr, nullptr);
            return false;
        }

        return true;
    }

    bool OnPlayerCanEnterMap(Player* player, MapEntry const* entry, InstanceTemplate const* /*instance*/, MapDifficulty const* /*mapDiff*/, bool /*loginCheck*/) override
    {
        if (!sConfigMgr->GetOption<bool>("ConquestCore.Enable", true))
            return true;

        if (player->IsGameMaster())
            return true;

        // Block entry to instances and raids
        if (entry && (entry->IsDungeon() || entry->IsRaid()))
        {
            LOG_INFO("module", "ConquestRestrictions: Player {} attempted to enter instance/raid (map {}), blocking", 
                player->GetName(), entry->MapID);
            player->SendTransferAborted(entry->MapID, TRANSFER_ABORT_MAP_NOT_ALLOWED);
            return false;
        }

        return true;
    }

    void OnPlayerMapChanged(Player* player) override
    {
        if (!sConfigMgr->GetOption<bool>("ConquestCore.Enable", true))
            return;

        if (player->IsGameMaster())
            return;

        uint32 mapId = player->GetMapId();
        Map* map = player->GetMap();
        
        if (!map)
            return;

        // Block Outland (530) and Northrend (571)
        if (mapId == MAP_OUTLAND || mapId == MAP_NORTHREND)
        {
            LOG_INFO("module", "ConquestRestrictions: Player {} attempted to enter blocked map {}, teleporting to homebind", 
                player->GetName(), mapId);

            TeleportPlayerToHomebind(player);
            return;
        }

        // Note: Instance/raid blocking is handled in OnPlayerCanEnterMap
        // If a player somehow enters an instance despite the hook, we log it but don't teleport
        // The OnPlayerCanEnterMap hook should prevent entry like a level requirement
        if (map->IsDungeon() || map->IsRaid())
        {
            LOG_INFO("module", "ConquestRestrictions: Player {} entered instance/raid (map {}) despite restriction - this should not happen", 
                player->GetName(), mapId);
        }
    }

    bool OnPlayerCanJoinInArenaQueue(Player* player, ObjectGuid /*BattlemasterGuid*/, uint8 /*arenaslot*/, BattlegroundTypeId /*BGTypeID*/, uint8 /*joinAsGroup*/, uint8 /*IsRated*/, GroupJoinBattlegroundResult& err) override
    {
        if (!sConfigMgr->GetOption<bool>("ConquestCore.Enable", true))
            return true;

        if (player->IsGameMaster())
            return true;

        // Block arena queue
        LOG_INFO("module", "ConquestRestrictions: Player {} attempted to join arena queue, blocking", player->GetName());
        err = ERR_BATTLEGROUND_JOIN_FAILED;
        return false;
    }

    bool OnPlayerCanJoinInBattlegroundQueue(Player* player, ObjectGuid /*BattlemasterGuid*/, BattlegroundTypeId /*BGTypeID*/, uint8 /*joinAsGroup*/, GroupJoinBattlegroundResult& err) override
    {
        if (!sConfigMgr->GetOption<bool>("ConquestCore.Enable", true))
            return true;

        if (player->IsGameMaster())
            return true;

        // Block battleground queue
        LOG_INFO("module", "ConquestRestrictions: Player {} attempted to join battleground queue, blocking", player->GetName());
        err = ERR_BATTLEGROUND_JOIN_FAILED;
        return false;
    }

    void OnPlayerAddToBattleground(Player* player, Battleground* /*bg*/) override
    {
        if (!sConfigMgr->GetOption<bool>("ConquestCore.Enable", true))
            return;

        if (player->IsGameMaster())
            return;

        // Remove player from battleground immediately
        if (player->InBattleground())
        {
            LOG_INFO("module", "ConquestRestrictions: Player {} attempted to join battleground, removing", player->GetName());
            player->LeaveBattleground();
        }
    }

    void OnPlayerStoreNewItem(Player* player, Item* item, uint32 /*count*/) override
    {
        if (!sConfigMgr->GetOption<bool>("ConquestCore.Enable", true))
            return;

        if (player->IsGameMaster())
            return;

        if (!item)
            return;

        // Explicitly remove binding on all items stored in inventory
        // This ensures items are never soulbound, regardless of their template settings
        item->SetBinding(false);
    }

private:
    bool IsMountItem(ItemTemplate const* proto) const
    {
        if (!proto)
            return false;

        // Check if item has a spell that grants SPELL_AURA_MOUNTED
        for (uint8 i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
        {
            if (proto->Spells[i].SpellId > 0)
            {
                SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(proto->Spells[i].SpellId);
                if (spellInfo)
                {
                    // Check if spell has SPELL_AURA_MOUNTED aura
                    if (spellInfo->HasAura(SPELL_AURA_MOUNTED))
                    {
                        return true;
                    }
                }
            }
        }

        return false;
    }
};

// Add all scripts
void AddConquestRestrictionsScripts()
{
    new ConquestRestrictionsPlayer();
}

