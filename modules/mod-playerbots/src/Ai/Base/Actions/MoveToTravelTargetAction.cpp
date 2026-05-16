/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "MoveToTravelTargetAction.h"

#include "ChooseRpgTargetAction.h"
#include "ConquestWaypointMgr.h"
#include "GameTime.h"
#include "LootObjectStack.h"
#include "MotionMaster.h"
#include "OutdoorPvPConquest.h"
#include "PathGenerator.h"
#include "Playerbots.h"
#include "TravelNode.h"

#include <unordered_map>

namespace
{
    // CONQUEST PATCH — route hybride :
    // - >300y de la cible : suivre les waypoints du ConquestWaypointMgr
    //   (graphe de GOs invisibles 400100 + Dijkstra). Chemins curés à la main,
    //   jamais "vol dans le vide" car les WPs sont placés par GM sur du sol valide.
    // - <300y : navmesh direct (PathGenerator) pour l'approche fine.
    // Cache par bot pour éviter de recalculer le full path à chaque tick.
    struct ConquestRoute
    {
        uint32 destSignature = 0;       // empreinte de la destination finale
        std::vector<WorldPosition> waypoints;
        size_t currentIdx = 0;
        float lastX = 0.0f;
        float lastY = 0.0f;
        uint32 lastProgressMs = 0;      // dernier moment où le bot a réellement bougé
    };

    static std::unordered_map<uint64, ConquestRoute> s_routes;

    uint32 ComputeDestSignature(uint32 mapId, float x, float y)
    {
        // hash très simple : on tolère ~5y de dérive sur la destination
        int32 ix = static_cast<int32>(x / 5.0f);
        int32 iy = static_cast<int32>(y / 5.0f);
        return (mapId * 2654435761u) ^ static_cast<uint32>(ix * 73856093) ^
               static_cast<uint32>(iy * 19349663);
    }
}

// Exporté pour invalidation depuis ConquestCommands::HandleWaypointsReload
void ConquestClearAllRoutes()
{
    s_routes.clear();
}

bool MoveToTravelTargetAction::Execute(Event /*event*/)
{
    TravelTarget* target = AI_VALUE(TravelTarget*, "travel target");

    WorldPosition botLocation(bot);
    WorldLocation location = *target->getPosition();

    LOG_INFO("playerbots", "MoveToTravel: bot='{}' map={} pos=({:.0f},{:.0f}) -> targetMap={} dist={:.0f} status={} motionType={} isMoving={}",
             bot->GetName(), bot->GetMapId(), bot->GetPositionX(), bot->GetPositionY(),
             location.GetMapId(),
             botLocation.distance(WorldPosition(location.GetMapId(), location.GetPositionX(), location.GetPositionY(), location.GetPositionZ(), 0.0f)),
             (uint32)target->getStatus(),
             (uint32)bot->GetMotionMaster()->GetCurrentMovementGeneratorType(),
             bot->isMoving());

    Group* group = bot->GetGroup();
    if (group && !urand(0, 1) && bot == botAI->GetGroupLeader() && !bot->IsInCombat())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member == bot)
                continue;

            if (!member->IsAlive())
                continue;

            if (!member->isMoving())
                continue;

            PlayerbotAI* memberBotAI = GET_PLAYERBOT_AI(member);
            if (memberBotAI && !memberBotAI->HasStrategy("follow", BOT_STATE_NON_COMBAT))
                continue;

            WorldPosition memberPos(member);
            WorldPosition targetPos = *target->getPosition();

            float memberDistance = botLocation.distance(memberPos);

            if (memberDistance < 50.0f)
                continue;
            if (memberDistance > sPlayerbotAIConfig.reactDistance * 20)
                continue;

            // float memberAngle = botLocation.getAngleBetween(targetPos, memberPos);

            // if (botLocation.getMapId() == targetPos.getMapId() && botLocation.getMapId() == memberPos.getMapId() &&
            // memberAngle < static_cast<float>(M_PI) / 2) //We are heading that direction anyway.
            //     continue;

            if (!urand(0, 5))
            {
                std::ostringstream out;
                if (botAI->GetMaster() && !bot->GetGroup()->IsMember(botAI->GetMaster()->GetGUID()))
                    out << "Waiting a bit for ";
                else
                    out << "Please hurry up ";

                out << member->GetName();

                botAI->TellMasterNoFacing(out);
            }

            target->setExpireIn(target->getTimeLeft() + sPlayerbotAIConfig.maxWaitForMove);

            botAI->SetNextCheckDelay(sPlayerbotAIConfig.maxWaitForMove);

            return true;
        }
    }

    float maxDistance = target->getDestination()->getRadiusMin();

    // Spread bots around the target but keep the offset stable per
    // (bot, destination) pair. Previously the angle and radius were
    // re-rolled every time the action re-entered (i.e. every tick the
    // bot wasn't already moving), which made bots oscillate between
    // two random points around the same quest POI instead of
    // committing to one approach.
    uint32 botLow = bot->GetGUID().GetCounter();
    int32 destSeed = static_cast<int32>(location.GetPositionX()) * 73856093 ^
                     static_cast<int32>(location.GetPositionY()) * 19349663;
    uint32 seed = botLow ^ static_cast<uint32>(destSeed);
    float angle = 2.0f * static_cast<float>(M_PI) * static_cast<float>(seed % 1000) / 1000.0f;
    float mod = 0.5f + static_cast<float>((seed / 1000) % 1000) / 2000.0f;  // [0.5, 1.0]

    if (target->getMaxTravelTime() > target->getTimeLeft())  // The bot is late. Speed it up.
    {
        // distance = sPlayerbotAIConfig.fleeDistance;
        // angle = bot->GetAngle(location.GetPositionX(), location.GetPositionY());
        // location = botLocation.getLocation();
    }

    float x = location.GetPositionX();
    float y = location.GetPositionY();
    float z = location.GetPositionZ();
    float mapId = location.GetMapId();

    // CONQUEST PATCH — détour opportuniste : si le bot passe à <250y d'une
    // bannière qui n'est PAS encore fully-locked pour sa faction, on override
    // la cible vers cette bannière (le jitter sera appliqué juste après).
    if (g_conquestInstance && bot->GetMapId() == (uint32)mapId)
    {
        auto opp = g_conquestInstance->FindOpportunisticBanner(
            bot->GetMapId(), bot->GetPositionX(), bot->GetPositionY(),
            bot->GetTeamId(), 250.0f);
        if (opp.found)
        {
            x = opp.x;
            y = opp.y;
            z = opp.z;
            mapId = static_cast<float>(opp.mapId);
        }
    }

    // Place le bot sur un ANNEAU 30y autour de la bannière (direction angulaire
    // déterministe par (bot, bannière) — déjà calculée dans angle). Évite
    // l'empilement sur le GPS exact du GO. Combiné avec advance threshold 20y,
    // le bot finit à 10-50y du centre, dans la zone de capture (45y).
    (void)mod;
    constexpr float RING_DIST = 30.0f;
    x += cos(angle) * RING_DIST;
    y += sin(angle) * RING_DIST;

    // CONQUEST PATCH — Routing hybride basé sur ConquestWaypointMgr :
    //
    // 1. >300y de la cible finale : on suit le path retourné par Dijkstra sur
    //    le graphe de waypoints GO 400100 placés manuellement (chemins curés,
    //    respectent toujours la topologie). Si aucun WP placé sur cette map ou
    //    pas de route → on tombe sur le sub-step 100y direct (cas dégradé).
    // 2. <300y : navmesh direct via PathGenerator (système fiable pour les
    //    courtes distances).
    // Stuck detection : si pas bougé >5y en 15s on saute le waypoint courant.
    bool sameMap = (bot->GetMapId() == (uint32)mapId);
    bool canMove = false;

    if (sameMap)
    {
        uint64 guid = bot->GetGUID().GetRawValue();
        uint32 destSig = ComputeDestSignature((uint32)mapId, x, y);
        ConquestRoute& route = s_routes[guid];

        // (Re)plan si destination a changé ou route vide
        if (route.destSignature != destSig || route.waypoints.empty())
        {
            route.destSignature = destSig;
            route.waypoints.clear();
            route.currentIdx = 0;
            route.lastX = bot->GetPositionX();
            route.lastY = bot->GetPositionY();
            route.lastProgressMs = GameTime::GetGameTimeMS().count();

            WorldPosition endPos(mapId, x, y, z, 0.0f);
            float finalDist = botLocation.distance(endPos);

            // Si on est déjà à <300y, pas besoin de waypoints — phase navmesh directe
            if (finalDist > 300.0f)
            {
                std::vector<Position> wpRoute = sConquestWaypointMgr->GetRoute(
                    (uint32)mapId,
                    bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),
                    x, y, z);
                for (Position const& p : wpRoute)
                {
                    WorldPosition wp((uint32)mapId, p.GetPositionX(), p.GetPositionY(),
                                     p.GetPositionZ(), 0.0f);
                    if (botLocation.distance(wp) < 10.0f) continue; // skip points trop proches
                    route.waypoints.push_back(wp);
                }
            }
            // Toujours ajouter la destination finale en dernier (phase approche navmesh)
            route.waypoints.push_back(endPos);

            LOG_INFO("playerbots", "ConquestRoute: '{}' planned {} waypoints to ({:.0f},{:.0f}) [WPs in graph: {}]",
                     bot->GetName(), route.waypoints.size(), x, y,
                     sConquestWaypointMgr->GetWaypointCount());
        }

        // Avance dans la séquence si on est proche du waypoint courant
        while (route.currentIdx < route.waypoints.size())
        {
            WorldPosition const& wp = route.waypoints[route.currentIdx];
            if (botLocation.distance(wp) < 20.0f)
                route.currentIdx++;
            else
                break;
        }

        // Stuck detection : si pas de progression depuis 15s :
        //   - sur un WP non-final → skip vers le WP suivant
        //   - sur le WP final (la bannière) → teleport direct dans le rayon
        //     pour débloquer (sinon obstacle navmesh = camping infini).
        uint32 nowMs = GameTime::GetGameTimeMS().count();
        float moved = sqrt(pow(bot->GetPositionX() - route.lastX, 2) +
                           pow(bot->GetPositionY() - route.lastY, 2));
        if (moved > 5.0f)
        {
            route.lastX = bot->GetPositionX();
            route.lastY = bot->GetPositionY();
            route.lastProgressMs = nowMs;
        }
        else if (nowMs - route.lastProgressMs > 15000)
        {
            if (route.currentIdx + 1 < route.waypoints.size())
            {
                LOG_INFO("playerbots", "ConquestRoute: '{}' stuck on wp[{}], skipping",
                         bot->GetName(), route.currentIdx);
                route.currentIdx++;
                route.lastProgressMs = nowMs;
            }
            else if (route.currentIdx < route.waypoints.size())
            {
                // Stuck sur le DERNIER WP (la bannière) : teleport direct dans
                // un rayon de 35y pour débloquer (obstacle navmesh ou bots agglutinés).
                WorldPosition const& final = route.waypoints[route.currentIdx];
                float ang = frand(0.0f, 2.0f * static_cast<float>(M_PI));
                float jx = final.GetPositionX() + cos(ang) * 25.0f;
                float jy = final.GetPositionY() + sin(ang) * 25.0f;
                LOG_INFO("playerbots", "ConquestRoute: '{}' stuck on FINAL wp, teleporting to banner zone",
                         bot->GetName());
                bot->TeleportTo((uint32)mapId, jx, jy, final.GetPositionZ() + 1.0f, bot->GetOrientation());
                route.lastProgressMs = nowMs;
            }
        }

        if (route.currentIdx < route.waypoints.size())
        {
            WorldPosition const& wp = route.waypoints[route.currentIdx];
            float wpx = wp.GetPositionX();
            float wpy = wp.GetPositionY();
            float wpz = wp.GetPositionZ();

            // Navmesh path bot → waypoint (sans forceDestination)
            PathGenerator pg(bot);
            bool ok = pg.CalculatePath(wpx, wpy, wpz, false);
            Movement::PointsArray const& poly = pg.GetPath();
            bool pathClean = ok && !(pg.GetPathType() & PATHFIND_NOPATH) && poly.size() > 2;

            if (pathClean)
            {
                Movement::PointsArray polyCopy = poly;
                bot->GetMotionMaster()->Clear();
                bot->GetMotionMaster()->MoveSplinePath(&polyCopy, FORCED_MOVEMENT_RUN);
                canMove = true;
            }
            else
            {
                // Sub-step 100y vers le waypoint sans forceDestination → si
                // navmesh ne trouve toujours rien, le bot s'arrête (au lieu de
                // traverser des murs). Le stuck timer relancera au tick suivant.
                float dx = wpx - bot->GetPositionX();
                float dy = wpy - bot->GetPositionY();
                float dist = sqrt(dx * dx + dy * dy);
                float subX = wpx, subY = wpy, subZ = wpz;
                if (dist > 100.0f)
                {
                    float ratio = 100.0f / dist;
                    subX = bot->GetPositionX() + dx * ratio;
                    subY = bot->GetPositionY() + dy * ratio;
                    subZ = bot->GetPositionZ() + (wpz - bot->GetPositionZ()) * ratio;
                }
                bot->GetMotionMaster()->Clear();
                bot->GetMotionMaster()->MovePoint(0, subX, subY, subZ, FORCED_MOVEMENT_RUN,
                                                  0, 0, true, false);
                canMove = true;
            }
        }
    }
    else
    {
        // cross-map → on ne peut pas pathfinder, fallback teleport via MoveTo
        canMove = MoveTo(mapId, x, y, z, false, false);
    }

    if (!canMove && !target->isForced())
    {
        target->incRetry(true);

        if (target->isMaxRetry(true))
            target->setStatus(TRAVEL_STATUS_COOLDOWN);
    }
    else
        target->setRetry(true);

    return canMove;
}

bool MoveToTravelTargetAction::isUseful()
{
    if (!botAI->AllowActivity(TRAVEL_ACTIVITY))
        return false;

    if (!context->GetValue<TravelTarget*>("travel target")->Get()->isTraveling())
        return false;

    if (bot->HasUnitState(UNIT_STATE_IN_FLIGHT))
        return false;

    if (bot->IsFlying())
        return false;

    if (bot->isMoving())
        return false;

    // CONQUEST PATCH — on ne dépend plus du "can move around" qui exige
    // "group ready" (HP > 80%, Mana > seuil). Pour les bots Conquest en route
    // vers un banner, ces seuils bloquent 95% des bots. On garde les checks
    // critiques (combat, stay strategy) directement ici.
    if (bot->IsInCombat())
        return false;

    if (botAI->HasStrategy("stay", BOT_STATE_NON_COMBAT))
        return false;

    LootObject loot = AI_VALUE(LootObject, "loot target");
    if (loot.IsLootPossible(bot))
        return false;

    if (!ChooseRpgTargetAction::isFollowValid(bot,
                                              *context->GetValue<TravelTarget*>("travel target")->Get()->getPosition()))
        return false;

    return true;
}
