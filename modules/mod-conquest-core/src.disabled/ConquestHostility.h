/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

#pragma once

#include "ScriptMgr.h"
#include "UnitScript.h"
#include "Guild.h"

class Conquest_PvP_Override : public PlayerScript
{
public:
    Conquest_PvP_Override();

    void OnPlayerPVPKill(Player* killer, Player* killed) override;
    void OnPlayerIsFFAPvP(Player* player, bool& result) override;
    void OnPlayerUpdateArea(Player* player, uint32 /*oldArea*/, uint32 /*newArea*/) override;
    void OnPlayerFfaPvpStateUpdate(Player* player, bool result) override;
};

class Conquest_Hostility_Override : public UnitScript
{
public:
    Conquest_Hostility_Override();

    bool IfNormalReaction(Unit const* unit, Unit const* target, ReputationRank& repRank) override;
};

class Conquest_Guild_Update : public GuildScript
{
public:
    Conquest_Guild_Update();

    void OnAddMember(Guild* /*guild*/, Player* player, uint8& /*plRank*/) override;
    void OnRemoveMember(Guild* /*guild*/, Player* player, bool /*isDisbanding*/, bool /*isKicked*/) override;
};
