/*
 * Conquest Frontline — Player + GM commands
 *
 * .conquest points              → affiche tes points (PB + PC + killstreak)
 * .conquest waypoints reload    → rescanne le graphe de waypoints sans restart (GM)
 * .conquest waypoints count     → count des waypoints + edges chargés (GM)
 */

#include "ConquestKillStreak.h"
#include "ConquestPoints.h"
#include "ConquestWaypointMgr.h"
#include "OutdoorPvPConquest.h"
#include "Chat.h"
#include "Player.h"
#include "ScriptMgr.h"

using namespace Acore::ChatCommands;

class conquest_commandscript : public CommandScript
{
public:
    conquest_commandscript() : CommandScript("conquest_commandscript") {}

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable waypointsTable =
        {
            { "reload", HandleWaypointsReload, SEC_GAMEMASTER, Console::Yes },
            { "count",  HandleWaypointsCount,  SEC_GAMEMASTER, Console::Yes },
        };
        static ChatCommandTable bannersTable =
        {
            { "rescan", HandleBannersRescan, SEC_GAMEMASTER, Console::Yes },
        };
        static ChatCommandTable conquestTable =
        {
            { "points",    HandleConquestPoints, SEC_PLAYER, Console::No },
            { "waypoints", waypointsTable },
            { "banners",   bannersTable },
        };
        static ChatCommandTable cmdTable =
        {
            { "conquest", conquestTable },
        };
        return cmdTable;
    }

    static bool HandleConquestPoints(ChatHandler* handler)
    {
        Player* player = handler->GetPlayer();
        if (!player)
            return false;
        uint32 guid = player->GetGUID().GetCounter();
        uint32 pb = ConquestPoints::GetBattlePoints(guid);
        uint32 pc = ConquestPoints::GetConquestPoints(guid);
        uint32 cur = ConquestKillStreak::GetCurrentStreak(guid);
        uint32 max = ConquestKillStreak::GetMaxStreak(guid);
        handler->PSendSysMessage("|cffffae42== Conqu\xC3\xAAte ==|r");
        handler->PSendSysMessage("|cffffae42Points de Bataille  : {}|r", pb);
        handler->PSendSysMessage("|cff00ff00Points de Conqu\xC3\xAAte  : {}|r", pc);
        handler->PSendSysMessage("|cffff8800Kill streak actuel  : {} (record : {})|r", cur, max);
        return true;
    }

    static bool HandleWaypointsReload(ChatHandler* handler)
    {
        sConquestWaypointMgr->Load();
        // Invalide les routes mises en cache par bot pour qu'elles soient
        // re-planifiées avec le nouveau graphe
        extern void ConquestClearAllRoutes();
        ConquestClearAllRoutes();
        handler->PSendSysMessage("Conquest waypoints reloaded : {} WPs / {} edges (bot routes cleared)",
                                 sConquestWaypointMgr->GetWaypointCount(),
                                 sConquestWaypointMgr->GetEdgeCount());
        return true;
    }

    static bool HandleWaypointsCount(ChatHandler* handler)
    {
        handler->PSendSysMessage("Conquest waypoints : {} WPs / {} edges",
                                 sConquestWaypointMgr->GetWaypointCount(),
                                 sConquestWaypointMgr->GetEdgeCount());
        return true;
    }

    static bool HandleBannersRescan(ChatHandler* handler)
    {
        if (!g_conquestInstance)
        {
            handler->PSendSysMessage("OutdoorPvPConquest not initialized.");
            return false;
        }
        uint32 found = g_conquestInstance->RescanAllBanners(true);
        handler->PSendSysMessage("Conquest banners rescan : {} live GOs trouves et re-enregistres", found);
        return true;
    }
};

void AddSC_ConquestCommands()
{
    new conquest_commandscript();
}
