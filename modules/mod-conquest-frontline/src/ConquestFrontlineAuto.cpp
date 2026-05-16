/*
 * Conquest Frontline — Auto-détection des bannières de capture
 *
 * Hook AllGameObjectScript::OnGameObjectAddWorld qui détecte les GO d'entry 400010
 * spawnés dans le monde (via .gob add 400010 ou via SQL) et les enregistre
 * automatiquement comme capture points dans OutdoorPvPConquest.
 */

#include "OutdoorPvPConquest.h"
#include "ConquestWaypointMgr.h"
#include "AllGameObjectScript.h"
#include "GameObject.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "Config.h"
#include "Log.h"

class ConquestFrontlineAutoDetect : public AllGameObjectScript
{
public:
    ConquestFrontlineAutoDetect() : AllGameObjectScript("ConquestFrontlineAutoDetect") {}

    void OnGameObjectAddWorld(GameObject* go) override
    {
        if (!sConfigMgr->GetOption<bool>("ConquestFrontline.Enable", true))
            return;

        if (!go)
            return;

        // Banners de capture : auto-register dans OutdoorPvPConquest
        uint32 expectedEntry = sConfigMgr->GetOption<uint32>("ConquestFrontline.BannerEntry", CONQUEST_BANNER_ENTRY);
        if (go->GetEntry() == expectedEntry)
        {
            if (!g_conquestInstance)
            {
                LOG_WARN("conquest",
                    "OnGameObjectAddWorld: bannière 400010 spawn mais OutdoorPvPConquest pas encore initialisé. "
                    "Verifie que SetupOutdoorPvP() a tourné au démarrage du serveur.");
                return;
            }
            g_conquestInstance->RegisterCapturePoint(go);
            return;
        }

        // Waypoints (GO 400100) : force phaseMask=2 pour les rendre invisibles aux
        // joueurs (phase 1). Les GMs auront phaseMask=3 (phase 1+2) via le
        // PlayerScript ci-dessous → ils les voient.
        if (go->GetEntry() == CONQUEST_WAYPOINT_ENTRY)
        {
            if (go->GetPhaseMask() != 2)
                go->SetPhaseMask(2, true);
        }
    }
};

// PlayerScript : passe les GM en phaseMask=3 (phase 1 + 2) à la connexion pour
// qu'ils voient les waypoints (phase 2) ET le monde normal (phase 1). Reset à
// 1 quand ils désactivent .gm off.
class ConquestGMPhase : public PlayerScript
{
public:
    ConquestGMPhase() : PlayerScript("ConquestGMPhase") {}

    void OnPlayerLogin(Player* player) override
    {
        if (!player)
            return;
        if (player->IsGameMaster() && player->GetPhaseMask() != 3)
            player->SetPhaseMask(3, true);
    }
};

void AddSC_ConquestFrontlineAuto()
{
    new ConquestFrontlineAutoDetect();
    new ConquestGMPhase();
}
