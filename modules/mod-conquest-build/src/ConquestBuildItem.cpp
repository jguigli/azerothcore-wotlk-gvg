/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

#include "ConquestBuildCommon.h"
#include "Item.h"
#include "Spell.h"
#include "Log.h"
#include "ConquestMarkerSpawner.h"
#include "ConquestRecruitmentKit.h"

// Item script for building walls
class item_conquest_build_wall : public ItemScript
{
public:
    item_conquest_build_wall() : ItemScript("item_conquest_build_wall") { }

    bool OnUse(Player* player, Item* item, SpellCastTargets const& targets) override
    {
        ConquestBuildConfig config = ConquestBuildMgr::instance()->GetConfig();

        if (!config.enabled)
    {
            ChatHandler(player->GetSession()).PSendSysMessage(CONQUEST_BUILD_DISABLED);
            return false;
        }

        // Check if player is in combat
        if (player->IsInCombat())
        {
            ChatHandler(player->GetSession()).SendSysMessage("Vous ne pouvez pas utiliser cet objet en combat!");
            return false;
        }

        // Determine which gameobject to use based on item entry
        uint32 gobId = config.wallGobId; // Default to configured wall
        if (item->GetEntry() == 80000) // Kit de Grand Mur
        {
            gobId = 400020; // Grand Mur Conquest
        }
        else if (item->GetEntry() == 80004) // Kit de Mur
        {
            gobId = 400022; // Mur Conquest
        }

        // Spawn wall at clicked location
        bool success = ConquestBuildMgr::instance()->SpawnStructure(player, gobId, CONQUEST_BUILD_WALL, &targets);
        
        if (success)
                {
            // Save spell ID and item entry BEFORE destroying the item (item pointer becomes invalid after DestroyItemCount)
            uint32 spellId = 0;
            uint32 itemEntry = item->GetEntry();
            if (item->GetTemplate() && item->GetTemplate()->Spells[0].SpellId > 0)
            {
                spellId = item->GetTemplate()->Spells[0].SpellId;
            }
            
            // Remove the item (single use)
            player->DestroyItemCount(itemEntry, 1, true);
        
            // Add 1 second cooldown to prevent spam
            if (spellId > 0)
        {
                player->AddSpellCooldown(spellId, itemEntry, 1000, true, false);
            }
        }

        return true;
    }
};

// Item script for building towers
class item_conquest_build_tower : public ItemScript
{
public:
    item_conquest_build_tower() : ItemScript("item_conquest_build_tower") { }

    bool OnUse(Player* player, Item* item, SpellCastTargets const& targets) override
    {
        ConquestBuildConfig config = ConquestBuildMgr::instance()->GetConfig();

        if (!config.enabled)
        {
            ChatHandler(player->GetSession()).PSendSysMessage(CONQUEST_BUILD_DISABLED);
            return false;
        }

        // Check if player is in combat
        if (player->IsInCombat())
        {
            ChatHandler(player->GetSession()).SendSysMessage("Vous ne pouvez pas utiliser cet objet en combat!");
            return false;
        }

        // Determine which gameobject to use based on item entry
        uint32 gobId = config.towerGobId; // Default to configured tower
        if (item->GetEntry() == 80001) // Kit de Grande Tour
        {
            gobId = 400021; // Grande Tour Conquest
        }
        else if (item->GetEntry() == 80005) // Kit de Tour
        {
            gobId = 400023; // Tour Conquest
        }

        // Spawn tower at clicked location
        bool success = ConquestBuildMgr::instance()->SpawnStructure(player, gobId, CONQUEST_BUILD_TOWER, &targets);
        
        if (success)
        {
            // Save spell ID and item entry BEFORE destroying the item (item pointer becomes invalid after DestroyItemCount)
            uint32 spellId = 0;
            uint32 itemEntry = item->GetEntry();
            if (item->GetTemplate() && item->GetTemplate()->Spells[0].SpellId > 0)
            {
                spellId = item->GetTemplate()->Spells[0].SpellId;
            }
            
            // Remove the item (single use)
            player->DestroyItemCount(itemEntry, 1, true);
                    
            // Add 1 second cooldown to prevent spam
            if (spellId > 0)
            {
                player->AddSpellCooldown(spellId, itemEntry, 1000, true, false);
                }
        }

        return true;
                }
};

// Item script for building gates
class item_conquest_build_gate : public ItemScript
{
public:
    item_conquest_build_gate() : ItemScript("item_conquest_build_gate") { }

    bool OnUse(Player* player, Item* item, SpellCastTargets const& targets) override
    {
        ConquestBuildConfig config = ConquestBuildMgr::instance()->GetConfig();

        if (!config.enabled)
        {
            ChatHandler(player->GetSession()).PSendSysMessage(CONQUEST_BUILD_DISABLED);
            return false;
        }

        // Check if player is in combat
        if (player->IsInCombat())
        {
            ChatHandler(player->GetSession()).SendSysMessage("Vous ne pouvez pas utiliser cet objet en combat!");
            return false;
        }

        // Check if player is in a guild
        if (!player->GetGuildId())
        {
            ChatHandler(player->GetSession()).SendSysMessage("Vous devez être dans une guilde pour placer une herse!");
            return false;
        }

        // Spawn gate at clicked location
        bool success = ConquestBuildMgr::instance()->SpawnStructure(player, config.gateGobId, CONQUEST_BUILD_GATE, &targets);
        
        if (success)
    {
            // Save spell ID and item entry BEFORE destroying the item (item pointer becomes invalid after DestroyItemCount)
            uint32 spellId = 0;
            uint32 itemEntry = item->GetEntry();
            if (item->GetTemplate() && item->GetTemplate()->Spells[0].SpellId > 0)
                                {
                spellId = item->GetTemplate()->Spells[0].SpellId;
            }
            
            // Remove the item (single use)
            player->DestroyItemCount(itemEntry, 1, true);
            
            // Add 1 second cooldown to prevent spam
            if (spellId > 0)
        {
                player->AddSpellCooldown(spellId, itemEntry, 1000, true, false);
                }
            }

        return true;
    }
};

// Item script for building small gate (herse)
class item_conquest_build_herse : public ItemScript
{
public:
    item_conquest_build_herse() : ItemScript("item_conquest_build_herse") { }

    bool OnUse(Player* player, Item* item, SpellCastTargets const& targets) override
    {
        ConquestBuildConfig config = ConquestBuildMgr::instance()->GetConfig();

        if (!config.enabled)
        {
            ChatHandler(player->GetSession()).PSendSysMessage(CONQUEST_BUILD_DISABLED);
            return false;
        }

        // Check if player is in combat
        if (player->IsInCombat())
        {
            ChatHandler(player->GetSession()).SendSysMessage("Vous ne pouvez pas utiliser cet objet en combat!");
            return false;
        }

        // Check if player is in a guild
        if (!player->GetGuildId())
        {
            ChatHandler(player->GetSession()).SendSysMessage("Vous devez être dans une guilde pour placer une herse!");
            return false;
        }

        // Spawn small gate at clicked location (uses small tower and small gate)
        bool success = ConquestBuildMgr::instance()->SpawnStructure(player, 400026, CONQUEST_BUILD_GATE_SMALL, &targets);
        
        if (success)
        {
            // Save spell ID and item entry BEFORE destroying the item (item pointer becomes invalid after DestroyItemCount)
            uint32 spellId = 0;
            uint32 itemEntry = item->GetEntry();
            if (item->GetTemplate() && item->GetTemplate()->Spells[0].SpellId > 0)
            {
                spellId = item->GetTemplate()->Spells[0].SpellId;
            }
            
            // Remove the item (single use)
            player->DestroyItemCount(itemEntry, 1, true);
            
            // Add 1 second cooldown to prevent spam
            if (spellId > 0)
            {
                player->AddSpellCooldown(spellId, itemEntry, 1000, true, false);
            }
        }

        return true;
    }
};

// Item script for building teleport beacon
class item_conquest_build_teleport_beacon : public ItemScript
{
public:
    item_conquest_build_teleport_beacon() : ItemScript("item_conquest_build_teleport_beacon") { }

    bool OnUse(Player* player, Item* item, SpellCastTargets const& targets) override
    {
        ConquestBuildConfig config = ConquestBuildMgr::instance()->GetConfig();

        if (!config.enabled)
        {
            ChatHandler(player->GetSession()).PSendSysMessage(CONQUEST_BUILD_DISABLED);
            return false;
        }

        // Check if player is in combat
        if (player->IsInCombat())
        {
            ChatHandler(player->GetSession()).SendSysMessage("Vous ne pouvez pas utiliser cet objet en combat!");
            return false;
        }

        // Spawn teleport beacon at clicked location
        bool success = ConquestBuildMgr::instance()->SpawnStructure(player, 400024, CONQUEST_BUILD_TELEPORT_BEACON, &targets);
        
        if (success)
        {
            // Save spell ID and item entry BEFORE destroying the item (item pointer becomes invalid after DestroyItemCount)
            uint32 spellId = 0;
            uint32 itemEntry = item->GetEntry();
            if (item->GetTemplate() && item->GetTemplate()->Spells[0].SpellId > 0)
            {
                spellId = item->GetTemplate()->Spells[0].SpellId;
            }
            
            // Remove the item (single use)
            player->DestroyItemCount(itemEntry, 1, true);
            
            // Add 1 second cooldown to prevent spam
            if (spellId > 0)
            {
                player->AddSpellCooldown(spellId, itemEntry, 1000, true, false);
            }
        }

        return true;
    }
};

// Item script for building goblin workshop
class item_conquest_build_atelier_gobelin : public ItemScript
{
public:
    item_conquest_build_atelier_gobelin() : ItemScript("item_conquest_build_atelier_gobelin") { }

    bool OnUse(Player* player, Item* item, SpellCastTargets const& targets) override
    {
        ConquestBuildConfig config = ConquestBuildMgr::instance()->GetConfig();

        if (!config.enabled)
        {
            ChatHandler(player->GetSession()).PSendSysMessage(CONQUEST_BUILD_DISABLED);
            return false;
        }

        // Check if player is in combat
        if (player->IsInCombat())
        {
            ChatHandler(player->GetSession()).SendSysMessage("Vous ne pouvez pas utiliser cet objet en combat!");
            return false;
        }

        // Spawn goblin workshop at clicked location
        bool success = ConquestBuildMgr::instance()->SpawnStructure(player, 400027, CONQUEST_BUILD_ATELIER_GOBELIN, &targets);
        
        if (success)
        {
            // Save spell ID and item entry BEFORE destroying the item (item pointer becomes invalid after DestroyItemCount)
            uint32 spellId = 0;
            uint32 itemEntry = item->GetEntry();
            if (item->GetTemplate() && item->GetTemplate()->Spells[0].SpellId > 0)
            {
                spellId = item->GetTemplate()->Spells[0].SpellId;
            }
            
            // Remove the item (single use)
            player->DestroyItemCount(itemEntry, 1, true);
            
            // Add 1 second cooldown to prevent spam
            if (spellId > 0)
            {
                player->AddSpellCooldown(spellId, itemEntry, 1000, true, false);
            }
        }

        return true;
    }
};

// Item script for recovering structures
class item_conquest_build_recovery : public ItemScript
{
public:
    item_conquest_build_recovery() : ItemScript("item_conquest_build_recovery") { }

    bool OnUse(Player* player, Item* item, SpellCastTargets const& /*targets*/) override
    {
        ConquestBuildConfig config = ConquestBuildMgr::instance()->GetConfig();

        if (!config.enabled)
        {
            ChatHandler(player->GetSession()).PSendSysMessage(CONQUEST_BUILD_DISABLED);
            return false;
        }

        // Check if player is in combat
        if (player->IsInCombat())
        {
            ChatHandler(player->GetSession()).SendSysMessage("Vous ne pouvez pas utiliser cet objet en combat!");
            return false;
        }

        // Search for ALL Conquest structures near the player (within 10 yards)
        float searchRadius = 10.0f;
        
        // Search for all Conquest structures (walls, towers, gates, levers)
        std::list<GameObject*> goList;
        player->GetGameObjectListWithEntryInGrid(goList, config.wallGobId, searchRadius);
        
        // Also search for Grand Mur (400020)
        std::list<GameObject*> grandWallList;
        player->GetGameObjectListWithEntryInGrid(grandWallList, 400020, searchRadius);
        goList.insert(goList.end(), grandWallList.begin(), grandWallList.end());
        
        std::list<GameObject*> towerList;
        player->GetGameObjectListWithEntryInGrid(towerList, config.towerGobId, searchRadius);
        goList.insert(goList.end(), towerList.begin(), towerList.end());
        
        // Also search for Grande Tour (400021)
        std::list<GameObject*> grandTowerList;
        player->GetGameObjectListWithEntryInGrid(grandTowerList, 400021, searchRadius);
        goList.insert(goList.end(), grandTowerList.begin(), grandTowerList.end());
        
        // Also search for Mur (400022) and Tour (400023)
        std::list<GameObject*> smallWallList;
        player->GetGameObjectListWithEntryInGrid(smallWallList, 400022, searchRadius);
        goList.insert(goList.end(), smallWallList.begin(), smallWallList.end());
        
        std::list<GameObject*> smallTowerList;
        player->GetGameObjectListWithEntryInGrid(smallTowerList, 400023, searchRadius);
        goList.insert(goList.end(), smallTowerList.begin(), smallTowerList.end());
        
        // Also search for Balise de Téléportation (400024)
        std::list<GameObject*> beaconList;
        player->GetGameObjectListWithEntryInGrid(beaconList, 400024, searchRadius);
        goList.insert(goList.end(), beaconList.begin(), beaconList.end());
        
        std::list<GameObject*> gateList;
        player->GetGameObjectListWithEntryInGrid(gateList, config.gateGobId, searchRadius);
        goList.insert(goList.end(), gateList.begin(), gateList.end());
        
        // Also search for small gate (400026) and big gate (400025)
        std::list<GameObject*> smallGateList;
        player->GetGameObjectListWithEntryInGrid(smallGateList, 400026, searchRadius);
        goList.insert(goList.end(), smallGateList.begin(), smallGateList.end());
        
        std::list<GameObject*> bigGateList;
        player->GetGameObjectListWithEntryInGrid(bigGateList, 400025, searchRadius);
        goList.insert(goList.end(), bigGateList.begin(), bigGateList.end());
        
        // Also search for batiments (400029, 400029, 400030)
        std::list<GameObject*> batimentList;
        player->GetGameObjectListWithEntryInGrid(batimentList, 400029, searchRadius);
        goList.insert(goList.end(), batimentList.begin(), batimentList.end());
        
        std::list<GameObject*> batiment2List;
        player->GetGameObjectListWithEntryInGrid(batiment2List, 400029, searchRadius);
        goList.insert(goList.end(), batiment2List.begin(), batiment2List.end());
        
        std::list<GameObject*> batimentToitList;
        player->GetGameObjectListWithEntryInGrid(batimentToitList, 400030, searchRadius);
        goList.insert(goList.end(), batimentToitList.begin(), batimentToitList.end());
        
        // Filter valid Conquest structures owned by the player
        std::vector<GameObject*> validStructures;
        for (GameObject* gobj : goList)
        {
            if (!gobj)
                continue;
            
            uint32 goEntry = gobj->GetEntry();
            
            // Check if it's a Conquest structure
            if (goEntry != config.wallGobId && goEntry != config.towerGobId && goEntry != config.gateGobId && 
                goEntry != 400001 && goEntry != 400020 && goEntry != 400021 &&
                goEntry != 400022 && goEntry != 400023 && goEntry != 400024 && goEntry != 400025 && goEntry != 400026 &&
                goEntry != 400029 && goEntry != 400029 && goEntry != 400030)
            {
                continue;
            }
            
            // Check if player owns this structure
            ObjectGuid::LowType spawnId = gobj->GetSpawnId();
            QueryResult result = WorldDatabase.Query(
                "SELECT player_guid FROM conquest_build_structures WHERE guid = {}", 
                spawnId
            );
            
            if (!result)
                continue;
            
            Field* fields = result->Fetch();
            uint64 ownerGuid = fields[0].Get<uint64>();
            
            if (ownerGuid != player->GetGUID().GetCounter())
                continue;
            
            // Check if the structure has full health (only structures with 100% health can be recovered)
            if (gobj->GetGoType() == GAMEOBJECT_TYPE_DESTRUCTIBLE_BUILDING)
            {
                uint32 currentHealth = gobj->GetGOValue()->Building.Health;
                uint32 maxHealth = gobj->GetGOValue()->Building.MaxHealth;
                
                if (maxHealth > 0 && currentHealth < maxHealth)
                    continue;
            }
            
            validStructures.push_back(gobj);
        }
        
        if (validStructures.empty())
        {
            ChatHandler(player->GetSession()).SendSysMessage("Aucune structure Conquest récupérable trouvée près de vous (rayon 10y).");
            return false;
        }
        
        // Process all valid structures
        uint32 recoveredCount = 0;
        std::set<uint64> processedGroups; // Track processed groups to avoid duplicates
        
        for (GameObject* go : validStructures)
        {
            uint32 goEntry = go->GetEntry();
            ObjectGuid::LowType spawnId = go->GetSpawnId();
            
            QueryResult result = WorldDatabase.Query(
                "SELECT player_guid, build_type, UNIX_TIMESTAMP(build_time), group_id FROM conquest_build_structures WHERE guid = {}", 
                spawnId
            );
            
            if (!result)
                continue;
            
            Field* fields = result->Fetch();
            uint32 buildType = fields[1].Get<uint32>();
            uint64 groupId = fields[3].Get<uint64>();
            
            // Skip if this group was already processed
            if (groupId > 0 && processedGroups.find(groupId) != processedGroups.end())
                continue;
            
            // Mark this group as processed
            if (groupId > 0)
                processedGroups.insert(groupId);
        
        // Check if this is a fortification system (gate) or a fort
        bool isFortification = (buildType == CONQUEST_BUILD_GATE || buildType == CONQUEST_BUILD_GATE_SMALL || goEntry == config.gateGobId || goEntry == 400026);
        bool isFort = (buildType == CONQUEST_BUILD_FORT || buildType == CONQUEST_BUILD_GRAND_FORT || buildType == CONQUEST_BUILD_GRAND_FORT_2 || buildType == CONQUEST_BUILD_GRAND_FORT_3);
        
            // If part of a fort system, remove entire group and give back all component kits
            if (isFort && groupId > 0)
            {
                // This is part of a fort system
                // Remove the entire group using RemoveStructure which handles groups
                ConquestBuildMgr::instance()->RemoveStructure(spawnId, player->GetMap());
                
                // Remove the current GameObject from world (already handled by RemoveStructure, but be safe)
                go->SetRespawnTime(0);
                go->Delete();
                
                // Count structures in the group to determine what kits to give back
                QueryResult groupResult = WorldDatabase.Query(
                    "SELECT entry FROM conquest_build_structures WHERE group_id = {} AND entry NOT IN (24388, 34944, 400001)",
                    groupId
                );
                
                uint32 wallCount = 0;
                uint32 towerCount = 0;
                uint32 gateCount = 0;
                
                if (groupResult)
                {
                    do
                    {
                        Field* fields = groupResult->Fetch();
                        uint32 entry = fields[0].Get<uint32>();
                        
                        if (buildType == CONQUEST_BUILD_FORT)
                        {
                            if (entry == 400022) // Small wall
                                wallCount++;
                            else if (entry == 400023) // Small tower
                                towerCount++;
                            else if (entry == 400026) // Small gate
                                gateCount++;
                        }
                        else if (buildType == CONQUEST_BUILD_GRAND_FORT || buildType == CONQUEST_BUILD_GRAND_FORT_2 || buildType == CONQUEST_BUILD_GRAND_FORT_3)
                        {
                            if (entry == 400020) // Grand wall
                                wallCount++;
                            else if (entry == 400021) // Grande tower
                                towerCount++;
                            else if (entry == 400025) // Grande gate
                                gateCount++;
                        }
                    } while (groupResult->NextRow());
                }
                
                // Give back the appropriate kits
                if (buildType == CONQUEST_BUILD_FORT)
                {
                    if (wallCount > 0)
                        player->AddItem(80004, wallCount); // Kit de mur
                    if (towerCount > 0)
                        player->AddItem(80005, towerCount); // Kit de tour
                    if (gateCount > 0)
                        player->AddItem(80007, gateCount); // Kit de herse
                }
                else if (buildType == CONQUEST_BUILD_GRAND_FORT || buildType == CONQUEST_BUILD_GRAND_FORT_2)
                {
                    if (wallCount > 0)
                        player->AddItem(80000, wallCount); // Kit de grand mur
                    if (towerCount > 0)
                        player->AddItem(80001, towerCount); // Kit de grande tour
                    if (gateCount > 0)
                        player->AddItem(80003, gateCount); // Kit de grande herse
                }
                
                recoveredCount++;
            }
            // If part of a fortification system (gate), remove entire group and give back ONE gate kit
            else if (isFortification && groupId > 0)
            {
                // This is part of a fortification system (2 towers + 1 gate + 2 levers)
                // Remove the entire group using RemoveStructure which handles groups
                ConquestBuildMgr::instance()->RemoveStructure(spawnId, player->GetMap());
                
                // Remove the current GameObject from world (already handled by RemoveStructure, but be safe)
                go->SetRespawnTime(0);
                go->Delete();
                
                // Give back the appropriate gate kit based on gate entry
                uint32 gateKitEntry = 80003; // Default to big gate kit
                if (goEntry == 400026) // Small gate
                {
                    gateKitEntry = 80007; // Small gate kit
                }
                player->AddItem(gateKitEntry, 1);
                recoveredCount++;
            }
            else
            {
                // Regular standalone structure (wall or tower)
                // For towers, check if there are linked benches and cannons (group_id = spawnId)
                if (buildType == CONQUEST_BUILD_TOWER || goEntry == config.towerGobId || goEntry == 400021 || goEntry == 400023)
                {
                    // Check if this tower has linked benches and cannons (they use the tower's guid as group_id)
                    QueryResult linkedResult = WorldDatabase.Query("SELECT guid, entry FROM conquest_build_structures WHERE group_id = {} AND entry IN (24388, 34944)", spawnId);
                    if (linkedResult)
                    {
                        uint32 benchCount = 0;
                        uint32 cannonCount = 0;
                        
                        // Delete all linked structures (benches and cannons)
                        do
                        {
                            Field* fields = linkedResult->Fetch();
                            uint64 linkedGuid = fields[0].Get<uint64>();
                            uint32 linkedEntry = fields[1].Get<uint32>();
                            
                            // Check if it's a GameObject (bench) or Creature (cannon)
                            if (linkedEntry == 24388) // Stone bench
                            {
                                benchCount++;
                                
                                // Delete from tracking table first
                                WorldDatabase.Execute("DELETE FROM conquest_build_structures WHERE guid = {}", linkedGuid);
                                
                                // Delete from world gameobject table
                                WorldDatabase.Execute("DELETE FROM gameobject WHERE guid = {}", linkedGuid);
                                
                                // Delete from world if map is provided
                                auto benchBounds = player->GetMap()->GetGameObjectBySpawnIdStore().equal_range(linkedGuid);
                                for (auto itr = benchBounds.first; itr != benchBounds.second; ++itr)
                                {
                                    GameObject* benchGobj = itr->second;
                                    if (benchGobj)
                                    {
                                        benchGobj->SetRespawnTime(0);
                                        benchGobj->Delete();
                                    }
                                }
                                
                                // Also remove from grid
                                if (GameObjectData const* benchData = sObjectMgr->GetGameObjectData(linkedGuid))
                                {
                                    sObjectMgr->RemoveGameobjectFromGrid(linkedGuid, benchData);
                                }
                            }
                            else if (linkedEntry == 34944) // Cannon
                            {
                                cannonCount++;
                                
                                // Delete from tracking table first
                                WorldDatabase.Execute("DELETE FROM conquest_build_structures WHERE guid = {}", linkedGuid);
                                
                                // Delete from world creature table
                                WorldDatabase.Execute("DELETE FROM creature WHERE guid = {}", linkedGuid);
                                
                                // Delete from world if map is provided
                                auto cannonBounds = player->GetMap()->GetCreatureBySpawnIdStore().equal_range(linkedGuid);
                                for (auto itr = cannonBounds.first; itr != cannonBounds.second; ++itr)
                                {
                                    Creature* cannon = itr->second;
                                    if (cannon)
                                    {
                                        cannon->SetRespawnTime(0);
                                        cannon->RemoveCorpse();
                                        cannon->AddObjectToRemoveList();
                                    }
                                }
                                
                                // Also remove from grid
                                if (CreatureData const* cannonData = sObjectMgr->GetCreatureData(linkedGuid))
                                {
                                    sObjectMgr->RemoveCreatureFromGrid(linkedGuid, cannonData);
                                }
                            }
                        } while (linkedResult->NextRow());
                        
                        LOG_INFO("module", "ConquestBuild: Recovered tower - deleted {} benches and {} cannons", benchCount, cannonCount);
                    }
                    else
                    {
                        LOG_INFO("module", "ConquestBuild: No linked benches/cannons found for tower guid {}", spawnId);
                    }
                }
                
                // Determine which item to give back based on game object entry
                uint32 itemToGive = 80000; // Default to Grand Mur
                if (goEntry == 400022)
                    itemToGive = 80004; // Kit de Mur
                else if (goEntry == 400023)
                    itemToGive = 80005; // Kit de Tour
                else if (goEntry == 400024)
                    itemToGive = 80006; // Kit de Balise de Téléportation
                else if (goEntry == 400020)
                    itemToGive = 80000; // Kit de Grand Mur
                else if (goEntry == 400021)
                    itemToGive = 80001; // Kit de Grande Tour
                else if (goEntry == 400029 || goEntry == 400029 || goEntry == 400030)
                    itemToGive = 0; // Batiments - pas de kit de récupération pour l'instant
                else if (goEntry == config.wallGobId && goEntry != 400020 && goEntry != 400022)
                    itemToGive = 80000; // Kit de Grand Mur (fallback for other wall entries)
                else if (goEntry == config.towerGobId && goEntry != 400021 && goEntry != 400023)
                    itemToGive = 80001; // Kit de Grande Tour (fallback for other tower entries)

                // Remove from tracking table
                WorldDatabase.Execute("DELETE FROM conquest_build_structures WHERE guid = {}", spawnId);
                
                // Delete from world gameobject table
                WorldDatabase.Execute("DELETE FROM gameobject WHERE guid = {}", spawnId);
                
                // Remove from world - use the same method as RemoveStructure
                auto bounds = player->GetMap()->GetGameObjectBySpawnIdStore().equal_range(spawnId);
                for (auto itr = bounds.first; itr != bounds.second; ++itr)
                {
                    GameObject* gobj = itr->second;
                    if (gobj)
                    {
                        gobj->SetRespawnTime(0);
                        gobj->Delete();
                    }
                }

                // Give the item back to player (if there's a kit for it)
                if (itemToGive > 0)
                {
                    player->AddItem(itemToGive, 1);
                }
                recoveredCount++;
            }
        }
        
        if (recoveredCount > 0)
        {
            ChatHandler(player->GetSession()).PSendSysMessage("{} structure(s) récupérée(s) avec succès!", recoveredCount);
        }
        
        // Add 1 second cooldown to prevent spam (even if recovery failed)
        if (item->GetTemplate()->Spells[0].SpellId > 0)
        {
            player->AddSpellCooldown(item->GetTemplate()->Spells[0].SpellId, item->GetEntry(), 1000, true, false);
        }
        
        // DO NOT consume the recovery tool - return true to indicate success
        return true;
    }
};

// Item script for building fort
class item_conquest_build_fort : public ItemScript
{
public:
    item_conquest_build_fort() : ItemScript("item_conquest_build_fort") { }

    bool OnUse(Player* player, Item* item, SpellCastTargets const& targets) override
    {
        ConquestBuildConfig config = ConquestBuildMgr::instance()->GetConfig();

        if (!config.enabled)
        {
            ChatHandler(player->GetSession()).PSendSysMessage(CONQUEST_BUILD_DISABLED);
            return false;
        }

        // Check if player is in combat
        if (player->IsInCombat())
        {
            ChatHandler(player->GetSession()).SendSysMessage("Vous ne pouvez pas utiliser cet objet en combat!");
            return false;
        }

        // Check if player is in a guild
        if (!player->GetGuildId())
        {
            ChatHandler(player->GetSession()).SendSysMessage("Vous devez être dans une guilde pour placer un fort!");
            return false;
        }

        // Spawn fort at clicked location (uses small structures: mur 400022, tour 400023, herse 400026)
        bool success = ConquestBuildMgr::instance()->SpawnStructure(player, 400026, CONQUEST_BUILD_FORT, &targets);
        
        if (success)
        {
            // Save spell ID and item entry BEFORE destroying the item (item pointer becomes invalid after DestroyItemCount)
            uint32 spellId = 0;
            uint32 itemEntry = item->GetEntry();
            if (item->GetTemplate() && item->GetTemplate()->Spells[0].SpellId > 0)
            {
                spellId = item->GetTemplate()->Spells[0].SpellId;
    }

            // Remove the item (single use)
            player->DestroyItemCount(itemEntry, 1, true);
            
            // Add 1 second cooldown to prevent spam
            if (spellId > 0)
            {
                player->AddSpellCooldown(spellId, itemEntry, 1000, true, false);
            }
        }

        return true;
    }
};

// Item script for building grand fort
class item_conquest_build_grand_fort : public ItemScript
{
public:
    item_conquest_build_grand_fort() : ItemScript("item_conquest_build_grand_fort") { }

    bool OnUse(Player* player, Item* item, SpellCastTargets const& targets) override
    {
        ConquestBuildConfig config = ConquestBuildMgr::instance()->GetConfig();

        if (!config.enabled)
        {
            ChatHandler(player->GetSession()).PSendSysMessage(CONQUEST_BUILD_DISABLED);
            return false;
        }

        // Check if player is in combat
        if (player->IsInCombat())
        {
            ChatHandler(player->GetSession()).SendSysMessage("Vous ne pouvez pas utiliser cet objet en combat!");
            return false;
        }

        // Check if player is in a guild
        if (!player->GetGuildId())
        {
            ChatHandler(player->GetSession()).SendSysMessage("Vous devez être dans une guilde pour placer un grand fort!");
            return false;
        }

        // Spawn grand fort at clicked location (uses large structures: grand mur 400020, grande tour 400021, grande herse 400025)
        bool success = ConquestBuildMgr::instance()->SpawnStructure(player, 400025, CONQUEST_BUILD_GRAND_FORT, &targets);

        if (success)
        {
            // Save spell ID and item entry BEFORE destroying the item (item pointer becomes invalid after DestroyItemCount)
            uint32 spellId = 0;
            uint32 itemEntry = item->GetEntry();
            if (item->GetTemplate() && item->GetTemplate()->Spells[0].SpellId > 0)
        {
                spellId = item->GetTemplate()->Spells[0].SpellId;
        }

            // Remove the item (single use)
            player->DestroyItemCount(itemEntry, 1, true);
            
            // Add 1 second cooldown to prevent spam
            if (spellId > 0)
            {
                player->AddSpellCooldown(spellId, itemEntry, 1000, true, false);
            }
        }

        return true;
    }
};

// Item script for building fort with ramparts (rempart 1)
class item_conquest_build_fort_rempart_1 : public ItemScript
{
public:
    item_conquest_build_fort_rempart_1() : ItemScript("item_conquest_build_fort_rempart_1") { }

    bool OnUse(Player* player, Item* item, SpellCastTargets const& targets) override
    {
        ConquestBuildConfig config = ConquestBuildMgr::instance()->GetConfig();

        if (!config.enabled)
        {
            ChatHandler(player->GetSession()).PSendSysMessage(CONQUEST_BUILD_DISABLED);
            return false;
        }

        // Check if player is in combat
        if (player->IsInCombat())
        {
            ChatHandler(player->GetSession()).SendSysMessage("Vous ne pouvez pas utiliser cet objet en combat!");
            return false;
        }

        // Check if player is in a guild
        if (!player->GetGuildId())
        {
            ChatHandler(player->GetSession()).SendSysMessage("Vous devez être dans une guilde pour placer un fort rempart!");
            return false;
        }

        // Determine if this is a grand fort (80011) or small fort based on item entry
        uint32 itemEntry = item->GetEntry();
        bool isGrandFort = (itemEntry == 80011); // Kit de grand fort 2
        
        // Calculate gate position (same logic as SpawnStructure for fort)
        float playerX = player->GetPositionX();
        float playerY = player->GetPositionY();
        float playerZ = player->GetPositionZ();
        float playerOrientation = player->GetOrientation();
        
        float gateX = playerX;
        float gateY = playerY;
        float gateZ = playerZ;
        float gateOrientation = playerOrientation;
        
        uint32 gateEntry;
        ConquestBuildType buildType;
        
        if (isGrandFort)
        {
            // Grand fort: advance gate 2 yards forward (same as CONQUEST_BUILD_GRAND_FORT in SpawnStructure)
            gateX += 2.0f * cos(playerOrientation);
            gateY += 2.0f * sin(playerOrientation);
            gateEntry = 400025; // Grande herse
            buildType = CONQUEST_BUILD_GRAND_FORT;
        }
        else
        {
            // Small fort: advance gate 2 yards forward (same as CONQUEST_BUILD_FORT in SpawnStructure)
            gateX += 2.0f * cos(playerOrientation);
            gateY += 2.0f * sin(playerOrientation);
            gateEntry = 400026; // Petite herse
            buildType = CONQUEST_BUILD_FORT;
        }
        
        // First, spawn the fort at clicked location
        LOG_INFO("module", "ConquestBuild: Spawning {} fort with ramparts for player {} (item entry: {})", 
            isGrandFort ? "grand" : "small", player->GetName(), itemEntry);
        
        uint64 fortGateGuid = 0;
        uint64 fortGroupId = 0;
        bool success = ConquestBuildMgr::instance()->SpawnStructure(player, gateEntry, buildType, &targets, &fortGateGuid, &fortGroupId);
        
        LOG_INFO("module", "ConquestBuild: Fort spawn result: {}, gate position calculated: ({}, {}, {}), guid: {}, groupId: {}", 
            success, gateX, gateY, gateZ, fortGateGuid, fortGroupId);
        
        if (success)
        {
            if (fortGroupId > 0)
            {
                uint32 guildId = player->GetGuildId();
                
                LOG_INFO("module", "ConquestBuild: Calling SpawnFortRamparts with groupId: {}", fortGroupId);
                
                // Spawn ramparts around the fort using calculated position
                ConquestBuildMgr::instance()->SpawnFortRamparts(player, gateX, gateY, gateZ, gateOrientation, fortGroupId, guildId);
                
                LOG_INFO("module", "ConquestBuild: SpawnFortRamparts called successfully");
            }
            else
            {
                LOG_ERROR("module", "ConquestBuild: Failed to get groupId. success: {}, groupId: {}", success, fortGroupId);
            }
            
            // Save spell ID and item entry BEFORE destroying the item (item pointer becomes invalid after DestroyItemCount)
            uint32 spellId = 0;
            // itemEntry already declared above
            if (item->GetTemplate() && item->GetTemplate()->Spells[0].SpellId > 0)
            {
                spellId = item->GetTemplate()->Spells[0].SpellId;
    }

            // Remove the item (single use)
            player->DestroyItemCount(itemEntry, 1, true);
            
            // Add 1 second cooldown to prevent spam
            if (spellId > 0)
            {
                player->AddSpellCooldown(spellId, itemEntry, 1000, true, false);
            }
        }

        return true;
    }
};

class item_conquest_build_fort_rempart_2 : public ItemScript
{
public:
    item_conquest_build_fort_rempart_2() : ItemScript("item_conquest_build_fort_rempart_2") { }

    bool OnUse(Player* player, Item* item, SpellCastTargets const& targets) override
    {
        ConquestBuildConfig config = ConquestBuildMgr::instance()->GetConfig();

        if (!config.enabled)
        {
            ChatHandler(player->GetSession()).PSendSysMessage(CONQUEST_BUILD_DISABLED);
            return false;
        }

        // Check if player is in combat
        if (player->IsInCombat())
        {
            ChatHandler(player->GetSession()).SendSysMessage("Vous ne pouvez pas utiliser cet objet en combat!");
            return false;
        }

        // Check if player is in a guild
        if (!player->GetGuildId())
        {
            ChatHandler(player->GetSession()).SendSysMessage("Vous devez être dans une guilde pour placer un fort rempart!");
            return false;
        }

        // Determine if this is a grand fort (80012) based on item entry
        uint32 itemEntry = item->GetEntry();
        bool isGrandFort = (itemEntry == 80012); // Kit de grand fort 3
        
        // Get spawn position from targets (where player clicked)
        float spawnX, spawnY, spawnZ;
        if (targets.GetDstPos())
        {
            WorldLocation const* destLoc = targets.GetDstPos();
            spawnX = destLoc->GetPositionX();
            spawnY = destLoc->GetPositionY();
            spawnZ = destLoc->GetPositionZ();
        }
        else
        {
            // Fallback to player position
            spawnX = player->GetPositionX();
            spawnY = player->GetPositionY();
            spawnZ = player->GetPositionZ();
        }
        
        // Use player orientation for all structures - this is fixed regardless of click position
        float playerOrientation = player->GetOrientation();
        
        // Calculate gate position - spawn at clicked location, but advance 2 yards forward based on player orientation
        float gateX = spawnX + 2.0f * cos(playerOrientation);
        float gateY = spawnY + 2.0f * sin(playerOrientation);
        float gateZ = spawnZ;
        float gateOrientation = playerOrientation; // All structures use player orientation
        
        uint32 gateEntry;
        ConquestBuildType buildType;
        
        if (isGrandFort)
        {
            // Grand fort 2: use CONQUEST_BUILD_GRAND_FORT_2
            gateEntry = 400025; // Grande herse
            buildType = CONQUEST_BUILD_GRAND_FORT_2;
        }
        else
        {
            // Small fort
            gateEntry = 400026; // Petite herse
            buildType = CONQUEST_BUILD_FORT;
        }
        
        // First, spawn the fort at clicked location with Z offset
        LOG_INFO("module", "ConquestBuild: Spawning {} fort with ramparts for player {} (item entry: {}) at Z offset -3", 
            isGrandFort ? "grand" : "small", player->GetName(), itemEntry);
        
        uint64 fortGateGuid = 0;
        uint64 fortGroupId = 0;
        bool success = ConquestBuildMgr::instance()->SpawnStructure(player, gateEntry, buildType, &targets, &fortGateGuid, &fortGroupId);
        
        LOG_INFO("module", "ConquestBuild: Fort spawn result: {}, gate position calculated: ({}, {}, {}), guid: {}, groupId: {}", 
            success, gateX, gateY, gateZ, fortGateGuid, fortGroupId);
        
        if (success)
        {
            if (fortGroupId > 0)
            {
                uint32 guildId = player->GetGuildId();
                
                // Récupérer la position et l'orientation réelles du fort depuis la base de données pour garantir la cohérence
                float actualGateX = gateX;
                float actualGateY = gateY;
                float actualGateZ = gateZ;
                float actualGateOrientation = gateOrientation;
                
                // Essayer de récupérer le GameObject depuis la map
                GameObject* fortGate = nullptr;
                auto bounds = player->GetMap()->GetGameObjectBySpawnIdStore().equal_range(fortGateGuid);
                for (auto itr = bounds.first; itr != bounds.second; ++itr)
                {
                    fortGate = itr->second;
                    if (fortGate && fortGate->GetEntry() == gateEntry)
                    {
                        break;
                    }
                }
                
                if (fortGate)
                {
                    actualGateX = fortGate->GetPositionX();
                    actualGateY = fortGate->GetPositionY();
                    actualGateZ = fortGate->GetPositionZ();
                    actualGateOrientation = fortGate->GetOrientation();
                    LOG_INFO("module", "ConquestBuild: Retrieved actual fort gate position: ({}, {}, {}), orientation: {}", 
                        actualGateX, actualGateY, actualGateZ, actualGateOrientation);
                }
                else
                {
                    // Si le GameObject n'est pas encore chargé, récupérer depuis la base de données
                    QueryResult result = WorldDatabase.Query("SELECT position_x, position_y, position_z, orientation FROM gameobject WHERE guid = {}", fortGateGuid);
                    if (result)
                    {
                        Field* fields = result->Fetch();
                        actualGateX = fields[0].Get<float>();
                        actualGateY = fields[1].Get<float>();
                        actualGateZ = fields[2].Get<float>();
                        actualGateOrientation = fields[3].Get<float>();
                        LOG_INFO("module", "ConquestBuild: Retrieved actual fort gate position from DB: ({}, {}, {}), orientation: {}", 
                            actualGateX, actualGateY, actualGateZ, actualGateOrientation);
                    }
                }
                
                // Utiliser l'orientation du joueur pour toutes les structures (fixe, indépendante de la position de clic)
                float structuresOrientation = playerOrientation;
                
                // Utiliser la position réelle de la herse pour les remparts
                float rampartGateX = actualGateX;
                float rampartGateY = actualGateY;
                float rampartGateZ = actualGateZ;
                
                LOG_INFO("module", "ConquestBuild: Calling SpawnFortRamparts with groupId: {}", fortGroupId);
                
                // Spawn ramparts around the fort using actual position and player orientation
                // Note: actualGateOrientation is rotated 90 degrees, but we use playerOrientation for ramparts
                ConquestBuildMgr::instance()->SpawnFortRamparts(player, rampartGateX, rampartGateY, rampartGateZ, structuresOrientation, fortGroupId, guildId);
                
                LOG_INFO("module", "ConquestBuild: SpawnFortRamparts called successfully");
                
                // Spawn GameObject 400029 - toutes les positions sont calculées par rapport à la position originale du fort
                // Reculer de 15 yards (12 + 3) depuis la herse originale pour les grands forts
                float batimentX = rampartGateX - 19.0f * cos(structuresOrientation) + 1.0f * cos(structuresOrientation - M_PI / 2.0f); // Reculer de 15 yards depuis la herse originale, décaler de 1 yard à droite
                float batimentY = rampartGateY - 19.0f * sin(structuresOrientation) + 1.0f * sin(structuresOrientation - M_PI / 2.0f); // Reculer de 15 yards depuis la herse originale, décaler de 1 yard à droite
                float batimentZ = rampartGateZ - 3.0f; // Abaisser de 3 yards par rapport au fort
                float batimentOrientation = structuresOrientation - 135.0f * M_PI / 180.0f; // Pivoter de -135 degrés par rapport à l'orientation du joueur
                
                // Rotation de 90 degrés sur l'axe Z (quaternion)
                // Quaternion pour rotation de 90 degrés autour de l'axe Z: (0, 0, sin(45°), cos(45°))
                float angleZ = M_PI / 4.0f; // 45 degrés en radians (la moitié de 90 degrés pour le quaternion)
                G3D::Quat batimentRotation(0, 0, sin(angleZ), cos(angleZ));
                
                LOG_INFO("module", "ConquestBuild: Spawning batiment GameObject 400029 at position ({}, {}, {}) with orientation {} and Z rotation", 
                    batimentX, batimentY, batimentZ, batimentOrientation);
                
                // Create the batiment GameObject
                GameObject* batimentGo = new GameObject();
                ObjectGuid::LowType batimentGuid = player->GetMap()->GenerateLowGuid<HighGuid::GameObject>();
                
                if (batimentGo->Create(batimentGuid, 400029, player->GetMap(), player->GetPhaseMask(),
                                    batimentX, batimentY, batimentZ, batimentOrientation,
                                    batimentRotation, 0, GO_STATE_READY))
                {
                    batimentGo->SaveToDB(player->GetMapId(), 1 << player->GetMap()->GetSpawnMode(), player->GetPhaseMask());
                    ObjectGuid::LowType batimentSpawnId = batimentGo->GetSpawnId();
                    delete batimentGo;
                    
                    // Respawn from DB
                    batimentGo = new GameObject();
                    if (batimentGo->LoadGameObjectFromDB(batimentSpawnId, player->GetMap(), true))
                    {
                        // Add to grid
                        if (GameObjectData const* batimentData = sObjectMgr->GetGameObjectData(batimentSpawnId))
                        {
                            sObjectMgr->AddGameobjectToGrid(batimentSpawnId, batimentData);
                        }
                        
                        // Initialize destructible state
                        if (batimentGo->GetGoType() == GAMEOBJECT_TYPE_DESTRUCTIBLE_BUILDING)
                        {
                            batimentGo->SetDestructibleState(GO_DESTRUCTIBLE_INTACT, nullptr, true);
                        }
                        
                        // Save to custom tracking table with same group_id as the fort
                        WorldDatabase.Execute("INSERT INTO conquest_build_structures (guid, player_guid, guild_id, entry, map, position_x, position_y, position_z, orientation, build_type, group_id) VALUES ({}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {})",
                            batimentSpawnId, player->GetGUID().GetCounter(), guildId, 400029,
                            player->GetMapId(), batimentX, batimentY, batimentZ, batimentOrientation, CONQUEST_BUILD_WALL, fortGroupId);
                        
                        LOG_INFO("module", "ConquestBuild: Batiment GameObject 400029 spawned successfully with spawnId: {}", batimentSpawnId);
                    }
                    else
                    {
                        delete batimentGo;
                        LOG_ERROR("module", "ConquestBuild: Failed to load batiment GameObject 400029 from DB");
                    }
                }
                else
                {
                    delete batimentGo;
                    LOG_ERROR("module", "ConquestBuild: Failed to create batiment GameObject 400029");
                }
            }
            else
            {
                LOG_ERROR("module", "ConquestBuild: Failed to get groupId. success: {}, groupId: {}", success, fortGroupId);
            }
            
            // Save spell ID and item entry BEFORE destroying the item (item pointer becomes invalid after DestroyItemCount)
            uint32 spellId = 0;
            // itemEntry already declared above
            if (item->GetTemplate() && item->GetTemplate()->Spells[0].SpellId > 0)
            {
                spellId = item->GetTemplate()->Spells[0].SpellId;
            }

            // Remove the item (single use)
            player->DestroyItemCount(itemEntry, 1, true);
            
            // Add 1 second cooldown to prevent spam
            if (spellId > 0)
            {
                player->AddSpellCooldown(spellId, itemEntry, 1000, true, false);
            }
        }

        return true;
    }
};

class item_conquest_build_fort_rempart_3 : public ItemScript
{
public:
    item_conquest_build_fort_rempart_3() : ItemScript("item_conquest_build_fort_rempart_3") { }

    bool OnUse(Player* player, Item* item, SpellCastTargets const& targets) override
    {
        ConquestBuildConfig config = ConquestBuildMgr::instance()->GetConfig();

        if (!config.enabled)
        {
            ChatHandler(player->GetSession()).PSendSysMessage(CONQUEST_BUILD_DISABLED);
            return false;
        }

        // Check if player is in combat
        if (player->IsInCombat())
        {
            ChatHandler(player->GetSession()).SendSysMessage("Vous ne pouvez pas utiliser cet objet en combat!");
            return false;
        }

        // Check if player is in a guild
        if (!player->GetGuildId())
        {
            ChatHandler(player->GetSession()).SendSysMessage("Vous devez être dans une guilde pour placer un fort rempart!");
            return false;
        }

        // Get spawn position from targets (where player clicked)
        float spawnX, spawnY, spawnZ;
        if (targets.GetDstPos())
        {
            WorldLocation const* destLoc = targets.GetDstPos();
            spawnX = destLoc->GetPositionX();
            spawnY = destLoc->GetPositionY();
            spawnZ = destLoc->GetPositionZ();
        }
        else
        {
            // Fallback to player position
            spawnX = player->GetPositionX();
            spawnY = player->GetPositionY();
            spawnZ = player->GetPositionZ();
        }
        
        // Use player orientation for all structures - this is fixed regardless of click position
        float playerOrientation = player->GetOrientation();
        
        // Calculate gate position - spawn at clicked location, but advance 2 yards forward based on player orientation
        float gateX = spawnX + 2.0f * cos(playerOrientation);
        float gateY = spawnY + 2.0f * sin(playerOrientation);
        float gateZ = spawnZ;
        float gateOrientation = playerOrientation; // All structures use player orientation
        
        // Grand fort 3: use CONQUEST_BUILD_GRAND_FORT_3
        uint32 gateEntry = 400025; // Grande herse
        ConquestBuildType buildType = CONQUEST_BUILD_GRAND_FORT_3;
        
        // First, spawn the fort at clicked location
        LOG_INFO("module", "ConquestBuild: Spawning grand fort 3 with grand ramparts for player {} (item entry: 80013)", 
            player->GetName());
        
        uint64 fortGateGuid = 0;
        uint64 fortGroupId = 0;
        bool success = ConquestBuildMgr::instance()->SpawnStructure(player, gateEntry, buildType, &targets, &fortGateGuid, &fortGroupId);
        
        LOG_INFO("module", "ConquestBuild: Fort spawn result: {}, gate position calculated: ({}, {}, {}), guid: {}, groupId: {}", 
            success, gateX, gateY, gateZ, fortGateGuid, fortGroupId);
        
        if (success)
        {
            if (fortGroupId > 0)
            {
                uint32 guildId = player->GetGuildId();
                
                // Récupérer la position réelle du fort depuis la base de données (position du clic)
                float actualGateX = gateX;
                float actualGateY = gateY;
                float actualGateZ = gateZ;
                
                // Essayer de récupérer le GameObject depuis la map
                GameObject* fortGate = nullptr;
                auto bounds = player->GetMap()->GetGameObjectBySpawnIdStore().equal_range(fortGateGuid);
                for (auto itr = bounds.first; itr != bounds.second; ++itr)
                {
                    fortGate = itr->second;
                    if (fortGate && fortGate->GetEntry() == gateEntry)
                    {
                        break;
                    }
                }
                
                if (fortGate)
                {
                    actualGateX = fortGate->GetPositionX();
                    actualGateY = fortGate->GetPositionY();
                    actualGateZ = fortGate->GetPositionZ();
                    LOG_INFO("module", "ConquestBuild: Retrieved actual fort gate position: ({}, {}, {})", 
                        actualGateX, actualGateY, actualGateZ);
                }
                else
                {
                    // Si le GameObject n'est pas encore chargé, récupérer depuis la base de données
                    QueryResult result = WorldDatabase.Query("SELECT position_x, position_y, position_z FROM gameobject WHERE guid = {}", fortGateGuid);
                    if (result)
                    {
                        Field* fields = result->Fetch();
                        actualGateX = fields[0].Get<float>();
                        actualGateY = fields[1].Get<float>();
                        actualGateZ = fields[2].Get<float>();
                        LOG_INFO("module", "ConquestBuild: Retrieved actual fort gate position from DB: ({}, {}, {})", 
                            actualGateX, actualGateY, actualGateZ);
                    }
                }
                
                // Utiliser l'orientation du joueur pour toutes les structures (fixe, indépendante de la position de clic)
                float structuresOrientation = playerOrientation;
                
                // Utiliser la position réelle de la herse pour les remparts
                float rampartGateX = actualGateX;
                float rampartGateY = actualGateY;
                float rampartGateZ = actualGateZ;
                
                LOG_INFO("module", "ConquestBuild: Calling SpawnFortRampartsGrand with groupId: {}", fortGroupId);
                
                // Spawn grand ramparts around the fort using original position (before 3-yard advance) and player orientation
                // Note: actualGateOrientation is rotated 90 degrees, but we use playerOrientation for ramparts
                ConquestBuildMgr::instance()->SpawnFortRampartsGrand(player, rampartGateX, rampartGateY, rampartGateZ, structuresOrientation, fortGroupId, guildId);
                
                LOG_INFO("module", "ConquestBuild: SpawnFortRampartsGrand called successfully");
                
                // Spawn GameObject 400029 - toutes les positions sont calculées par rapport à la position originale du fort
                // Reculer de 15 yards (12 + 3) depuis la herse originale pour les grands forts
                float batimentX = rampartGateX - 19.0f * cos(structuresOrientation) + 1.5f * cos(structuresOrientation - M_PI / 2.0f); // Reculer de 15 yards depuis la herse originale, décaler de 1.5 yard à droite
                float batimentY = rampartGateY - 19.0f * sin(structuresOrientation) + 1.5f * sin(structuresOrientation - M_PI / 2.0f); // Reculer de 15 yards depuis la herse originale, décaler de 1.5 yard à droite
                float batimentZ = rampartGateZ - 3.0f; // Abaisser de 3 yards par rapport au fort
                float batimentOrientation = structuresOrientation - 135.0f * M_PI / 180.0f; // Pivoter de -135 degrés par rapport à l'orientation du joueur
                
                // Rotation de 90 degrés sur l'axe Z (quaternion)
                // Quaternion pour rotation de 90 degrés autour de l'axe Z: (0, 0, sin(45°), cos(45°))
                float angleZ = M_PI / 4.0f; // 45 degrés en radians (la moitié de 90 degrés pour le quaternion)
                G3D::Quat batimentRotation(0, 0, sin(angleZ), cos(angleZ));
                
                LOG_INFO("module", "ConquestBuild: Spawning batiment GameObject 400029 at position ({}, {}, {}) with orientation {} and Z rotation", 
                    batimentX, batimentY, batimentZ, batimentOrientation);
                
                // Create the batiment GameObject
                GameObject* batimentGo = new GameObject();
                ObjectGuid::LowType batimentGuid = player->GetMap()->GenerateLowGuid<HighGuid::GameObject>();
                
                if (batimentGo->Create(batimentGuid, 400029, player->GetMap(), player->GetPhaseMask(),
                                    batimentX, batimentY, batimentZ, batimentOrientation,
                                    batimentRotation, 0, GO_STATE_READY))
                {
                    batimentGo->SaveToDB(player->GetMapId(), 1 << player->GetMap()->GetSpawnMode(), player->GetPhaseMask());
                    ObjectGuid::LowType batimentSpawnId = batimentGo->GetSpawnId();
                    delete batimentGo;
                    
                    // Respawn from DB
                    batimentGo = new GameObject();
                    if (batimentGo->LoadGameObjectFromDB(batimentSpawnId, player->GetMap(), true))
                    {
                        // Add to grid
                        if (GameObjectData const* batimentData = sObjectMgr->GetGameObjectData(batimentSpawnId))
                        {
                            sObjectMgr->AddGameobjectToGrid(batimentSpawnId, batimentData);
                        }
                        
                        // Initialize destructible state
                        if (batimentGo->GetGoType() == GAMEOBJECT_TYPE_DESTRUCTIBLE_BUILDING)
                        {
                            batimentGo->SetDestructibleState(GO_DESTRUCTIBLE_INTACT, nullptr, true);
                        }
                        
                        // Save to custom tracking table with same group_id as the fort
                        WorldDatabase.Execute("INSERT INTO conquest_build_structures (guid, player_guid, guild_id, entry, map, position_x, position_y, position_z, orientation, build_type, group_id) VALUES ({}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {})",
                            batimentSpawnId, player->GetGUID().GetCounter(), guildId, 400029,
                            player->GetMapId(), batimentX, batimentY, batimentZ, batimentOrientation, CONQUEST_BUILD_WALL, fortGroupId);
                        
                        LOG_INFO("module", "ConquestBuild: Batiment GameObject 400029 spawned successfully with spawnId: {}", batimentSpawnId);
                    }
                    else
                    {
                        delete batimentGo;
                        LOG_ERROR("module", "ConquestBuild: Failed to load batiment GameObject 400029 from DB");
                    }
                }
                else
                {
                    delete batimentGo;
                    LOG_ERROR("module", "ConquestBuild: Failed to create batiment GameObject 400029");
                }
                
                // Spawn atelier gobelin 400027 - toutes les positions sont calculées par rapport à la position originale du fort
                // Reculer de 73 yards (70 + 3) depuis la herse originale, puis avancé de 5 yards, puis décalé de 2 yards sur la droite
                float atelierX = rampartGateX - 77.0f * cos(structuresOrientation) + 5.0f * cos(structuresOrientation) + 2.0f * cos(structuresOrientation - M_PI / 2.0f); // 73 yards derrière la herse originale, puis avancé de 5 yards, puis décalé de 2 yards sur la droite
                float atelierY = rampartGateY - 77.0f * sin(structuresOrientation) + 5.0f * sin(structuresOrientation) + 2.0f * sin(structuresOrientation - M_PI / 2.0f); // 73 yards derrière la herse originale, puis avancé de 5 yards, puis décalé de 2 yards sur la droite
                float atelierZ = rampartGateZ; // Même hauteur que la herse originale
                float atelierOrientation = structuresOrientation + M_PI / 2.0f; // Pivoter de 90 degrés vers la gauche par rapport à l'orientation du joueur
                
                LOG_INFO("module", "ConquestBuild: Spawning atelier gobelin 400027 at position ({}, {}, {}) with orientation {}", 
                    atelierX, atelierY, atelierZ, atelierOrientation);
                
                bool atelierSuccess = ConquestBuildMgr::instance()->SpawnSimpleGameObject(player, 400027, atelierX, atelierY, atelierZ, atelierOrientation, fortGroupId, guildId, CONQUEST_BUILD_ATELIER_GOBELIN);
                if (!atelierSuccess)
                {
                    LOG_ERROR("module", "ConquestBuild: Failed to spawn atelier gobelin 400027");
                }
            }
            else
            {
                LOG_ERROR("module", "ConquestBuild: Failed to get groupId. success: {}, groupId: {}", success, fortGroupId);
            }
            
            // Save spell ID and item entry BEFORE destroying the item (item pointer becomes invalid after DestroyItemCount)
            uint32 spellId = 0;
            uint32 itemEntry = item->GetEntry();
            if (item->GetTemplate() && item->GetTemplate()->Spells[0].SpellId > 0)
            {
                spellId = item->GetTemplate()->Spells[0].SpellId;
            }

            // Remove the item (single use)
            player->DestroyItemCount(itemEntry, 1, true);
            
            // Add 1 second cooldown to prevent spam
            if (spellId > 0)
            {
                player->AddSpellCooldown(spellId, itemEntry, 1000, true, false);
            }
        }
        else
        {
            ChatHandler(player->GetSession()).SendSysMessage("Erreur lors de la création du fort.");
            return false;
        }

        return true;
    }
};

// Forward declarations
void AddConquestBuildItemScripts();
void AddConquestBuildGameObjectScripts();

// Add item scripts
void AddConquestBuildItemScripts()
{
    new item_conquest_build_wall();
    new item_conquest_build_tower();
    new item_conquest_build_gate();
    new item_conquest_build_herse();
    new item_conquest_build_fort();
    new item_conquest_build_grand_fort();
    new item_conquest_build_fort_rempart_1();
    new item_conquest_build_fort_rempart_2();
    new item_conquest_build_fort_rempart_3();
    new item_conquest_build_teleport_beacon();
    new item_conquest_build_atelier_gobelin();
    new item_conquest_build_recovery();
}

// Main function to add all scripts (called from loader.cpp)
void AddConquestBuildScripts()
{
    AddConquestBuildItemScripts();
    AddConquestBuildGameObjectScripts();
    AddSC_conquest_marker_spawner();
    AddSC_conquest_recruitment_kit();
}
