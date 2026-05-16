/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

#pragma once

#include <map>
#include <cstdint>
#include <vector>

struct PlayerStats
{
    uint64_t guid = 0;
    uint32_t kills = 0;
    uint32_t deaths = 0;
    uint64_t last_updated = 0;
};

struct GuildStats
{
    uint32_t guildId = 0;
    uint32_t zoneControlled = 0;
    uint64_t goldEarned = 0;
};

class ConquestCore
{
public:
    static ConquestCore& Instance();

    void OnPlayerKill(uint64_t killerGuid, uint64_t victimGuid);
    void OnPlayerDeath(uint64_t victimGuid);
    void OnPlayerLogin(uint64_t playerGuid);

    // relations API
    void LoadRelationsFromDB();
    void SaveRelationsToDB();
    void SetGuildRelation(uint32_t a, uint32_t b, int8_t relation, uint32_t expire = 0);
    int8_t GetGuildRelation(uint32_t a, uint32_t b) const;
    bool AreGuildsHostile(uint32_t a, uint32_t b) const;

private:
    ConquestCore();
    std::map<uint64_t, PlayerStats> playerStats;
    std::map<uint32_t, GuildStats> guildStats;
    // key is ((uint64_t)a << 32) | b
    std::map<uint64_t, int8_t> guildRelations;
};

