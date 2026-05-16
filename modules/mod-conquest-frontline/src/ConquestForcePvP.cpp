/*
 * Conquest Frontline — Force PvP permanent.
 *
 * Sur le serveur Conquest, tous les joueurs sont en guerre permanente sur les maps Azeroth.
 * On force le flag PvP au login + à chaque changement de map/zone, ce qui :
 *   1. Autorise les attaques cross-faction sans /pvp manuel
 *   2. Rend les joueurs éligibles à OutdoorPvP::IsOutdoorPvPActive (capture des banners)
 */

#include "Config.h"
#include "GameTime.h"
#include "Log.h"
#include "Player.h"
#include "ScriptMgr.h"

#include <unordered_map>

namespace
{
    constexpr uint32 MAP_EK       = 0;
    constexpr uint32 MAP_KALIMDOR = 1;

    bool ShouldForcePvP(Player* player)
    {
        if (!player)
            return false;
        if (player->IsGameMaster())
            return false;
        uint32 mapId = player->GetMapId();
        return mapId == MAP_EK || mapId == MAP_KALIMDOR;
    }

    void ApplyPvPFlag(Player* player, bool log = true)
    {
        if (!ShouldForcePvP(player))
            return;
        // override=true → flag permanent, ne déclenche pas le timer 300s d'auto-désactivation
        player->UpdatePvP(true, true);
        // Pour fiabilité, on positionne aussi directement le byte flag (au cas où le timer
        // resterait en mémoire et réinitialise via UpdatePvPFlag).
        player->SetPvP(true);
        if (log)
            LOG_INFO("conquest", "ConquestForcePvP: PvP enforced on '{}' (map={})",
                     player->GetName(), player->GetMapId());
    }

    // Re-apply périodique throttled à 1 Hz pour éviter la désactivation du flag
    // par les timers internes / changements de zone PvE / etc.
    static std::unordered_map<uint64, uint32> s_lastReapply;
}

class ConquestForcePvP : public PlayerScript
{
public:
    ConquestForcePvP() : PlayerScript("ConquestForcePvP") { }

    void OnPlayerLogin(Player* player) override
    {
        if (!sConfigMgr->GetOption<bool>("ConquestFrontline.ForcePvP", true))
            return;
        ApplyPvPFlag(player);
    }

    void OnPlayerMapChanged(Player* player) override
    {
        if (!sConfigMgr->GetOption<bool>("ConquestFrontline.ForcePvP", true))
            return;
        ApplyPvPFlag(player);
    }

    void OnPlayerUpdateZone(Player* player, uint32 /*newZone*/, uint32 /*newArea*/) override
    {
        if (!sConfigMgr->GetOption<bool>("ConquestFrontline.ForcePvP", true))
            return;
        ApplyPvPFlag(player);
    }

    // Re-apply périodique : check chaque seconde si IsPvP() est toujours true,
    // sinon ré-appliquer. Évite que le timer interne AC décrémente / qu'une
    // sous-zone PvE override le flag.
    void OnPlayerBeforeUpdate(Player* player, uint32 /*p_time*/) override
    {
        if (!sConfigMgr->GetOption<bool>("ConquestFrontline.ForcePvP", true))
            return;
        if (!player || !player->IsInWorld())
            return;
        if (!ShouldForcePvP(player))
            return;
        if (player->IsPvP())
            return; // déjà bon
        uint32 nowMs = GameTime::GetGameTimeMS().count();
        uint64 guid = player->GetGUID().GetRawValue();
        auto it = s_lastReapply.find(guid);
        if (it != s_lastReapply.end() && nowMs - it->second < 1000)
            return;
        s_lastReapply[guid] = nowMs;
        ApplyPvPFlag(player, false); // silent (sinon spam logs)
    }
};

void AddSC_ConquestForcePvP()
{
    new ConquestForcePvP();
}
