/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

#include "ConquestBuildCommon.h"
#include "Item.h"
#include "Spell.h"
#include "GameObjectAI.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "CellImpl.h"
#include "Log.h"
#include "Guild.h"
#include "GuildMgr.h"

void ConquestBuildMgr::LoadConfig()
{
    config.enabled = sConfigMgr->GetOption<bool>("ConquestBuild.Enable", true);
    config.itemId = sConfigMgr->GetOption<uint32>("ConquestBuild.ItemId", 0);
    config.wallGobId = sConfigMgr->GetOption<uint32>("ConquestBuild.WallGobId", 400020);
    config.towerGobId = sConfigMgr->GetOption<uint32>("ConquestBuild.TowerGobId", 400021);
    config.gateGobId = sConfigMgr->GetOption<uint32>("ConquestBuild.GateGobId", 400026);
    config.minDistanceBetweenStructures = sConfigMgr->GetOption<float>("ConquestBuild.MinDistance", 10.0f);
    config.maxStructuresPerPlayer = sConfigMgr->GetOption<uint32>("ConquestBuild.MaxStructuresPerPlayer", 200);
}

bool ConquestBuildMgr::CanBuildAt(Player* player, Position const& /*pos*/)
{
    // Check if too close to other structures - Wall (190397, 400020, 400022)
    GameObject* nearWall = player->FindNearestGameObject(config.wallGobId, config.minDistanceBetweenStructures);
    if (nearWall)
    {
        return false;
    }
    GameObject* nearGrandWall = player->FindNearestGameObject(400020, config.minDistanceBetweenStructures);
    if (nearGrandWall)
    {
        return false;
    }
    GameObject* nearSmallWall = player->FindNearestGameObject(400022, config.minDistanceBetweenStructures);
    if (nearSmallWall)
    {
        return false;
    }

    // Check if too close to other structures - Tower (190398, 400021, 400023)
    GameObject* nearTower = player->FindNearestGameObject(config.towerGobId, config.minDistanceBetweenStructures);
    if (nearTower)
    {
        return false;
    }
    GameObject* nearGrandTower = player->FindNearestGameObject(400021, config.minDistanceBetweenStructures);
    if (nearGrandTower)
    {
        return false;
    }
    GameObject* nearSmallTower = player->FindNearestGameObject(400023, config.minDistanceBetweenStructures);
    if (nearSmallTower)
    {
        return false;
    }

    return true;
}

uint32 ConquestBuildMgr::GetPlayerStructureCount(uint64 playerGuid)
{
    // Only count main structures (walls, towers, gates), not accessories (benches, cannons, levers)
    // Entry 24388 = Stone bench, 34944 = Cannon, 400001 = Lever
    QueryResult result = WorldDatabase.Query(
        "SELECT COUNT(*) FROM conquest_build_structures WHERE player_guid = {} AND entry NOT IN (24388, 34944, 400001)",
        playerGuid);
    if (result)
    {
        Field* fields = result->Fetch();
        uint32 count = fields[0].Get<uint32>();
        return count;
    }
    return 0;
}

void ConquestBuildMgr::SpawnStoneBenchesForTower(Player* player, float towerX, float towerY, float towerZ, float towerOrientation, uint64 groupId, uint32 guildId, float lateralOffset)
{
    const uint32 STONE_BENCH_ENTRY = 24388;
    const uint32 NUM_BENCHES = 18;
    const float BENCH_HEIGHT_STEP = 2.0f; // 2 yards between each bench
    
    // Calculate position behind the tower (relative to tower orientation)
    float behindAngle = towerOrientation + M_PI; // 180 degrees (behind)
    float benchZ = towerZ; // Start at ground level
    
    // Calculate lateral offset direction (perpendicular to behind angle)
    float lateralAngle = towerOrientation + M_PI / 2.0f; // 90 degrees (left side)
    
    for (uint32 i = 0; i < NUM_BENCHES; ++i)
    {
        // Determine distance based on bench index
        float benchDistance;
        if (i < 3)
            benchDistance = 10.0f; // First 3 benches at 10 yards
        else if (i < 6)
            benchDistance = 9.0f;  // Next 3 benches at 9 yards
        else
            benchDistance = 7.0f;  // Remaining 12 benches at 7 yards
        
        // Calculate position for this bench (behind + lateral offset)
        float benchX = towerX + benchDistance * cos(behindAngle) + lateralOffset * cos(lateralAngle);
        float benchY = towerY + benchDistance * sin(behindAngle) + lateralOffset * sin(lateralAngle);
        float currentZ = benchZ + (i * BENCH_HEIGHT_STEP);
        
        GameObject* bench = new GameObject();
        ObjectGuid::LowType benchGuid = player->GetMap()->GenerateLowGuid<HighGuid::GameObject>();
        G3D::Quat benchRotation(0, 0, 0, 1);
        
        if (bench->Create(benchGuid, STONE_BENCH_ENTRY, player->GetMap(), player->GetPhaseMask(),
                        benchX, benchY, currentZ, towerOrientation,
                        benchRotation, 0, GO_STATE_READY))
        {
            bench->SaveToDB(player->GetMapId(), 1 << player->GetMap()->GetSpawnMode(), player->GetPhaseMask());
            ObjectGuid::LowType benchSpawnId = bench->GetSpawnId();
            delete bench;
            
            // Add to grid BEFORE loading from DB
            GameObjectData const* benchData = sObjectMgr->GetGameObjectData(benchSpawnId);
            if (benchData)
            {
                sObjectMgr->AddGameobjectToGrid(benchSpawnId, benchData);
            }
            
            // Respawn from DB
            bench = new GameObject();
            if (bench->LoadGameObjectFromDB(benchSpawnId, player->GetMap(), true))
            {
                // Save bench to database with group_id linking it to the tower
                WorldDatabase.Execute("INSERT INTO conquest_build_structures (guid, player_guid, guild_id, entry, map, position_x, position_y, position_z, orientation, build_type, group_id) VALUES ({}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {})",
                    benchSpawnId, player->GetGUID().GetCounter(), guildId, STONE_BENCH_ENTRY,
                    player->GetMapId(), benchX, benchY, currentZ, towerOrientation, CONQUEST_BUILD_TOWER, groupId);
            }
            else
            {
                if (benchData)
                {
                    sObjectMgr->RemoveGameobjectFromGrid(benchSpawnId, benchData);
                }
                delete bench;
            }
        }
        else
        {
            delete bench;
        }
    }
}

void ConquestBuildMgr::SpawnCannonsForTower(Player* player, float towerX, float towerY, float towerZ, float towerOrientation, uint64 groupId, uint32 guildId)
{
    const uint32 CANNON_ENTRY = 34944;
    const float CANNON_HEIGHT_OFFSET = 35.53454f; // Height difference: 166.94254 - 131.408 = 35.53454 yards above tower base
    const float CANNON_DISTANCE = 6.5f; // 6.5 yards from center
    
    // Calculate cannon Z position relative to tower base
    float cannonZ = towerZ + CANNON_HEIGHT_OFFSET;
    
    // Calculate positions for 3 cannons
    // Canon en face (devant): orientation de la tour
    float frontAngle = towerOrientation;
    float frontX = towerX + CANNON_DISTANCE * cos(frontAngle);
    float frontY = towerY + CANNON_DISTANCE * sin(frontAngle);
    float frontZ = cannonZ;
    float frontOrientation = towerOrientation;
    
    // Canon à gauche: orientation + 90° (M_PI/2)
    float leftAngle = towerOrientation + M_PI / 2.0f;
    float leftX = towerX + CANNON_DISTANCE * cos(leftAngle);
    float leftY = towerY + CANNON_DISTANCE * sin(leftAngle);
    float leftZ = cannonZ;
    float leftOrientation = towerOrientation + M_PI / 2.0f;
    
    // Canon à droite: orientation - 90° (-M_PI/2)
    float rightAngle = towerOrientation - M_PI / 2.0f;
    float rightX = towerX + CANNON_DISTANCE * cos(rightAngle);
    float rightY = towerY + CANNON_DISTANCE * sin(rightAngle);
    float rightZ = cannonZ;
    float rightOrientation = towerOrientation - M_PI / 2.0f;
    
    // Array of cannon positions and orientations
    struct CannonPos {
        float x, y, z, orientation;
    };
    CannonPos cannons[3] = {
        {frontX, frontY, frontZ, frontOrientation},
        {leftX, leftY, leftZ, leftOrientation},
        {rightX, rightY, rightZ, rightOrientation}
    };
    
    for (uint32 i = 0; i < 3; ++i)
    {
        Creature* cannon = new Creature();
        ObjectGuid::LowType cannonGuid = player->GetMap()->GenerateLowGuid<HighGuid::Unit>();
        
        if (cannon->Create(cannonGuid, player->GetMap(), player->GetPhaseMask(), CANNON_ENTRY, 0, 
                        cannons[i].x, cannons[i].y, cannons[i].z, cannons[i].orientation))
        {
            cannon->SaveToDB(player->GetMapId(), 1 << player->GetMap()->GetSpawnMode(), player->GetPhaseMask());
            ObjectGuid::LowType cannonSpawnId = cannon->GetSpawnId();
            delete cannon;
            
            // Add to grid BEFORE loading from DB
            CreatureData const* cannonData = sObjectMgr->GetCreatureData(cannonSpawnId);
            if (cannonData)
            {
                sObjectMgr->AddCreatureToGrid(cannonSpawnId, cannonData);
            }
            
            // Respawn from DB
            cannon = new Creature();
            if (cannon->LoadCreatureFromDB(cannonSpawnId, player->GetMap(), true, true))
            {
                // Set guild name as subname if player is in a guild
                if (guildId > 0)
                {
                    if (Guild* guild = sGuildMgr->GetGuildById(guildId))
                    {
                        cannon->SetCustomSubName(guild->GetName());
                    }
                }
                
                // Save cannon to database with group_id linking it to the tower
                // Note: We use entry = CANNON_ENTRY and build_type = CONQUEST_BUILD_TOWER
                WorldDatabase.Execute("INSERT INTO conquest_build_structures (guid, player_guid, guild_id, entry, map, position_x, position_y, position_z, orientation, build_type, group_id) VALUES ({}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {})",
                    cannonSpawnId, player->GetGUID().GetCounter(), guildId, CANNON_ENTRY,
                    player->GetMapId(), cannons[i].x, cannons[i].y, cannons[i].z, cannons[i].orientation, CONQUEST_BUILD_TOWER, groupId);
            }
            else
            {
                if (cannonData)
                {
                    sObjectMgr->RemoveCreatureFromGrid(cannonSpawnId, cannonData);
                }
                delete cannon;
            }
        }
        else
        {
            delete cannon;
        }
    }
}

void ConquestBuildMgr::SpawnStoneBenchesForSmallTower(Player* player, float towerX, float towerY, float towerZ, float towerOrientation, uint64 groupId, uint32 guildId, float lateralOffset)
{
    const uint32 STONE_BENCH_ENTRY = 24388;
    const uint32 NUM_BENCHES = 10; // 10 benches for small tower (removed last 2)
    const float BENCH_HEIGHT_STEP = 2.0f; // 1.5 yards between each bench (less height)
    
    // Calculate position behind the tower (relative to tower orientation)
    float behindAngle = towerOrientation + M_PI; // 180 degrees (behind)
    // Tower is sunk 9 yards into the ground, so add 9 yards to start benches at ground level
    float benchZ = towerZ + 9.0f; // Start at ground level (tower is -9 yards, so +9 = ground level)
    
    // Calculate lateral offset direction (perpendicular to behind angle)
    float lateralAngle = towerOrientation + M_PI / 2.0f; // 90 degrees (left side)
    
    for (uint32 i = 0; i < NUM_BENCHES; ++i)
    {
        // Determine distance based on bench index (closer to center for small tower)
        float benchDistance;
        if (i < 1)
            benchDistance = 5.0f; // First 2 benches at 6 yards
        else if (i < 2)
            benchDistance = 4.5f; // First bench at 6 yards
        else if (i < 7)
            benchDistance = 4.0f;  // Next 2 benches at 5.5 yards
        else
            benchDistance = 3.7f;  // Remaining 8 benches at 5 yards
        
        // Calculate position for this bench (behind + lateral offset)
        float benchX = towerX + benchDistance * cos(behindAngle) + lateralOffset * cos(lateralAngle);
        float benchY = towerY + benchDistance * sin(behindAngle) + lateralOffset * sin(lateralAngle);
        float currentZ = benchZ + (i * BENCH_HEIGHT_STEP);
        
        GameObject* bench = new GameObject();
        ObjectGuid::LowType benchGuid = player->GetMap()->GenerateLowGuid<HighGuid::GameObject>();
        G3D::Quat benchRotation(0, 0, 0, 1);
        
        if (bench->Create(benchGuid, STONE_BENCH_ENTRY, player->GetMap(), player->GetPhaseMask(),
                        benchX, benchY, currentZ, towerOrientation,
                        benchRotation, 0, GO_STATE_READY))
        {
            bench->SaveToDB(player->GetMapId(), 1 << player->GetMap()->GetSpawnMode(), player->GetPhaseMask());
            ObjectGuid::LowType benchSpawnId = bench->GetSpawnId();
            delete bench;
            
            // Add to grid BEFORE loading from DB
            GameObjectData const* benchData = sObjectMgr->GetGameObjectData(benchSpawnId);
            if (benchData)
            {
                sObjectMgr->AddGameobjectToGrid(benchSpawnId, benchData);
            }
            
            // Respawn from DB
            bench = new GameObject();
            if (bench->LoadGameObjectFromDB(benchSpawnId, player->GetMap(), true))
            {
                // Save bench to database with group_id linking it to the tower
                WorldDatabase.Execute("INSERT INTO conquest_build_structures (guid, player_guid, guild_id, entry, map, position_x, position_y, position_z, orientation, build_type, group_id) VALUES ({}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {})",
                    benchSpawnId, player->GetGUID().GetCounter(), guildId, STONE_BENCH_ENTRY,
                    player->GetMapId(), benchX, benchY, currentZ, towerOrientation, CONQUEST_BUILD_TOWER, groupId);
            }
            else
            {
                if (benchData)
                {
                    sObjectMgr->RemoveGameobjectFromGrid(benchSpawnId, benchData);
                }
                delete bench;
            }
        }
        else
        {
            delete bench;
        }
    }
}

void ConquestBuildMgr::SpawnCannonForSmallTower(Player* player, float towerX, float towerY, float towerZ, float towerOrientation, uint64 groupId, uint32 guildId)
{
    const uint32 CANNON_ENTRY = 34944;
    const float CANNON_HEIGHT_OFFSET = 28.0f; // Height offset for small tower (raised by 3 yards)
    const float CANNON_DISTANCE = 3.0f; // 3 yards forward from center
    
    // Calculate cannon Z position relative to tower base
    float cannonZ = towerZ + CANNON_HEIGHT_OFFSET;
    
    // Single cannon 3 yards forward, facing tower orientation
    float cannonX = towerX + CANNON_DISTANCE * cos(towerOrientation);
    float cannonY = towerY + CANNON_DISTANCE * sin(towerOrientation);
    float cannonOrientation = towerOrientation;
    
    Creature* cannon = new Creature();
    ObjectGuid::LowType cannonGuid = player->GetMap()->GenerateLowGuid<HighGuid::Unit>();
    
    if (cannon->Create(cannonGuid, player->GetMap(), player->GetPhaseMask(), CANNON_ENTRY, 0, 
                    cannonX, cannonY, cannonZ, cannonOrientation))
    {
        cannon->SaveToDB(player->GetMapId(), 1 << player->GetMap()->GetSpawnMode(), player->GetPhaseMask());
        ObjectGuid::LowType cannonSpawnId = cannon->GetSpawnId();
        delete cannon;
        
        // Add to grid BEFORE loading from DB
        CreatureData const* cannonData = sObjectMgr->GetCreatureData(cannonSpawnId);
        if (cannonData)
        {
            sObjectMgr->AddCreatureToGrid(cannonSpawnId, cannonData);
        }
        
        // Respawn from DB
        cannon = new Creature();
        if (cannon->LoadCreatureFromDB(cannonSpawnId, player->GetMap(), true, true))
        {
            // Set guild name as subname if player is in a guild
            if (guildId > 0)
            {
                if (Guild* guild = sGuildMgr->GetGuildById(guildId))
                {
                    cannon->SetCustomSubName(guild->GetName());
                }
            }
            
            // Save cannon to database with group_id linking it to the tower
            WorldDatabase.Execute("INSERT INTO conquest_build_structures (guid, player_guid, guild_id, entry, map, position_x, position_y, position_z, orientation, build_type, group_id) VALUES ({}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {})",
                cannonSpawnId, player->GetGUID().GetCounter(), guildId, CANNON_ENTRY,
                player->GetMapId(), cannonX, cannonY, cannonZ, cannonOrientation, CONQUEST_BUILD_TOWER, groupId);
        }
        else
        {
            if (cannonData)
            {
                sObjectMgr->RemoveCreatureFromGrid(cannonSpawnId, cannonData);
            }
            delete cannon;
        }
    }
    else
    {
        delete cannon;
    }
}

void ConquestBuildMgr::SpawnLeversForGate(Player* player, float gateX, float gateY, float gateZ, float gateOrientation, uint64 groupId, uint32 guildId, ConquestBuildType buildType)
{
    // Spawn 2 levers on the RIGHT side of the gate, spaced along the gate axis
    // Gate is rotated 90 degrees, so levers should be on the right (perpendicular axis)
    float leverSideDistance = 8.0f; // 8 yards to the right of the gate
    float leverSpacing = 6.0f; // 6 yards spacing along the gate axis
    float perpAngle = gateOrientation + M_PI / 2.0f; // Perpendicular angle
    
    // Pour les remparts (CONQUEST_BUILD_GATE_SMALL), placer un levier à l'extérieur et l'autre à l'intérieur
    bool isRampartGate = (buildType == CONQUEST_BUILD_GATE_SMALL);
    
    float lever1X, lever1Y, lever2X, lever2Y;
    
    if (isRampartGate)
    {
        // Lever 1 à l'extérieur (côté droit de la herse, vers l'extérieur du fort)
        lever1X = gateX - leverSideDistance * cos(perpAngle);
        lever1Y = gateY - leverSideDistance * sin(perpAngle);
        
        // Lever 2 à l'intérieur (côté gauche de la herse, vers l'intérieur du fort)
        lever2X = gateX + leverSideDistance * cos(perpAngle);
        lever2Y = gateY + leverSideDistance * sin(perpAngle);
    }
    else
    {
        // Lever 1 position (right side, slightly forward)
        lever1X = gateX - leverSideDistance * cos(perpAngle) + leverSpacing * cos(gateOrientation);
        lever1Y = gateY - leverSideDistance * sin(perpAngle) + leverSpacing * sin(gateOrientation);
        
        // Lever 2 position (right side, slightly backward)
        lever2X = gateX - leverSideDistance * cos(perpAngle) - leverSpacing * cos(gateOrientation);
        lever2Y = gateY - leverSideDistance * sin(perpAngle) - leverSpacing * sin(gateOrientation);
    }
    
    float lever1Z = gateZ;
    float lever2Z = gateZ;
    
    G3D::Quat towerRotation(0, 0, 0, 1);
    
    // Spawn LEVER 1 (right side, forward)
    GameObject* lever1 = new GameObject();
    ObjectGuid::LowType lever1Guid = player->GetMap()->GenerateLowGuid<HighGuid::GameObject>();
    
    if (lever1->Create(lever1Guid, 400001, player->GetMap(), player->GetPhaseMask(),
                    lever1X, lever1Y, lever1Z, gateOrientation,
                    towerRotation, 0, GO_STATE_READY))
    {
        lever1->SaveToDB(player->GetMapId(), 1 << player->GetMap()->GetSpawnMode(), player->GetPhaseMask());
        ObjectGuid::LowType lever1SpawnId = lever1->GetSpawnId();
        delete lever1;
        
        // Add to grid BEFORE loading from DB
        GameObjectData const* lever1Data = sObjectMgr->GetGameObjectData(lever1SpawnId);
        if (lever1Data)
        {
            sObjectMgr->AddGameobjectToGrid(lever1SpawnId, lever1Data);
        }
        
        // Respawn from DB
        lever1 = new GameObject();
        if (lever1->LoadGameObjectFromDB(lever1SpawnId, player->GetMap(), true))
        {
            // Save lever to database with group_id
            WorldDatabase.Execute("INSERT INTO conquest_build_structures (guid, player_guid, guild_id, entry, map, position_x, position_y, position_z, orientation, build_type, group_id) VALUES ({}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {})",
                lever1SpawnId, player->GetGUID().GetCounter(), guildId, 400001,
                player->GetMapId(), lever1X, lever1Y, lever1Z, gateOrientation, buildType, groupId);
        }
        else
        {
            if (lever1Data)
            {
                sObjectMgr->RemoveGameobjectFromGrid(lever1SpawnId, lever1Data);
            }
            delete lever1;
        }
    }
    else
    {
        delete lever1;
    }
    
    // Spawn LEVER 2 (right side, backward)
    GameObject* lever2 = new GameObject();
    ObjectGuid::LowType lever2Guid = player->GetMap()->GenerateLowGuid<HighGuid::GameObject>();
    
    if (lever2->Create(lever2Guid, 400001, player->GetMap(), player->GetPhaseMask(),
                    lever2X, lever2Y, lever2Z, gateOrientation,
                    towerRotation, 0, GO_STATE_READY))
    {
        lever2->SaveToDB(player->GetMapId(), 1 << player->GetMap()->GetSpawnMode(), player->GetPhaseMask());
        ObjectGuid::LowType lever2SpawnId = lever2->GetSpawnId();
        delete lever2;
        
        // Add to grid BEFORE loading from DB
        GameObjectData const* lever2Data = sObjectMgr->GetGameObjectData(lever2SpawnId);
        if (lever2Data)
        {
            sObjectMgr->AddGameobjectToGrid(lever2SpawnId, lever2Data);
        }
        
        // Respawn from DB
        lever2 = new GameObject();
        if (lever2->LoadGameObjectFromDB(lever2SpawnId, player->GetMap(), true))
        {
            // Save lever to database with group_id
            WorldDatabase.Execute("INSERT INTO conquest_build_structures (guid, player_guid, guild_id, entry, map, position_x, position_y, position_z, orientation, build_type, group_id) VALUES ({}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {})",
                lever2SpawnId, player->GetGUID().GetCounter(), guildId, 400001,
                player->GetMapId(), lever2X, lever2Y, lever2Z, gateOrientation, buildType, groupId);
        }
        else
        {
            if (lever2Data)
            {
                sObjectMgr->RemoveGameobjectFromGrid(lever2SpawnId, lever2Data);
            }
            delete lever2;
        }
    }
    else
    {
        delete lever2;
    }
}

bool ConquestBuildMgr::SpawnStructure(Player* player, uint32 gobEntry, ConquestBuildType buildType, SpellCastTargets const* targets, uint64* outGuidLow, uint64* outGroupId)
{
    if (!config.enabled)
    {
        ChatHandler(player->GetSession()).PSendSysMessage(CONQUEST_BUILD_DISABLED);
        return false;
    }

    // Check player structure limit
    if (GetPlayerStructureCount(player->GetGUID().GetCounter()) >= config.maxStructuresPerPlayer)
    {
        ChatHandler(player->GetSession()).PSendSysMessage(CONQUEST_BUILD_MAX_REACHED);
        return false;
    }

    // Get position from spell target or default position
    float x, y, z;
    float maxDistance = 40.0f;
    
    if (targets && targets->GetDstPos())
    {
        WorldLocation const* destLoc = targets->GetDstPos();
        x = destLoc->GetPositionX();
        y = destLoc->GetPositionY();
        z = destLoc->GetPositionZ();
        
        float distance = player->GetDistance(x, y, z);
        if (distance > maxDistance)
        {
            ChatHandler(player->GetSession()).PSendSysMessage("Trop loin! Distance maximum: 40 yards.");
            return false;
        }
    }
    else
    {
        player->GetClosePoint(x, y, z, 0.0f, 5.0f);
    }
    
    Position pos;
    pos.Relocate(x, y, z, player->GetOrientation());

    if (!CanBuildAt(player, pos))
    {
        ChatHandler(player->GetSession()).PSendSysMessage(CONQUEST_BUILD_TOO_CLOSE);
        return false;
    }

    GameObjectTemplate const* goInfo = sObjectMgr->GetGameObjectTemplate(gobEntry);
    if (!goInfo)
    {
        ChatHandler(player->GetSession()).PSendSysMessage("Erreur: Template du game object {} introuvable. Redémarrez le serveur après avoir ajouté le template.", gobEntry);
        return false;
    }
    
    // Prepare spawn parameters
    SpawnParams params;
    params.player = player;
    params.gobEntry = gobEntry;
    params.buildType = buildType;
    params.x = x;
    params.y = y;
    params.z = z;
    params.orientation = player->GetOrientation();
    params.guildId = player->GetGuildId();
    params.guidLow = player->GetMap()->GenerateLowGuid<HighGuid::GameObject>();
    params.groupId = 0;
    
    bool success = false;
    
    // Call appropriate spawn function based on build type
    switch (buildType)
    {
        case CONQUEST_BUILD_WALL:
            success = SpawnWall(params);
            break;
            
        case CONQUEST_BUILD_TOWER:
            success = SpawnTower(params);
            break;
            
        case CONQUEST_BUILD_GATE:
            success = SpawnGate(params, false);
            break;
            
        case CONQUEST_BUILD_GATE_SMALL:
            success = SpawnGate(params, true);
            break;
            
        case CONQUEST_BUILD_FORT:
            success = SpawnFort(params, false, false);
            break;
            
        case CONQUEST_BUILD_GRAND_FORT:
            success = SpawnFort(params, true, false);
            break;
            
        case CONQUEST_BUILD_GRAND_FORT_2:
            success = SpawnFort(params, true, true);
            break;
            
        case CONQUEST_BUILD_GRAND_FORT_3:
            success = SpawnFort(params, true, false);
            break;
            
        case CONQUEST_BUILD_TELEPORT_BEACON:
        case CONQUEST_BUILD_ATELIER_GOBELIN:
            success = SpawnSimpleStructure(params);
            break;
            
        default:
            ChatHandler(player->GetSession()).PSendSysMessage("Type de construction non supporté.");
            return false;
    }
    
    if (!success)
    {
        return false;
    }

    // Return guidLow and groupId via output parameters if provided
    if (outGuidLow)
        *outGuidLow = params.guidLow;
    if (outGroupId)
        *outGroupId = params.groupId;

    ChatHandler(player->GetSession()).PSendSysMessage(CONQUEST_BUILD_SUCCESS);
    return true;
}

// Helper function to create and save a GameObject
bool ConquestBuildMgr::CreateAndSaveGameObject(SpawnParams& params, float finalX, float finalY, float finalZ, float finalOrientation)
{
    GameObject* go = new GameObject();
    G3D::Quat rotation(0, 0, 0, 1);
    
    if (!go->Create(params.guidLow, params.gobEntry, params.player->GetMap(), params.player->GetPhaseMask(),
                    finalX, finalY, finalZ, finalOrientation,
                    rotation, 0, GO_STATE_READY))
    {
        delete go;
        ChatHandler(params.player->GetSession()).PSendSysMessage(CONQUEST_BUILD_ERROR);
        return false;
    }

    go->SaveToDB(params.player->GetMapId(), 1 << params.player->GetMap()->GetSpawnMode(), params.player->GetPhaseMask());
    params.guidLow = go->GetSpawnId();
    delete go;
    
    GameObjectData const* goData = sObjectMgr->GetGameObjectData(params.guidLow);
    if (!goData)
    {
        ChatHandler(params.player->GetSession()).PSendSysMessage(CONQUEST_BUILD_ERROR);
        return false;
    }
    
    sObjectMgr->AddGameobjectToGrid(params.guidLow, goData);

    go = new GameObject();
    if (!go->LoadGameObjectFromDB(params.guidLow, params.player->GetMap(), true))
    {
        sObjectMgr->RemoveGameobjectFromGrid(params.guidLow, goData);
        delete go;
        ChatHandler(params.player->GetSession()).PSendSysMessage(CONQUEST_BUILD_ERROR);
        return false;
    }

    if (go->GetGoType() == GAMEOBJECT_TYPE_DESTRUCTIBLE_BUILDING)
    {
        go->SetDestructibleState(GO_DESTRUCTIBLE_INTACT, nullptr, true);
    }

    WorldDatabase.Execute("INSERT INTO conquest_build_structures (guid, player_guid, guild_id, entry, map, position_x, position_y, position_z, orientation, build_type, group_id) VALUES ({}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {})",
        params.guidLow,
        params.player->GetGUID().GetCounter(),
        params.guildId,
        params.gobEntry,
        params.player->GetMapId(),
        finalX, finalY, finalZ,
        finalOrientation,
        params.buildType,
        params.groupId);

    return true;
}

// Spawn a simple wall
bool ConquestBuildMgr::SpawnWall(SpawnParams& params)
{
    float finalZ = params.z;
    if (params.gobEntry == 400022) // Small wall
    {
        finalZ -= 4.0f;
    }
    
    params.groupId = 0;
    return CreateAndSaveGameObject(params, params.x, params.y, finalZ, params.orientation);
}

// Spawn a tower with benches and cannons
bool ConquestBuildMgr::SpawnTower(SpawnParams& params)
{
    float finalZ = params.z;
    if (params.gobEntry == 400023) // Small tower
    {
        finalZ -= 8.0f;
    }
    
    params.groupId = params.guidLow;
    
    if (!CreateAndSaveGameObject(params, params.x, params.y, finalZ, params.orientation))
    {
        return false;
    }
    
    if (params.gobEntry == 400023)
    {
        SpawnStoneBenchesForSmallTower(params.player, params.x, params.y, finalZ, params.orientation, params.groupId, params.guildId, 0.0f);
        SpawnCannonForSmallTower(params.player, params.x, params.y, finalZ, params.orientation, params.groupId, params.guildId);
        }
        else
        {
        SpawnStoneBenchesForTower(params.player, params.x, params.y, finalZ, params.orientation, params.groupId, params.guildId, 0.0f);
        SpawnCannonsForTower(params.player, params.x, params.y, finalZ, params.orientation, params.groupId, params.guildId);
    }
    
    return true;
}

// Spawn a gate system (2 towers + gate + 2 levers)
bool ConquestBuildMgr::SpawnGate(SpawnParams& params, bool isSmallGate)
{
    float gateOrientation = params.orientation + M_PI / 2.0f; // Rotate 90 degrees
    
    params.groupId = params.guidLow;
    
    if (!CreateAndSaveGameObject(params, params.x, params.y, params.z, gateOrientation))
    {
        return false;
    }
    
    // Calculate perpendicular direction (90 degrees from player orientation)
    float perpAngle = params.orientation + M_PI / 2.0f;
    float towerDistance = isSmallGate ? 10.3f : 20.0f;
    uint32 towerEntry = isSmallGate ? 400023 : config.towerGobId;
        
        // Tower LEFT position
    float leftX = params.x + towerDistance * cos(perpAngle);
    float leftY = params.y + towerDistance * sin(perpAngle);
    float leftZ = params.z;
        
        // Tower RIGHT position  
    float rightX = params.x - towerDistance * cos(perpAngle);
    float rightY = params.y - towerDistance * sin(perpAngle);
    float rightZ = params.z;
    
    if (isSmallGate)
        {
            leftZ -= 8.0f;
            rightZ -= 8.0f;
        }
        
        // Spawn LEFT tower
        GameObject* leftTower = new GameObject();
    ObjectGuid::LowType leftGuid = params.player->GetMap()->GenerateLowGuid<HighGuid::GameObject>();
        G3D::Quat towerRotation(0, 0, 0, 1);
        
    if (leftTower->Create(leftGuid, towerEntry, params.player->GetMap(), params.player->GetPhaseMask(),
                        leftX, leftY, leftZ, params.orientation,
                            towerRotation, 0, GO_STATE_READY))
        {
        leftTower->SaveToDB(params.player->GetMapId(), 1 << params.player->GetMap()->GetSpawnMode(), params.player->GetPhaseMask());
            ObjectGuid::LowType leftSpawnId = leftTower->GetSpawnId();
            delete leftTower;
            
            leftTower = new GameObject();
        if (leftTower->LoadGameObjectFromDB(leftSpawnId, params.player->GetMap(), true))
            {
                if (GameObjectData const* leftTowerData = sObjectMgr->GetGameObjectData(leftSpawnId))
                {
                    sObjectMgr->AddGameobjectToGrid(leftSpawnId, leftTowerData);
                }
                
                WorldDatabase.Execute("INSERT INTO conquest_build_structures (guid, player_guid, guild_id, entry, map, position_x, position_y, position_z, orientation, build_type, group_id) VALUES ({}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {})",
                leftSpawnId, params.player->GetGUID().GetCounter(), params.guildId, towerEntry,
                params.player->GetMapId(), leftX, leftY, leftZ, params.orientation, CONQUEST_BUILD_TOWER, params.groupId);
                
            if (isSmallGate)
                {
                SpawnStoneBenchesForSmallTower(params.player, leftX, leftY, leftZ, params.orientation, params.groupId, params.guildId, 0.0f);
                SpawnCannonForSmallTower(params.player, leftX, leftY, leftZ, params.orientation, params.groupId, params.guildId);
                }
                else
                {
                SpawnStoneBenchesForTower(params.player, leftX, leftY, leftZ, params.orientation, params.groupId, params.guildId, 0.0f);
                SpawnCannonsForTower(params.player, leftX, leftY, leftZ, params.orientation, params.groupId, params.guildId);
                }
            }
            else
            {
                delete leftTower;
            }
        }
        else
        {
            delete leftTower;
        }
        
        // Spawn RIGHT tower
        GameObject* rightTower = new GameObject();
    ObjectGuid::LowType rightGuid = params.player->GetMap()->GenerateLowGuid<HighGuid::GameObject>();
        
    if (rightTower->Create(rightGuid, towerEntry, params.player->GetMap(), params.player->GetPhaseMask(),
                        rightX, rightY, rightZ, params.orientation,
                            towerRotation, 0, GO_STATE_READY))
        {
        rightTower->SaveToDB(params.player->GetMapId(), 1 << params.player->GetMap()->GetSpawnMode(), params.player->GetPhaseMask());
            ObjectGuid::LowType rightSpawnId = rightTower->GetSpawnId();
            delete rightTower;
            
            rightTower = new GameObject();
        if (rightTower->LoadGameObjectFromDB(rightSpawnId, params.player->GetMap(), true))
            {
                if (GameObjectData const* rightTowerData = sObjectMgr->GetGameObjectData(rightSpawnId))
                {
                    sObjectMgr->AddGameobjectToGrid(rightSpawnId, rightTowerData);
                }
                
                WorldDatabase.Execute("INSERT INTO conquest_build_structures (guid, player_guid, guild_id, entry, map, position_x, position_y, position_z, orientation, build_type, group_id) VALUES ({}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {})",
                rightSpawnId, params.player->GetGUID().GetCounter(), params.guildId, towerEntry,
                params.player->GetMapId(), rightX, rightY, rightZ, params.orientation, CONQUEST_BUILD_TOWER, params.groupId);
                
            if (isSmallGate)
                {
                SpawnStoneBenchesForSmallTower(params.player, rightX, rightY, rightZ, params.orientation, params.groupId, params.guildId, 0.0f);
                SpawnCannonForSmallTower(params.player, rightX, rightY, rightZ, params.orientation, params.groupId, params.guildId);
                }
                else
                {
                SpawnStoneBenchesForTower(params.player, rightX, rightY, rightZ, params.orientation, params.groupId, params.guildId, 0.0f);
                SpawnCannonsForTower(params.player, rightX, rightY, rightZ, params.orientation, params.groupId, params.guildId);
                }
            }
            else
            {
                delete rightTower;
            }
        }
        else
        {
            delete rightTower;
        }
        
    // Spawn 2 levers
    SpawnLeversForGate(params.player, params.x, params.y, params.z, params.orientation, params.groupId, params.guildId, params.buildType);
    
    return true;
}

// Spawn a simple structure (teleport beacon, atelier gobelin, etc.)
bool ConquestBuildMgr::SpawnSimpleStructure(SpawnParams& params)
{
    float finalOrientation = params.orientation;
    
    if (params.gobEntry == 400024) // Teleport beacon
    {
        finalOrientation -= M_PI; // Rotate -180 degrees
    }
    else if (params.gobEntry == 400027) // Atelier gobelin
    {
        finalOrientation += M_PI / 2.0f; // Rotate 90 degrees
    }
    
    params.groupId = 0;
    return CreateAndSaveGameObject(params, params.x, params.y, params.z, finalOrientation);
}

// Spawn a fort (complex structure with ramparts)
bool ConquestBuildMgr::SpawnFort(SpawnParams& params, bool isGrandFort, bool isGrandFort2)
{
    float gateX = params.x;
    float gateY = params.y;
    float gateZ = params.z;
    
    // Advance gate 2 yards forward (same for all forts)
    gateX += 2.0f * cos(params.orientation);
    gateY += 2.0f * sin(params.orientation);
    
    // For kits 80012 and 80013 (grand forts), rotate gate 90 degrees from player orientation
    // For other forts, rotate 90 degrees (original behavior)
    float gateOrientation = params.orientation + M_PI / 2.0f; // Always rotate 90 degrees
    
    params.groupId = params.guidLow;
    
    LOG_INFO("module", "ConquestBuild: SpawnFort - Creating fort gate at ({}, {}, {}) with orientation {}, entry: {}, isGrandFort: {}, isGrandFort2: {}", 
        gateX, gateY, gateZ, gateOrientation, params.gobEntry, isGrandFort, isGrandFort2);
    
    // Create the main gate GameObject
    if (!CreateAndSaveGameObject(params, gateX, gateY, gateZ, gateOrientation))
    {
        LOG_ERROR("module", "ConquestBuild: SpawnFort - Failed to create and save GameObject");
        return false;
    }
    
    LOG_INFO("module", "ConquestBuild: SpawnFort - Successfully created fort gate with guid: {}, groupId: {}", params.guidLow, params.groupId);
    
    // Spawn the main fort structures (gate towers, side walls, back towers, levers)
    SpawnFortMainStructures(params.player, gateX, gateY, gateZ, gateOrientation, params.groupId, params.guildId, params.buildType);
    
    // Note: Ramparts are spawned separately via SpawnFortRamparts called from item script
    
    return true;
}

void ConquestBuildMgr::RemoveStructure(uint64 goSpawnId, Map* map)
{
    // Check if this structure is part of a group (fortification system)
    QueryResult result = WorldDatabase.Query("SELECT group_id, map FROM conquest_build_structures WHERE guid = {}", goSpawnId);
    if (result)
    {
        Field* fields = result->Fetch();
        uint64 groupId = fields[0].Get<uint64>();
        // mapId is retrieved but not currently used - may be needed for future validation
        (void)fields[1].Get<uint32>(); // Suppress unused variable warning
        
        // If it's part of a group, delete ALL structures in the group
        if (groupId > 0)
        {
            // Remove fires for all structures in the group first
            QueryResult groupStructuresResult = WorldDatabase.Query("SELECT guid FROM conquest_build_structures WHERE group_id = {} AND entry NOT IN (179065, 179085, 24388, 34944, 400001)", groupId);
            if (groupStructuresResult)
            {
                do
                {
                    Field* structFields = groupStructuresResult->Fetch();
                    uint64 structGuid = structFields[0].Get<uint64>();
                    RemoveFiresForStructure(structGuid, map);
                } while (groupStructuresResult->NextRow());
            }

            // Get all GUIDs and entries in this group
            QueryResult groupResult = WorldDatabase.Query("SELECT guid, entry FROM conquest_build_structures WHERE group_id = {}", groupId);
            if (groupResult)
            {
                do
                {
                    Field* groupFields = groupResult->Fetch();
                    uint64 groupGuid = groupFields[0].Get<uint64>();
                    uint32 groupEntry = groupFields[1].Get<uint32>();
                    
                    // Check if it's a GameObject or Creature
                    if (groupEntry == 34944) // Cannon (Creature)
                    {
                        // Delete from world creature table
                        WorldDatabase.Execute("DELETE FROM creature WHERE guid = {}", groupGuid);
                        
                        // If we have a map reference, try to delete the Creature from the world
                        if (map)
                        {
                            auto bounds = map->GetCreatureBySpawnIdStore().equal_range(groupGuid);
                            for (auto itr = bounds.first; itr != bounds.second; ++itr)
                            {
                                Creature* creature = itr->second;
                                if (creature)
                                {
                                    creature->AddObjectToRemoveList();
                                }
                            }
                        }
                    }
                    else // GameObject (bench, tower, gate, lever, etc.)
                    {
                        // Delete from world gameobject table
                        WorldDatabase.Execute("DELETE FROM gameobject WHERE guid = {}", groupGuid);
                        
                        // If we have a map reference, try to delete the GameObject from the world
                        if (map)
                        {
                            auto bounds = map->GetGameObjectBySpawnIdStore().equal_range(groupGuid);
                            for (auto itr = bounds.first; itr != bounds.second; ++itr)
                            {
                                GameObject* gobj = itr->second;
                                if (gobj)
                                {
                                    gobj->SetRespawnTime(0);
                                    gobj->Delete();
                                }
                            }
                        }
                    }
                } while (groupResult->NextRow());
            }
            
            // Delete all entries from tracking table
            WorldDatabase.Execute("DELETE FROM conquest_build_structures WHERE group_id = {}", groupId);
        }
    }
    
    // Remove fires for the central structure before deleting it
    RemoveFiresForStructure(goSpawnId, map);
    
    // Also delete the central piece itself
    WorldDatabase.Execute("DELETE FROM gameobject WHERE guid = {}", goSpawnId);
    WorldDatabase.Execute("DELETE FROM conquest_build_structures WHERE guid = {}", goSpawnId);
    
    // Delete from world if map is provided
    if (map)
    {
        auto bounds = map->GetGameObjectBySpawnIdStore().equal_range(goSpawnId);
        for (auto itr = bounds.first; itr != bounds.second; ++itr)
        {
            GameObject* gobj = itr->second;
            if (gobj)
            {
                gobj->SetRespawnTime(0);
                gobj->Delete();
            }
        }
    }
}

void ConquestBuildMgr::CheckAndSpawnFires(GameObject* structure, int32 healthChange)
{
    LOG_ERROR("server", "[ConquestBuild] CheckAndSpawnFires called for entry: {}, healthChange: {}", structure ? structure->GetEntry() : 0, healthChange);
    
    if (!structure || structure->GetGoType() != GAMEOBJECT_TYPE_DESTRUCTIBLE_BUILDING)
    {
        LOG_ERROR("server", "[ConquestBuild] CheckAndSpawnFires: Invalid structure or not destructible");
        return;
    }

    uint32 currentHealth = structure->GetGOValue()->Building.Health;
    uint32 maxHealth = structure->GetGOValue()->Building.MaxHealth;

    LOG_ERROR("server", "[ConquestBuild] CheckAndSpawnFires: currentHealth={}, maxHealth={}", currentHealth, maxHealth);

    if (maxHealth == 0)
    {
        LOG_ERROR("server", "[ConquestBuild] CheckAndSpawnFires: maxHealth is 0");
        return;
    }

    // Calculate new health after the change (OnModifyHealth is called before health is updated)
    int32 newHealth = (int32)currentHealth + healthChange;
    if (newHealth < 0)
        newHealth = 0;
    else if (newHealth > (int32)maxHealth)
        newHealth = maxHealth;

    LOG_ERROR("server", "[ConquestBuild] CheckAndSpawnFires: newHealth={}, threshold={}", newHealth, maxHealth / 2);

    // Check if new health is below 50%
    if (newHealth >= (int32)(maxHealth / 2))
    {
        LOG_ERROR("server", "[ConquestBuild] CheckAndSpawnFires: Health >= 50%, no fires needed");
        return; // Health will be >= 50%, no fires needed
    }

    // Check if fires already exist for this structure (both in DB and in world)
    ObjectGuid::LowType structureSpawnId = structure->GetSpawnId();
    QueryResult existingFires = WorldDatabase.Query(
        "SELECT guid FROM conquest_build_structures WHERE group_id = {} AND entry IN (179065, 179085)",
        structureSpawnId
    );
  
    if (existingFires)
    {
        // Verify that fires actually exist in the world (not just in DB)
        bool firesExistInWorld = false;
        do
        {
            Field* fields = existingFires->Fetch();
            uint64 fireGuid = fields[0].Get<uint64>();
            
            // Check if fire exists in the world
            auto bounds = structure->GetMap()->GetGameObjectBySpawnIdStore().equal_range(fireGuid);
            if (bounds.first != bounds.second)
            {
                firesExistInWorld = true;
                LOG_ERROR("server", "[ConquestBuild] CheckAndSpawnFires: Fire {} exists in world", fireGuid);
                break;
            }
        } while (existingFires->NextRow());
        
        if (firesExistInWorld)
        {
            LOG_ERROR("server", "[ConquestBuild] CheckAndSpawnFires: Fires already exist in world");
            return; // Fires already exist in world
        }
        else
        {
            LOG_ERROR("server", "[ConquestBuild] CheckAndSpawnFires: Fires exist in DB but not in world, cleaning up and respawning");
            // Clean up orphaned fire entries from DB
            WorldDatabase.Execute(
                "DELETE FROM conquest_build_structures WHERE group_id = {} AND entry IN (179065, 179085)",
                structureSpawnId
            );
            // Continue to spawn fires
        }
    }

    LOG_ERROR("server", "[ConquestBuild] CheckAndSpawnFires: Proceeding to spawn fires for entry: {}", structure->GetEntry());

    uint32 goEntry = structure->GetEntry();
    float structureX = structure->GetPositionX();
    float structureY = structure->GetPositionY();
    float structureZ = structure->GetPositionZ();
    float structureOrientation = structure->GetOrientation();
    Map* map = structure->GetMap();
    uint32 mapId = structure->GetMapId();
    uint32 phaseMask = structure->GetPhaseMask();

    // Get player_guid and guild_id from structure
    QueryResult structureData = WorldDatabase.Query(
        "SELECT player_guid, guild_id FROM conquest_build_structures WHERE guid = {}",
        structureSpawnId
    );

    if (!structureData)
        return;

    Field* fields = structureData->Fetch();
    uint64 playerGuid = fields[0].Get<uint64>();
    uint32 guildId = fields[1].Get<uint32>();

    // Determine fire configuration based on structure type
    bool isSmallStructure = (goEntry == 400022 || goEntry == 400023); // Mur Conquest or Tour Conquest
    bool isLargeStructure = (goEntry == 400020 || goEntry == 400021 || goEntry == 400024); // Grand Mur, Grande Tour, or Balise

    if (isSmallStructure)
    {
        // Two fires (179065): one 5 yards forward, one 5 yards backward (10 yards total separation)
        float forwardX = structureX + 4.0f * cos(structureOrientation);
        float forwardY = structureY + 4.0f * sin(structureOrientation);
        float backwardX = structureX - 4.0f * cos(structureOrientation);
        float backwardY = structureY - 4.0f * sin(structureOrientation);
        
        LOG_ERROR("server", "[ConquestBuild] CheckAndSpawnFires: Spawning fires for small structure (entry: {})", goEntry);
        LOG_ERROR("server", "[ConquestBuild] CheckAndSpawnFires: Forward fire at ({}, {}, {}), Backward fire at ({}, {}, {})", 
            forwardX, forwardY, structureZ - 1.0f, backwardX, backwardY, structureZ - 1.0f);

        // Forward fire
        LOG_ERROR("server", "[ConquestBuild] CheckAndSpawnFires: Creating forward fire for small structure");
        GameObject* fireForward = new GameObject();
        ObjectGuid::LowType fireForwardGuid = map->GenerateLowGuid<HighGuid::GameObject>();
        G3D::Quat fireRotation(0, 0, 0, 1);

        if (fireForward->Create(fireForwardGuid, 179065, map, phaseMask,
                               forwardX, forwardY, structureZ - 1.0f, structureOrientation,
                               fireRotation, 0, GO_STATE_READY))
        {
            LOG_ERROR("server", "[ConquestBuild] CheckAndSpawnFires: Forward fire for small structure created successfully");
            fireForward->SaveToDB(mapId, 1 << map->GetSpawnMode(), phaseMask);
            ObjectGuid::LowType fireForwardSpawnId = fireForward->GetSpawnId();
            delete fireForward;

            GameObjectData const* fireForwardData = sObjectMgr->GetGameObjectData(fireForwardSpawnId);
            if (fireForwardData)
            {
                sObjectMgr->AddGameobjectToGrid(fireForwardSpawnId, fireForwardData);
            }

            fireForward = new GameObject();
            if (fireForward->LoadGameObjectFromDB(fireForwardSpawnId, map, true))
            {
                LOG_ERROR("server", "[ConquestBuild] CheckAndSpawnFires: Forward fire for small structure loaded successfully, spawnId: {}", fireForwardSpawnId);
                // Force update visibility to ensure clients see the fire
                fireForward->UpdateObjectVisibility(true);
                // Verify fire is in world
                auto fireBounds = map->GetGameObjectBySpawnIdStore().equal_range(fireForwardSpawnId);
                if (fireBounds.first != fireBounds.second)
                {
                    LOG_ERROR("server", "[ConquestBuild] CheckAndSpawnFires: Forward fire for small structure is in world map, spawnId: {}", fireForwardSpawnId);
                }
                else
                {
                    LOG_ERROR("server", "[ConquestBuild] CheckAndSpawnFires: WARNING - Forward fire for small structure NOT in world map, spawnId: {}", fireForwardSpawnId);
                }
                WorldDatabase.Execute(
                    "INSERT INTO conquest_build_structures (guid, player_guid, guild_id, entry, map, position_x, position_y, position_z, orientation, build_type, group_id) VALUES ({}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {})",
                    fireForwardSpawnId, playerGuid, guildId, 179065, mapId,
                    forwardX, forwardY, structureZ - 1.0f, structureOrientation,
                    CONQUEST_BUILD_WALL, structureSpawnId
                );
                LOG_ERROR("server", "[ConquestBuild] CheckAndSpawnFires: Forward fire for small structure saved to tracking table");
            }
            else
            {
                LOG_ERROR("server", "[ConquestBuild] CheckAndSpawnFires: Failed to load forward fire for small structure from DB, spawnId: {}", fireForwardSpawnId);
                if (fireForwardData)
                {
                    sObjectMgr->RemoveGameobjectFromGrid(fireForwardSpawnId, fireForwardData);
                }
                delete fireForward;
            }
        }
        else
        {
            LOG_ERROR("server", "[ConquestBuild] CheckAndSpawnFires: Failed to create forward fire for small structure");
            delete fireForward;
        }

        // Backward fire
        LOG_ERROR("server", "[ConquestBuild] CheckAndSpawnFires: Creating backward fire for small structure");
        GameObject* fireBackward = new GameObject();
        ObjectGuid::LowType fireBackwardGuid = map->GenerateLowGuid<HighGuid::GameObject>();

        if (fireBackward->Create(fireBackwardGuid, 179065, map, phaseMask,
                                backwardX, backwardY, structureZ - 1.0f, structureOrientation,
                                fireRotation, 0, GO_STATE_READY))
        {
            LOG_ERROR("server", "[ConquestBuild] CheckAndSpawnFires: Backward fire for small structure created successfully");
            fireBackward->SaveToDB(mapId, 1 << map->GetSpawnMode(), phaseMask);
            ObjectGuid::LowType fireBackwardSpawnId = fireBackward->GetSpawnId();
            delete fireBackward;

            GameObjectData const* fireBackwardData = sObjectMgr->GetGameObjectData(fireBackwardSpawnId);
            if (fireBackwardData)
            {
                sObjectMgr->AddGameobjectToGrid(fireBackwardSpawnId, fireBackwardData);
            }

            fireBackward = new GameObject();
            if (fireBackward->LoadGameObjectFromDB(fireBackwardSpawnId, map, true))
            {
                LOG_ERROR("server", "[ConquestBuild] CheckAndSpawnFires: Backward fire for small structure loaded successfully, spawnId: {}", fireBackwardSpawnId);
                // Force update visibility to ensure clients see the fire
                fireBackward->UpdateObjectVisibility(true);
                // Verify fire is in world
                auto fireBounds = map->GetGameObjectBySpawnIdStore().equal_range(fireBackwardSpawnId);
                if (fireBounds.first != fireBounds.second)
                {
                    LOG_ERROR("server", "[ConquestBuild] CheckAndSpawnFires: Backward fire for small structure is in world map, spawnId: {}", fireBackwardSpawnId);
                }
                else
                {
                    LOG_ERROR("server", "[ConquestBuild] CheckAndSpawnFires: WARNING - Backward fire for small structure NOT in world map, spawnId: {}", fireBackwardSpawnId);
                }
                WorldDatabase.Execute(
                    "INSERT INTO conquest_build_structures (guid, player_guid, guild_id, entry, map, position_x, position_y, position_z, orientation, build_type, group_id) VALUES ({}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {})",
                    fireBackwardSpawnId, playerGuid, guildId, 179065, mapId,
                    backwardX, backwardY, structureZ - 1.0f, structureOrientation,
                    CONQUEST_BUILD_WALL, structureSpawnId
                );
                LOG_ERROR("server", "[ConquestBuild] CheckAndSpawnFires: Backward fire for small structure saved to tracking table");
            }
            else
            {
                LOG_ERROR("server", "[ConquestBuild] CheckAndSpawnFires: Failed to load backward fire for small structure from DB, spawnId: {}", fireBackwardSpawnId);
                if (fireBackwardData)
                {
                    sObjectMgr->RemoveGameobjectFromGrid(fireBackwardSpawnId, fireBackwardData);
                }
                delete fireBackward;
            }
        }
        else
        {
            LOG_ERROR("server", "[ConquestBuild] CheckAndSpawnFires: Failed to create backward fire for small structure");
            delete fireBackward;
        }
    }
    else if (isLargeStructure)
    {
        LOG_ERROR("server", "[ConquestBuild] CheckAndSpawnFires: Spawning fires for large structure (entry: {})", goEntry);
        // Two fires (179085): one 10 yards forward, one 10 yards backward (20 yards total separation)
        float forwardX = structureX + 9.0f * cos(structureOrientation);
        float forwardY = structureY + 9.0f * sin(structureOrientation);
        float backwardX = structureX - 9.0f * cos(structureOrientation);
        float backwardY = structureY - 9.0f * sin(structureOrientation);
        
        LOG_ERROR("server", "[ConquestBuild] CheckAndSpawnFires: Forward fire at ({}, {}, {}), Backward fire at ({}, {}, {})", 
            forwardX, forwardY, structureZ - 1.0f, backwardX, backwardY, structureZ - 1.0f);

        // Forward fire
        LOG_ERROR("server", "[ConquestBuild] CheckAndSpawnFires: Creating forward fire");
        GameObject* fireForward = new GameObject();
        ObjectGuid::LowType fireForwardGuid = map->GenerateLowGuid<HighGuid::GameObject>();
        G3D::Quat fireRotation(0, 0, 0, 1);

        if (fireForward->Create(fireForwardGuid, 179085, map, phaseMask,
                               forwardX, forwardY, structureZ - 1.0f, structureOrientation,
                               fireRotation, 0, GO_STATE_READY))
        {
            LOG_ERROR("server", "[ConquestBuild] CheckAndSpawnFires: Forward fire created successfully");
            fireForward->SaveToDB(mapId, 1 << map->GetSpawnMode(), phaseMask);
            ObjectGuid::LowType fireForwardSpawnId = fireForward->GetSpawnId();
            delete fireForward;

            GameObjectData const* fireForwardData = sObjectMgr->GetGameObjectData(fireForwardSpawnId);
            if (fireForwardData)
            {
                sObjectMgr->AddGameobjectToGrid(fireForwardSpawnId, fireForwardData);
            }

            fireForward = new GameObject();
            if (fireForward->LoadGameObjectFromDB(fireForwardSpawnId, map, true))
            {
                LOG_ERROR("server", "[ConquestBuild] CheckAndSpawnFires: Forward fire loaded successfully, spawnId: {}", fireForwardSpawnId);
                // Force update visibility to ensure clients see the fire
                fireForward->UpdateObjectVisibility(true);
                // Verify fire is in world
                auto fireBounds = map->GetGameObjectBySpawnIdStore().equal_range(fireForwardSpawnId);
                if (fireBounds.first != fireBounds.second)
                {
                    LOG_ERROR("server", "[ConquestBuild] CheckAndSpawnFires: Forward fire is in world map, spawnId: {}", fireForwardSpawnId);
                }
                else
                {
                    LOG_ERROR("server", "[ConquestBuild] CheckAndSpawnFires: WARNING - Forward fire NOT in world map, spawnId: {}", fireForwardSpawnId);
                }
                WorldDatabase.Execute(
                    "INSERT INTO conquest_build_structures (guid, player_guid, guild_id, entry, map, position_x, position_y, position_z, orientation, build_type, group_id) VALUES ({}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {})",
                    fireForwardSpawnId, playerGuid, guildId, 179085, mapId,
                    forwardX, forwardY, structureZ - 1.0f, structureOrientation,
                    CONQUEST_BUILD_WALL, structureSpawnId
                );
                LOG_ERROR("server", "[ConquestBuild] CheckAndSpawnFires: Forward fire saved to tracking table");
            }
            else
            {
                LOG_ERROR("server", "[ConquestBuild] CheckAndSpawnFires: Failed to load forward fire from DB, spawnId: {}", fireForwardSpawnId);
                if (fireForwardData)
                {
                    sObjectMgr->RemoveGameobjectFromGrid(fireForwardSpawnId, fireForwardData);
                }
                delete fireForward;
            }
        }
        else
        {
            LOG_ERROR("server", "[ConquestBuild] CheckAndSpawnFires: Failed to create forward fire");
            delete fireForward;
        }

        // Backward fire
        LOG_ERROR("server", "[ConquestBuild] CheckAndSpawnFires: Creating backward fire");
        GameObject* fireBackward = new GameObject();
        ObjectGuid::LowType fireBackwardGuid = map->GenerateLowGuid<HighGuid::GameObject>();

        if (fireBackward->Create(fireBackwardGuid, 179085, map, phaseMask,
                                backwardX, backwardY, structureZ - 1.0f, structureOrientation,
                                fireRotation, 0, GO_STATE_READY))
        {
            LOG_ERROR("server", "[ConquestBuild] CheckAndSpawnFires: Backward fire created successfully");
            fireBackward->SaveToDB(mapId, 1 << map->GetSpawnMode(), phaseMask);
            ObjectGuid::LowType fireBackwardSpawnId = fireBackward->GetSpawnId();
            delete fireBackward;

            GameObjectData const* fireBackwardData = sObjectMgr->GetGameObjectData(fireBackwardSpawnId);
            if (fireBackwardData)
            {
                sObjectMgr->AddGameobjectToGrid(fireBackwardSpawnId, fireBackwardData);
            }

            fireBackward = new GameObject();
            if (fireBackward->LoadGameObjectFromDB(fireBackwardSpawnId, map, true))
            {
                LOG_ERROR("server", "[ConquestBuild] CheckAndSpawnFires: Backward fire loaded successfully, spawnId: {}", fireBackwardSpawnId);
                // Force update visibility to ensure clients see the fire
                fireBackward->UpdateObjectVisibility(true);
                // Verify fire is in world
                auto fireBounds = map->GetGameObjectBySpawnIdStore().equal_range(fireBackwardSpawnId);
                if (fireBounds.first != fireBounds.second)
                {
                    LOG_ERROR("server", "[ConquestBuild] CheckAndSpawnFires: Backward fire is in world map, spawnId: {}", fireBackwardSpawnId);
                }
                else
                {
                    LOG_ERROR("server", "[ConquestBuild] CheckAndSpawnFires: WARNING - Backward fire NOT in world map, spawnId: {}", fireBackwardSpawnId);
                }
                WorldDatabase.Execute(
                    "INSERT INTO conquest_build_structures (guid, player_guid, guild_id, entry, map, position_x, position_y, position_z, orientation, build_type, group_id) VALUES ({}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {})",
                    fireBackwardSpawnId, playerGuid, guildId, 179085, mapId,
                    backwardX, backwardY, structureZ - 1.0f, structureOrientation,
                    CONQUEST_BUILD_WALL, structureSpawnId
                );
                LOG_ERROR("server", "[ConquestBuild] CheckAndSpawnFires: Backward fire saved to tracking table");
            }
            else
            {
                LOG_ERROR("server", "[ConquestBuild] CheckAndSpawnFires: Failed to load backward fire from DB, spawnId: {}", fireBackwardSpawnId);
                if (fireBackwardData)
                {
                    sObjectMgr->RemoveGameobjectFromGrid(fireBackwardSpawnId, fireBackwardData);
                }
                delete fireBackward;
            }
        }
        else
        {
            LOG_ERROR("server", "[ConquestBuild] CheckAndSpawnFires: Failed to create backward fire");
            delete fireBackward;
        }
    }
}

void ConquestBuildMgr::RemoveFiresForStructure(uint64 structureSpawnId, Map* map)
{
    // Find all fires associated with this structure (group_id = structureSpawnId)
    QueryResult firesResult = WorldDatabase.Query(
        "SELECT guid FROM conquest_build_structures WHERE group_id = {} AND entry IN (179065, 179085)",
        structureSpawnId
    );

    if (!firesResult)
        return;

    do
    {
        Field* fields = firesResult->Fetch();
        uint64 fireGuid = fields[0].Get<uint64>();

        // Delete from database
        WorldDatabase.Execute("DELETE FROM gameobject WHERE guid = {}", fireGuid);
        WorldDatabase.Execute("DELETE FROM conquest_build_structures WHERE guid = {}", fireGuid);

        // Delete from world if map is provided
        if (map)
        {
            auto bounds = map->GetGameObjectBySpawnIdStore().equal_range(fireGuid);
            for (auto itr = bounds.first; itr != bounds.second; ++itr)
            {
                GameObject* fire = itr->second;
                if (fire)
                {
                    fire->SetRespawnTime(0);
                    fire->Delete();
                }
            }
        }
    } while (firesResult->NextRow());
}

bool ConquestBuildMgr::SpawnSimpleGameObject(Player* player, uint32 gobEntry, float x, float y, float z, float orientation, uint64 groupId, uint32 guildId, ConquestBuildType buildType)
{
    LOG_INFO("module", "ConquestBuild: SpawnSimpleGameObject called - entry: {}, pos: ({}, {}, {}), orientation: {}, groupId: {}", 
        gobEntry, x, y, z, orientation, groupId);
    
    GameObject* go = new GameObject();
    ObjectGuid::LowType guidLow = player->GetMap()->GenerateLowGuid<HighGuid::GameObject>();
    G3D::Quat rotation(0, 0, 0, 1);
    
    if (go->Create(guidLow, gobEntry, player->GetMap(), player->GetPhaseMask(),
                   x, y, z, orientation, rotation, 0, GO_STATE_READY))
    {
        LOG_INFO("module", "ConquestBuild: GameObject created successfully, saving to DB...");
        go->SaveToDB(player->GetMapId(), 1 << player->GetMap()->GetSpawnMode(), player->GetPhaseMask());
        ObjectGuid::LowType spawnId = go->GetSpawnId();
        delete go;
        
        GameObjectData const* goData = sObjectMgr->GetGameObjectData(spawnId);
        if (goData)
        {
            sObjectMgr->AddGameobjectToGrid(spawnId, goData);
            LOG_INFO("module", "ConquestBuild: GameObject added to grid, spawnId: {}", spawnId);
        }
        else
        {
            LOG_ERROR("module", "ConquestBuild: Failed to get GameObjectData for spawnId: {}", spawnId);
        }
        
        go = new GameObject();
        if (go->LoadGameObjectFromDB(spawnId, player->GetMap(), true))
        {
            LOG_INFO("module", "ConquestBuild: GameObject loaded from DB successfully, inserting into conquest_build_structures...");
            WorldDatabase.Execute("INSERT INTO conquest_build_structures (guid, player_guid, guild_id, entry, map, position_x, position_y, position_z, orientation, build_type, group_id) VALUES ({}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {})",
                spawnId, player->GetGUID().GetCounter(), guildId, gobEntry,
                player->GetMapId(), x, y, z, orientation, buildType, groupId);
            LOG_INFO("module", "ConquestBuild: SpawnSimpleGameObject completed successfully for entry: {}", gobEntry);
            return true;
        }
        else
        {
            LOG_ERROR("module", "ConquestBuild: Failed to load GameObject from DB, spawnId: {}", spawnId);
            if (goData)
            {
                sObjectMgr->RemoveGameobjectFromGrid(spawnId, goData);
            }
            delete go;
        }
    }
    else
    {
        LOG_ERROR("module", "ConquestBuild: Failed to create GameObject, entry: {}", gobEntry);
        delete go;
    }
    
    return false;
}

void ConquestBuildMgr::SpawnFortMainStructures(Player* player, float fortGateX, float fortGateY, float fortGateZ, float fortGateOrientation, uint64 groupId, uint32 guildId, ConquestBuildType buildType)
{
    LOG_INFO("module", "ConquestBuild: SpawnFortMainStructures called - gate pos: ({}, {}, {}), orientation: {}, groupId: {}, buildType: {}", 
        fortGateX, fortGateY, fortGateZ, fortGateOrientation, groupId, buildType);
    
    // Determine which structures to use
    uint32 wallEntry = 400022; // Small wall
    uint32 towerEntry = 400023; // Small tower
    
    bool isGrandFort = (buildType == CONQUEST_BUILD_GRAND_FORT || buildType == CONQUEST_BUILD_GRAND_FORT_2 || buildType == CONQUEST_BUILD_GRAND_FORT_3);
    if (isGrandFort)
    {
        wallEntry = 400020; // Grand wall
        towerEntry = 400021; // Grande tower
    }
    
    // fortGateOrientation is always rotated 90 degrees from player orientation
    // So we need to reverse it to get the original player orientation
    float playerOrientation = fortGateOrientation - M_PI / 2.0f;
    float backwardAngle = playerOrientation + M_PI; // Back (wall)
    float leftAngle = playerOrientation + M_PI / 2.0f; // Left (wall)
    float rightAngle = playerOrientation - M_PI / 2.0f; // Right (wall)
    
    float BACK_TOWER_DISTANCE = 20.0f; // Distance from center for back towers
    float BACK_TOWER_SPACING = 12.0f; // Spacing between back towers
    float SIDE_WALL_DISTANCE = 20.0f; // Distance from center for side walls
    
    // Adjust distances based on fort type
    if (buildType == CONQUEST_BUILD_FORT)
    {
        BACK_TOWER_DISTANCE = 22.0f;
        SIDE_WALL_DISTANCE = 10.0f;
    }
    else if (isGrandFort)
    {
        BACK_TOWER_DISTANCE = 45.0f;
        BACK_TOWER_SPACING = 20.0f;
        SIDE_WALL_DISTANCE = 22.0f;
    }
    
    // Calculate perpendicular direction for gate towers (90 degrees from player orientation)
    float perpAngle = playerOrientation + M_PI / 2.0f;
    float gateTowerDistance = 20.0f; // Distance from center for gate towers (for large gate)
    if (buildType == CONQUEST_BUILD_FORT)
    {
        gateTowerDistance = 10.3f; // Distance for small gate (20.6 yards apart)
    }
    
    // Use fort gate position as center
    float fortCenterX = fortGateX;
    float fortCenterY = fortGateY;
    
    // Helper function to spawn a structure
    auto spawnStructure = [&](uint32 entry, float x, float y, float z, float orientation, ConquestBuildType structBuildType) -> bool {
        return SpawnSimpleGameObject(player, entry, x, y, z, orientation, groupId, guildId, structBuildType);
    };
    
    // LEFT tower position (for gate)
    float gateLeftTowerX = fortCenterX + gateTowerDistance * cos(perpAngle);
    float gateLeftTowerY = fortCenterY + gateTowerDistance * sin(perpAngle);
    float gateLeftTowerZ = fortGateZ;
    if (towerEntry == 400023) // Small tower
    {
        gateLeftTowerZ -= 8.0f;
    }
    
    spawnStructure(towerEntry, gateLeftTowerX, gateLeftTowerY, gateLeftTowerZ, playerOrientation, CONQUEST_BUILD_TOWER);
    
    // Spawn benches and cannons for the tower
    float gateLeftLateralOffset = isGrandFort ? -4.0f : 0.0f;
    if (towerEntry == 400023)
    {
        SpawnStoneBenchesForSmallTower(player, gateLeftTowerX, gateLeftTowerY, gateLeftTowerZ, playerOrientation, groupId, guildId, gateLeftLateralOffset);
        SpawnCannonForSmallTower(player, gateLeftTowerX, gateLeftTowerY, gateLeftTowerZ, playerOrientation, groupId, guildId);
    }
    else
    {
        SpawnStoneBenchesForTower(player, gateLeftTowerX, gateLeftTowerY, gateLeftTowerZ, playerOrientation, groupId, guildId, gateLeftLateralOffset);
        SpawnCannonsForTower(player, gateLeftTowerX, gateLeftTowerY, gateLeftTowerZ, playerOrientation, groupId, guildId);
    }
    
    // RIGHT tower position (for gate)
    float gateRightTowerX = fortCenterX - gateTowerDistance * cos(perpAngle);
    float gateRightTowerY = fortCenterY - gateTowerDistance * sin(perpAngle);
    float gateRightTowerZ = fortGateZ;
    if (towerEntry == 400023) // Small tower
    {
        gateRightTowerZ -= 8.0f;
    }
    
    spawnStructure(towerEntry, gateRightTowerX, gateRightTowerY, gateRightTowerZ, playerOrientation, CONQUEST_BUILD_TOWER);
    
    // Spawn benches and cannons for the tower
    float gateRightLateralOffset = isGrandFort ? 4.0f : 0.0f;
    if (towerEntry == 400023)
    {
        SpawnStoneBenchesForSmallTower(player, gateRightTowerX, gateRightTowerY, gateRightTowerZ, playerOrientation, groupId, guildId, gateRightLateralOffset);
        SpawnCannonForSmallTower(player, gateRightTowerX, gateRightTowerY, gateRightTowerZ, playerOrientation, groupId, guildId);
    }
    else
    {
        SpawnStoneBenchesForTower(player, gateRightTowerX, gateRightTowerY, gateRightTowerZ, playerOrientation, groupId, guildId, gateRightLateralOffset);
        SpawnCannonsForTower(player, gateRightTowerX, gateRightTowerY, gateRightTowerZ, playerOrientation, groupId, guildId);
    }
    
    // Spawn WALL on left
    float sideWallSpacing = BACK_TOWER_SPACING;
    if (isGrandFort)
    {
        sideWallSpacing = BACK_TOWER_SPACING + 2.0f;
    }
    float leftWallX = fortGateX + SIDE_WALL_DISTANCE * cos(backwardAngle) - sideWallSpacing * cos(leftAngle);
    float leftWallY = fortGateY + SIDE_WALL_DISTANCE * sin(backwardAngle) - sideWallSpacing * sin(leftAngle);
    float leftWallZ = fortGateZ;
    if (wallEntry == 400022) // Small wall
    {
        leftWallZ -= 4.0f;
    }
    
    spawnStructure(wallEntry, leftWallX, leftWallY, leftWallZ, leftAngle, buildType);
    
    // Spawn WALL on right
    float rightWallX = fortGateX + SIDE_WALL_DISTANCE * cos(backwardAngle) + sideWallSpacing * cos(leftAngle);
    float rightWallY = fortGateY + SIDE_WALL_DISTANCE * sin(backwardAngle) + sideWallSpacing * sin(leftAngle);
    float rightWallZ = fortGateZ;
    if (wallEntry == 400022) // Small wall
    {
        rightWallZ -= 4.0f;
    }
    
    spawnStructure(wallEntry, rightWallX, rightWallY, rightWallZ, rightAngle, buildType);
    
    // Spawn back structure: TOWER, WALL, TOWER (from left to right when facing backward)
    // Left tower position
    float backLeftTowerX = fortGateX + BACK_TOWER_DISTANCE * cos(backwardAngle) - BACK_TOWER_SPACING * cos(leftAngle);
    float backLeftTowerY = fortGateY + BACK_TOWER_DISTANCE * sin(backwardAngle) - BACK_TOWER_SPACING * sin(leftAngle);
    float backLeftTowerZ = fortGateZ;
    if (towerEntry == 400023) // Small tower
    {
        backLeftTowerZ -= 8.0f;
    }
    
    spawnStructure(towerEntry, backLeftTowerX, backLeftTowerY, backLeftTowerZ, backwardAngle, buildType);
    
    // Spawn benches and cannons for the tower
    float backLeftLateralOffset = isGrandFort ? -4.0f : 0.0f;
    if (towerEntry == 400023)
    {
        SpawnStoneBenchesForSmallTower(player, backLeftTowerX, backLeftTowerY, backLeftTowerZ, backwardAngle, groupId, guildId, backLeftLateralOffset);
        SpawnCannonForSmallTower(player, backLeftTowerX, backLeftTowerY, backLeftTowerZ, backwardAngle, groupId, guildId);
    }
    else
    {
        SpawnStoneBenchesForTower(player, backLeftTowerX, backLeftTowerY, backLeftTowerZ, backwardAngle, groupId, guildId, backLeftLateralOffset);
        SpawnCannonsForTower(player, backLeftTowerX, backLeftTowerY, backLeftTowerZ, backwardAngle, groupId, guildId);
    }
    
    // Center wall behind (between the two towers)
    float backCenterWallX = fortGateX + BACK_TOWER_DISTANCE * cos(backwardAngle);
    float backCenterWallY = fortGateY + BACK_TOWER_DISTANCE * sin(backwardAngle);
    float backCenterWallZ = fortGateZ;
    if (wallEntry == 400022) // Small wall
    {
        backCenterWallZ -= 4.0f;
    }
    
    spawnStructure(wallEntry, backCenterWallX, backCenterWallY, backCenterWallZ, backwardAngle, buildType);
    
    // Right tower position
    float backRightTowerX = fortGateX + BACK_TOWER_DISTANCE * cos(backwardAngle) + BACK_TOWER_SPACING * cos(leftAngle);
    float backRightTowerY = fortGateY + BACK_TOWER_DISTANCE * sin(backwardAngle) + BACK_TOWER_SPACING * sin(leftAngle);
    float backRightTowerZ = fortGateZ;
    if (towerEntry == 400023) // Small tower
    {
        backRightTowerZ -= 8.0f;
    }
    
    spawnStructure(towerEntry, backRightTowerX, backRightTowerY, backRightTowerZ, backwardAngle, buildType);
    
    // Spawn benches and cannons for the tower
    float backRightLateralOffset = isGrandFort ? 4.0f : 0.0f;
    if (towerEntry == 400023)
    {
        SpawnStoneBenchesForSmallTower(player, backRightTowerX, backRightTowerY, backRightTowerZ, backwardAngle, groupId, guildId, backRightLateralOffset);
        SpawnCannonForSmallTower(player, backRightTowerX, backRightTowerY, backRightTowerZ, backwardAngle, groupId, guildId);
    }
    else
    {
        SpawnStoneBenchesForTower(player, backRightTowerX, backRightTowerY, backRightTowerZ, backwardAngle, groupId, guildId, backRightLateralOffset);
        SpawnCannonsForTower(player, backRightTowerX, backRightTowerY, backRightTowerZ, backwardAngle, groupId, guildId);
    }
    
    // Spawn 2 levers for the gate
    float leverSideDistance = 8.0f;
    float leverSpacing = 6.0f;
    
    // Lever 1 position (right side, slightly forward)
    float lever1X = fortCenterX - leverSideDistance * cos(perpAngle) + leverSpacing * cos(playerOrientation);
    float lever1Y = fortCenterY - leverSideDistance * sin(perpAngle) + leverSpacing * sin(playerOrientation);
    float lever1Z = fortGateZ;
    
    // Lever 2 position (right side, slightly backward)
    float lever2X = fortCenterX - leverSideDistance * cos(perpAngle) - leverSpacing * cos(playerOrientation);
    float lever2Y = fortCenterY - leverSideDistance * sin(perpAngle) - leverSpacing * sin(playerOrientation);
    float lever2Z = fortGateZ;
    
    // Élever les leviers de 0.5 yard en Z pour les grands forts (80012 et 80013)
    if (buildType == CONQUEST_BUILD_GRAND_FORT_2 || buildType == CONQUEST_BUILD_GRAND_FORT_3)
    {
        lever1Z += 0.5f;
        lever2Z += 0.5f;
    }
    
    spawnStructure(400001, lever1X, lever1Y, lever1Z, playerOrientation, buildType);
    spawnStructure(400001, lever2X, lever2Y, lever2Z, playerOrientation, buildType);
    
    LOG_INFO("module", "ConquestBuild: SpawnFortMainStructures completed successfully");
}

void ConquestBuildMgr::SpawnFortRamparts(Player* player, float fortGateX, float fortGateY, float fortGateZ, float fortGateOrientation, uint64 groupId, uint32 guildId)
{
    LOG_INFO("module", "ConquestBuild: SpawnFortRamparts called - gate pos: ({}, {}, {}), orientation: {}, groupId: {}", 
        fortGateX, fortGateY, fortGateZ, fortGateOrientation, groupId);
    
    // Detect fort type (for logging purposes)
    QueryResult result = WorldDatabase.Query(
        "SELECT entry FROM conquest_build_structures WHERE group_id = {} AND entry IN (400025, 400020, 400021) LIMIT 1",
        groupId
    );
    if (result)
    {
        LOG_INFO("module", "ConquestBuild: Detected grand fort (large structures found in group)");
    }
    else
    {
        LOG_INFO("module", "ConquestBuild: Detected small fort (no large structures found in group)");
    }
    
    // Use small structures for ramparts (always use normal size for ramparts)
    uint32 gateEntry = 400026; // Small gate
    uint32 towerEntry = 400023; // Small tower
    uint32 wallEntry = 400022; // Small wall
    float GATE_TOWER_DISTANCE = 10.3f; // Distance from gate center to tower center (for small gate)
    
    // Fort dimensions (same for both types)
    const float TOWER_SIZE = 7.75f; // Tower size
    const float WALL_LENGTH = 10.0f; // Wall length
    const float OUTER_WALL_TO_TOWER_OFFSET = 6.5f; // Décalage entre le deuxième mur et la tour d'extrémité
    const float OUTER_WALL_LATERAL_OFFSET_FRONT = 7.5f; // Décalage latéral du deuxième mur pour les remparts avant (vers l'extérieur)
    const float OUTER_TOWER_LATERAL_OFFSET_FRONT = 2.5f; // Décalage latéral des tours extérieures avant (réduit de 5 yards par rapport aux murs, rapprochées de 2 yards vers le centre)
    
    // Rampart spacing
    // Check if this is a grand fort (80012 or 80013) by checking if group contains grand gate (400025)
    QueryResult grandFortCheck = WorldDatabase.Query(
        "SELECT entry FROM conquest_build_structures WHERE group_id = {} AND entry = 400025 LIMIT 1",
        groupId
    );
    bool isGrandFortKit = (grandFortCheck != nullptr);
    const float RAMPART_SPACING_FRONT = isGrandFortKit ? 30.0f : 40.0f; // Réduit à 30 yards pour les grands forts (80012 et 80013)
    
    // Calculate directions
    float forwardAngle = fortGateOrientation; // Direction the fort gate faces (front)
    float backwardAngle = fortGateOrientation + M_PI; // Back
    float perpAngle = fortGateOrientation + M_PI / 2.0f; // Perpendicular (for gate towers)
    
    // Helper function to spawn a structure at a position
    auto spawnAt = [&](uint32 entry, float x, float y, float z, float orientation, ConquestBuildType buildType) {
        bool result = SpawnSimpleGameObject(player, entry, x, y, z, orientation, groupId, guildId, buildType);
        LOG_INFO("module", "ConquestBuild: Spawned structure entry {} at ({}, {}, {}) - result: {}", entry, x, y, z, result);
        return result;
    };
    
    // FRONT RAMPART: Tour-Mur-Mur-Herse-Mur-Mur-Tour à 35 yards devant la herse principale
    float frontRampartGateX = fortGateX + RAMPART_SPACING_FRONT * cos(forwardAngle);
    float frontRampartGateY = fortGateY + RAMPART_SPACING_FRONT * sin(forwardAngle);
    
    // Front rampart left tower (tour intérieure, à côté de la herse)
    float frontRampartLeftTowerX = frontRampartGateX + GATE_TOWER_DISTANCE * cos(perpAngle);
    float frontRampartLeftTowerY = frontRampartGateY + GATE_TOWER_DISTANCE * sin(perpAngle);
    spawnAt(towerEntry, frontRampartLeftTowerX, frontRampartLeftTowerY, fortGateZ - 8.0f, forwardAngle, CONQUEST_BUILD_TOWER);
    SpawnStoneBenchesForSmallTower(player, frontRampartLeftTowerX, frontRampartLeftTowerY, fortGateZ - 8.0f, forwardAngle, groupId, guildId, 0.0f);
    SpawnCannonForSmallTower(player, frontRampartLeftTowerX, frontRampartLeftTowerY, fortGateZ - 8.0f, forwardAngle, groupId, guildId);
    
    // Front rampart left inner wall (premier mur, entre la tour et la herse)
    float frontRampartLeftInnerWallX = frontRampartLeftTowerX + (TOWER_SIZE / 2.0f + WALL_LENGTH / 2.0f) * cos(perpAngle);
    float frontRampartLeftInnerWallY = frontRampartLeftTowerY + (TOWER_SIZE / 2.0f + WALL_LENGTH / 2.0f) * sin(perpAngle);
    spawnAt(wallEntry, frontRampartLeftInnerWallX, frontRampartLeftInnerWallY, fortGateZ - 4.0f, forwardAngle, CONQUEST_BUILD_WALL);
    
    // Front rampart left outer wall (deuxième mur, décalé de 6.5 yards de la tour d'extrémité et vers la gauche)
    float frontRampartLeftEndTowerBaseX = frontRampartLeftInnerWallX + (WALL_LENGTH / 2.0f + WALL_LENGTH + OUTER_WALL_TO_TOWER_OFFSET + TOWER_SIZE / 2.0f) * cos(perpAngle);
    float frontRampartLeftEndTowerBaseY = frontRampartLeftInnerWallY + (WALL_LENGTH / 2.0f + WALL_LENGTH + OUTER_WALL_TO_TOWER_OFFSET + TOWER_SIZE / 2.0f) * sin(perpAngle);
    // Décalage latéral de la tour d'extrémité vers la gauche (vers l'extérieur, direction perpAngle) - rapprochée de 3 yards vers le centre
    float frontRampartLeftEndTowerX = frontRampartLeftEndTowerBaseX + OUTER_TOWER_LATERAL_OFFSET_FRONT * cos(perpAngle) - 3.0f * cos(perpAngle);
    float frontRampartLeftEndTowerY = frontRampartLeftEndTowerBaseY + OUTER_TOWER_LATERAL_OFFSET_FRONT * sin(perpAngle) - 3.0f * sin(perpAngle);
    
    // Positionner le deuxième mur entre le premier mur et la tour d'extrémité, décalé vers la gauche (extérieur)
    float frontRampartLeftOuterWallBaseX = frontRampartLeftInnerWallX + (WALL_LENGTH / 2.0f + WALL_LENGTH / 2.0f) * cos(perpAngle);
    float frontRampartLeftOuterWallBaseY = frontRampartLeftInnerWallY + (WALL_LENGTH / 2.0f + WALL_LENGTH / 2.0f) * sin(perpAngle);
    // Décalage latéral vers la gauche (vers l'extérieur, direction perpAngle)
    float frontRampartLeftOuterWallX = frontRampartLeftOuterWallBaseX + OUTER_WALL_LATERAL_OFFSET_FRONT * cos(perpAngle);
    float frontRampartLeftOuterWallY = frontRampartLeftOuterWallBaseY + OUTER_WALL_LATERAL_OFFSET_FRONT * sin(perpAngle);
    spawnAt(wallEntry, frontRampartLeftOuterWallX, frontRampartLeftOuterWallY, fortGateZ - 4.0f, forwardAngle, CONQUEST_BUILD_WALL);
    
    // Front rampart left end tower (tour d'extrémité, décalée de 6.5 yards du deuxième mur et vers la gauche)
    spawnAt(towerEntry, frontRampartLeftEndTowerX, frontRampartLeftEndTowerY, fortGateZ - 8.0f, forwardAngle, CONQUEST_BUILD_TOWER);
    // Décalage latéral des stone benches vers le centre (vers la droite) - offset réduit pour petite tour
    float frontLeftEndTowerLateralOffset = -4.0f; // Vers le centre (vers la droite) - augmenté
    SpawnStoneBenchesForSmallTower(player, frontRampartLeftEndTowerX, frontRampartLeftEndTowerY, fortGateZ - 8.0f, forwardAngle, groupId, guildId, frontLeftEndTowerLateralOffset);
    SpawnCannonForSmallTower(player, frontRampartLeftEndTowerX, frontRampartLeftEndTowerY, fortGateZ - 8.0f, forwardAngle, groupId, guildId);
    
    // Front rampart gate (classic orientation for kit 80012)
    spawnAt(gateEntry, frontRampartGateX, frontRampartGateY, fortGateZ, forwardAngle, CONQUEST_BUILD_GATE_SMALL);
    
    // Spawn levers for front rampart gate (classic orientation for kit 80012)
    SpawnLeversForGate(player, frontRampartGateX, frontRampartGateY, fortGateZ, forwardAngle, groupId, guildId, CONQUEST_BUILD_GATE_SMALL);
    
    // Front rampart right tower (tour intérieure, à côté de la herse)
    float frontRampartRightTowerX = frontRampartGateX - GATE_TOWER_DISTANCE * cos(perpAngle);
    float frontRampartRightTowerY = frontRampartGateY - GATE_TOWER_DISTANCE * sin(perpAngle);
    spawnAt(towerEntry, frontRampartRightTowerX, frontRampartRightTowerY, fortGateZ - 8.0f, forwardAngle, CONQUEST_BUILD_TOWER);
    SpawnStoneBenchesForSmallTower(player, frontRampartRightTowerX, frontRampartRightTowerY, fortGateZ - 8.0f, forwardAngle, groupId, guildId, 0.0f);
    SpawnCannonForSmallTower(player, frontRampartRightTowerX, frontRampartRightTowerY, fortGateZ - 8.0f, forwardAngle, groupId, guildId);
    
    // Front rampart right inner wall (premier mur, entre la tour et la herse)
    float frontRampartRightInnerWallX = frontRampartRightTowerX - (TOWER_SIZE / 2.0f + WALL_LENGTH / 2.0f) * cos(perpAngle);
    float frontRampartRightInnerWallY = frontRampartRightTowerY - (TOWER_SIZE / 2.0f + WALL_LENGTH / 2.0f) * sin(perpAngle);
    spawnAt(wallEntry, frontRampartRightInnerWallX, frontRampartRightInnerWallY, fortGateZ - 4.0f, forwardAngle, CONQUEST_BUILD_WALL);
    
    // Front rampart right outer wall (deuxième mur, décalé de 6.5 yards de la tour d'extrémité et vers la droite)
    float frontRampartRightEndTowerBaseX = frontRampartRightInnerWallX - (WALL_LENGTH / 2.0f + WALL_LENGTH + OUTER_WALL_TO_TOWER_OFFSET + TOWER_SIZE / 2.0f) * cos(perpAngle);
    float frontRampartRightEndTowerBaseY = frontRampartRightInnerWallY - (WALL_LENGTH / 2.0f + WALL_LENGTH + OUTER_WALL_TO_TOWER_OFFSET + TOWER_SIZE / 2.0f) * sin(perpAngle);
    // Décalage latéral de la tour d'extrémité vers la droite (vers l'extérieur, direction opposée à perpAngle) - rapprochée de 3 yards vers le centre
    float frontRampartRightEndTowerX = frontRampartRightEndTowerBaseX - OUTER_TOWER_LATERAL_OFFSET_FRONT * cos(perpAngle) + 3.0f * cos(perpAngle);
    float frontRampartRightEndTowerY = frontRampartRightEndTowerBaseY - OUTER_TOWER_LATERAL_OFFSET_FRONT * sin(perpAngle) + 3.0f * sin(perpAngle);
    
    // Positionner le deuxième mur entre le premier mur et la tour d'extrémité, décalé vers la droite (extérieur)
    float frontRampartRightOuterWallBaseX = frontRampartRightInnerWallX - (WALL_LENGTH / 2.0f + WALL_LENGTH / 2.0f) * cos(perpAngle);
    float frontRampartRightOuterWallBaseY = frontRampartRightInnerWallY - (WALL_LENGTH / 2.0f + WALL_LENGTH / 2.0f) * sin(perpAngle);
    // Décalage latéral vers la droite (vers l'extérieur, direction opposée à perpAngle)
    float frontRampartRightOuterWallX = frontRampartRightOuterWallBaseX - OUTER_WALL_LATERAL_OFFSET_FRONT * cos(perpAngle);
    float frontRampartRightOuterWallY = frontRampartRightOuterWallBaseY - OUTER_WALL_LATERAL_OFFSET_FRONT * sin(perpAngle);
    spawnAt(wallEntry, frontRampartRightOuterWallX, frontRampartRightOuterWallY, fortGateZ - 4.0f, forwardAngle, CONQUEST_BUILD_WALL);
    
    // Front rampart right end tower (tour d'extrémité, décalée de 6.5 yards du deuxième mur et vers la droite)
    spawnAt(towerEntry, frontRampartRightEndTowerX, frontRampartRightEndTowerY, fortGateZ - 8.0f, forwardAngle, CONQUEST_BUILD_TOWER);
    // Décalage latéral des stone benches vers le centre (vers la gauche) - offset réduit pour petite tour
    float frontRightEndTowerLateralOffset = 4.0f; // Vers le centre (vers la gauche) - augmenté
    SpawnStoneBenchesForSmallTower(player, frontRampartRightEndTowerX, frontRampartRightEndTowerY, fortGateZ - 8.0f, forwardAngle, groupId, guildId, frontRightEndTowerLateralOffset);
    SpawnCannonForSmallTower(player, frontRampartRightEndTowerX, frontRampartRightEndTowerY, fortGateZ - 8.0f, forwardAngle, groupId, guildId);
    
    // BACK RAMPART: Tour-Mur-Mur-Herse-Mur-Mur-Tour (identique à la ligne avant, positionnée en bas)
    // Calculate total distance of lateral lines to position back rampart correctly
    // The lateral lines go from frontRampartLeftEndTower to backRampartLeftEndTower
    // We need to calculate the total offset needed for the lateral lines
    // Offsets for lateral lines (defined before they are used):
    const float FIRST_WALL_DOWNWARD_OFFSET_LATERAL = 3.0f;
    const float WALL_TO_TOWER_SPACING_LATERAL = 1.0f;
    const float FIRST_TOWER_DOWNWARD_OFFSET_LATERAL = 1.0f;
    const float SECOND_TOWER_DOWNWARD_OFFSET_LATERAL = 0.0f;
    const float PERFECT_TOWER_TO_WALL_OFFSET_LATERAL = TOWER_SIZE / 2.0f + 1.0f + WALL_LENGTH / 2.0f + 2.0f;
    const float PERFECT_WALL_TO_WALL_OFFSET_LATERAL = WALL_LENGTH / 2.0f + 7.5f + WALL_LENGTH / 2.0f;
    
    // Calculate total lateral line distance
    float totalLateralDistance = 
        (TOWER_SIZE / 2.0f + WALL_LENGTH / 2.0f + FIRST_WALL_DOWNWARD_OFFSET_LATERAL) + // First wall
        (WALL_LENGTH / 2.0f + WALL_TO_TOWER_SPACING_LATERAL + TOWER_SIZE / 2.0f + FIRST_TOWER_DOWNWARD_OFFSET_LATERAL) + // First tower
        PERFECT_TOWER_TO_WALL_OFFSET_LATERAL + // Second wall
        PERFECT_WALL_TO_WALL_OFFSET_LATERAL + // Third wall
        PERFECT_WALL_TO_WALL_OFFSET_LATERAL + // Fourth wall
        (WALL_LENGTH / 2.0f + WALL_TO_TOWER_SPACING_LATERAL + TOWER_SIZE / 2.0f + SECOND_TOWER_DOWNWARD_OFFSET_LATERAL) + // Second tower
        (TOWER_SIZE / 2.0f + WALL_LENGTH / 2.0f + 2.0f); // Fifth wall + distance to back tower
    
    // Calculate the center point between the two front end towers to center the back rampart
    float frontRampartCenterX = (frontRampartLeftEndTowerX + frontRampartRightEndTowerX) / 2.0f;
    float frontRampartCenterY = (frontRampartLeftEndTowerY + frontRampartRightEndTowerY) / 2.0f;
    
    // Position back rampart gate at the end of lateral lines, centered and slightly further back
    // Add extra distance so that the back rampart end towers connect properly to the lateral line walls
    float extraDistanceForConnection = TOWER_SIZE / 2.0f + WALL_LENGTH / 2.0f; // Distance needed for tower to connect to wall
    float backRampartGateX = frontRampartCenterX + (totalLateralDistance + extraDistanceForConnection) * cos(backwardAngle);
    float backRampartGateY = frontRampartCenterY + (totalLateralDistance + extraDistanceForConnection) * sin(backwardAngle);
    
    // Back rampart left tower (tour intérieure, à côté de la herse) - même calcul que devant
    float backRampartLeftTowerX = backRampartGateX + GATE_TOWER_DISTANCE * cos(perpAngle);
    float backRampartLeftTowerY = backRampartGateY + GATE_TOWER_DISTANCE * sin(perpAngle);
    spawnAt(towerEntry, backRampartLeftTowerX, backRampartLeftTowerY, fortGateZ - 8.0f, backwardAngle, CONQUEST_BUILD_TOWER);
    SpawnStoneBenchesForSmallTower(player, backRampartLeftTowerX, backRampartLeftTowerY, fortGateZ - 8.0f, backwardAngle, groupId, guildId, 0.0f);
    SpawnCannonForSmallTower(player, backRampartLeftTowerX, backRampartLeftTowerY, fortGateZ - 8.0f, backwardAngle, groupId, guildId);
    
    // Back rampart left inner wall (premier mur, entre la tour et la herse) - même calcul que devant
    float backRampartLeftInnerWallX = backRampartLeftTowerX + (TOWER_SIZE / 2.0f + WALL_LENGTH / 2.0f) * cos(perpAngle);
    float backRampartLeftInnerWallY = backRampartLeftTowerY + (TOWER_SIZE / 2.0f + WALL_LENGTH / 2.0f) * sin(perpAngle);
    spawnAt(wallEntry, backRampartLeftInnerWallX, backRampartLeftInnerWallY, fortGateZ - 4.0f, backwardAngle, CONQUEST_BUILD_WALL);
    
    // Back rampart left outer wall (deuxième mur, décalé de 6.5 yards de la tour d'extrémité et vers la gauche) - même calcul que devant
    float backRampartLeftEndTowerBaseX = backRampartLeftInnerWallX + (WALL_LENGTH / 2.0f + WALL_LENGTH + OUTER_WALL_TO_TOWER_OFFSET + TOWER_SIZE / 2.0f) * cos(perpAngle);
    float backRampartLeftEndTowerBaseY = backRampartLeftInnerWallY + (WALL_LENGTH / 2.0f + WALL_LENGTH + OUTER_WALL_TO_TOWER_OFFSET + TOWER_SIZE / 2.0f) * sin(perpAngle);
    // Décalage latéral de la tour d'extrémité vers la gauche (vers l'extérieur, direction perpAngle) - rapprochée de 3 yards vers le centre
    float backRampartLeftEndTowerX = backRampartLeftEndTowerBaseX + OUTER_TOWER_LATERAL_OFFSET_FRONT * cos(perpAngle) - 3.0f * cos(perpAngle);
    float backRampartLeftEndTowerY = backRampartLeftEndTowerBaseY + OUTER_TOWER_LATERAL_OFFSET_FRONT * sin(perpAngle) - 3.0f * sin(perpAngle);
    
    // Positionner le deuxième mur entre le premier mur et la tour d'extrémité, décalé vers la gauche (extérieur) - même que devant
    float backRampartLeftOuterWallBaseX = backRampartLeftInnerWallX + (WALL_LENGTH / 2.0f + WALL_LENGTH / 2.0f) * cos(perpAngle);
    float backRampartLeftOuterWallBaseY = backRampartLeftInnerWallY + (WALL_LENGTH / 2.0f + WALL_LENGTH / 2.0f) * sin(perpAngle);
    // Décalage latéral vers la gauche (vers l'extérieur, direction perpAngle) - même que devant
    float backRampartLeftOuterWallX = backRampartLeftOuterWallBaseX + OUTER_WALL_LATERAL_OFFSET_FRONT * cos(perpAngle);
    float backRampartLeftOuterWallY = backRampartLeftOuterWallBaseY + OUTER_WALL_LATERAL_OFFSET_FRONT * sin(perpAngle);
    spawnAt(wallEntry, backRampartLeftOuterWallX, backRampartLeftOuterWallY, fortGateZ - 4.0f, backwardAngle, CONQUEST_BUILD_WALL);
    
    // Back rampart left end tower (tour d'extrémité, décalée de 6.5 yards du deuxième mur et vers la gauche) - même que devant
    spawnAt(towerEntry, backRampartLeftEndTowerX, backRampartLeftEndTowerY, fortGateZ - 8.0f, backwardAngle, CONQUEST_BUILD_TOWER);
    // Décalage latéral des stone benches vers la gauche (vers l'extérieur) - offset réduit pour petite tour
    float backLeftEndTowerLateralOffset = 4.0f; // Vers la gauche (vers l'extérieur) - augmenté
    SpawnStoneBenchesForSmallTower(player, backRampartLeftEndTowerX, backRampartLeftEndTowerY, fortGateZ - 8.0f, backwardAngle, groupId, guildId, backLeftEndTowerLateralOffset);
    SpawnCannonForSmallTower(player, backRampartLeftEndTowerX, backRampartLeftEndTowerY, fortGateZ - 8.0f, backwardAngle, groupId, guildId);
    
    // Back rampart gate (classic orientation for kit 80012)
    spawnAt(gateEntry, backRampartGateX, backRampartGateY, fortGateZ, backwardAngle, CONQUEST_BUILD_GATE_SMALL);
    
    // Spawn levers for back rampart gate (classic orientation for kit 80012)
    SpawnLeversForGate(player, backRampartGateX, backRampartGateY, fortGateZ, backwardAngle, groupId, guildId, CONQUEST_BUILD_GATE_SMALL);
    
    // Back rampart right tower (tour intérieure, à côté de la herse) - même calcul que devant
    float backRampartRightTowerX = backRampartGateX - GATE_TOWER_DISTANCE * cos(perpAngle);
    float backRampartRightTowerY = backRampartGateY - GATE_TOWER_DISTANCE * sin(perpAngle);
    spawnAt(towerEntry, backRampartRightTowerX, backRampartRightTowerY, fortGateZ - 8.0f, backwardAngle, CONQUEST_BUILD_TOWER);
    SpawnStoneBenchesForSmallTower(player, backRampartRightTowerX, backRampartRightTowerY, fortGateZ - 8.0f, backwardAngle, groupId, guildId, 0.0f);
    SpawnCannonForSmallTower(player, backRampartRightTowerX, backRampartRightTowerY, fortGateZ - 8.0f, backwardAngle, groupId, guildId);
    
    // Back rampart right inner wall (premier mur, entre la tour et la herse) - même calcul que devant
    float backRampartRightInnerWallX = backRampartRightTowerX - (TOWER_SIZE / 2.0f + WALL_LENGTH / 2.0f) * cos(perpAngle);
    float backRampartRightInnerWallY = backRampartRightTowerY - (TOWER_SIZE / 2.0f + WALL_LENGTH / 2.0f) * sin(perpAngle);
    spawnAt(wallEntry, backRampartRightInnerWallX, backRampartRightInnerWallY, fortGateZ - 4.0f, backwardAngle, CONQUEST_BUILD_WALL);
    
    // Back rampart right outer wall (deuxième mur, décalé de 6.5 yards de la tour d'extrémité et vers la droite) - même calcul que devant
    float backRampartRightEndTowerBaseX = backRampartRightInnerWallX - (WALL_LENGTH / 2.0f + WALL_LENGTH + OUTER_WALL_TO_TOWER_OFFSET + TOWER_SIZE / 2.0f) * cos(perpAngle);
    float backRampartRightEndTowerBaseY = backRampartRightInnerWallY - (WALL_LENGTH / 2.0f + WALL_LENGTH + OUTER_WALL_TO_TOWER_OFFSET + TOWER_SIZE / 2.0f) * sin(perpAngle);
    // Décalage latéral de la tour d'extrémité vers la droite (vers l'extérieur, direction opposée à perpAngle) - rapprochée de 3 yards vers le centre
    float backRampartRightEndTowerX = backRampartRightEndTowerBaseX - OUTER_TOWER_LATERAL_OFFSET_FRONT * cos(perpAngle) + 3.0f * cos(perpAngle);
    float backRampartRightEndTowerY = backRampartRightEndTowerBaseY - OUTER_TOWER_LATERAL_OFFSET_FRONT * sin(perpAngle) + 3.0f * sin(perpAngle);
    
    // Positionner le deuxième mur entre le premier mur et la tour d'extrémité, décalé vers la droite (extérieur) - même que devant
    float backRampartRightOuterWallBaseX = backRampartRightInnerWallX - (WALL_LENGTH / 2.0f + WALL_LENGTH / 2.0f) * cos(perpAngle);
    float backRampartRightOuterWallBaseY = backRampartRightInnerWallY - (WALL_LENGTH / 2.0f + WALL_LENGTH / 2.0f) * sin(perpAngle);
    // Décalage latéral vers la droite (vers l'extérieur, direction opposée à perpAngle) - même que devant
    float backRampartRightOuterWallX = backRampartRightOuterWallBaseX - OUTER_WALL_LATERAL_OFFSET_FRONT * cos(perpAngle);
    float backRampartRightOuterWallY = backRampartRightOuterWallBaseY - OUTER_WALL_LATERAL_OFFSET_FRONT * sin(perpAngle);
    spawnAt(wallEntry, backRampartRightOuterWallX, backRampartRightOuterWallY, fortGateZ - 4.0f, backwardAngle, CONQUEST_BUILD_WALL);
    
    // Back rampart right end tower (tour d'extrémité, décalée de 6.5 yards du deuxième mur et vers la droite) - même que devant
    spawnAt(towerEntry, backRampartRightEndTowerX, backRampartRightEndTowerY, fortGateZ - 8.0f, backwardAngle, CONQUEST_BUILD_TOWER);
    // Décalage latéral des stone benches vers la droite (vers l'extérieur) - offset réduit pour petite tour
    float backRightEndTowerLateralOffset = -4.0f; // Vers la droite (vers l'extérieur) - augmenté
    SpawnStoneBenchesForSmallTower(player, backRampartRightEndTowerX, backRampartRightEndTowerY, fortGateZ - 8.0f, backwardAngle, groupId, guildId, backRightEndTowerLateralOffset);
    SpawnCannonForSmallTower(player, backRampartRightEndTowerX, backRampartRightEndTowerY, fortGateZ - 8.0f, backwardAngle, groupId, guildId);
    
    // LEFT SIDE: 5 structures (5 murs + 2 tours) entre la tour avant gauche et la tour arrière gauche
    
    // Offsets parfaits calculés depuis les exemples
    // Offset parfait tour->mur : première tour -> deuxième mur
    const float PERFECT_TOWER_TO_WALL_OFFSET = TOWER_SIZE / 2.0f + 1.0f + WALL_LENGTH / 2.0f + 2.0f; // = 3.875 + 1.0 + 5.0 + 2.0 = 11.875
    // Offset parfait mur->mur : deuxième mur -> troisième mur
    const float PERFECT_WALL_TO_WALL_OFFSET = WALL_LENGTH / 2.0f + 7.5f + WALL_LENGTH / 2.0f; // = 5.0 + 7.5 + 5.0 = 17.5
    
    // Premier mur : décalage latéral de 3 yards vers l'intérieur
    const float INWARD_OFFSET = 3.0f; // Décalage latéral de 3 yards vers l'intérieur pour les lignes latérales
    const float FIRST_WALL_DOWNWARD_OFFSET = 3.0f; // Premier mur commence 3 yards plus en dessous
    const float WALL_TO_TOWER_SPACING = 1.0f; // Espacement de 1 yard entre les murs et la tour
    const float FIRST_TOWER_DOWNWARD_OFFSET = 1.0f; // Première tour décalée de 1 yard vers le bas
    const float SECOND_TOWER_DOWNWARD_OFFSET = 0.0f; // Deuxième tour sans décalage vertical (remontée de 3 yards)
    
    // Position de base du premier mur (3 yards plus en dessous de la tour)
    float firstLeftWallBaseX = frontRampartLeftEndTowerX + (TOWER_SIZE / 2.0f + WALL_LENGTH / 2.0f + FIRST_WALL_DOWNWARD_OFFSET) * cos(backwardAngle);
    float firstLeftWallBaseY = frontRampartLeftEndTowerY + (TOWER_SIZE / 2.0f + WALL_LENGTH / 2.0f + FIRST_WALL_DOWNWARD_OFFSET) * sin(backwardAngle);
    
    // Décalage latéral vers l'intérieur (vers la droite, donc négatif par rapport à perpAngle) + 4 yards vers l'extérieur
    float lateralOffsetX = -INWARD_OFFSET * cos(perpAngle) + 4.0f * cos(perpAngle);
    float lateralOffsetY = -INWARD_OFFSET * sin(perpAngle) + 4.0f * sin(perpAngle);
    
    // Premier mur avec décalage latéral
    float firstLeftWallX = firstLeftWallBaseX + lateralOffsetX;
    float firstLeftWallY = firstLeftWallBaseY + lateralOffsetY;
    spawnAt(wallEntry, firstLeftWallX, firstLeftWallY, fortGateZ - 4.0f, perpAngle, CONQUEST_BUILD_WALL);
    
    // Première tour : espacée du premier mur (1 yard d'espacement) et décalée de 1 yard vers le bas
    float firstTowerOffset = TOWER_SIZE / 2.0f + WALL_LENGTH / 2.0f + FIRST_WALL_DOWNWARD_OFFSET + WALL_LENGTH / 2.0f + WALL_TO_TOWER_SPACING + TOWER_SIZE / 2.0f + FIRST_TOWER_DOWNWARD_OFFSET;
    float firstTowerBaseX = frontRampartLeftEndTowerX + firstTowerOffset * cos(backwardAngle);
    float firstTowerBaseY = frontRampartLeftEndTowerY + firstTowerOffset * sin(backwardAngle);
    float firstTowerX = firstTowerBaseX + lateralOffsetX;
    float firstTowerY = firstTowerBaseY + lateralOffsetY;
    spawnAt(towerEntry, firstTowerX, firstTowerY, fortGateZ - 8.0f, perpAngle, CONQUEST_BUILD_TOWER);
    SpawnStoneBenchesForSmallTower(player, firstTowerX, firstTowerY, fortGateZ - 8.0f, perpAngle, groupId, guildId, 0.0f);
    SpawnCannonForSmallTower(player, firstTowerX, firstTowerY, fortGateZ - 8.0f, perpAngle, groupId, guildId);
    
    // Deuxième mur : utilise l'offset parfait tour->mur
    float secondWallOffset = firstTowerOffset + PERFECT_TOWER_TO_WALL_OFFSET;
    float secondWallBaseX = frontRampartLeftEndTowerX + secondWallOffset * cos(backwardAngle);
    float secondWallBaseY = frontRampartLeftEndTowerY + secondWallOffset * sin(backwardAngle);
    float secondWallX = secondWallBaseX + lateralOffsetX;
    float secondWallY = secondWallBaseY + lateralOffsetY;
    spawnAt(wallEntry, secondWallX, secondWallY, fortGateZ - 4.0f, perpAngle, CONQUEST_BUILD_WALL);
    
    // Troisième mur : utilise l'offset parfait mur->mur
    float thirdWallOffset = secondWallOffset + PERFECT_WALL_TO_WALL_OFFSET;
    float thirdWallBaseX = frontRampartLeftEndTowerX + thirdWallOffset * cos(backwardAngle);
    float thirdWallBaseY = frontRampartLeftEndTowerY + thirdWallOffset * sin(backwardAngle);
    float thirdWallX = thirdWallBaseX + lateralOffsetX;
    float thirdWallY = thirdWallBaseY + lateralOffsetY;
    spawnAt(wallEntry, thirdWallX, thirdWallY, fortGateZ - 4.0f, perpAngle, CONQUEST_BUILD_WALL);
    
    // Quatrième mur : ajouté au milieu, utilise l'offset parfait mur->mur
    float fourthWallOffset = thirdWallOffset + PERFECT_WALL_TO_WALL_OFFSET;
    float fourthWallBaseX = frontRampartLeftEndTowerX + fourthWallOffset * cos(backwardAngle);
    float fourthWallBaseY = frontRampartLeftEndTowerY + fourthWallOffset * sin(backwardAngle);
    float fourthWallX = fourthWallBaseX + lateralOffsetX;
    float fourthWallY = fourthWallBaseY + lateralOffsetY;
    spawnAt(wallEntry, fourthWallX, fourthWallY, fortGateZ - 4.0f, perpAngle, CONQUEST_BUILD_WALL);
    
    // Deuxième tour : utilise l'offset parfait mur->tour (inverse de tour->mur, mais sans le décalage vertical)
    float secondTowerOffset = fourthWallOffset + WALL_LENGTH / 2.0f + WALL_TO_TOWER_SPACING + TOWER_SIZE / 2.0f + SECOND_TOWER_DOWNWARD_OFFSET;
    float secondTowerBaseX = frontRampartLeftEndTowerX + secondTowerOffset * cos(backwardAngle);
    float secondTowerBaseY = frontRampartLeftEndTowerY + secondTowerOffset * sin(backwardAngle);
    float secondTowerX = secondTowerBaseX + lateralOffsetX;
    float secondTowerY = secondTowerBaseY + lateralOffsetY;
    spawnAt(towerEntry, secondTowerX, secondTowerY, fortGateZ - 8.0f, perpAngle, CONQUEST_BUILD_TOWER);
    SpawnStoneBenchesForSmallTower(player, secondTowerX, secondTowerY, fortGateZ - 8.0f, perpAngle, groupId, guildId, 0.0f);
    SpawnCannonForSmallTower(player, secondTowerX, secondTowerY, fortGateZ - 8.0f, perpAngle, groupId, guildId);
    
    // Cinquième mur : calculé pour se connecter à la tour arrière gauche
    // Distance entre les tours avant et arrière
    float distanceToBackTower = sqrt(pow(backRampartLeftEndTowerX - frontRampartLeftEndTowerX, 2) + pow(backRampartLeftEndTowerY - frontRampartLeftEndTowerY, 2));
    // Position du cinquième mur : juste avant la tour arrière (en utilisant l'offset parfait tour->mur depuis la tour arrière, mais en sens inverse) - éloigné de 2 yards
    float fifthWallOffset = distanceToBackTower - (TOWER_SIZE / 2.0f + WALL_LENGTH / 2.0f) - 2.0f;
    float fifthWallBaseX = frontRampartLeftEndTowerX + fifthWallOffset * cos(backwardAngle);
    float fifthWallBaseY = frontRampartLeftEndTowerY + fifthWallOffset * sin(backwardAngle);
    float fifthWallX = fifthWallBaseX + lateralOffsetX;
    float fifthWallY = fifthWallBaseY + lateralOffsetY;
    spawnAt(wallEntry, fifthWallX, fifthWallY, fortGateZ - 4.0f, perpAngle, CONQUEST_BUILD_WALL);
    
    // RIGHT SIDE: 5 structures (5 murs + 2 tours) entre la tour avant droite et la tour arrière droite
    
    // Premier mur : pas de décalage latéral
    // Position de base du premier mur (3 yards plus en dessous de la tour)
    float firstRightWallBaseX = frontRampartRightEndTowerX + (TOWER_SIZE / 2.0f + WALL_LENGTH / 2.0f + FIRST_WALL_DOWNWARD_OFFSET) * cos(backwardAngle);
    float firstRightWallBaseY = frontRampartRightEndTowerY + (TOWER_SIZE / 2.0f + WALL_LENGTH / 2.0f + FIRST_WALL_DOWNWARD_OFFSET) * sin(backwardAngle);
    
    // Décalage latéral vers l'intérieur (vers la gauche, donc positif par rapport à perpAngle) + 4 yards vers l'extérieur
    float rightLateralOffsetX = INWARD_OFFSET * cos(perpAngle) - 4.0f * cos(perpAngle);
    float rightLateralOffsetY = INWARD_OFFSET * sin(perpAngle) - 4.0f * sin(perpAngle);
    
    // Premier mur avec décalage latéral
    float firstRightWallX = firstRightWallBaseX + rightLateralOffsetX;
    float firstRightWallY = firstRightWallBaseY + rightLateralOffsetY;
    spawnAt(wallEntry, firstRightWallX, firstRightWallY, fortGateZ - 4.0f, perpAngle, CONQUEST_BUILD_WALL);
    
    // Première tour : espacée du premier mur (1 yard d'espacement) et décalée de 1 yard vers le bas
    float rightFirstTowerOffset = TOWER_SIZE / 2.0f + WALL_LENGTH / 2.0f + FIRST_WALL_DOWNWARD_OFFSET + WALL_LENGTH / 2.0f + WALL_TO_TOWER_SPACING + TOWER_SIZE / 2.0f + FIRST_TOWER_DOWNWARD_OFFSET;
    float rightFirstTowerBaseX = frontRampartRightEndTowerX + rightFirstTowerOffset * cos(backwardAngle);
    float rightFirstTowerBaseY = frontRampartRightEndTowerY + rightFirstTowerOffset * sin(backwardAngle);
    float rightFirstTowerX = rightFirstTowerBaseX + rightLateralOffsetX;
    float rightFirstTowerY = rightFirstTowerBaseY + rightLateralOffsetY;
    spawnAt(towerEntry, rightFirstTowerX, rightFirstTowerY, fortGateZ - 8.0f, perpAngle + M_PI, CONQUEST_BUILD_TOWER);
    SpawnStoneBenchesForSmallTower(player, rightFirstTowerX, rightFirstTowerY, fortGateZ - 8.0f, perpAngle + M_PI, groupId, guildId, 0.0f);
    SpawnCannonForSmallTower(player, rightFirstTowerX, rightFirstTowerY, fortGateZ - 8.0f, perpAngle + M_PI, groupId, guildId);
    
    // Deuxième mur : utilise l'offset parfait tour->mur
    float rightSecondWallOffset = rightFirstTowerOffset + PERFECT_TOWER_TO_WALL_OFFSET;
    float rightSecondWallBaseX = frontRampartRightEndTowerX + rightSecondWallOffset * cos(backwardAngle);
    float rightSecondWallBaseY = frontRampartRightEndTowerY + rightSecondWallOffset * sin(backwardAngle);
    float rightSecondWallX = rightSecondWallBaseX + rightLateralOffsetX;
    float rightSecondWallY = rightSecondWallBaseY + rightLateralOffsetY;
    spawnAt(wallEntry, rightSecondWallX, rightSecondWallY, fortGateZ - 4.0f, perpAngle, CONQUEST_BUILD_WALL);
    
    // Troisième mur : utilise l'offset parfait mur->mur
    float rightThirdWallOffset = rightSecondWallOffset + PERFECT_WALL_TO_WALL_OFFSET;
    float rightThirdWallBaseX = frontRampartRightEndTowerX + rightThirdWallOffset * cos(backwardAngle);
    float rightThirdWallBaseY = frontRampartRightEndTowerY + rightThirdWallOffset * sin(backwardAngle);
    float rightThirdWallX = rightThirdWallBaseX + rightLateralOffsetX;
    float rightThirdWallY = rightThirdWallBaseY + rightLateralOffsetY;
    spawnAt(wallEntry, rightThirdWallX, rightThirdWallY, fortGateZ - 4.0f, perpAngle, CONQUEST_BUILD_WALL);
    
    // Quatrième mur : ajouté au milieu, utilise l'offset parfait mur->mur
    float rightFourthWallOffset = rightThirdWallOffset + PERFECT_WALL_TO_WALL_OFFSET;
    float rightFourthWallBaseX = frontRampartRightEndTowerX + rightFourthWallOffset * cos(backwardAngle);
    float rightFourthWallBaseY = frontRampartRightEndTowerY + rightFourthWallOffset * sin(backwardAngle);
    float rightFourthWallX = rightFourthWallBaseX + rightLateralOffsetX;
    float rightFourthWallY = rightFourthWallBaseY + rightLateralOffsetY;
    spawnAt(wallEntry, rightFourthWallX, rightFourthWallY, fortGateZ - 4.0f, perpAngle, CONQUEST_BUILD_WALL);
    
    // Deuxième tour : utilise l'offset parfait mur->tour (inverse de tour->mur, mais sans le décalage vertical)
    float rightSecondTowerOffset = rightFourthWallOffset + WALL_LENGTH / 2.0f + WALL_TO_TOWER_SPACING + TOWER_SIZE / 2.0f + SECOND_TOWER_DOWNWARD_OFFSET;
    float rightSecondTowerBaseX = frontRampartRightEndTowerX + rightSecondTowerOffset * cos(backwardAngle);
    float rightSecondTowerBaseY = frontRampartRightEndTowerY + rightSecondTowerOffset * sin(backwardAngle);
    float rightSecondTowerX = rightSecondTowerBaseX + rightLateralOffsetX;
    float rightSecondTowerY = rightSecondTowerBaseY + rightLateralOffsetY;
    spawnAt(towerEntry, rightSecondTowerX, rightSecondTowerY, fortGateZ - 8.0f, perpAngle + M_PI, CONQUEST_BUILD_TOWER);
    SpawnStoneBenchesForSmallTower(player, rightSecondTowerX, rightSecondTowerY, fortGateZ - 8.0f, perpAngle + M_PI, groupId, guildId, 0.0f);
    SpawnCannonForSmallTower(player, rightSecondTowerX, rightSecondTowerY, fortGateZ - 8.0f, perpAngle + M_PI, groupId, guildId);
    
    // Cinquième mur : calculé pour se connecter à la tour arrière droite
    // Distance entre les tours avant et arrière
    float rightDistanceToBackTower = sqrt(pow(backRampartRightEndTowerX - frontRampartRightEndTowerX, 2) + pow(backRampartRightEndTowerY - frontRampartRightEndTowerY, 2));
    // Position du cinquième mur : juste avant la tour arrière (en utilisant l'offset parfait tour->mur depuis la tour arrière, mais en sens inverse) - éloigné de 2 yards
    float rightFifthWallOffset = rightDistanceToBackTower - (TOWER_SIZE / 2.0f + WALL_LENGTH / 2.0f) - 2.0f;
    float rightFifthWallBaseX = frontRampartRightEndTowerX + rightFifthWallOffset * cos(backwardAngle);
    float rightFifthWallBaseY = frontRampartRightEndTowerY + rightFifthWallOffset * sin(backwardAngle);
    float rightFifthWallX = rightFifthWallBaseX + rightLateralOffsetX;
    float rightFifthWallY = rightFifthWallBaseY + rightLateralOffsetY;
    spawnAt(wallEntry, rightFifthWallX, rightFifthWallY, fortGateZ - 4.0f, perpAngle, CONQUEST_BUILD_WALL);
}

void ConquestBuildMgr::SpawnFortRampartsGrand(Player* player, float fortGateX, float fortGateY, float fortGateZ, float fortGateOrientation, uint64 groupId, uint32 guildId)
{
    LOG_INFO("module", "ConquestBuild: SpawnFortRampartsGrand called - gate pos: ({}, {}, {}), orientation: {}, groupId: {}", 
        fortGateX, fortGateY, fortGateZ, fortGateOrientation, groupId);
    
    // Use grand structures for ramparts
    uint32 gateEntry = 400025; // Grand gate
    uint32 towerEntry = 400021; // Grande tower
    uint32 wallEntry = 400020; // Grand wall
    float GATE_TOWER_DISTANCE = 20.0f; // Distance from gate center to tower center (for grand gate)
    
    // Fort dimensions for grand structures (larger than small)
    const float TOWER_SIZE = 15.5f; // Grand tower size (larger than small tower 7.75f)
    const float WALL_LENGTH = 20.0f; // Grand wall length (larger than small wall 10.0f)
    const float OUTER_WALL_TO_TOWER_OFFSET = 13.0f; // Décalage entre le deuxième mur et la tour d'extrémité (larger than small 6.5f)
    const float OUTER_WALL_LATERAL_OFFSET_FRONT = 15.0f; // Décalage latéral du deuxième mur pour les remparts avant (larger than small 7.5f)
    const float OUTER_TOWER_LATERAL_OFFSET_FRONT = 5.0f; // Décalage latéral des tours extérieures avant (larger than small 2.5f)
    
    // Rampart spacing
    // This function is only called for grand forts (80013), so reduce spacing
    const float RAMPART_SPACING_FRONT = 60.0f; // Réduit à 60 yards pour le grand fort 80013 (au lieu de 70)
    
    // Calculate directions
    float forwardAngle = fortGateOrientation; // Direction the fort gate faces (front)
    float backwardAngle = fortGateOrientation + M_PI; // Back
    float perpAngle = fortGateOrientation + M_PI / 2.0f; // Perpendicular (for gate towers)
    
    // Helper function to spawn a structure at a position
    auto spawnAt = [&](uint32 entry, float x, float y, float z, float orientation, ConquestBuildType buildType) {
        bool result = SpawnSimpleGameObject(player, entry, x, y, z, orientation, groupId, guildId, buildType);
        LOG_INFO("module", "ConquestBuild: Spawned grand structure entry {} at ({}, {}, {}) - result: {}", entry, x, y, z, result);
        return result;
    };
    
    // FRONT RAMPART: Tour-Mur-Mur-Herse-Mur-Mur-Tour
    float frontRampartGateX = fortGateX + RAMPART_SPACING_FRONT * cos(forwardAngle);
    float frontRampartGateY = fortGateY + RAMPART_SPACING_FRONT * sin(forwardAngle);
    
    // Front rampart left tower (tour intérieure, à côté de la herse)
    float frontRampartLeftTowerX = frontRampartGateX + GATE_TOWER_DISTANCE * cos(perpAngle);
    float frontRampartLeftTowerY = frontRampartGateY + GATE_TOWER_DISTANCE * sin(perpAngle);
    spawnAt(towerEntry, frontRampartLeftTowerX, frontRampartLeftTowerY, fortGateZ, forwardAngle, CONQUEST_BUILD_TOWER);
    SpawnStoneBenchesForTower(player, frontRampartLeftTowerX, frontRampartLeftTowerY, fortGateZ, forwardAngle, groupId, guildId, 0.0f);
    SpawnCannonsForTower(player, frontRampartLeftTowerX, frontRampartLeftTowerY, fortGateZ, forwardAngle, groupId, guildId);
    
    // Front rampart left inner wall (premier mur, entre la tour et la herse)
    float frontRampartLeftInnerWallX = frontRampartLeftTowerX + (TOWER_SIZE / 2.0f + WALL_LENGTH / 2.0f) * cos(perpAngle);
    float frontRampartLeftInnerWallY = frontRampartLeftTowerY + (TOWER_SIZE / 2.0f + WALL_LENGTH / 2.0f) * sin(perpAngle);
    spawnAt(wallEntry, frontRampartLeftInnerWallX, frontRampartLeftInnerWallY, fortGateZ, forwardAngle, CONQUEST_BUILD_WALL);
    
    // Front rampart left outer wall (deuxième mur, décalé de la tour d'extrémité et vers la gauche)
    float frontRampartLeftEndTowerBaseX = frontRampartLeftInnerWallX + (WALL_LENGTH / 2.0f + WALL_LENGTH + OUTER_WALL_TO_TOWER_OFFSET + TOWER_SIZE / 2.0f) * cos(perpAngle);
    float frontRampartLeftEndTowerBaseY = frontRampartLeftInnerWallY + (WALL_LENGTH / 2.0f + WALL_LENGTH + OUTER_WALL_TO_TOWER_OFFSET + TOWER_SIZE / 2.0f) * sin(perpAngle);
    // Décalage latéral de la tour d'extrémité vers la gauche (vers l'extérieur, direction perpAngle) - rapprochée de 3 yards vers le centre
    float frontRampartLeftEndTowerX = frontRampartLeftEndTowerBaseX + OUTER_TOWER_LATERAL_OFFSET_FRONT * cos(perpAngle) - 3.0f * cos(perpAngle);
    float frontRampartLeftEndTowerY = frontRampartLeftEndTowerBaseY + OUTER_TOWER_LATERAL_OFFSET_FRONT * sin(perpAngle) - 3.0f * sin(perpAngle);
    
    // Positionner le deuxième mur entre le premier mur et la tour d'extrémité, décalé vers la gauche (extérieur)
    float frontRampartLeftOuterWallBaseX = frontRampartLeftInnerWallX + (WALL_LENGTH / 2.0f + WALL_LENGTH / 2.0f) * cos(perpAngle);
    float frontRampartLeftOuterWallBaseY = frontRampartLeftInnerWallY + (WALL_LENGTH / 2.0f + WALL_LENGTH / 2.0f) * sin(perpAngle);
    // Décalage latéral vers la gauche (vers l'extérieur, direction perpAngle)
    float frontRampartLeftOuterWallX = frontRampartLeftOuterWallBaseX + OUTER_WALL_LATERAL_OFFSET_FRONT * cos(perpAngle);
    float frontRampartLeftOuterWallY = frontRampartLeftOuterWallBaseY + OUTER_WALL_LATERAL_OFFSET_FRONT * sin(perpAngle);
    spawnAt(wallEntry, frontRampartLeftOuterWallX, frontRampartLeftOuterWallY, fortGateZ, forwardAngle, CONQUEST_BUILD_WALL);
    
    // Front rampart left end tower (tour d'extrémité, décalée du deuxième mur et vers la gauche)
    spawnAt(towerEntry, frontRampartLeftEndTowerX, frontRampartLeftEndTowerY, fortGateZ, forwardAngle, CONQUEST_BUILD_TOWER);
    // Décalage latéral des stone benches vers le centre (vers la droite)
    float frontLeftEndTowerLateralOffset = -6.0f; // Vers le centre (vers la droite) - augmenté
    SpawnStoneBenchesForTower(player, frontRampartLeftEndTowerX, frontRampartLeftEndTowerY, fortGateZ, forwardAngle, groupId, guildId, frontLeftEndTowerLateralOffset);
    SpawnCannonsForTower(player, frontRampartLeftEndTowerX, frontRampartLeftEndTowerY, fortGateZ, forwardAngle, groupId, guildId);
    
    // Front rampart gate (rotated 90 degrees)
    spawnAt(gateEntry, frontRampartGateX, frontRampartGateY, fortGateZ, forwardAngle + M_PI / 2.0f, CONQUEST_BUILD_GATE);
    
    // Spawn levers for front rampart gate (rotated 90 degrees)
    SpawnLeversForGate(player, frontRampartGateX, frontRampartGateY, fortGateZ, forwardAngle + M_PI / 2.0f, groupId, guildId, CONQUEST_BUILD_GRAND_FORT);
    
    // Front rampart right tower (tour intérieure, à côté de la herse)
    float frontRampartRightTowerX = frontRampartGateX - GATE_TOWER_DISTANCE * cos(perpAngle);
    float frontRampartRightTowerY = frontRampartGateY - GATE_TOWER_DISTANCE * sin(perpAngle);
    spawnAt(towerEntry, frontRampartRightTowerX, frontRampartRightTowerY, fortGateZ, forwardAngle, CONQUEST_BUILD_TOWER);
    SpawnStoneBenchesForTower(player, frontRampartRightTowerX, frontRampartRightTowerY, fortGateZ, forwardAngle, groupId, guildId, 0.0f);
    SpawnCannonsForTower(player, frontRampartRightTowerX, frontRampartRightTowerY, fortGateZ, forwardAngle, groupId, guildId);
    
    // Front rampart right inner wall (premier mur, entre la tour et la herse)
    float frontRampartRightInnerWallX = frontRampartRightTowerX - (TOWER_SIZE / 2.0f + WALL_LENGTH / 2.0f) * cos(perpAngle);
    float frontRampartRightInnerWallY = frontRampartRightTowerY - (TOWER_SIZE / 2.0f + WALL_LENGTH / 2.0f) * sin(perpAngle);
    spawnAt(wallEntry, frontRampartRightInnerWallX, frontRampartRightInnerWallY, fortGateZ, forwardAngle, CONQUEST_BUILD_WALL);
    
    // Front rampart right outer wall (deuxième mur, décalé de la tour d'extrémité et vers la droite)
    float frontRampartRightEndTowerBaseX = frontRampartRightInnerWallX - (WALL_LENGTH / 2.0f + WALL_LENGTH + OUTER_WALL_TO_TOWER_OFFSET + TOWER_SIZE / 2.0f) * cos(perpAngle);
    float frontRampartRightEndTowerBaseY = frontRampartRightInnerWallY - (WALL_LENGTH / 2.0f + WALL_LENGTH + OUTER_WALL_TO_TOWER_OFFSET + TOWER_SIZE / 2.0f) * sin(perpAngle);
    // Décalage latéral de la tour d'extrémité vers la droite (vers l'extérieur, direction opposée à perpAngle) - rapprochée de 3 yards vers le centre
    float frontRampartRightEndTowerX = frontRampartRightEndTowerBaseX - OUTER_TOWER_LATERAL_OFFSET_FRONT * cos(perpAngle) + 3.0f * cos(perpAngle);
    float frontRampartRightEndTowerY = frontRampartRightEndTowerBaseY - OUTER_TOWER_LATERAL_OFFSET_FRONT * sin(perpAngle) + 3.0f * sin(perpAngle);
    
    // Positionner le deuxième mur entre le premier mur et la tour d'extrémité, décalé vers la droite (extérieur)
    float frontRampartRightOuterWallBaseX = frontRampartRightInnerWallX - (WALL_LENGTH / 2.0f + WALL_LENGTH / 2.0f) * cos(perpAngle);
    float frontRampartRightOuterWallBaseY = frontRampartRightInnerWallY - (WALL_LENGTH / 2.0f + WALL_LENGTH / 2.0f) * sin(perpAngle);
    // Décalage latéral vers la droite (vers l'extérieur, direction opposée à perpAngle)
    float frontRampartRightOuterWallX = frontRampartRightOuterWallBaseX - OUTER_WALL_LATERAL_OFFSET_FRONT * cos(perpAngle);
    float frontRampartRightOuterWallY = frontRampartRightOuterWallBaseY - OUTER_WALL_LATERAL_OFFSET_FRONT * sin(perpAngle);
    spawnAt(wallEntry, frontRampartRightOuterWallX, frontRampartRightOuterWallY, fortGateZ, forwardAngle, CONQUEST_BUILD_WALL);
    
    // Front rampart right end tower (tour d'extrémité, décalée du deuxième mur et vers la droite)
    spawnAt(towerEntry, frontRampartRightEndTowerX, frontRampartRightEndTowerY, fortGateZ, forwardAngle, CONQUEST_BUILD_TOWER);
    // Décalage latéral des stone benches vers le centre (vers la gauche)
    float frontRightEndTowerLateralOffset = 6.0f; // Vers le centre (vers la gauche) - augmenté
    SpawnStoneBenchesForTower(player, frontRampartRightEndTowerX, frontRampartRightEndTowerY, fortGateZ, forwardAngle, groupId, guildId, frontRightEndTowerLateralOffset);
    SpawnCannonsForTower(player, frontRampartRightEndTowerX, frontRampartRightEndTowerY, fortGateZ, forwardAngle, groupId, guildId);
    
    // BACK RAMPART: Tour-Mur-Mur-Herse-Mur-Mur-Tour (identique à la ligne avant, positionnée en bas)
    // Calculate total distance of lateral lines to position back rampart correctly
    // The lateral lines go from frontRampartLeftEndTower to backRampartLeftEndTower
    // We need to calculate the total offset needed for the lateral lines
    // Offsets for lateral lines (will be defined later in the function, but we need them here)
    // We'll use the same offsets that are defined for the lateral lines below
    const float FIRST_WALL_DOWNWARD_OFFSET_LATERAL_GRAND = 3.0f;
    const float WALL_TO_TOWER_SPACING_LATERAL_GRAND = 1.0f;
    const float FIRST_TOWER_DOWNWARD_OFFSET_LATERAL_GRAND = 1.0f;
    const float SECOND_TOWER_DOWNWARD_OFFSET_LATERAL_GRAND = 0.0f;
    // Use the PERFECT offsets that will be defined for grand structures (see below)
    // These will be defined later, but we reference them here
    const float PERFECT_TOWER_TO_WALL_OFFSET_LATERAL_GRAND = TOWER_SIZE / 2.0f + 2.0f + WALL_LENGTH / 2.0f + 4.0f; // = 7.75 + 2.0 + 10.0 + 4.0 = 23.75
    const float PERFECT_WALL_TO_WALL_OFFSET_LATERAL_GRAND = WALL_LENGTH / 2.0f + 15.0f + WALL_LENGTH / 2.0f; // = 10.0 + 15.0 + 10.0 = 35.0
    
    // Calculate total lateral line distance
    float totalLateralDistanceGrand = 
        (TOWER_SIZE / 2.0f + WALL_LENGTH / 2.0f + FIRST_WALL_DOWNWARD_OFFSET_LATERAL_GRAND) + // First wall
        (WALL_LENGTH / 2.0f + WALL_TO_TOWER_SPACING_LATERAL_GRAND + TOWER_SIZE / 2.0f + FIRST_TOWER_DOWNWARD_OFFSET_LATERAL_GRAND) + // First tower
        PERFECT_TOWER_TO_WALL_OFFSET_LATERAL_GRAND + // Second wall
        PERFECT_WALL_TO_WALL_OFFSET_LATERAL_GRAND + // Third wall
        PERFECT_WALL_TO_WALL_OFFSET_LATERAL_GRAND + // Fourth wall
        (WALL_LENGTH / 2.0f + WALL_TO_TOWER_SPACING_LATERAL_GRAND + TOWER_SIZE / 2.0f + SECOND_TOWER_DOWNWARD_OFFSET_LATERAL_GRAND) + // Second tower
        (TOWER_SIZE / 2.0f + WALL_LENGTH / 2.0f + 2.0f); // Fifth wall + distance to back tower
    
    // Calculate the center point between the two front end towers to center the back rampart
    float frontRampartCenterX = (frontRampartLeftEndTowerX + frontRampartRightEndTowerX) / 2.0f;
    float frontRampartCenterY = (frontRampartLeftEndTowerY + frontRampartRightEndTowerY) / 2.0f;
    
    // Position back rampart gate at the end of lateral lines, centered and slightly further back
    // Add extra distance so that the back rampart end towers connect properly to the lateral line walls
    float extraDistanceForConnectionGrand = TOWER_SIZE / 2.0f + WALL_LENGTH / 2.0f; // Distance needed for tower to connect to wall
    float backRampartGateX = frontRampartCenterX + (totalLateralDistanceGrand + extraDistanceForConnectionGrand) * cos(backwardAngle);
    float backRampartGateY = frontRampartCenterY + (totalLateralDistanceGrand + extraDistanceForConnectionGrand) * sin(backwardAngle);
    
    // Back rampart left tower (tour intérieure, à côté de la herse) - même calcul que devant
    float backRampartLeftTowerX = backRampartGateX + GATE_TOWER_DISTANCE * cos(perpAngle);
    float backRampartLeftTowerY = backRampartGateY + GATE_TOWER_DISTANCE * sin(perpAngle);
    spawnAt(towerEntry, backRampartLeftTowerX, backRampartLeftTowerY, fortGateZ, backwardAngle, CONQUEST_BUILD_TOWER);
    SpawnStoneBenchesForTower(player, backRampartLeftTowerX, backRampartLeftTowerY, fortGateZ, backwardAngle, groupId, guildId, 0.0f);
    SpawnCannonsForTower(player, backRampartLeftTowerX, backRampartLeftTowerY, fortGateZ, backwardAngle, groupId, guildId);
    
    // Back rampart left inner wall (premier mur, entre la tour et la herse) - même calcul que devant
    float backRampartLeftInnerWallX = backRampartLeftTowerX + (TOWER_SIZE / 2.0f + WALL_LENGTH / 2.0f) * cos(perpAngle);
    float backRampartLeftInnerWallY = backRampartLeftTowerY + (TOWER_SIZE / 2.0f + WALL_LENGTH / 2.0f) * sin(perpAngle);
    spawnAt(wallEntry, backRampartLeftInnerWallX, backRampartLeftInnerWallY, fortGateZ, backwardAngle, CONQUEST_BUILD_WALL);
    
    // Back rampart left outer wall (deuxième mur, décalé de la tour d'extrémité et vers la gauche) - même calcul que devant
    float backRampartLeftEndTowerBaseX = backRampartLeftInnerWallX + (WALL_LENGTH / 2.0f + WALL_LENGTH + OUTER_WALL_TO_TOWER_OFFSET + TOWER_SIZE / 2.0f) * cos(perpAngle);
    float backRampartLeftEndTowerBaseY = backRampartLeftInnerWallY + (WALL_LENGTH / 2.0f + WALL_LENGTH + OUTER_WALL_TO_TOWER_OFFSET + TOWER_SIZE / 2.0f) * sin(perpAngle);
    // Décalage latéral de la tour d'extrémité vers la gauche (vers l'extérieur, direction perpAngle) - rapprochée de 3 yards vers le centre
    float backRampartLeftEndTowerX = backRampartLeftEndTowerBaseX + OUTER_TOWER_LATERAL_OFFSET_FRONT * cos(perpAngle) - 3.0f * cos(perpAngle);
    float backRampartLeftEndTowerY = backRampartLeftEndTowerBaseY + OUTER_TOWER_LATERAL_OFFSET_FRONT * sin(perpAngle) - 3.0f * sin(perpAngle);
    
    // Positionner le deuxième mur entre le premier mur et la tour d'extrémité, décalé vers la gauche (extérieur) - même que devant
    float backRampartLeftOuterWallBaseX = backRampartLeftInnerWallX + (WALL_LENGTH / 2.0f + WALL_LENGTH / 2.0f) * cos(perpAngle);
    float backRampartLeftOuterWallBaseY = backRampartLeftInnerWallY + (WALL_LENGTH / 2.0f + WALL_LENGTH / 2.0f) * sin(perpAngle);
    // Décalage latéral vers la gauche (vers l'extérieur, direction perpAngle) - même que devant
    float backRampartLeftOuterWallX = backRampartLeftOuterWallBaseX + OUTER_WALL_LATERAL_OFFSET_FRONT * cos(perpAngle);
    float backRampartLeftOuterWallY = backRampartLeftOuterWallBaseY + OUTER_WALL_LATERAL_OFFSET_FRONT * sin(perpAngle);
    spawnAt(wallEntry, backRampartLeftOuterWallX, backRampartLeftOuterWallY, fortGateZ, backwardAngle, CONQUEST_BUILD_WALL);
    
    // Back rampart left end tower (tour d'extrémité, décalée du deuxième mur et vers la gauche) - même que devant
    spawnAt(towerEntry, backRampartLeftEndTowerX, backRampartLeftEndTowerY, fortGateZ, backwardAngle, CONQUEST_BUILD_TOWER);
    // Décalage latéral des stone benches vers la gauche (vers l'extérieur)
    float backLeftEndTowerLateralOffset = 6.0f; // Vers la gauche (vers l'extérieur) - augmenté
    SpawnStoneBenchesForTower(player, backRampartLeftEndTowerX, backRampartLeftEndTowerY, fortGateZ, backwardAngle, groupId, guildId, backLeftEndTowerLateralOffset);
    SpawnCannonsForTower(player, backRampartLeftEndTowerX, backRampartLeftEndTowerY, fortGateZ, backwardAngle, groupId, guildId);
    
    // Back rampart gate (rotated 90 degrees)
    spawnAt(gateEntry, backRampartGateX, backRampartGateY, fortGateZ, backwardAngle + M_PI / 2.0f, CONQUEST_BUILD_GATE);
    
    // Spawn levers for back rampart gate (rotated 90 degrees)
    SpawnLeversForGate(player, backRampartGateX, backRampartGateY, fortGateZ, backwardAngle + M_PI / 2.0f, groupId, guildId, CONQUEST_BUILD_GRAND_FORT);
    
    // Back rampart right tower (tour intérieure, à côté de la herse) - même calcul que devant
    float backRampartRightTowerX = backRampartGateX - GATE_TOWER_DISTANCE * cos(perpAngle);
    float backRampartRightTowerY = backRampartGateY - GATE_TOWER_DISTANCE * sin(perpAngle);
    spawnAt(towerEntry, backRampartRightTowerX, backRampartRightTowerY, fortGateZ, backwardAngle, CONQUEST_BUILD_TOWER);
    SpawnStoneBenchesForTower(player, backRampartRightTowerX, backRampartRightTowerY, fortGateZ, backwardAngle, groupId, guildId, 0.0f);
    SpawnCannonsForTower(player, backRampartRightTowerX, backRampartRightTowerY, fortGateZ, backwardAngle, groupId, guildId);
    
    // Back rampart right inner wall (premier mur, entre la tour et la herse) - même calcul que devant
    float backRampartRightInnerWallX = backRampartRightTowerX - (TOWER_SIZE / 2.0f + WALL_LENGTH / 2.0f) * cos(perpAngle);
    float backRampartRightInnerWallY = backRampartRightTowerY - (TOWER_SIZE / 2.0f + WALL_LENGTH / 2.0f) * sin(perpAngle);
    spawnAt(wallEntry, backRampartRightInnerWallX, backRampartRightInnerWallY, fortGateZ, backwardAngle, CONQUEST_BUILD_WALL);
    
    // Back rampart right outer wall (deuxième mur, décalé de la tour d'extrémité et vers la droite) - même calcul que devant
    float backRampartRightEndTowerBaseX = backRampartRightInnerWallX - (WALL_LENGTH / 2.0f + WALL_LENGTH + OUTER_WALL_TO_TOWER_OFFSET + TOWER_SIZE / 2.0f) * cos(perpAngle);
    float backRampartRightEndTowerBaseY = backRampartRightInnerWallY - (WALL_LENGTH / 2.0f + WALL_LENGTH + OUTER_WALL_TO_TOWER_OFFSET + TOWER_SIZE / 2.0f) * sin(perpAngle);
    // Décalage latéral de la tour d'extrémité vers la droite (vers l'extérieur, direction opposée à perpAngle) - rapprochée de 3 yards vers le centre
    float backRampartRightEndTowerX = backRampartRightEndTowerBaseX - OUTER_TOWER_LATERAL_OFFSET_FRONT * cos(perpAngle) + 3.0f * cos(perpAngle);
    float backRampartRightEndTowerY = backRampartRightEndTowerBaseY - OUTER_TOWER_LATERAL_OFFSET_FRONT * sin(perpAngle) + 3.0f * sin(perpAngle);
    
    // Positionner le deuxième mur entre le premier mur et la tour d'extrémité, décalé vers la droite (extérieur) - même que devant
    float backRampartRightOuterWallBaseX = backRampartRightInnerWallX - (WALL_LENGTH / 2.0f + WALL_LENGTH / 2.0f) * cos(perpAngle);
    float backRampartRightOuterWallBaseY = backRampartRightInnerWallY - (WALL_LENGTH / 2.0f + WALL_LENGTH / 2.0f) * sin(perpAngle);
    // Décalage latéral vers la droite (vers l'extérieur, direction opposée à perpAngle) - même que devant
    float backRampartRightOuterWallX = backRampartRightOuterWallBaseX - OUTER_WALL_LATERAL_OFFSET_FRONT * cos(perpAngle);
    float backRampartRightOuterWallY = backRampartRightOuterWallBaseY - OUTER_WALL_LATERAL_OFFSET_FRONT * sin(perpAngle);
    spawnAt(wallEntry, backRampartRightOuterWallX, backRampartRightOuterWallY, fortGateZ, backwardAngle, CONQUEST_BUILD_WALL);
    
    // Back rampart right end tower (tour d'extrémité, décalée du deuxième mur et vers la droite) - même que devant
    spawnAt(towerEntry, backRampartRightEndTowerX, backRampartRightEndTowerY, fortGateZ, backwardAngle, CONQUEST_BUILD_TOWER);
    // Décalage latéral des stone benches vers la droite (vers l'extérieur)
    float backRightEndTowerLateralOffset = -6.0f; // Vers la droite (vers l'extérieur) - augmenté
    SpawnStoneBenchesForTower(player, backRampartRightEndTowerX, backRampartRightEndTowerY, fortGateZ, backwardAngle, groupId, guildId, backRightEndTowerLateralOffset);
    SpawnCannonsForTower(player, backRampartRightEndTowerX, backRampartRightEndTowerY, fortGateZ, backwardAngle, groupId, guildId);
    
    // LEFT SIDE: 5 structures (5 murs + 2 tours) entre la tour avant gauche et la tour arrière gauche
    // Offsets parfaits calculés pour grandes structures (doublés par rapport aux petites)
    const float PERFECT_TOWER_TO_WALL_OFFSET = TOWER_SIZE / 2.0f + 2.0f + WALL_LENGTH / 2.0f + 4.0f; // = 7.75 + 2.0 + 10.0 + 4.0 = 23.75
    const float PERFECT_WALL_TO_WALL_OFFSET = WALL_LENGTH / 2.0f + 15.0f + WALL_LENGTH / 2.0f; // = 10.0 + 15.0 + 10.0 = 35.0
    
    // Premier mur : décalage latéral de 3 yards vers l'intérieur
    const float INWARD_OFFSET = 3.0f; // Décalage latéral de 3 yards vers l'intérieur pour les lignes latérales
    const float FIRST_WALL_DOWNWARD_OFFSET = 3.0f; // Premier mur commence 3 yards plus en dessous
    const float WALL_TO_TOWER_SPACING = 1.0f; // Espacement de 1 yard entre les murs et la tour
    const float FIRST_TOWER_DOWNWARD_OFFSET = 1.0f; // Première tour décalée de 1 yard vers le bas
    const float SECOND_TOWER_DOWNWARD_OFFSET = 0.0f; // Deuxième tour sans décalage vertical
    
    // Position de base du premier mur (3 yards plus en dessous de la tour)
    float firstLeftWallBaseX = frontRampartLeftEndTowerX + (TOWER_SIZE / 2.0f + WALL_LENGTH / 2.0f + FIRST_WALL_DOWNWARD_OFFSET) * cos(backwardAngle);
    float firstLeftWallBaseY = frontRampartLeftEndTowerY + (TOWER_SIZE / 2.0f + WALL_LENGTH / 2.0f + FIRST_WALL_DOWNWARD_OFFSET) * sin(backwardAngle);
    
    // Décalage latéral vers l'intérieur (vers la droite, donc négatif par rapport à perpAngle) + 4 yards vers l'extérieur
    float lateralOffsetX = -INWARD_OFFSET * cos(perpAngle) + 4.0f * cos(perpAngle);
    float lateralOffsetY = -INWARD_OFFSET * sin(perpAngle) + 4.0f * sin(perpAngle);
    
    // Premier mur avec décalage latéral
    float firstLeftWallX = firstLeftWallBaseX + lateralOffsetX;
    float firstLeftWallY = firstLeftWallBaseY + lateralOffsetY;
    spawnAt(wallEntry, firstLeftWallX, firstLeftWallY, fortGateZ, perpAngle, CONQUEST_BUILD_WALL);
    
    // Première tour : espacée du premier mur (1 yard d'espacement) et décalée de 1 yard vers le bas
    float firstTowerOffset = TOWER_SIZE / 2.0f + WALL_LENGTH / 2.0f + FIRST_WALL_DOWNWARD_OFFSET + WALL_LENGTH / 2.0f + WALL_TO_TOWER_SPACING + TOWER_SIZE / 2.0f + FIRST_TOWER_DOWNWARD_OFFSET;
    float firstTowerBaseX = frontRampartLeftEndTowerX + firstTowerOffset * cos(backwardAngle);
    float firstTowerBaseY = frontRampartLeftEndTowerY + firstTowerOffset * sin(backwardAngle);
    float firstTowerX = firstTowerBaseX + lateralOffsetX;
    float firstTowerY = firstTowerBaseY + lateralOffsetY;
    spawnAt(towerEntry, firstTowerX, firstTowerY, fortGateZ, perpAngle, CONQUEST_BUILD_TOWER);
    SpawnStoneBenchesForTower(player, firstTowerX, firstTowerY, fortGateZ, perpAngle, groupId, guildId, 0.0f);
    SpawnCannonsForTower(player, firstTowerX, firstTowerY, fortGateZ, perpAngle, groupId, guildId);
    
    // Deuxième mur : utilise l'offset parfait tour->mur
    float secondWallOffset = firstTowerOffset + PERFECT_TOWER_TO_WALL_OFFSET;
    float secondWallBaseX = frontRampartLeftEndTowerX + secondWallOffset * cos(backwardAngle);
    float secondWallBaseY = frontRampartLeftEndTowerY + secondWallOffset * sin(backwardAngle);
    float secondWallX = secondWallBaseX + lateralOffsetX;
    float secondWallY = secondWallBaseY + lateralOffsetY;
    spawnAt(wallEntry, secondWallX, secondWallY, fortGateZ, perpAngle, CONQUEST_BUILD_WALL);
    
    // Troisième mur : utilise l'offset parfait mur->mur
    float thirdWallOffset = secondWallOffset + PERFECT_WALL_TO_WALL_OFFSET;
    float thirdWallBaseX = frontRampartLeftEndTowerX + thirdWallOffset * cos(backwardAngle);
    float thirdWallBaseY = frontRampartLeftEndTowerY + thirdWallOffset * sin(backwardAngle);
    float thirdWallX = thirdWallBaseX + lateralOffsetX;
    float thirdWallY = thirdWallBaseY + lateralOffsetY;
    spawnAt(wallEntry, thirdWallX, thirdWallY, fortGateZ, perpAngle, CONQUEST_BUILD_WALL);
    
    // Quatrième mur : ajouté au milieu, utilise l'offset parfait mur->mur
    float fourthWallOffset = thirdWallOffset + PERFECT_WALL_TO_WALL_OFFSET;
    float fourthWallBaseX = frontRampartLeftEndTowerX + fourthWallOffset * cos(backwardAngle);
    float fourthWallBaseY = frontRampartLeftEndTowerY + fourthWallOffset * sin(backwardAngle);
    float fourthWallX = fourthWallBaseX + lateralOffsetX;
    float fourthWallY = fourthWallBaseY + lateralOffsetY;
    spawnAt(wallEntry, fourthWallX, fourthWallY, fortGateZ, perpAngle, CONQUEST_BUILD_WALL);
    
    // Deuxième tour : utilise l'offset parfait mur->tour (inverse de tour->mur, mais sans le décalage vertical)
    float secondTowerOffset = fourthWallOffset + WALL_LENGTH / 2.0f + WALL_TO_TOWER_SPACING + TOWER_SIZE / 2.0f + SECOND_TOWER_DOWNWARD_OFFSET;
    float secondTowerBaseX = frontRampartLeftEndTowerX + secondTowerOffset * cos(backwardAngle);
    float secondTowerBaseY = frontRampartLeftEndTowerY + secondTowerOffset * sin(backwardAngle);
    float secondTowerX = secondTowerBaseX + lateralOffsetX;
    float secondTowerY = secondTowerBaseY + lateralOffsetY;
    spawnAt(towerEntry, secondTowerX, secondTowerY, fortGateZ, perpAngle, CONQUEST_BUILD_TOWER);
    SpawnStoneBenchesForTower(player, secondTowerX, secondTowerY, fortGateZ, perpAngle, groupId, guildId, 0.0f);
    SpawnCannonsForTower(player, secondTowerX, secondTowerY, fortGateZ, perpAngle, groupId, guildId);
    
    // Cinquième mur : calculé pour se connecter à la tour arrière gauche
    // Distance entre les tours avant et arrière
    float distanceToBackTower = sqrt(pow(backRampartLeftEndTowerX - frontRampartLeftEndTowerX, 2) + pow(backRampartLeftEndTowerY - frontRampartLeftEndTowerY, 2));
    // Position du cinquième mur : juste avant la tour arrière (en utilisant l'offset parfait tour->mur depuis la tour arrière, mais en sens inverse) - éloigné de 2 yards
    float fifthWallOffset = distanceToBackTower - (TOWER_SIZE / 2.0f + WALL_LENGTH / 2.0f) - 2.0f;
    float fifthWallBaseX = frontRampartLeftEndTowerX + fifthWallOffset * cos(backwardAngle);
    float fifthWallBaseY = frontRampartLeftEndTowerY + fifthWallOffset * sin(backwardAngle);
    float fifthWallX = fifthWallBaseX + lateralOffsetX;
    float fifthWallY = fifthWallBaseY + lateralOffsetY;
    spawnAt(wallEntry, fifthWallX, fifthWallY, fortGateZ, perpAngle, CONQUEST_BUILD_WALL);
    
    // RIGHT SIDE: 5 structures (5 murs + 2 tours) entre la tour avant droite et la tour arrière droite
    // Position de base du premier mur (3 yards plus en dessous de la tour)
    float firstRightWallBaseX = frontRampartRightEndTowerX + (TOWER_SIZE / 2.0f + WALL_LENGTH / 2.0f + FIRST_WALL_DOWNWARD_OFFSET) * cos(backwardAngle);
    float firstRightWallBaseY = frontRampartRightEndTowerY + (TOWER_SIZE / 2.0f + WALL_LENGTH / 2.0f + FIRST_WALL_DOWNWARD_OFFSET) * sin(backwardAngle);
    
    // Décalage latéral vers l'intérieur (vers la gauche, donc positif par rapport à perpAngle) + 4 yards vers l'extérieur
    float rightLateralOffsetX = INWARD_OFFSET * cos(perpAngle) - 4.0f * cos(perpAngle);
    float rightLateralOffsetY = INWARD_OFFSET * sin(perpAngle) - 4.0f * sin(perpAngle);
    
    // Premier mur avec décalage latéral
    float firstRightWallX = firstRightWallBaseX + rightLateralOffsetX;
    float firstRightWallY = firstRightWallBaseY + rightLateralOffsetY;
    spawnAt(wallEntry, firstRightWallX, firstRightWallY, fortGateZ, perpAngle, CONQUEST_BUILD_WALL);
    
    // Première tour : espacée du premier mur (1 yard d'espacement) et décalée de 1 yard vers le bas
    float rightFirstTowerOffset = TOWER_SIZE / 2.0f + WALL_LENGTH / 2.0f + FIRST_WALL_DOWNWARD_OFFSET + WALL_LENGTH / 2.0f + WALL_TO_TOWER_SPACING + TOWER_SIZE / 2.0f + FIRST_TOWER_DOWNWARD_OFFSET;
    float rightFirstTowerBaseX = frontRampartRightEndTowerX + rightFirstTowerOffset * cos(backwardAngle);
    float rightFirstTowerBaseY = frontRampartRightEndTowerY + rightFirstTowerOffset * sin(backwardAngle);
    float rightFirstTowerX = rightFirstTowerBaseX + rightLateralOffsetX;
    float rightFirstTowerY = rightFirstTowerBaseY + rightLateralOffsetY;
    spawnAt(towerEntry, rightFirstTowerX, rightFirstTowerY, fortGateZ, perpAngle + M_PI, CONQUEST_BUILD_TOWER);
    SpawnStoneBenchesForTower(player, rightFirstTowerX, rightFirstTowerY, fortGateZ, perpAngle + M_PI, groupId, guildId, 0.0f);
    SpawnCannonsForTower(player, rightFirstTowerX, rightFirstTowerY, fortGateZ, perpAngle + M_PI, groupId, guildId);
    
    // Deuxième mur : utilise l'offset parfait tour->mur
    float rightSecondWallOffset = rightFirstTowerOffset + PERFECT_TOWER_TO_WALL_OFFSET;
    float rightSecondWallBaseX = frontRampartRightEndTowerX + rightSecondWallOffset * cos(backwardAngle);
    float rightSecondWallBaseY = frontRampartRightEndTowerY + rightSecondWallOffset * sin(backwardAngle);
    float rightSecondWallX = rightSecondWallBaseX + rightLateralOffsetX;
    float rightSecondWallY = rightSecondWallBaseY + rightLateralOffsetY;
    spawnAt(wallEntry, rightSecondWallX, rightSecondWallY, fortGateZ, perpAngle, CONQUEST_BUILD_WALL);
    
    // Troisième mur : utilise l'offset parfait mur->mur
    float rightThirdWallOffset = rightSecondWallOffset + PERFECT_WALL_TO_WALL_OFFSET;
    float rightThirdWallBaseX = frontRampartRightEndTowerX + rightThirdWallOffset * cos(backwardAngle);
    float rightThirdWallBaseY = frontRampartRightEndTowerY + rightThirdWallOffset * sin(backwardAngle);
    float rightThirdWallX = rightThirdWallBaseX + rightLateralOffsetX;
    float rightThirdWallY = rightThirdWallBaseY + rightLateralOffsetY;
    spawnAt(wallEntry, rightThirdWallX, rightThirdWallY, fortGateZ, perpAngle, CONQUEST_BUILD_WALL);
    
    // Quatrième mur : ajouté au milieu, utilise l'offset parfait mur->mur
    float rightFourthWallOffset = rightThirdWallOffset + PERFECT_WALL_TO_WALL_OFFSET;
    float rightFourthWallBaseX = frontRampartRightEndTowerX + rightFourthWallOffset * cos(backwardAngle);
    float rightFourthWallBaseY = frontRampartRightEndTowerY + rightFourthWallOffset * sin(backwardAngle);
    float rightFourthWallX = rightFourthWallBaseX + rightLateralOffsetX;
    float rightFourthWallY = rightFourthWallBaseY + rightLateralOffsetY;
    spawnAt(wallEntry, rightFourthWallX, rightFourthWallY, fortGateZ, perpAngle, CONQUEST_BUILD_WALL);
    
    // Deuxième tour : utilise l'offset parfait mur->tour (inverse de tour->mur, mais sans le décalage vertical)
    float rightSecondTowerOffset = rightFourthWallOffset + WALL_LENGTH / 2.0f + WALL_TO_TOWER_SPACING + TOWER_SIZE / 2.0f + SECOND_TOWER_DOWNWARD_OFFSET;
    float rightSecondTowerBaseX = frontRampartRightEndTowerX + rightSecondTowerOffset * cos(backwardAngle);
    float rightSecondTowerBaseY = frontRampartRightEndTowerY + rightSecondTowerOffset * sin(backwardAngle);
    float rightSecondTowerX = rightSecondTowerBaseX + rightLateralOffsetX;
    float rightSecondTowerY = rightSecondTowerBaseY + rightLateralOffsetY;
    spawnAt(towerEntry, rightSecondTowerX, rightSecondTowerY, fortGateZ, perpAngle + M_PI, CONQUEST_BUILD_TOWER);
    SpawnStoneBenchesForTower(player, rightSecondTowerX, rightSecondTowerY, fortGateZ, perpAngle + M_PI, groupId, guildId, 0.0f);
    SpawnCannonsForTower(player, rightSecondTowerX, rightSecondTowerY, fortGateZ, perpAngle + M_PI, groupId, guildId);
    
    // Cinquième mur : calculé pour se connecter à la tour arrière droite
    // Distance entre les tours avant et arrière
    float rightDistanceToBackTower = sqrt(pow(backRampartRightEndTowerX - frontRampartRightEndTowerX, 2) + pow(backRampartRightEndTowerY - frontRampartRightEndTowerY, 2));
    // Position du cinquième mur : juste avant la tour arrière (en utilisant l'offset parfait tour->mur depuis la tour arrière, mais en sens inverse) - éloigné de 2 yards
    float rightFifthWallOffset = rightDistanceToBackTower - (TOWER_SIZE / 2.0f + WALL_LENGTH / 2.0f) - 2.0f;
    float rightFifthWallBaseX = frontRampartRightEndTowerX + rightFifthWallOffset * cos(backwardAngle);
    float rightFifthWallBaseY = frontRampartRightEndTowerY + rightFifthWallOffset * sin(backwardAngle);
    float rightFifthWallX = rightFifthWallBaseX + rightLateralOffsetX;
    float rightFifthWallY = rightFifthWallBaseY + rightLateralOffsetY;
    spawnAt(wallEntry, rightFifthWallX, rightFifthWallY, fortGateZ, perpAngle, CONQUEST_BUILD_WALL);
    
    LOG_INFO("module", "ConquestBuild: SpawnFortRampartsGrand completed successfully");
}

