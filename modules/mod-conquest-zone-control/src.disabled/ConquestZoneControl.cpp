/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

#include "ScriptMgr.h"
#include "Player.h"
#include "Config.h"
#include "Log.h"
#include "GameObject.h"
#include "ScriptedGossip.h"
#include "Map.h"
#include "ObjectMgr.h"
#include "Chat.h"
#include "DatabaseEnv.h"
#include "CharacterDatabase.h"
#include "WorldDatabase.h"
#include "Guild.h"
#include "GuildMgr.h"
#include "GuildScript.h"
#include "Item.h"
#include "Creature.h"
#include "GameTime.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Cell.h"
#include "CellImpl.h"
#include "MapMgr.h"
#include "AllSpellScript.h"
#include "Spell.h"
#include "SpellInfo.h"
#include "UpdateFieldFlags.h"
#include "UpdateData.h"
#include "AllGameObjectScript.h"
#include <map>
#include <unordered_map>
#include <vector>
#include <list>

// Banner entries
#define BANNER_DISPUTED_HORDE    400010
#define BANNER_DISPUTED_ALLIANCE 400011
#define BANNER_HORDE             400012
#define BANNER_ALLIANCE          400013

// Fire GameObject entry (appears during capture)
#define CAPTURE_FIRE_GOB_ENTRY 193411

// Capture and reward defaults (overridable via config)
constexpr uint32 DEFAULT_CAPTURE_TIME_MS = 60000;      // 1 minute
constexpr uint32 DEFAULT_REWARD_GOLD_AMOUNT = 10;
constexpr uint32 DEFAULT_REWARD_WOOD_AMOUNT = 10;
constexpr uint32 DEFAULT_REWARD_INTERVAL_SECONDS = 600;

// Reward items
#define REWARD_GOLD_ITEM_ENTRY 80020 // Or Conquest
#define REWARD_WOOD_ITEM_ENTRY 80021 // Bois Conquest

// Despawn radius: 200 yards
#define DESPAWN_RADIUS 200.0f

namespace
{
    // Structure to track capture progress
    struct CaptureProgress
    {
        ObjectGuid playerGuid;
        ObjectGuid bannerGuid;
        ObjectGuid fireGobGuid; // GameObject 193411 (fire)
        int64 startTime; // Use int64 to match GetGameTimeMS().count()
        bool isCapturing;
        uint32 captureId; // Unique ID for this capture attempt to prevent old events from executing
    };
    
    // Counter for unique capture IDs
    uint32 s_captureIdCounter = 0;

    // Map to track ongoing captures: bannerGuid -> CaptureProgress
    std::map<ObjectGuid, CaptureProgress> s_captureProgress;

    // Map to track reward timers: zoneGuid -> last reward time
    std::map<ObjectGuid, uint32> s_rewardTimers;

    struct BannerInfo
    {
        ObjectGuid guid;
        uint32 mapId;
        uint32 instanceId;
        Position pos;
        G3D::Quat rotation;
        uint32 entry;
    };

    std::unordered_map<ObjectGuid::LowType, BannerInfo> s_bannerInfo;
    uint32 s_captureTimeMs = DEFAULT_CAPTURE_TIME_MS;
    uint32 s_rewardGoldAmount = DEFAULT_REWARD_GOLD_AMOUNT;
    uint32 s_rewardWoodAmount = DEFAULT_REWARD_WOOD_AMOUNT;
    uint32 s_rewardIntervalSeconds = DEFAULT_REWARD_INTERVAL_SECONDS;

    // Global reward timer (shared across all players)
    // Use timestamp-based approach to ensure rewards are processed exactly once per interval
    uint32 s_lastRewardProcessTime = 0;

    bool IsEnabled()
    {
        return sConfigMgr->GetOption<bool>("ConquestZoneControl.Enable", true);
    }

    void LoadConfigValues()
    {
        s_captureTimeMs = sConfigMgr->GetOption<uint32>("ConquestZoneControl.CaptureTimeMs", DEFAULT_CAPTURE_TIME_MS);
        if (s_captureTimeMs == 0)
            s_captureTimeMs = DEFAULT_CAPTURE_TIME_MS;

        s_rewardGoldAmount = sConfigMgr->GetOption<uint32>("ConquestZoneControl.RewardGoldAmount", DEFAULT_REWARD_GOLD_AMOUNT);
        s_rewardWoodAmount = sConfigMgr->GetOption<uint32>("ConquestZoneControl.RewardWoodAmount", DEFAULT_REWARD_WOOD_AMOUNT);

        s_rewardIntervalSeconds = sConfigMgr->GetOption<uint32>("ConquestZoneControl.RewardIntervalSeconds", DEFAULT_REWARD_INTERVAL_SECONDS);
        if (s_rewardIntervalSeconds == 0)
            s_rewardIntervalSeconds = DEFAULT_REWARD_INTERVAL_SECONDS;

        LOG_INFO("module", "ConquestZoneControl: Config loaded (CaptureTime={}ms, RewardGold={}, RewardWood={}, RewardInterval={}s)",
            s_captureTimeMs, s_rewardGoldAmount, s_rewardWoodAmount, s_rewardIntervalSeconds);
    }

    void RegisterBannerInfo(GameObject* banner)
    {
        if (!banner)
            return;

        BannerInfo info;
        info.guid = banner->GetGUID();
        info.mapId = banner->GetMapId();
        info.instanceId = banner->GetInstanceId();
        info.pos.Relocate(banner);
        info.rotation = banner->GetWorldRotation();
        info.entry = banner->GetEntry();

        s_bannerInfo[banner->GetGUID().GetCounter()] = info;
        LOG_DEBUG("module", "ConquestZoneControl: Registered banner info for guid {}", banner->GetGUID().GetCounter());
    }

    void UnregisterBannerInfo(GameObject* banner)
    {
        if (!banner)
            return;

        s_bannerInfo.erase(banner->GetGUID().GetCounter());
        LOG_DEBUG("module", "ConquestZoneControl: Unregistered banner info for guid {}", banner->GetGUID().GetCounter());
    }

    // Spawn fire GameObject at banner position
    GameObject* SpawnFireGameObject(GameObject* banner)
    {
        if (!banner)
            return nullptr;

        Map* map = banner->GetMap();
        if (!map)
            return nullptr;

        GameObjectTemplate const* goInfo = sObjectMgr->GetGameObjectTemplate(CAPTURE_FIRE_GOB_ENTRY);
        if (!goInfo)
        {
            LOG_ERROR("module", "ConquestZoneControl: GameObject template not found for entry {}", CAPTURE_FIRE_GOB_ENTRY);
            return nullptr;
        }

        // Use Map::SummonGameObject to create and spawn the GameObject
        GameObject* fireGob = map->SummonGameObject(
            CAPTURE_FIRE_GOB_ENTRY,
            banner->GetPositionX(),
            banner->GetPositionY(),
            banner->GetPositionZ(),
            banner->GetOrientation(),
            0.0f, 0.0f, 0.0f, 0.0f, // rotation quaternion
            0, // respawn time (0 = permanent until despawned)
            false // checkTransport
        );

        if (!fireGob)
        {
            LOG_ERROR("module", "ConquestZoneControl: Failed to spawn fire GameObject");
            return nullptr;
        }

        LOG_INFO("module", "ConquestZoneControl: Spawned fire GameObject {} at banner {}", fireGob->GetGUID().GetCounter(), banner->GetGUID().GetCounter());
        return fireGob;
    }

    // Despawn fire GameObject
    void DespawnFireGameObject(ObjectGuid fireGobGuid, WorldObject const* referenceObject = nullptr)
    {
        if (!fireGobGuid)
            return;

        GameObject* fireGob = nullptr;
        if (referenceObject)
        {
            fireGob = ObjectAccessor::GetGameObject(*referenceObject, fireGobGuid);
        }
        
        if (fireGob)
        {
            fireGob->SetRespawnTime(0);
            fireGob->Delete();
            LOG_INFO("module", "ConquestZoneControl: Despawned fire GameObject {}", fireGobGuid.GetCounter());
        }
    }
    
    // Reset GameObject to a fully clickable state
    void ResetBannerGameObject(GameObject* banner)
    {
        if (!banner || !banner->IsInWorld())
            return;
        
        // Remove all blocking flags
        GameObjectFlags blockingFlags = GO_FLAG_IN_USE | GO_FLAG_NOT_SELECTABLE | GO_FLAG_INTERACT_COND;
        if (banner->HasGameObjectFlag(blockingFlags))
        {
            banner->RemoveGameObjectFlag(blockingFlags);
            LOG_INFO("module", "ConquestZoneControl: Removed blocking flags from banner {}", banner->GetGUID().GetCounter());
        }
        
        // Reset loot state
        if (banner->getLootState() != GO_READY)
        {
            banner->SetLootState(GO_READY);
        }
        
        // Reset GO state
        if (banner->GetGoState() != GO_STATE_READY)
        {
            banner->SetGoState(GO_STATE_READY);
        }
        
        // Reset door/button
        banner->ResetDoorOrButton();
        
        // Force visual update
        banner->UpdateObjectVisibility(true);
        
        LOG_INFO("module", "ConquestZoneControl: Banner {} fully reset to clickable state", banner->GetGUID().GetCounter());
    }

    // Get faction from banner entry (unused for now, kept for future use)
    /*uint8 GetBannerFaction(uint32 bannerEntry)
    {
        if (bannerEntry == BANNER_DISPUTED_ALLIANCE || bannerEntry == BANNER_ALLIANCE)
            return 1; // Alliance
        else if (bannerEntry == BANNER_DISPUTED_HORDE || bannerEntry == BANNER_HORDE)
            return 2; // Horde
        return 0; // Neutral/Disputed
    }*/

    // Despawn creatures in radius
    void DespawnCreaturesInRadius(GameObject* banner, float radius)
    {
        if (!banner)
            return;

        Map* map = banner->GetMap();
        if (!map)
            return;

        std::list<Creature*> creaturesToDespawn;
        
        // Find all creatures in radius using custom search
        Acore::AnyUnitInObjectRangeCheck check(banner, radius);
        Acore::CreatureListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(banner, creaturesToDespawn, check);
        Cell::VisitObjects(banner, searcher, radius);

        for (Creature* creature : creaturesToDespawn)
        {
            if (!creature || !creature->IsAlive())
                continue;

            // Skip players and pets
            if (creature->IsPlayer() || creature->IsPet())
                continue;

            // Store creature data before despawning
            ObjectGuid::LowType spawnId = creature->GetSpawnId();
            if (spawnId)
            {
                CharacterDatabase.Execute(
                    "INSERT INTO conquest_zone_control_despawned_creatures (zone_guid, creature_guid, spawn_id, map_id, x, y, z, o) "
                    "VALUES ({}, {}, {}, {}, {}, {}, {}, {}) "
                    "ON DUPLICATE KEY UPDATE spawn_id = VALUES(spawn_id)",
                    banner->GetGUID().GetCounter(),
                    creature->GetGUID().GetCounter(),
                    spawnId,
                    map->GetId(),
                    creature->GetPositionX(),
                    creature->GetPositionY(),
                    creature->GetPositionZ(),
                    creature->GetOrientation()
                );
            }

            // Despawn creature
            creature->DespawnOrUnsummon();
            LOG_INFO("module", "ConquestZoneControl: Despawned creature {} at zone {}", creature->GetEntry(), banner->GetGUID().GetCounter());
        }
    }

    // Respawn creatures for a zone
    void RespawnCreaturesForZone(ObjectGuid::LowType zoneGuid)
    {
        QueryResult result = CharacterDatabase.Query(
            "SELECT creature_guid, spawn_id, map_id, x, y, z, o FROM conquest_zone_control_despawned_creatures WHERE zone_guid = {}",
            zoneGuid
        );

        if (!result)
            return;

        do
        {
            Field* fields = result->Fetch();
            /*ObjectGuid::LowType creatureGuid = */fields[0].Get<uint64>();
            ObjectGuid::LowType spawnId = fields[1].Get<uint64>();
            uint32 mapId = fields[2].Get<uint32>();
            /*float x = */fields[3].Get<float>();
            /*float y = */fields[4].Get<float>();
            /*float z = */fields[5].Get<float>();
            /*float o = */fields[6].Get<float>();

            Map* map = sMapMgr->FindBaseMap(mapId);
            if (!map)
                continue;

            // Try to respawn the creature
            CreatureData const* data = sObjectMgr->GetCreatureData(spawnId);
            if (data)
            {
                Creature* creature = new Creature();
                if (creature->LoadCreatureFromDB(spawnId, map, true, true))
                {
                    LOG_INFO("module", "ConquestZoneControl: Respawned creature {} for zone {}", spawnId, zoneGuid);
                }
                else
                {
                    delete creature;
                }
            }
        } while (result->NextRow());

        // Clean up despawned creatures table
        CharacterDatabase.Execute("DELETE FROM conquest_zone_control_despawned_creatures WHERE zone_guid = {}", zoneGuid);
    }

    // Capture zone
    void CaptureZone(Player* player, GameObject* banner)
    {
        if (!player || !banner)
            return;

        Guild* guild = player->GetGuild();
        if (!guild)
        {
            ChatHandler(player->GetSession()).SendSysMessage("Vous devez etre dans une guilde pour capturer une zone.");
            return;
        }

        uint8 playerFaction = player->GetTeamId() == TEAM_ALLIANCE ? 1 : 2;
        ObjectGuid::LowType bannerGuid = banner->GetGUID().GetCounter();

        // Check if zone is already controlled
        QueryResult result = CharacterDatabase.Query(
            "SELECT guild_id, faction, original_entry FROM conquest_zone_control WHERE zone_guid = {}",
            bannerGuid
        );

        uint32 currentGuildId = 0;
        uint8 currentFaction = 0;
        uint32 originalEntry = 0;
        if (result)
        {
            Field* fields = result->Fetch();
            currentGuildId = fields[0].Get<uint32>();
            currentFaction = fields[1].Get<uint8>();
            originalEntry = fields[2].Get<uint32>();
        }

        // If zone is controlled by another guild, make it disputed first
        if (currentGuildId > 0 && currentGuildId != guild->GetId())
        {
            // Despawn fire GameObject if it exists (from ongoing capture)
            auto progressIt = s_captureProgress.find(banner->GetGUID());
            if (progressIt != s_captureProgress.end() && progressIt->second.fireGobGuid)
            {
                DespawnFireGameObject(progressIt->second.fireGobGuid, banner);
                progressIt->second.fireGobGuid.Clear();
            }
            
            // Revert banner entry/display to original disputed version
            // Use the original_entry stored in database, or fallback to current faction if not set
            uint32 disputedEntry = originalEntry;
            if (!disputedEntry || (disputedEntry != BANNER_DISPUTED_ALLIANCE && disputedEntry != BANNER_DISPUTED_HORDE))
            {
                // Fallback: determine based on current faction (for backward compatibility)
                disputedEntry = (currentFaction == 1) ? BANNER_DISPUTED_ALLIANCE : BANNER_DISPUTED_HORDE;
                LOG_INFO("module", "ConquestZoneControl: No original_entry found, using fallback: {}", disputedEntry);
            }
            GameObjectTemplate const* goInfo = sObjectMgr->GetGameObjectTemplate(disputedEntry);
            if (!goInfo)
            {
                LOG_ERROR("module", "ConquestZoneControl: GameObject template not found for disputed entry {}", disputedEntry);
                ChatHandler(player->GetSession()).SendSysMessage("Erreur: Banniere invalide.");
                return;
            }
            
            // Save old banner information
            Map* bannerMap = banner->GetMap();
            Position bannerPos = banner->GetPosition();
            float bannerO = banner->GetOrientation();
            ObjectGuid oldBannerGuid = banner->GetGUID();
            G3D::Quat rotation = banner->GetWorldRotation();
            
            LOG_INFO("module", "ConquestZoneControl: Creating disputed banner with entry {} (displayId: {})", 
                disputedEntry, goInfo->displayId);
            
            // Create a new GameObject with the disputed entry/displayId at the same position
            GameObject* newBanner = bannerMap->SummonGameObject(
                disputedEntry,
                bannerPos.GetPositionX(),
                bannerPos.GetPositionY(),
                bannerPos.GetPositionZ(),
                bannerO,
                rotation.x,
                rotation.y,
                rotation.z,
                rotation.w,
                0, // respawnTime = 0 means permanent
                false // checkTransport
            );
            
            if (!newBanner)
            {
                LOG_ERROR("module", "ConquestZoneControl: Failed to create new disputed banner GameObject with entry {}", disputedEntry);
                ChatHandler(player->GetSession()).SendSysMessage("Erreur: Impossible de creer la nouvelle banniere.");
                return;
            }
            
            // Verify and fix displayId if needed
            if (newBanner->GetDisplayId() != goInfo->displayId)
            {
                LOG_WARN("module", "ConquestZoneControl: Disputed banner displayId mismatch! Expected: {}, Got: {}. Fixing...", 
                    goInfo->displayId, newBanner->GetDisplayId());
                newBanner->SetDisplayId(goInfo->displayId);
                newBanner->UpdateObjectVisibility(true);
            }
            
            LOG_INFO("module", "ConquestZoneControl: Created disputed banner {} with entry {} (displayId: {})", 
                newBanner->GetGUID().GetCounter(), disputedEntry, newBanner->GetDisplayId());
            RegisterBannerInfo(newBanner);
            
            // Update database: remove old entry (new banner will be treated as disputed, no DB entry)
            CharacterDatabase.Execute("DELETE FROM conquest_zone_control WHERE zone_guid = {}", bannerGuid);
            
            // Update despawned creatures table with new GUID
            ObjectGuid::LowType newBannerGuid = newBanner->GetGUID().GetCounter();
            CharacterDatabase.Execute(
                "UPDATE conquest_zone_control_despawned_creatures SET zone_guid = {} WHERE zone_guid = {}",
                newBannerGuid,
                bannerGuid
            );
            
            // Delete the old banner
            banner->SetRespawnTime(0);
            banner->Delete();
            
            // Stop rewards for this zone
            s_rewardTimers.erase(oldBannerGuid);
            
            // Clean up any capture progress
            s_captureProgress.erase(oldBannerGuid);
            
            // Respawn creatures
            RespawnCreaturesForZone(newBannerGuid);
            
            LOG_INFO("module", "ConquestZoneControl: Created new disputed banner {} (entry: {}) and deleted old controlled banner {}", 
                newBannerGuid, disputedEntry, bannerGuid);
            
            ChatHandler(player->GetSession()).SendSysMessage("La zone est maintenant disputee. Cliquez a nouveau pour la capturer.");
            LOG_INFO("module", "ConquestZoneControl: Zone {} contested by guild {}", newBannerGuid, guild->GetId());
            return;
        }

        // If zone is already controlled by this guild, do nothing
        if (currentGuildId == guild->GetId())
        {
            ChatHandler(player->GetSession()).SendSysMessage("Votre guilde controle deja cette zone.");
            return;
        }

        uint32 controlledBannerEntry = (playerFaction == 1) ? BANNER_ALLIANCE : BANNER_HORDE;
        GameObjectTemplate const* goInfo = sObjectMgr->GetGameObjectTemplate(controlledBannerEntry);
        if (!goInfo)
        {
            LOG_ERROR("module", "ConquestZoneControl: GameObject template not found for entry {}", controlledBannerEntry);
            ChatHandler(player->GetSession()).SendSysMessage("Erreur: Banniere invalide.");
            return;
        }

        // Verify banner is still valid and in world
        if (!banner->IsInWorld() || !banner->isSpawned())
        {
            LOG_ERROR("module", "ConquestZoneControl: Banner {} is not in world or not spawned", bannerGuid);
            return;
        }

        // Save old banner information
        uint32 oldEntry = banner->GetEntry();
        uint32 oldDisplayId = banner->GetDisplayId();
        Map* bannerMap = banner->GetMap();
        Position bannerPos = banner->GetPosition();
        float bannerO = banner->GetOrientation();
        ObjectGuid oldBannerGuid = banner->GetGUID();
        G3D::Quat rotation = banner->GetWorldRotation();
        
        LOG_INFO("module", "ConquestZoneControl: Changing banner {} from entry {} (displayId: {}) to entry {} (displayId: {}), playerFaction={}", 
            bannerGuid, oldEntry, oldDisplayId, controlledBannerEntry, goInfo->displayId, playerFaction);
        
        // Create a new GameObject with the new entry/displayId at the same position
        GameObject* newBanner = bannerMap->SummonGameObject(
            controlledBannerEntry,
            bannerPos.GetPositionX(),
            bannerPos.GetPositionY(),
            bannerPos.GetPositionZ(),
            bannerO,
            rotation.x,
            rotation.y,
            rotation.z,
            rotation.w,
            0, // respawnTime = 0 means permanent
            false // checkTransport
        );
        
        if (!newBanner)
        {
            LOG_ERROR("module", "ConquestZoneControl: Failed to create new banner GameObject with entry {}", controlledBannerEntry);
            ChatHandler(player->GetSession()).SendSysMessage("Erreur: Impossible de creer la nouvelle banniere.");
            return;
        }
        
        // Verify and fix displayId if needed
        if (newBanner->GetDisplayId() != goInfo->displayId)
        {
            LOG_WARN("module", "ConquestZoneControl: Controlled banner displayId mismatch! Expected: {}, Got: {}. Fixing...", 
                goInfo->displayId, newBanner->GetDisplayId());
            newBanner->SetDisplayId(goInfo->displayId);
            newBanner->UpdateObjectVisibility(true);
        }
        RegisterBannerInfo(newBanner);
        
        LOG_INFO("module", "ConquestZoneControl: Created controlled banner {} with entry {} (displayId: {})", 
            newBanner->GetGUID().GetCounter(), controlledBannerEntry, newBanner->GetDisplayId());
        
        // Verify the displayId is correct
        if (newBanner->GetDisplayId() != goInfo->displayId)
        {
            LOG_WARN("module", "ConquestZoneControl: Banner displayId mismatch! Expected: {}, Got: {}", 
                goInfo->displayId, newBanner->GetDisplayId());
            newBanner->SetDisplayId(goInfo->displayId);
            newBanner->UpdateObjectVisibility(true);
        }
        
        // Determine original disputed entry (if this is a disputed banner being captured, use its entry)
        uint32 originalDisputedEntry = 0;
        uint32 currentBannerEntry = banner->GetEntry();
        if (currentBannerEntry == BANNER_DISPUTED_ALLIANCE || currentBannerEntry == BANNER_DISPUTED_HORDE)
        {
            // This is a disputed banner being captured - store its entry as original
            originalDisputedEntry = currentBannerEntry;
        }
        else
        {
            // This is a controlled banner being recaptured - get original from database
            QueryResult originalResult = CharacterDatabase.Query(
                "SELECT original_entry FROM conquest_zone_control WHERE zone_guid = {}",
                bannerGuid
            );
            if (originalResult)
            {
                originalDisputedEntry = originalResult->Fetch()[0].Get<uint32>();
            }
        }
        
        // Update database with new banner GUID
        ObjectGuid::LowType newBannerGuid = newBanner->GetGUID().GetCounter();
        
        // Delete old zone control entry and create new one with new GUID
        // Always initialize last_reward_time to current time for a fresh capture
        // This ensures the first reward comes after the full interval
        CharacterDatabase.Execute(
            "DELETE FROM conquest_zone_control WHERE zone_guid = {}",
            bannerGuid
        );
        
        CharacterDatabase.Execute(
            "REPLACE INTO conquest_zone_control (zone_guid, guild_id, faction, original_entry, captured_at, last_reward_time) VALUES ({}, {}, {}, {}, {}, {})",
            newBannerGuid,
            guild->GetId(),
            playerFaction,
            originalDisputedEntry,
            static_cast<uint32>(GameTime::GetGameTime().count()),
            static_cast<uint32>(GameTime::GetGameTime().count())
        );
        
        // Update despawned creatures table with new GUID
        CharacterDatabase.Execute(
            "UPDATE conquest_zone_control_despawned_creatures SET zone_guid = {} WHERE zone_guid = {}",
            newBannerGuid,
            bannerGuid
        );
        
        // Delete the old banner
        banner->SetRespawnTime(0);
        banner->Delete();
        
        LOG_INFO("module", "ConquestZoneControl: Created new banner {} (entry: {}) and deleted old banner {}", 
            newBannerGuid, controlledBannerEntry, bannerGuid);
        
        // Update banner reference for any ongoing captures
        auto oldProgressIt = s_captureProgress.find(oldBannerGuid);
        if (oldProgressIt != s_captureProgress.end())
        {
            // Update the banner GUID in the capture progress
            CaptureProgress progress = oldProgressIt->second;
            s_captureProgress.erase(oldProgressIt);
            progress.bannerGuid = newBanner->GetGUID();
            s_captureProgress[newBanner->GetGUID()] = progress;
        }
        
        // Use newBanner for subsequent operations
        banner = newBanner;
        bannerGuid = newBannerGuid;

        // Despawn fire GameObject
        auto progressIt = s_captureProgress.find(banner->GetGUID());
        if (progressIt != s_captureProgress.end() && progressIt->second.fireGobGuid)
        {
            DespawnFireGameObject(progressIt->second.fireGobGuid, banner);
            progressIt->second.fireGobGuid.Clear();
        }

        // Despawn creatures in radius
        DespawnCreaturesInRadius(banner, DESPAWN_RADIUS);

        // Initialize reward timer
        s_rewardTimers[banner->GetGUID()] = static_cast<uint32>(GameTime::GetGameTime().count());

        ChatHandler(player->GetSession()).PSendSysMessage("Zone capturee par votre guilde!");
        LOG_INFO("module", "ConquestZoneControl: Zone {} captured by guild {} (faction {})", bannerGuid, guild->GetId(), playerFaction);

        // Clean up capture progress (if not already cleaned up)
        if (progressIt != s_captureProgress.end())
        {
            s_captureProgress.erase(progressIt);
        }
    }

    // Helper function to give items to all online guild members
    void GiveItemsToGuildMembers(Guild* guild, uint32 itemEntry, uint32 count)
    {
        if (!guild || count == 0)
            return;

        uint32 membersGiven = 0;
        uint32 totalItemsGiven = 0;

        // Get all guild member GUIDs from database
        QueryResult result = CharacterDatabase.Query(
            "SELECT guid FROM guild_member WHERE guildid = {}",
            guild->GetId()
        );

        if (!result)
        {
            LOG_WARN("module", "ConquestZoneControl: No members found for guild {}", guild->GetId());
            return;
        }

        do
        {
            Field* fields = result->Fetch();
            ObjectGuid::LowType guidLow = fields[0].Get<uint32>();
            ObjectGuid playerGuid = ObjectGuid::Create<HighGuid::Player>(guidLow);

            // Find player if online
            Player* player = ObjectAccessor::FindConnectedPlayer(playerGuid);
            if (!player || !player->IsInWorld())
                continue;

            // Skip dead players - they should not receive rewards
            if (!player->IsAlive())
            {
                LOG_DEBUG("module", "ConquestZoneControl: Skipping dead player {} for rewards", player->GetName());
                continue;
            }

            // Try to add items to player's inventory
            uint32 remainingCount = count;
            
            // Check if player has space in inventory
            uint32 noSpaceForCount = 0;
            ItemPosCountVec dest;
            InventoryResult msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemEntry, remainingCount, &noSpaceForCount);
            
            if (msg != EQUIP_ERR_OK)
            {
                // Player doesn't have space for all items, try to give what we can
                if (noSpaceForCount > 0)
                {
                    remainingCount = noSpaceForCount;
                    // Re-check with the reduced count
                    dest.clear();
                    msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemEntry, remainingCount, &noSpaceForCount);
                }
                else
                {
                    // No space at all, skip this player
                    LOG_DEBUG("module", "ConquestZoneControl: Player {} has no space for item {}", player->GetName(), itemEntry);
                    continue;
                }
            }

            if (msg == EQUIP_ERR_OK && !dest.empty())
            {
                // Create and store the item
                Item* item = player->StoreNewItem(dest, itemEntry, true, 0);
                if (item)
                {
                    player->SendNewItem(item, remainingCount, true, false);
                    totalItemsGiven += remainingCount;
                    membersGiven++;
                    LOG_DEBUG("module", "ConquestZoneControl: Gave {} items (entry: {}) to player {}", 
                        remainingCount, itemEntry, player->GetName());
                }
                else
                {
                    LOG_ERROR("module", "ConquestZoneControl: Failed to create item {} for player {}", itemEntry, player->GetName());
                }
            }
            else
            {
                LOG_WARN("module", "ConquestZoneControl: Cannot store item {} for player {} (msg: {}, dest empty: {})", 
                    itemEntry, player->GetName(), msg, dest.empty());
            }
        } while (result->NextRow());

        if (membersGiven > 0)
        {
            LOG_INFO("module", "ConquestZoneControl: Gave {} items (entry: {}) to {} online guild members (total: {})", 
                count, itemEntry, membersGiven, totalItemsGiven);
        }
        else
        {
            LOG_WARN("module", "ConquestZoneControl: No online guild members to give items {} (entry: {})", count, itemEntry);
        }
    }

    // Process rewards for all controlled zones
    void ProcessRewards()
    {
        if (!IsEnabled())
            return;

        LOG_DEBUG("module", "ConquestZoneControl: ProcessRewards called");

        QueryResult result = CharacterDatabase.Query(
            "SELECT zone_guid, guild_id, last_reward_time FROM conquest_zone_control WHERE guild_id > 0"
        );

        if (!result)
        {
            LOG_DEBUG("module", "ConquestZoneControl: No controlled zones found");
            return;
        }

        uint32 currentTime = static_cast<uint32>(GameTime::GetGameTime().count());
        uint32 zonesProcessed = 0;

        do
        {
            Field* fields = result->Fetch();
            ObjectGuid::LowType zoneGuid = fields[0].Get<uint64>();
            uint32 guildId = fields[1].Get<uint32>();
            uint32 lastRewardTime = fields[2].Get<uint32>();

            uint32 timeSinceLastReward = currentTime - lastRewardTime;
            LOG_DEBUG("module", "ConquestZoneControl: Zone {} (guild {}): time since last reward: {}s (required: {}s)", 
                zoneGuid, guildId, timeSinceLastReward, s_rewardIntervalSeconds);

            // Check if reward interval has passed
            // Also verify that the zone is still controlled by checking if the entry still exists
            // This prevents rewards from being given when a zone is being recaptured
            // IMPORTANT: We only give ONE reward per check, even if multiple intervals have passed
            // This prevents accumulation of rewards
            if (timeSinceLastReward >= s_rewardIntervalSeconds)
            {
                // Double-check that the zone is still controlled (entry still exists in DB)
                QueryResult verifyResult = CharacterDatabase.Query(
                    "SELECT guild_id FROM conquest_zone_control WHERE zone_guid = {} AND guild_id = {}",
                    zoneGuid,
                    guildId
                );
                
                if (!verifyResult)
                {
                    LOG_DEBUG("module", "ConquestZoneControl: Zone {} no longer controlled by guild {}, skipping reward", zoneGuid, guildId);
                    continue; // Zone was recaptured or deleted, skip reward
                }
                
                Guild* guild = sGuildMgr->GetGuildById(guildId);
                if (guild)
                {
                    LOG_INFO("module", "ConquestZoneControl: Processing rewards for zone {} (guild {})", zoneGuid, guildId);
                    
                    // Give items to all online guild members
                    GiveItemsToGuildMembers(guild, REWARD_GOLD_ITEM_ENTRY, s_rewardGoldAmount);
                    GiveItemsToGuildMembers(guild, REWARD_WOOD_ITEM_ENTRY, s_rewardWoodAmount);

                    // Update last reward time to current time (not lastRewardTime + interval)
                    // This ensures that if multiple intervals passed, we only give one reward
                    // and the next reward will be given after the next interval
                    CharacterDatabase.Execute(
                        "UPDATE conquest_zone_control SET last_reward_time = {} WHERE zone_guid = {}",
                        currentTime,
                        zoneGuid
                    );

                    LOG_INFO("module", "ConquestZoneControl: Rewarded {} Or Conquest and {} Bois Conquest to online members of guild {} for zone {}", 
                        s_rewardGoldAmount, s_rewardWoodAmount, guildId, zoneGuid);
                    zonesProcessed++;
                }
                else
                {
                    LOG_WARN("module", "ConquestZoneControl: Guild {} not found for zone {}", guildId, zoneGuid);
                }
            }
        } while (result->NextRow());
        
        if (zonesProcessed > 0)
        {
            LOG_INFO("module", "ConquestZoneControl: Processed rewards for {} zones", zonesProcessed);
        }
    }
}

// GameObject Script for banners
class ConquestZoneControlBanner : public GameObjectScript
{
public:
    ConquestZoneControlBanner() : GameObjectScript("ConquestZoneControlBanner") { }

    bool OnGossipHello(Player* player, GameObject* go) override
    {
        if (!IsEnabled())
            return false;

        if (player->IsGameMaster())
            return false;

        // Log entry for debugging
        uint32 currentEntry = go->GetEntry();
        auto captureIt = s_captureProgress.find(go->GetGUID());
        bool hasCaptureProgress = (captureIt != s_captureProgress.end());
        LOG_INFO("module", "ConquestZoneControl: OnGossipHello called for banner {} (entry: {}, displayId: {}, ScriptId: {}) by player {}. Has capture progress: {}", 
            go->GetGUID().GetCounter(), currentEntry, go->GetDisplayId(), go->GetScriptId(), player->GetName(), hasCaptureProgress);
        
        if (hasCaptureProgress)
        {
            LOG_INFO("module", "ConquestZoneControl: Capture progress details - isCapturing: {}, playerGuid: {}, fireGobGuid: {}", 
                captureIt->second.isCapturing, captureIt->second.playerGuid.GetCounter(), captureIt->second.fireGobGuid.GetCounter());
        }
        
        // Check if this is one of our banner entries (400010-400013 all have the same ScriptName)
        // We need to check the entry directly because m_goInfo might not be updated after SetEntry()
        if (currentEntry != BANNER_DISPUTED_HORDE && 
            currentEntry != BANNER_DISPUTED_ALLIANCE &&
            currentEntry != BANNER_HORDE && 
            currentEntry != BANNER_ALLIANCE)
        {
            // Not one of our banners, let default behavior handle it
            LOG_INFO("module", "ConquestZoneControl: Banner {} has entry {}, not one of our banner entries, skipping", 
                go->GetGUID().GetCounter(), currentEntry);
            return false;
        }

        // Check if player is in a guild
        Guild* guild = player->GetGuild();
        if (!guild)
        {
            ChatHandler(player->GetSession()).SendSysMessage("Vous devez etre dans une guilde pour capturer une zone.");
            return true;
        }

        // Check current owner
        QueryResult result = CharacterDatabase.Query(
            "SELECT guild_id FROM conquest_zone_control WHERE zone_guid = {}",
            go->GetGUID().GetCounter()
        );

        if (result)
        {
            uint32 controllingGuildId = result->Fetch()[0].Get<uint32>();
            
            // If player's guild owns the banner, allow them to cancel but prevent starting a new capture
            if (controllingGuildId == guild->GetId())
            {
                auto it = s_captureProgress.find(go->GetGUID());
                if (it != s_captureProgress.end() && it->second.isCapturing)
                {
                    // Cancel the enemy capture
                    Player* enemyPlayer = ObjectAccessor::FindPlayer(it->second.playerGuid);
                    if (enemyPlayer && enemyPlayer->IsInWorld())
                    {
                        enemyPlayer->InterruptSpell(CURRENT_CHANNELED_SPELL);
                        ChatHandler(enemyPlayer->GetSession()).SendSysMessage("Votre capture a ete annulee par la guilde proprietaire!");
                    }
                    
                    DespawnFireGameObject(it->second.fireGobGuid, go);
                    s_captureProgress.erase(it);
                    ChatHandler(player->GetSession()).SendSysMessage("Capture ennemie annulee!");
                    LOG_INFO("module", "ConquestZoneControl: Player {} from owning guild {} cancelled capture of zone {}", 
                        player->GetName(), guild->GetId(), go->GetGUID().GetCounter());
                    return true;
                }
                else
                {
                    ChatHandler(player->GetSession()).SendSysMessage("Votre guilde controle deja cette zone.");
                    return true;
                }
            }
            // If another guild owns it, allow channeling (handled by spell script)
            // Continue to allow the click - the spell script will handle the capture
            LOG_INFO("module", "ConquestZoneControl: Player {} from guild {} clicking controlled banner owned by guild {}", 
                player->GetName(), guild->GetId(), controllingGuildId);
        }
        else
        {
            // No owner in database - this is a disputed banner, allow capture
            LOG_INFO("module", "ConquestZoneControl: Player {} from guild {} clicking disputed banner", 
                player->GetName(), guild->GetId());
        }

        // Check if already capturing - clean up any invalid entries first
        auto it = s_captureProgress.find(go->GetGUID());
        if (it != s_captureProgress.end() && it->second.isCapturing)
        {
            // If the clicking player is the same as the capturing player, allow them to restart
            // This handles the case where they left the area and want to try again
            if (it->second.playerGuid == player->GetGUID())
            {
                // Same player clicking again - check if they're in valid conditions now
                if (player->GetDistance(go) <= 10.0f && !player->IsInCombat() && player->IsAlive())
                {
                    // Player is back in range and conditions are met - allow restart
                    // Clean up the old capture progress first
                    if (it->second.fireGobGuid)
                    {
                        DespawnFireGameObject(it->second.fireGobGuid, go);
                    }
                    // Interrupt any ongoing channeling
                    player->InterruptSpell(CURRENT_CHANNELED_SPELL);
                    s_captureProgress.erase(it);
                    LOG_INFO("module", "ConquestZoneControl: Player {} restarted capture on banner {} (cleaned up previous attempt)", 
                        player->GetName(), go->GetGUID().GetCounter());
                    // Continue to start new capture below
                }
                else
                {
                    // Player is still out of range or conditions not met
                    ChatHandler(player->GetSession()).SendSysMessage("Vous devez etre a proximite de la banniere et hors combat pour capturer.");
                    return true;
                }
            }
            else
            {
                // Different player - verify the capturing player is still valid and in range
                Player* capturingPlayer = ObjectAccessor::FindPlayer(it->second.playerGuid);
                bool shouldCleanup = false;
                
                if (!capturingPlayer || !capturingPlayer->IsInWorld())
                {
                    // Capturing player is no longer valid - clean up
                    shouldCleanup = true;
                    LOG_INFO("module", "ConquestZoneControl: Capturing player not found or not in world, cleaning up");
                }
                else if (capturingPlayer->GetDistance(go) > 10.0f || capturingPlayer->IsInCombat() || !capturingPlayer->IsAlive())
                {
                    // Capturing player is out of range, in combat, or dead - clean up
                    shouldCleanup = true;
                    LOG_INFO("module", "ConquestZoneControl: Capturing player conditions not met (distance={}, inCombat={}, alive={}), cleaning up", 
                        capturingPlayer->GetDistance(go), capturingPlayer->IsInCombat(), capturingPlayer->IsAlive());
                }
                
                if (shouldCleanup)
                {
                    // Clean up fire GameObject if it exists
                    if (it->second.fireGobGuid)
                    {
                        DespawnFireGameObject(it->second.fireGobGuid, go);
                    }
                    // Interrupt any channeling
                    if (capturingPlayer && capturingPlayer->IsAlive() && capturingPlayer->IsInWorld())
                    {
                        capturingPlayer->InterruptSpell(CURRENT_CHANNELED_SPELL);
                    }
                    s_captureProgress.erase(it);
                    LOG_INFO("module", "ConquestZoneControl: Cleaned up invalid capture progress for banner {}. Remaining entries: {}", 
                        go->GetGUID().GetCounter(), s_captureProgress.size());
                    // Continue to allow new capture
                }
                else
                {
                    // Valid capture in progress by another player
                    ChatHandler(player->GetSession()).SendSysMessage("Quelqu'un capture deja cette zone.");
                    return true;
                }
            }
        }
        
        // Ensure GameObject is in a clickable state
        // Remove any flags that might block interactions
        GameObjectFlags blockingFlags = GO_FLAG_IN_USE | GO_FLAG_NOT_SELECTABLE | GO_FLAG_INTERACT_COND;
        if (go->HasGameObjectFlag(blockingFlags))
        {
            go->RemoveGameObjectFlag(blockingFlags);
            LOG_INFO("module", "ConquestZoneControl: Removed blocking flags (IN_USE, NOT_SELECTABLE, INTERACT_COND) from banner {}", go->GetGUID().GetCounter());
        }
        
        // Reset loot state if needed
        if (go->getLootState() != GO_READY)
        {
            go->SetLootState(GO_READY);
            LOG_INFO("module", "ConquestZoneControl: Reset loot state to GO_READY for banner {}", go->GetGUID().GetCounter());
        }
        
        // Reset door/button state if needed
        if (go->GetGoState() != GO_STATE_READY)
        {
            go->SetGoState(GO_STATE_READY);
            LOG_INFO("module", "ConquestZoneControl: Reset GO state to GO_STATE_READY for banner {}", go->GetGUID().GetCounter());
        }
        
        // Reset door/button to ensure it's in a usable state
        go->ResetDoorOrButton();
        
        // Force visual update to ensure client sees the GameObject as clickable
        go->UpdateObjectVisibility(true);
        
        LOG_INFO("module", "ConquestZoneControl: Banner {} reset and ready for interaction", go->GetGUID().GetCounter());

        // Start capture tracking - the channeling will happen naturally, we'll detect when it finishes
        CaptureProgress progress;
        progress.playerGuid = player->GetGUID();
        progress.bannerGuid = go->GetGUID();
        progress.startTime = GameTime::GetGameTimeMS().count();
        progress.isCapturing = true;
        progress.fireGobGuid.Clear();
        progress.captureId = ++s_captureIdCounter; // Assign unique ID
        s_captureProgress[go->GetGUID()] = progress;

        LOG_INFO("module", "ConquestZoneControl: Player {} started capturing zone {} - waiting for channeling to complete", 
            player->GetName(), go->GetGUID().GetCounter());

        return false; // Let the default channeling happen
    }
};

// Player Script for rewards and cleanup
class ConquestZoneControlPlayer : public PlayerScript
{
public:
    ConquestZoneControlPlayer() : PlayerScript("ConquestZoneControlPlayer") { }

    void OnPlayerJustDied(Player* player) override
    {
        if (!IsEnabled())
            return;

        // Cancel any ongoing capture when player dies
        for (auto it = s_captureProgress.begin(); it != s_captureProgress.end();)
        {
            if (it->second.playerGuid == player->GetGUID() && it->second.isCapturing)
            {
                // Clean up fire GameObject if it exists
                if (it->second.fireGobGuid)
                {
                    GameObject* banner = ObjectAccessor::GetGameObject(*player, it->second.bannerGuid);
                    if (banner)
                        DespawnFireGameObject(it->second.fireGobGuid, banner);
                    else
                        DespawnFireGameObject(it->second.fireGobGuid, player);
                }
                
                // Interrupt any channeling
                if (player->IsInWorld())
                {
                    player->InterruptSpell(CURRENT_CHANNELED_SPELL);
                }
                
                LOG_INFO("module", "ConquestZoneControl: Player {} died, cancelling capture", player->GetName());
                it = s_captureProgress.erase(it);
                break; // Only one capture per player
            }
            else
            {
                ++it;
            }
        }
    }

    void OnPlayerUpdate(Player* /*player*/, uint32 /*diff*/) override
    {
        if (!IsEnabled())
            return;

        // Process rewards based on configured interval
        // Use timestamp-based approach to ensure rewards are processed exactly once per interval
        // This prevents multiple calls when multiple players are online
        uint32 currentTime = static_cast<uint32>(GameTime::GetGameTime().count());
        
        // Check if enough time has passed since last reward processing
        if (s_lastRewardProcessTime == 0 || (currentTime - s_lastRewardProcessTime) >= s_rewardIntervalSeconds)
        {
            s_lastRewardProcessTime = currentTime;
            ProcessRewards();
        }

        // Check for interrupted captures (cleanup if channeling stopped or during capture phase)
        for (auto it = s_captureProgress.begin(); it != s_captureProgress.end();)
        {
            if (it->second.isCapturing)
            {
                Player* capturingPlayer = ObjectAccessor::FindPlayer(it->second.playerGuid);
                if (!capturingPlayer || !capturingPlayer->IsInWorld())
                {
                // Cancel capture if player not found or not in world
                GameObject* banner = capturingPlayer ? ObjectAccessor::GetGameObject(*capturingPlayer, it->second.bannerGuid) : nullptr;
                if (banner)
                {
                    DespawnFireGameObject(it->second.fireGobGuid, banner);
                    ResetBannerGameObject(banner);
                }
                else if (capturingPlayer)
                {
                    DespawnFireGameObject(it->second.fireGobGuid, capturingPlayer);
                }
                it = s_captureProgress.erase(it);
                continue;
                }
                
                GameObject* banner = ObjectAccessor::GetGameObject(*capturingPlayer, it->second.bannerGuid);
                
                // If fire GameObject is spawned, we're in capture phase (after channeling)
                // Otherwise, we're still in channeling phase
                if (it->second.fireGobGuid)
                {
                    // Capture phase: check distance and combat
                    if (!banner || 
                        capturingPlayer->GetDistance(banner) > 10.0f ||
                        capturingPlayer->IsInCombat() ||
                        !capturingPlayer->IsAlive())
                    {
                        // Cancel capture during capture phase
                        LOG_INFO("module", "ConquestZoneControl: Cancelling capture phase for player {} - despawn fire GameObject {}", 
                            capturingPlayer ? capturingPlayer->GetName() : "Unknown", it->second.fireGobGuid.GetCounter());
                        DespawnFireGameObject(it->second.fireGobGuid, banner);
                        
                        // Reset banner to fully clickable state
                        if (banner)
                        {
                            ResetBannerGameObject(banner);
                        }
                        
                        if (capturingPlayer && capturingPlayer->IsAlive())
                            ChatHandler(capturingPlayer->GetSession()).SendSysMessage("Capture interrompue.");
                        LOG_INFO("module", "ConquestZoneControl: Capture phase interrupted for player {} (distance={}, inCombat={}, alive={}). Erasing capture progress.", 
                            capturingPlayer ? capturingPlayer->GetName() : "Unknown", 
                            capturingPlayer ? capturingPlayer->GetDistance(banner) : 0.0f, 
                            capturingPlayer ? capturingPlayer->IsInCombat() : false,
                            capturingPlayer ? capturingPlayer->IsAlive() : false);
                        ObjectGuid bannerGuid = it->second.bannerGuid;
                        it = s_captureProgress.erase(it);
                        LOG_INFO("module", "ConquestZoneControl: Capture progress erased for banner {}. Remaining entries in s_captureProgress: {}", 
                            bannerGuid.GetCounter(), s_captureProgress.size());
                        continue;
                    }
                }
                else
                {
                    // Channeling phase: check if still channeling and conditions are met
                    Spell* currentSpell = capturingPlayer->GetCurrentSpell(CURRENT_CHANNELED_SPELL);
                    
                    // If not channeling anymore, let ConquestZoneControlChannelFinish handle it
                    if (!currentSpell)
                    {
                        ++it;
                        continue;
                    }
                    
                    // Still channeling - check conditions
                    if (!banner || 
                        capturingPlayer->GetDistance(banner) > 10.0f ||
                        capturingPlayer->IsInCombat() ||
                        !capturingPlayer->IsAlive())
                    {
                        // Conditions not met - interrupt channeling and clean up
                        LOG_INFO("module", "ConquestZoneControl: Cancelling channeling for player {} - interrupt spell and clean up", 
                            capturingPlayer->GetName());
                        if (capturingPlayer->IsAlive())
                            capturingPlayer->InterruptSpell(CURRENT_CHANNELED_SPELL);
                        
                        // Clean up fire GameObject if it exists
                        if (it->second.fireGobGuid)
                        {
                            DespawnFireGameObject(it->second.fireGobGuid, banner);
                        }
                        
                        // Reset banner to fully clickable state
                        if (banner)
                        {
                            ResetBannerGameObject(banner);
                        }
                        
                        if (capturingPlayer)
                            ChatHandler(capturingPlayer->GetSession()).SendSysMessage("Channeling interrompu.");
                        ObjectGuid bannerGuid = it->second.bannerGuid;
                        LOG_INFO("module", "ConquestZoneControl: Channeling interrupted for player {} (distance={}, inCombat={}, alive={}). Erasing capture progress.", 
                            capturingPlayer->GetName(), capturingPlayer->GetDistance(banner), capturingPlayer->IsInCombat(), capturingPlayer->IsAlive());
                        it = s_captureProgress.erase(it);
                        LOG_INFO("module", "ConquestZoneControl: Capture progress erased for banner {}. Remaining entries in s_captureProgress: {}", 
                            bannerGuid.GetCounter(), s_captureProgress.size());
                        continue;
                    }
                }
            }
            ++it;
        }
    }
};

// AllSpellScript to detect when channeling starts and finishes on banners
class ConquestZoneControlSpell : public AllSpellScript
{
public:
    ConquestZoneControlSpell() : AllSpellScript("ConquestZoneControlSpell") { }

    void OnSpellCast(Spell* spell, Unit* caster, SpellInfo const* spellInfo, bool /*skipCheck*/) override
    {
        if (!IsEnabled())
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        // Check if spell target is a banner GameObject
        GameObject* banner = nullptr;
        if (spell->m_targets.GetGOTarget())
        {
            banner = spell->m_targets.GetGOTarget();
        }

        if (!banner)
            return;

        uint32 bannerEntry = banner->GetEntry();
        
        // Only handle our banner entries
        if (bannerEntry != BANNER_DISPUTED_HORDE && 
            bannerEntry != BANNER_DISPUTED_ALLIANCE)
            return;

        // Only handle channeling spells
        if (!spellInfo->IsChanneled())
            return;
        
        LOG_INFO("module", "ConquestZoneControl: Player {} started channeling spell {} on banner {} (entry {})", 
            player->GetName(), spellInfo->Id, banner->GetGUID().GetCounter(), bannerEntry);

        Guild* guild = player->GetGuild();
        if (!guild)
            return;

        // Prevent owners from starting a new capture
        QueryResult result = CharacterDatabase.Query(
            "SELECT guild_id FROM conquest_zone_control WHERE zone_guid = {}",
            banner->GetGUID().GetCounter()
        );

        if (result)
        {
            uint32 controllingGuildId = result->Fetch()[0].Get<uint32>();
            if (controllingGuildId == guild->GetId())
                return;
        }

        // Start capture tracking (but don't spawn fire GameObject yet)
        CaptureProgress progress;
        progress.playerGuid = player->GetGUID();
        progress.bannerGuid = banner->GetGUID();
        progress.startTime = GameTime::GetGameTimeMS().count();
        progress.isCapturing = true;
        progress.fireGobGuid.Clear(); // Fire GameObject will be spawned after channeling completes
        progress.captureId = ++s_captureIdCounter; // Assign unique ID

        s_captureProgress[banner->GetGUID()] = progress;

        // Get spell duration (use configured capture time if duration is 0 or invalid)
        int32 spellDuration = spellInfo->GetDuration();
        if (spellDuration <= 0)
            spellDuration = s_captureTimeMs;
        
        LOG_INFO("module", "ConquestZoneControl: Channeling spell {} duration: {}ms", spellInfo->Id, spellDuration);
        
        // Store capture ID to verify event is still valid when it executes
        uint32 captureId = progress.captureId;
        
        // Schedule: after channeling completes, spawn fire GameObject and start 1 minute capture phase
        player->m_Events.AddEventAtOffset([player, banner, captureId]()
        {
            auto it = s_captureProgress.find(banner->GetGUID());
            if (it == s_captureProgress.end() || !it->second.isCapturing)
            {
                LOG_INFO("module", "ConquestZoneControl: Capture progress not found or not capturing");
                return;
            }

            // Verify this is still the same capture attempt
            if (it->second.captureId != captureId)
            {
                LOG_INFO("module", "ConquestZoneControl: Capture ID mismatch (old: {}, new: {}), event cancelled", 
                    captureId, it->second.captureId);
                return;
            }

            if (it->second.playerGuid != player->GetGUID())
            {
                LOG_INFO("module", "ConquestZoneControl: Player GUID mismatch");
                return;
            }

            // Check if player is still near and not in combat
            if (!player->IsInWorld() || player->GetDistance(banner) > 10.0f || player->IsInCombat())
            {
                LOG_INFO("module", "ConquestZoneControl: Player conditions not met for capture");
                // Reset banner before erasing
                ResetBannerGameObject(banner);
                s_captureProgress.erase(it);
                return;
            }

            // Channeling completed - spawn fire GameObject and start 1 minute capture
            LOG_INFO("module", "ConquestZoneControl: Channeling completed, spawning fire GameObject for player {}", player->GetName());
            
            GameObject* fireGob = SpawnFireGameObject(banner);
            if (fireGob)
            {
                it->second.fireGobGuid = fireGob->GetGUID();
                it->second.startTime = GameTime::GetGameTimeMS().count();
                
                ChatHandler(player->GetSession()).SendSysMessage("Capture en cours... (1 minute)");
                
                // Store capture ID for this phase
                uint32 phaseCaptureId = it->second.captureId;
                
                // Schedule capture completion after 1 minute
                player->m_Events.AddEventAtOffset([player, banner, phaseCaptureId]()
                {
                    auto it2 = s_captureProgress.find(banner->GetGUID());
                    if (it2 != s_captureProgress.end())
                    {
                        // Verify this is still the same capture attempt
                        if (it2->second.captureId != phaseCaptureId)
                        {
                            LOG_INFO("module", "ConquestZoneControl: Capture ID mismatch in phase (old: {}, new: {}), event cancelled", 
                                phaseCaptureId, it2->second.captureId);
                            return;
                        }
                        
                        if (it2->second.playerGuid == player->GetGUID() &&
                            it2->second.fireGobGuid &&
                            player->IsInWorld() && 
                            player->GetDistance(banner) < 10.0f &&
                            !player->IsInCombat())
                        {
                            LOG_INFO("module", "ConquestZoneControl: Capture phase completed, capturing zone");
                            // Capture phase completed - capture the zone
                            it2->second.isCapturing = false;
                            CaptureZone(player, banner);
                            s_captureProgress.erase(it2);
                        }
                        else
                        {
                            LOG_INFO("module", "ConquestZoneControl: Capture phase interrupted");
                            // Capture was interrupted
                            DespawnFireGameObject(it2->second.fireGobGuid, banner);
                            ResetBannerGameObject(banner);
                            s_captureProgress.erase(it2);
                        }
                    }
                }, 60s);
            }
            else
            {
                LOG_ERROR("module", "ConquestZoneControl: Failed to spawn fire GameObject");
                s_captureProgress.erase(it);
            }
        }, std::chrono::milliseconds(spellDuration + 500)); // Add 500ms buffer
    }

    void OnSpellCastCancel(Spell* /*spell*/, Unit* caster, SpellInfo const* spellInfo, bool /*bySelf*/) override
    {
        if (!IsEnabled())
            return;

        // Only handle channeling spells
        if (!spellInfo->IsChanneled())
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        // Find banner from capture progress and cancel
        for (auto it = s_captureProgress.begin(); it != s_captureProgress.end();)
        {
            if (it->second.playerGuid == player->GetGUID() && it->second.isCapturing)
            {
                // If fire GameObject was spawned, despawn it
                GameObject* banner = ObjectAccessor::GetGameObject(*player, it->second.bannerGuid);
                if (it->second.fireGobGuid)
                {
                    if (banner)
                        DespawnFireGameObject(it->second.fireGobGuid, banner);
                    else
                        DespawnFireGameObject(it->second.fireGobGuid, player);
                }
                
                // Reset banner to fully clickable state
                if (banner)
                {
                    ResetBannerGameObject(banner);
                }
                
                it = s_captureProgress.erase(it);
                LOG_INFO("module", "ConquestZoneControl: Player {} cancelled channeling on banner", player->GetName());
                break;
            }
            ++it;
        }
    }
};

// Player Script to detect when channeling finishes successfully
class ConquestZoneControlChannelFinish : public PlayerScript
{
public:
    ConquestZoneControlChannelFinish() : PlayerScript("ConquestZoneControlChannelFinish") { }

    void OnPlayerUpdate(Player* player, uint32 diff) override
    {
        if (!IsEnabled())
            return;

        // Check if player finished channeling
        static std::map<ObjectGuid, uint32> lastChannelCheck;
        uint32& lastCheck = lastChannelCheck[player->GetGUID()];
        lastCheck += diff;

        // Check every 500ms
        if (lastCheck < 500)
            return;
        lastCheck = 0;

        // Check if player was channeling but isn't anymore
        for (auto it = s_captureProgress.begin(); it != s_captureProgress.end();)
        {
            if (it->second.playerGuid == player->GetGUID() && it->second.isCapturing && !it->second.fireGobGuid)
            {
                Spell* currentSpell = player->GetCurrentSpell(CURRENT_CHANNELED_SPELL);
                GameObject* banner = ObjectAccessor::GetGameObject(*player, it->second.bannerGuid);
                
                if (!banner || !player->IsInWorld())
                {
                    if (banner)
                        ResetBannerGameObject(banner);
                    it = s_captureProgress.erase(it);
                    continue;
                }
                
                // Check if still channeling
                if (currentSpell)
                {
                    // Still channeling - check conditions
                    if (player->GetDistance(banner) > 10.0f || player->IsInCombat() || !player->IsAlive())
                    {
                        // Conditions not met - interrupt and clean up
                        if (player->IsAlive())
                            player->InterruptSpell(CURRENT_CHANNELED_SPELL);
                        
                        // Clean up fire GameObject if it exists
                        if (it->second.fireGobGuid)
                        {
                            DespawnFireGameObject(it->second.fireGobGuid, banner);
                        }
                        
                        // Reset banner to fully clickable state
                        ResetBannerGameObject(banner);
                        
                        LOG_INFO("module", "ConquestZoneControl: Channeling interrupted for player {} (conditions not met: distance={}, inCombat={}, alive={})", 
                            player->GetName(), player->GetDistance(banner), player->IsInCombat(), player->IsAlive());
                        it = s_captureProgress.erase(it);
                        continue;
                    }
                    // Otherwise, continue waiting for channeling to finish
                }
                else
                {
                    // Channeling finished - spawn fire GameObject and start 1 minute capture phase
                    // Check conditions first
                    if (player->GetDistance(banner) < 10.0f && !player->IsInCombat())
                    {
                        // Only spawn if not already spawned
                        if (!it->second.fireGobGuid)
                        {
                            LOG_INFO("module", "ConquestZoneControl: Player {} finished channeling, spawning fire GameObject and starting 1 minute capture", 
                                player->GetName());
                            
                            GameObject* fireGob = SpawnFireGameObject(banner);
                            if (fireGob)
                            {
                                it->second.fireGobGuid = fireGob->GetGUID();
                                it->second.startTime = GameTime::GetGameTimeMS().count(); // Reset timer for capture phase
                                
                                ChatHandler(player->GetSession()).SendSysMessage("Capture en cours... (1 minute)");
                                
                                // Store capture ID for this phase
                                uint32 phaseCaptureId = it->second.captureId;
                                
                                // Schedule capture completion after 1 minute
                                ObjectGuid bannerGuid = banner->GetGUID();
                                player->m_Events.AddEventAtOffset([player, bannerGuid, phaseCaptureId]()
                                {
                                    // Re-fetch banner from GUID to ensure it's still valid
                                    GameObject* banner = ObjectAccessor::GetGameObject(*player, bannerGuid);
                                    if (!banner || !player->IsInWorld())
                                    {
                                        LOG_INFO("module", "ConquestZoneControl: Banner or player no longer valid, cancelling capture");
                                        auto it2 = s_captureProgress.find(bannerGuid);
                                        if (it2 != s_captureProgress.end())
                                        {
                                            if (it2->second.fireGobGuid)
                                                DespawnFireGameObject(it2->second.fireGobGuid, player);
                                            if (banner)
                                                ResetBannerGameObject(banner);
                                            s_captureProgress.erase(it2);
                                        }
                                        return;
                                    }

                                    auto it2 = s_captureProgress.find(bannerGuid);
                                    if (it2 != s_captureProgress.end())
                                    {
                                        // Verify this is still the same capture attempt
                                        if (it2->second.captureId != phaseCaptureId)
                                        {
                                            LOG_INFO("module", "ConquestZoneControl: Capture ID mismatch in phase (old: {}, new: {}), event cancelled", 
                                                phaseCaptureId, it2->second.captureId);
                                            return;
                                        }
                                        
                                        if (it2->second.playerGuid == player->GetGUID() &&
                                            it2->second.fireGobGuid &&
                                            player->GetDistance(banner) < 10.0f &&
                                            !player->IsInCombat())
                                        {
                                            LOG_INFO("module", "ConquestZoneControl: Capture phase completed, capturing zone");
                                            // Capture phase completed - capture the zone
                                            it2->second.isCapturing = false;
                                            CaptureZone(player, banner);
                                            // Don't erase here - CaptureZone already does it
                                        }
                                        else
                                        {
                                            LOG_INFO("module", "ConquestZoneControl: Capture phase interrupted");
                                            // Capture was interrupted
                                            if (it2->second.fireGobGuid)
                                                DespawnFireGameObject(it2->second.fireGobGuid, banner);
                                            ResetBannerGameObject(banner);
                                            s_captureProgress.erase(it2);
                                        }
                                    }
                                }, 60s);
                            }
                            else
                            {
                                LOG_ERROR("module", "ConquestZoneControl: Failed to spawn fire GameObject");
                                it = s_captureProgress.erase(it);
                                continue;
                            }
                        }
                    }
                    else
                    {
                        // Channeling finished but conditions not met - cancel
                        // Clean up fire GameObject if it was spawned
                        if (it->second.fireGobGuid)
                        {
                            DespawnFireGameObject(it->second.fireGobGuid, banner);
                        }
                        
                        // Reset banner to fully clickable state
                        ResetBannerGameObject(banner);
                        
                        LOG_INFO("module", "ConquestZoneControl: Channeling finished but conditions not met for player {} (distance={}, inCombat={}, alive={})", 
                            player->GetName(), player->GetDistance(banner), player->IsInCombat(), player->IsAlive());
                        it = s_captureProgress.erase(it);
                        continue;
                    }
                }
            }
            ++it;
        }
    }
};

// AllGameObjectScript to intercept all GameObject clicks and handle our banners
// This ensures our banners work even if GetScriptId() returns the wrong value after SetEntry()
class ConquestZoneControlAllBanner : public AllGameObjectScript
{
public:
    ConquestZoneControlAllBanner() : AllGameObjectScript("ConquestZoneControlAllBanner") { }

    void OnGameObjectAddWorld(GameObject* go) override
    {
        if (!IsEnabled())
            return;

        // Check if this is one of our banner entries (400010-400013)
        uint32 entry = go->GetEntry();
        if (entry == BANNER_DISPUTED_HORDE || 
            entry == BANNER_DISPUTED_ALLIANCE ||
            entry == BANNER_HORDE || 
            entry == BANNER_ALLIANCE)
        {
            RegisterBannerInfo(go);
            ObjectGuid::LowType bannerGuid = go->GetGUID().GetCounter();
            
            // Check if this banner is already registered in the database
            QueryResult result = CharacterDatabase.Query(
                "SELECT zone_guid FROM conquest_zone_control WHERE zone_guid = {}",
                bannerGuid
            );
            
            if (!result)
            {
                // Banner not found in database - register it
                // Determine if it's a disputed banner (initial state)
                uint8 faction = 0; // 0 = neutral/disputed
                uint32 guildId = 0; // No guild owns it yet
                uint32 originalEntry = entry; // Store the original entry
                
                // If it's a controlled banner (Horde/Alliance), it means it was already captured
                // In this case, we should check if it's being spawned by the system or manually
                // For now, we'll treat manually spawned controlled banners as disputed
                if (entry == BANNER_HORDE || entry == BANNER_ALLIANCE)
                {
                    // This is a controlled banner spawned manually - treat as disputed
                    originalEntry = (entry == BANNER_HORDE) ? BANNER_DISPUTED_HORDE : BANNER_DISPUTED_ALLIANCE;
                }
                
                uint32 currentTime = static_cast<uint32>(GameTime::GetGameTime().count());
                
                CharacterDatabase.Execute(
                    "INSERT INTO conquest_zone_control (zone_guid, guild_id, faction, captured_at, last_reward_time, original_entry) "
                    "VALUES ({}, {}, {}, {}, {}, {}) "
                    "ON DUPLICATE KEY UPDATE original_entry = VALUES(original_entry)",
                    bannerGuid,
                    guildId,
                    faction,
                    currentTime,
                    currentTime,
                    originalEntry
                );
                
                LOG_INFO("module", "ConquestZoneControl: Automatically registered banner {} (entry: {}) in database", 
                    bannerGuid, entry);
            }
            else
            {
                LOG_DEBUG("module", "ConquestZoneControl: Banner {} (entry: {}) already registered in database", 
                    bannerGuid, entry);
            }
        }
    }

    bool CanGameObjectGossipHello(Player* player, GameObject* go) override
    {
        if (!IsEnabled())
            return false;

        // Check if this is one of our banner entries (400010-400013)
        uint32 entry = go->GetEntry();
        if (entry == BANNER_DISPUTED_HORDE || 
            entry == BANNER_DISPUTED_ALLIANCE ||
            entry == BANNER_HORDE || 
            entry == BANNER_ALLIANCE)
        {
            // This is one of our banners - handle it directly here
            // This bypasses the GetScriptId() issue
            LOG_INFO("module", "ConquestZoneControl: AllGameObjectScript handling click on banner {} (entry: {})", 
                go->GetGUID().GetCounter(), entry);
            
            // Call the same logic as ConquestZoneControlBanner::OnGossipHello
            // We'll create a temporary instance to reuse the logic
            static ConquestZoneControlBanner bannerScript;
            return bannerScript.OnGossipHello(player, go);
        }

        return false; // Not our banner, let default behavior handle it
    }

    void OnGameObjectRemoveWorld(GameObject* go) override
    {
        if (!IsEnabled() || !go)
            return;

        uint32 entry = go->GetEntry();
        if (entry == BANNER_DISPUTED_HORDE ||
            entry == BANNER_DISPUTED_ALLIANCE ||
            entry == BANNER_HORDE ||
            entry == BANNER_ALLIANCE)
        {
            UnregisterBannerInfo(go);

            // If a disputed banner was manually deleted (e.g., .gob delete),
            // ensure its DB entry is cleaned up so it can be respawned cleanly later.
            if (entry == BANNER_DISPUTED_HORDE || entry == BANNER_DISPUTED_ALLIANCE)
            {
                ObjectGuid::LowType bannerGuid = go->GetGUID().GetCounter();

                CharacterDatabase.Execute(
                    "DELETE FROM conquest_zone_control WHERE zone_guid = {}",
                    bannerGuid
                );

                CharacterDatabase.Execute(
                    "DELETE FROM conquest_zone_control_despawned_creatures WHERE zone_guid = {}",
                    bannerGuid
                );

                LOG_INFO("module", "ConquestZoneControl: Disputed banner {} deleted from world, cleaned DB entries", bannerGuid);
            }
        }
    }
};

// Guild Script to handle guild disband and clean up banners
class ConquestZoneControlGuild : public GuildScript
{
public:
    ConquestZoneControlGuild() : GuildScript("ConquestZoneControlGuild") { }

    // Helper function to clean up banners for a guild
    void CleanupGuildBanners(uint32 guildId)
    {
        LOG_INFO("module", "ConquestZoneControl: Cleaning up banners for guild {}", guildId);

        // Find all banners controlled by this guild
        QueryResult result = CharacterDatabase.Query(
            "SELECT zone_guid, original_entry FROM conquest_zone_control WHERE guild_id = {}",
            guildId
        );

        if (!result)
        {
            LOG_DEBUG("module", "ConquestZoneControl: No banners found for disbanded guild {}", guildId);
            return;
        }

        do
        {
            Field* fields = result->Fetch();
            ObjectGuid::LowType zoneGuid = fields[0].Get<uint64>();
            uint32 originalEntry = fields[1].Get<uint32>();

            auto infoIt = s_bannerInfo.find(zoneGuid);
            if (infoIt == s_bannerInfo.end())
            {
                LOG_WARN("module", "ConquestZoneControl: No banner info found for zone {}, skipping", zoneGuid);
                continue;
            }

            BannerInfo const& info = infoIt->second;
            Map* bannerMap = sMapMgr->FindMap(info.mapId, info.instanceId);
            if (!bannerMap)
                bannerMap = sMapMgr->FindBaseMap(info.mapId);
            if (!bannerMap)
            {
                LOG_WARN("module", "ConquestZoneControl: Map not found for banner {} (mapId: {}, instanceId: {})", 
                    zoneGuid, info.mapId, info.instanceId);
                continue;
            }

            Position bannerPos = info.pos;
            float bannerO = info.pos.GetOrientation();
            G3D::Quat rotation = info.rotation;

            uint32 disputedEntry = originalEntry;
            if (disputedEntry == 0)
            {
                uint32 currentEntry = info.entry;
                if (currentEntry == BANNER_HORDE)
                    disputedEntry = BANNER_DISPUTED_HORDE;
                else if (currentEntry == BANNER_ALLIANCE)
                    disputedEntry = BANNER_DISPUTED_ALLIANCE;
                else
                    disputedEntry = BANNER_DISPUTED_HORDE;
            }

            GameObjectTemplate const* goInfo = sObjectMgr->GetGameObjectTemplate(disputedEntry);
            if (!goInfo)
            {
                LOG_ERROR("module", "ConquestZoneControl: GameObject template not found for entry {}", disputedEntry);
                continue;
            }

            GameObject* newBanner = bannerMap->SummonGameObject(
                disputedEntry,
                bannerPos.GetPositionX(),
                bannerPos.GetPositionY(),
                bannerPos.GetPositionZ(),
                bannerO,
                rotation.x,
                rotation.y,
                rotation.z,
                rotation.w,
                0,
                false
            );

            if (!newBanner)
            {
                LOG_ERROR("module", "ConquestZoneControl: Failed to create disputed banner for zone {}", zoneGuid);
                continue;
            }

            if (newBanner->GetDisplayId() != goInfo->displayId)
            {
                newBanner->SetDisplayId(goInfo->displayId);
                newBanner->UpdateObjectVisibility(true);
            }
            RegisterBannerInfo(newBanner);

            RespawnCreaturesForZone(newBanner->GetGUID().GetCounter());

            if (GameObject* oldBanner = bannerMap->GetGameObject(info.guid))
            {
                oldBanner->SetRespawnTime(0);
                oldBanner->Delete();
            }
            else
            {
                LOG_WARN("module", "ConquestZoneControl: Banner {} not found in world, removed via cleanup", zoneGuid);
            }

            LOG_INFO("module", "ConquestZoneControl: Banner {} reset to disputed state (entry: {})", zoneGuid, disputedEntry);

            // Delete zone control entry from database
            CharacterDatabase.Execute(
                "DELETE FROM conquest_zone_control WHERE zone_guid = {}",
                zoneGuid
            );

            // Update despawned creatures table (remove entries for this zone)
            CharacterDatabase.Execute(
                "DELETE FROM conquest_zone_control_despawned_creatures WHERE zone_guid = {}",
                zoneGuid
            );

            s_bannerInfo.erase(zoneGuid);

        } while (result->NextRow());

        LOG_INFO("module", "ConquestZoneControl: Cleaned up all banners for guild {}", guildId);
    }

    void OnDisband(Guild* guild) override
    {
        if (!IsEnabled() || !guild)
            return;

        uint32 guildId = guild->GetId();
        CleanupGuildBanners(guildId);
    }

    void OnRemoveMember(Guild* guild, Player* player, bool isDisbanding, bool /*isKicked*/) override
    {
        if (!IsEnabled() || !guild || isDisbanding)
            return;

        uint32 guildId = guild->GetId();
        
        // Check member count from the Guild object itself
        // The hook is called BEFORE the member is removed from the Guild object
        // So if GetMemberCount() returns 1, it means this is the last member leaving
        uint32 memberCount = guild->GetMemberCount();
        
        LOG_INFO("module", "ConquestZoneControl: Player {} left guild {} (member count: {})", 
            player ? player->GetName() : "Unknown", guildId, memberCount);
        
        // If this is the last member (count == 1), clean up banners
        // The member will be removed after this hook, leaving 0 members
        if (memberCount <= 1)
        {
            LOG_INFO("module", "ConquestZoneControl: Last member left guild {}, cleaning up controlled banners", guildId);
            CleanupGuildBanners(guildId);
        }
    }
};

void AddConquestZoneControlScripts()
{
    LoadConfigValues();
    new ConquestZoneControlBanner();
    new ConquestZoneControlPlayer();
    new ConquestZoneControlSpell();
    new ConquestZoneControlChannelFinish();
    new ConquestZoneControlAllBanner();
    new ConquestZoneControlGuild();
}

