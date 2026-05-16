/*
 * Conquest Frontline — Wallet joueur, impl.
 */

#include "ConquestPoints.h"
#include "CharacterDatabase.h"
#include "DatabaseEnv.h"
#include "Chat.h"
#include "Player.h"
#include "Log.h"

namespace
{
    uint32 FetchPoints(uint32 guid, char const* column)
    {
        QueryResult result = CharacterDatabase.Query(
            "SELECT {} FROM character_conquest_points WHERE guid = {}", column, guid);
        if (!result)
            return 0;
        return result->Fetch()[0].Get<uint32>();
    }
}

namespace ConquestPoints
{

uint32 GetConquestPoints(uint32 guid)
{
    return FetchPoints(guid, "conquest_points");
}

uint32 GetBattlePoints(uint32 guid)
{
    return FetchPoints(guid, "battle_points");
}

void AddConquestPoints(Player* player, uint32 amount)
{
    if (!player || amount == 0)
        return;

    uint32 guid = player->GetGUID().GetCounter();
    CharacterDatabase.Execute(
        "INSERT INTO character_conquest_points (guid, conquest_points, updated_at) "
        "VALUES ({}, {}, UNIX_TIMESTAMP()) "
        "ON DUPLICATE KEY UPDATE conquest_points = conquest_points + {}, updated_at = UNIX_TIMESTAMP()",
        guid, amount, amount);

    if (player->GetSession())
        ChatHandler(player->GetSession()).PSendSysMessage(
            "|cff00ff00+{} Point(s) de Conqu\xC3\xAAte|r", amount);

    LOG_INFO("conquest", "ConquestPoints: +{} PC to '{}' (guid={})",
             amount, player->GetName(), guid);
}

void AddBattlePoints(Player* player, uint32 amount)
{
    if (!player || amount == 0)
        return;

    uint32 guid = player->GetGUID().GetCounter();
    CharacterDatabase.Execute(
        "INSERT INTO character_conquest_points (guid, battle_points, updated_at) "
        "VALUES ({}, {}, UNIX_TIMESTAMP()) "
        "ON DUPLICATE KEY UPDATE battle_points = battle_points + {}, updated_at = UNIX_TIMESTAMP()",
        guid, amount, amount);

    if (player->GetSession())
        ChatHandler(player->GetSession()).PSendSysMessage(
            "|cffffae42+{} Point(s) de Bataille|r", amount);

    LOG_INFO("conquest", "ConquestPoints: +{} PB to '{}' (guid={})",
             amount, player->GetName(), guid);
}

bool SpendConquestPoints(Player* player, uint32 amount)
{
    if (!player || amount == 0)
        return false;

    uint32 guid = player->GetGUID().GetCounter();
    if (GetConquestPoints(guid) < amount)
        return false;

    CharacterDatabase.Execute(
        "UPDATE character_conquest_points SET conquest_points = conquest_points - {}, "
        "updated_at = UNIX_TIMESTAMP() WHERE guid = {}",
        amount, guid);
    return true;
}

bool SpendBattlePoints(Player* player, uint32 amount)
{
    if (!player || amount == 0)
        return false;

    uint32 guid = player->GetGUID().GetCounter();
    if (GetBattlePoints(guid) < amount)
        return false;

    CharacterDatabase.Execute(
        "UPDATE character_conquest_points SET battle_points = battle_points - {}, "
        "updated_at = UNIX_TIMESTAMP() WHERE guid = {}",
        amount, guid);
    return true;
}

} // namespace ConquestPoints
