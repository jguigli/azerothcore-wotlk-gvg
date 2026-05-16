/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "TravelAction.h"

#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Playerbots.h"

bool TravelAction::Execute(Event /*event*/)
{
    TravelTarget* target = AI_VALUE(TravelTarget*, "travel target");

    if (bot->isMoving())
        return false;

    target->setStatus(TRAVEL_STATUS_WORK);

    // CONQUEST PATCH — priorise HOSTILE en premier pass. Pas de rpg target sur
    // friendlies (sinon MoveRandomAction::isUseful retourne false et le bot
    // reste immobile au lieu de patrouiller). En 2e pass on cherche un friendly
    // uniquement si aucun hostile trouvé.
    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, sPlayerbotAIConfig.sightDistance * 2);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitObjects(bot, searcher, sPlayerbotAIConfig.sightDistance);

    auto isValid = [&](Unit* u) {
        return u && u->IsInWorld() && !u->IsDuringRemoveFromWorld() &&
               u->GetMapId() == bot->GetMapId() && u->IsAlive() &&
               u->GetEntry() != target->getDestination()->getEntry() &&
               !u->IsInCombat();
    };

    // Pass 1 : hostile → pull target (combat engage)
    for (Unit* unit : targets)
    {
        if (!isValid(unit) || !bot->IsHostileTo(unit)) continue;
        SET_AI_VALUE(ObjectGuid, "pull target", unit->GetGUID());
        return true;
    }
    // Pass 2 : aucun hostile, pas d'rpg target non plus pour laisser MoveRandom fire

    return true;
}

bool TravelAction::isUseful()
{
    // CONQUEST PATCH — retrait du "false &&" qui désactivait silencieusement le travel
    return AI_VALUE(TravelTarget*, "travel target")->isActive() &&
           (!AI_VALUE(GuidPosition, "rpg target") || !AI_VALUE(ObjectGuid, "pull target"));
}

bool MoveToDarkPortalAction::Execute(Event /*event*/)
{
    if (bot->GetGroup())
    {
        if (bot->GetGroup()->GetLeaderGUID() != bot->GetGUID() &&
            !GET_PLAYERBOT_AI(GET_PLAYERBOT_AI(bot)->GetGroupLeader()))
            return false;
    }

    if (bot->GetLevel() > 57)
    {
        if ((bot->GetTeamId() == TEAM_ALLIANCE && bot->GetQuestStatus(10119) == QUEST_STATUS_NONE) ||
            (bot->GetTeamId() == TEAM_HORDE && bot->GetQuestStatus(9407) == QUEST_STATUS_NONE))
        {
            if (!bot->IsInCombat())
            {
                if (bot->GetTeamId() == TEAM_ALLIANCE)
                {
                    Quest const* quest = sObjectMgr->GetQuestTemplate(10119);
                    CreatureData const* creatureData = sRandomPlayerbotMgr.GetCreatureDataByEntry(16841);
                    if (quest && creatureData)
                    {
                        auto creatureBounds =
                            bot->GetMap()->GetCreatureBySpawnIdStore().equal_range(creatureData->spawnId);
                        if (creatureBounds.first != creatureBounds.second)
                            bot->AddQuest(quest, creatureBounds.first->second);
                    }
                }
                else
                {
                    Quest const* quest = sObjectMgr->GetQuestTemplate(9407);
                    CreatureData const* creatureData = sRandomPlayerbotMgr.GetCreatureDataByEntry(19254);
                    if (quest && creatureData)
                    {
                        auto creatureBounds =
                            bot->GetMap()->GetCreatureBySpawnIdStore().equal_range(creatureData->spawnId);
                        if (creatureBounds.first != creatureBounds.second)
                            bot->AddQuest(quest, creatureBounds.first->second);
                    }
                }
            }

            return MoveTo(0, -11815.1f, -3190.39f, -30.7338f, false, true);
        }

        return MoveTo(0, -11906.9f, -3208.53f, -14.8616f, false, true);
    }

    return false;
}

bool MoveToDarkPortalAction::isUseful() { return bot->GetLevel() > 54; }

bool DarkPortalAzerothAction::Execute(Event /*event*/)
{
    if (bot->GetLevel() > 57)
    {
        WorldPacket packet(CMSG_AREATRIGGER);
        packet << 4354;

        return GET_PLAYERBOT_AI(bot)->DoSpecificAction("reach area trigger", Event("travel action", packet));
    }

    return false;
}

bool DarkPortalAzerothAction::isUseful() { return bot->GetLevel() > 57; }

bool MoveFromDarkPortalAction::Execute(Event /*event*/)
{
    RESET_AI_VALUE(GuidPosition, "rpg target");

    if (bot->GetTeamId() == TEAM_ALLIANCE)
        return MoveTo(530, -319.261f, 1027.213, 54.172638f, false, true);

    return MoveTo(530, -180.444f, 1027.947, 54.181538f, false, true);
}
