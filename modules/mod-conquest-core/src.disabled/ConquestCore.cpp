/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

#include "ConquestCore.h"
#include "DatabaseEnv.h"
#include "QueryResult.h"
#include "Field.h"
#include "Log.h"
#include <ctime>

ConquestCore& ConquestCore::Instance()
{
    static ConquestCore instance;
    return instance;
}

ConquestCore::ConquestCore() {}

void ConquestCore::OnPlayerKill(uint64_t killerGuid, uint64_t victimGuid)
{
    auto &k = playerStats[killerGuid];
    k.guid = killerGuid;
    k.kills += 1;
    k.last_updated = (uint64_t)time(nullptr);

    auto &v = playerStats[victimGuid];
    v.guid = victimGuid;
    v.deaths += 1;
    v.last_updated = (uint64_t)time(nullptr);
}

void ConquestCore::OnPlayerDeath(uint64_t victimGuid)
{
    auto &v = playerStats[victimGuid];
    v.deaths += 1;
    v.last_updated = (uint64_t)time(nullptr);
}

void ConquestCore::OnPlayerLogin(uint64_t playerGuid)
{
    auto &p = playerStats[playerGuid];
    p.guid = playerGuid;
}

void ConquestCore::LoadRelationsFromDB()
{
    guildRelations.clear();
    QueryResult result = WorldDatabase.Query("SELECT guild_a, guild_b, relation, expire_at FROM conquest_guild_relations");
    if (!result) return;

    do
    {
        Field* fields = result->Fetch();
        uint32_t a = fields[0].Get<uint32>();
        uint32_t b = fields[1].Get<uint32>();
        int8_t rel = fields[2].Get<int8>();
        uint32_t expire = fields[3].Get<uint32>();
        uint64_t key = ((uint64_t)a << 32) | b;
        if (expire != 0 && expire < (uint32_t)time(nullptr))
            rel = 0; // expired -> neutral
        guildRelations[key] = rel;
    } while (result->NextRow());

    LOG_INFO("module", "Loaded {} guild relations.", guildRelations.size());
}

void ConquestCore::SaveRelationsToDB()
{
    // simple REPLACE loop
    for (auto &p : guildRelations)
    {
        uint64_t key = p.first;
        uint32_t a = (uint32_t)(key >> 32);
        uint32_t b = (uint32_t)(key & 0xFFFFFFFF);
        int8_t rel = p.second;
        WorldDatabase.Execute("REPLACE INTO conquest_guild_relations (guild_a, guild_b, relation, expire_at) VALUES ({},{},{},0)", a, b, (int)rel);
    }
}

void ConquestCore::SetGuildRelation(uint32_t a, uint32_t b, int8_t relation, uint32_t /*expire*/)
{
    uint64_t key = ((uint64_t)a << 32) | b;
    guildRelations[key] = relation;
    // Also store reciprocal if you want symmetry
    uint64_t key2 = ((uint64_t)b << 32) | a;
    guildRelations[key2] = relation;
    // persist
    SaveRelationsToDB();
}

int8_t ConquestCore::GetGuildRelation(uint32_t a, uint32_t b) const
{
    uint64_t key = ((uint64_t)a << 32) | b;
    auto it = guildRelations.find(key);
    if (it != guildRelations.end())
        return it->second;
    return 0;
}

bool ConquestCore::AreGuildsHostile(uint32_t a, uint32_t b) const
{
    int8_t rel = GetGuildRelation(a,b);
    if (rel == -1) return true;
    if (rel == 1) return false;
    // default behavior: different guilds are hostile
    if (a != b) return true;
    return false;
}

