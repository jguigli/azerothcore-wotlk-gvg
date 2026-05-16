/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

#ifndef CONQUEST_BUILD_COMMON_H
#define CONQUEST_BUILD_COMMON_H

#include "ScriptMgr.h"
#include "Player.h"
#include "Config.h"
#include "Chat.h"
#include "Item.h"
#include "GameObject.h"
#include "DatabaseEnv.h"
#include "MapMgr.h"
#include "ObjectMgr.h"
#include "ObjectGuid.h"
#include "Map.h"
#include "Creature.h"
#include "Spell.h"
#include <list>
#include <cmath>

enum ConquestBuildStrings
{
    CONQUEST_BUILD_DISABLED = 35411,
    CONQUEST_BUILD_SUCCESS = 35412,
    CONQUEST_BUILD_ERROR = 35413,
    CONQUEST_BUILD_INVALID_LOCATION = 35414,
    CONQUEST_BUILD_TOO_CLOSE = 35415,
    CONQUEST_BUILD_MAX_REACHED = 35416
};

enum ConquestBuildType
{
    CONQUEST_BUILD_WALL = 0,
    CONQUEST_BUILD_TOWER = 1,
    CONQUEST_BUILD_GATE = 2,
    CONQUEST_BUILD_TELEPORT_BEACON = 3,
    CONQUEST_BUILD_ATELIER_GOBELIN = 4,
    CONQUEST_BUILD_GATE_SMALL = 5,
    CONQUEST_BUILD_FORT = 6,
    CONQUEST_BUILD_GRAND_FORT = 7,
    CONQUEST_BUILD_GRAND_FORT_2 = 8,
    CONQUEST_BUILD_GRAND_FORT_3 = 9
};

struct ConquestBuildConfig
{
    uint32 itemId;
    uint32 wallGobId;
    uint32 towerGobId;
    uint32 gateGobId;
    float minDistanceBetweenStructures;
    uint32 maxStructuresPerPlayer;
    bool enabled;
};

class ConquestBuildMgr
{
public:
    static ConquestBuildMgr* instance()
    {
        static ConquestBuildMgr instance;
        return &instance;
    }

    void LoadConfig();
    ConquestBuildConfig GetConfig() const { return config; }
    bool CanBuildAt(Player* player, Position const& pos);
    uint32 GetPlayerStructureCount(uint64 playerGuid);
    void SpawnStoneBenchesForTower(Player* player, float towerX, float towerY, float towerZ, float towerOrientation, uint64 groupId, uint32 guildId, float lateralOffset = 0.0f);
    void SpawnCannonsForTower(Player* player, float towerX, float towerY, float towerZ, float towerOrientation, uint64 groupId, uint32 guildId);
    void SpawnStoneBenchesForSmallTower(Player* player, float towerX, float towerY, float towerZ, float towerOrientation, uint64 groupId, uint32 guildId, float lateralOffset = 0.0f);
    void SpawnLeversForGate(Player* player, float gateX, float gateY, float gateZ, float gateOrientation, uint64 groupId, uint32 guildId, ConquestBuildType buildType);
    void SpawnCannonForSmallTower(Player* player, float towerX, float towerY, float towerZ, float towerOrientation, uint64 groupId, uint32 guildId);
    bool SpawnStructure(Player* player, uint32 gobEntry, ConquestBuildType buildType, SpellCastTargets const* targets = nullptr, uint64* outGuidLow = nullptr, uint64* outGroupId = nullptr);
    void RemoveStructure(uint64 goSpawnId, Map* map = nullptr);
    void CheckAndSpawnFires(GameObject* structure, int32 healthChange = 0);
    void RemoveFiresForStructure(uint64 structureSpawnId, Map* map);
    bool SpawnSimpleGameObject(Player* player, uint32 gobEntry, float x, float y, float z, float orientation, uint64 groupId, uint32 guildId, ConquestBuildType buildType);
    void SpawnFortRamparts(Player* player, float fortGateX, float fortGateY, float fortGateZ, float fortGateOrientation, uint64 groupId, uint32 guildId);
    void SpawnFortRampartsGrand(Player* player, float fortGateX, float fortGateY, float fortGateZ, float fortGateOrientation, uint64 groupId, uint32 guildId);
    void SpawnFortMainStructures(Player* player, float fortGateX, float fortGateY, float fortGateZ, float fortGateOrientation, uint64 groupId, uint32 guildId, ConquestBuildType buildType);

private:
    // Structure spawn parameters
    struct SpawnParams
    {
        Player* player;
        uint32 gobEntry;
        ConquestBuildType buildType;
        float x, y, z;
        float orientation;
        uint32 guildId;
        ObjectGuid::LowType guidLow;
        uint64 groupId;
    };

    // Common spawn logic
    bool CreateAndSaveGameObject(SpawnParams& params, float finalX, float finalY, float finalZ, float finalOrientation);
    
    // Build type specific spawn functions
    bool SpawnWall(SpawnParams& params);
    bool SpawnTower(SpawnParams& params);
    bool SpawnGate(SpawnParams& params, bool isSmallGate);
    bool SpawnFort(SpawnParams& params, bool isGrandFort, bool isGrandFort2);
    bool SpawnSimpleStructure(SpawnParams& params);

    ConquestBuildConfig config;
};

#endif // CONQUEST_BUILD_COMMON_H

