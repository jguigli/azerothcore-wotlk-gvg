/*
 * Conquest Map Restrict — restreint l'accès aux maps non-Azeroth + instances + BG
 *
 * Règles :
 *   - Map 0 (Eastern Kingdoms) et 1 (Kalimdor) : toujours autorisé
 *   - Map 530 (Outland) : bloqué SAUF si la zone du joueur est dans la whitelist
 *     (par défaut : Eversong, Ghostlands, Silvermoon, Isle of Quel'Danas,
 *      Azuremyst, Bloodmyst, Exodar)
 *   - Map 571 (Northrend) : bloqué SAUF whitelist (vide par défaut)
 *   - Toutes maps de type Dungeon/Raid : bloqué
 *   - BG et Arènes : bloqué via hook spécifique
 *
 * Hooks utilisés :
 *   - OnPlayerCanEnterMap         : bloque l'entrée map
 *   - OnPlayerMapChanged          : safety net si on est arrivé quand même
 *   - OnPlayerCanJoinInBattlegroundQueue
 *   - OnPlayerCanJoinInArenaQueue
 */

#include "ScriptMgr.h"
#include "Player.h"
#include "Map.h"
#include "MapMgr.h"
#include "DBCStores.h"
#include "Battleground.h"
#include "Config.h"
#include "Log.h"
#include <sstream>
#include <unordered_set>

namespace
{
    constexpr uint32 CONQUEST_MAP_EK        = 0;
    constexpr uint32 CONQUEST_MAP_KALIMDOR  = 1;
    constexpr uint32 CONQUEST_MAP_OUTLAND   = 530;
    constexpr uint32 CONQUEST_MAP_NORTHREND = 571;

    bool IsEnabled()
    {
        return sConfigMgr->GetOption<bool>("ConquestMapRestrict.Enable", true);
    }

    bool BlockInstances()
    {
        return sConfigMgr->GetOption<bool>("ConquestMapRestrict.BlockInstances", true);
    }

    bool BlockBattlegrounds()
    {
        return sConfigMgr->GetOption<bool>("ConquestMapRestrict.BlockBattlegrounds", true);
    }

    std::unordered_set<uint32> ParseZoneList(std::string const& csv)
    {
        std::unordered_set<uint32> result;
        std::stringstream ss(csv);
        std::string token;
        while (std::getline(ss, token, ','))
        {
            // trim
            size_t a = token.find_first_not_of(" \t\"");
            size_t b = token.find_last_not_of(" \t\"");
            if (a == std::string::npos)
                continue;
            std::string clean = token.substr(a, b - a + 1);
            try
            {
                uint32 id = std::stoul(clean);
                result.insert(id);
            }
            catch (...)
            {
                // ignore invalid entries
            }
        }
        return result;
    }

    std::unordered_set<uint32> GetWhitelist530()
    {
        static std::unordered_set<uint32> cached;
        static bool initialized = false;
        if (!initialized)
        {
            cached = ParseZoneList(sConfigMgr->GetOption<std::string>(
                "ConquestMapRestrict.WhitelistZones530", "3430,3433,3487,4080,3524,3525,3557"));
            initialized = true;
        }
        return cached;
    }

    std::unordered_set<uint32> GetWhitelist571()
    {
        static std::unordered_set<uint32> cached;
        static bool initialized = false;
        if (!initialized)
        {
            cached = ParseZoneList(sConfigMgr->GetOption<std::string>(
                "ConquestMapRestrict.WhitelistZones571", ""));
            initialized = true;
        }
        return cached;
    }

    // Vérifie si la map+zone est autorisée selon les règles Conquest
    bool IsAllowedDestination(uint32 mapId, uint32 zoneId)
    {
        if (mapId == CONQUEST_MAP_EK || mapId == CONQUEST_MAP_KALIMDOR)
            return true;

        if (mapId == CONQUEST_MAP_OUTLAND)
        {
            auto const& wl = GetWhitelist530();
            return wl.find(zoneId) != wl.end();
        }

        if (mapId == CONQUEST_MAP_NORTHREND)
        {
            auto const& wl = GetWhitelist571();
            return wl.find(zoneId) != wl.end();
        }

        // Autre map (kalimdor instances, lost isles, etc.) — bloqué par défaut
        return false;
    }
} // namespace

class ConquestMapRestrictPlayer : public PlayerScript
{
public:
    ConquestMapRestrictPlayer() : PlayerScript("ConquestMapRestrictPlayer") { }

    // Bloque l'entrée dans une map non-autorisée
    bool OnPlayerCanEnterMap(Player* player, MapEntry const* entry, InstanceTemplate const* /*instance*/,
                              MapDifficulty const* /*mapDiff*/, bool /*loginCheck*/) override
    {
        if (!IsEnabled())
            return true;

        if (!player || !entry)
            return true;

        if (player->IsGameMaster())
            return true;

        // Block instances / raids
        if (BlockInstances() && (entry->IsDungeon() || entry->IsRaid()))
        {
            LOG_INFO("conquest", "MapRestrict: '{}' attempted to enter dungeon/raid (map {}), refused",
                     player->GetName(), entry->MapID);
            player->SendTransferAborted(entry->MapID, TRANSFER_ABORT_MAP_NOT_ALLOWED);
            return false;
        }

        // Block Outland/Northrend zones non-whitelistées
        // Note : on ne connait pas encore la zone exacte d'arrivée à ce stade,
        // on filtre juste sur map. Le hook OnPlayerMapChanged ci-dessous fera le check précis
        // si on tente une zone interdite après l'entrée.
        if (entry->MapID == CONQUEST_MAP_OUTLAND || entry->MapID == CONQUEST_MAP_NORTHREND)
        {
            // Si la whitelist est vide pour cette map → on bloque l'entrée map directement
            auto const& wl = (entry->MapID == CONQUEST_MAP_OUTLAND) ? GetWhitelist530() : GetWhitelist571();
            if (wl.empty())
            {
                LOG_INFO("conquest", "MapRestrict: '{}' attempted to enter blocked map {} (no whitelist)",
                         player->GetName(), entry->MapID);
                player->SendTransferAborted(entry->MapID, TRANSFER_ABORT_MAP_NOT_ALLOWED);
                return false;
            }
            // Sinon laisse passer, le hook OnPlayerMapChanged vérifiera la zone
        }

        return true;
    }

    // Safety net : si le joueur arrive sur une zone interdite (téléport, hearthstone, portail),
    // on le téléport hors de cette zone
    void OnPlayerMapChanged(Player* player) override
    {
        if (!IsEnabled())
            return;

        if (!player)
            return;

        if (player->IsGameMaster())
            return;

        uint32 mapId = player->GetMapId();
        uint32 zoneId = player->GetZoneId();

        if (IsAllowedDestination(mapId, zoneId))
            return;

        // Zone interdite — kick vers homebind ou capitale par défaut
        LOG_INFO("conquest", "MapRestrict: '{}' arrived in forbidden zone (map={}, zone={}), teleporting home",
                 player->GetName(), mapId, zoneId);

        // Tp vers homebind (qui devrait être la capitale grâce à mod-conquest-respawn-capital)
        player->TeleportTo(player->m_homebindMapId, player->m_homebindX, player->m_homebindY,
                            player->m_homebindZ, 0.0f);
    }

    // Bloque la queue BG
    bool OnPlayerCanJoinInBattlegroundQueue(Player* player, ObjectGuid /*battlemasterGuid*/,
                                             BattlegroundTypeId /*bgTypeId*/, uint8 /*joinAsGroup*/,
                                             GroupJoinBattlegroundResult& err) override
    {
        if (!IsEnabled() || !BlockBattlegrounds())
            return true;

        if (!player || player->IsGameMaster())
            return true;

        LOG_INFO("conquest", "MapRestrict: '{}' attempted to join BG queue, refused", player->GetName());
        err = ERR_BATTLEGROUND_JOIN_FAILED;
        return false;
    }

    // Bloque la queue Arena
    bool OnPlayerCanJoinInArenaQueue(Player* player, ObjectGuid /*battlemasterGuid*/, uint8 /*arenaslot*/,
                                      BattlegroundTypeId /*bgTypeId*/, uint8 /*joinAsGroup*/,
                                      uint8 /*isRated*/, GroupJoinBattlegroundResult& err) override
    {
        if (!IsEnabled() || !BlockBattlegrounds())
            return true;

        if (!player || player->IsGameMaster())
            return true;

        LOG_INFO("conquest", "MapRestrict: '{}' attempted to join arena queue, refused", player->GetName());
        err = ERR_BATTLEGROUND_JOIN_FAILED;
        return false;
    }
};

void AddSC_ConquestMapRestrict()
{
    new ConquestMapRestrictPlayer();
}
