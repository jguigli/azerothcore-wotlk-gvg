/*
 * Conquest Frontline — PlayerScript pour 2 mécaniques :
 *   1. Defense redispatch 30s post-capture (ConquestScheduleDefenseRedispatch)
 *   2. Drift detection : bot avec ConquestBannerDestination en WORK et qui s'est
 *      éloigné >50y du banner → on remet status TRAVEL → action re-fire → revient
 *      vers la zone de capture. Throttled à 1 check/sec par bot.
 */

#include "GameTime.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "RandomPlayerbotMgr.h"
#include "ScriptMgr.h"
#include "TravelMgr.h"

#include <unordered_map>
#include <cmath>

// Déclaré dans OutdoorPvPConquest.cpp
bool ConquestPopRedispatchIfReady(uint64 botGuid, uint32 nowMs);
void ConquestScheduleDefenseRedispatch(uint64 botGuid, uint32 atMs);

// Déclaré dans RandomPlayerbotMgr.cpp (wrappers externes vers le registre de
// groupes). 0 = bot pas en groupe.
uint64 ConquestGetGroupLeader(uint64 botGuid);
void   ConquestSignalGroupRedispatch(uint64 leaderGuid);

// Boat trip system : check si le bot a un trip stashed et a atteint le dock
// source. Si oui, TP au dock de destination + rearme la TravelTarget finale.
bool   ConquestCheckBoatArrival(Player* bot);

namespace
{
    // Throttle drift check : last check timestamp per bot
    static std::unordered_map<uint64, uint32> s_lastDriftCheck;
}

class conquest_defense_redispatch : public PlayerScript
{
public:
    conquest_defense_redispatch() : PlayerScript("conquest_defense_redispatch") {}

    // Hook resurrection : bot mort -> respawn en capitale -> on schedule un
    // redispatch en 1s pour le repousser vers une banner immediatement.
    // Sans ca, le bot attend son prochain RandomTeleportForLevel periodique
    // (60-300s) ce qui donne l'illusion qu'il est stuck en capitale.
    void OnPlayerResurrect(Player* player, float /*restorePercent*/, bool& /*applySickness*/) override
    {
        if (!player || !player->IsInWorld())
            return;
        PlayerbotAI* ai = GET_PLAYERBOT_AI(player);
        if (!ai)
            return; // joueur reel, ne pas toucher
        uint32 nowMs = GameTime::GetGameTimeMS().count();
        ConquestScheduleDefenseRedispatch(player->GetGUID().GetRawValue(), nowMs + 1000);
    }

    void OnPlayerBeforeUpdate(Player* player, uint32 /*p_time*/) override
    {
        if (!player || !player->IsInWorld())
            return;
        PlayerbotAI* ai = GET_PLAYERBOT_AI(player);
        if (!ai)
            return;

        uint32 nowMs = GameTime::GetGameTimeMS().count();
        uint64 guid = player->GetGUID().GetRawValue();

        // 1. Defense redispatch après 30s
        if (ConquestPopRedispatchIfReady(guid, nowMs))
        {
            uint64 leader = ConquestGetGroupLeader(guid);
            if (leader)
            {
                // Toujours signaler le flag du leader (idempotent), puis
                // reveiller le leader. Couvre le cas ou le leader lui-meme
                // n'a pas son trigger (parce qu'il est hors du rayon 200y
                // de capture). Sans ca, si seul un non-leader est dans le
                // rayon, le squad reste stuck a vie.
                ConquestSignalGroupRedispatch(leader);
                if (Player* leaderPlayer = ObjectAccessor::FindPlayer(ObjectGuid(leader)))
                    sRandomPlayerbotMgr.RandomTeleportForLevel(leaderPlayer);
                // Non-leader : on s'arrete la (le leader dispatche le squad).
                if (guid != leader)
                    return;
                // Leader : on est deja revenu de notre propre RTfL via la
                // ligne ci-dessus (leaderPlayer == player), on a deja
                // dispatche, donc rien de plus a faire.
                return;
            }
            // Pas de groupe : dispatch solo (legacy path).
            sRandomPlayerbotMgr.RandomTeleportForLevel(player);
            return;
        }

        // 2. Boat arrival : si le bot a un trip stashed et touche le dock,
        // on TP cross-map et on re-arme sa TravelTarget vers la bannière.
        // Throttle commun avec drift detection (1 Hz suffit).
        auto it = s_lastDriftCheck.find(guid);
        if (it != s_lastDriftCheck.end() && nowMs - it->second < 1000)
            return;
        s_lastDriftCheck[guid] = nowMs;

        if (ConquestCheckBoatArrival(player))
            return;

        // 3. Drift detection

        TravelTarget* tt = ai->GetAiObjectContext()->GetValue<TravelTarget*>("travel target")->Get();
        if (!tt || !tt->getDestination()) return;
        if (tt->getDestination()->getName() != "ConquestBannerDestination") return;
        if (tt->getStatus() != TRAVEL_STATUS_WORK) return;

        WorldLocation const* pos = tt->getPosition();
        if (!pos || pos->GetMapId() != player->GetMapId()) return;

        float dx = player->GetPositionX() - pos->GetPositionX();
        float dy = player->GetPositionY() - pos->GetPositionY();
        if (std::sqrt(dx * dx + dy * dy) > 50.0f)
        {
            tt->setStatus(TRAVEL_STATUS_TRAVEL); // déclenche MoveToTravelTargetAction → walk back
        }
    }
};

void AddSC_ConquestDefenseRedispatch()
{
    new conquest_defense_redispatch();
}
