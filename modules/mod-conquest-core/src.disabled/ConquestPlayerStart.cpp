/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

#include "Config.h"
#include "DBCStores.h"
#include "Log.h"
#include "MapMgr.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "SpellMgr.h"
#include "World.h"

#include <algorithm>
#include <cmath>

namespace
{
constexpr uint32 DEFAULT_START_LEVEL = 80;
constexpr uint32 START_MAP_ID = 1;
constexpr float START_X = 16201.588f;
constexpr float START_Y = 16211.277f;
constexpr float START_Z = 1.1370115f;
constexpr float START_O = 1.1048489f;
constexpr uint32 START_AREA_ID = 816;

uint32 GetConfiguredStartLevel()
{
    uint32 requestedLevel = sConfigMgr->GetOption<uint32>("ConquestCore.PlayerStart.Level", DEFAULT_START_LEVEL);
    uint32 maxServerLevel = sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL);
    requestedLevel = std::clamp(requestedLevel, 1u, maxServerLevel);
    return requestedLevel;
}

void LearnAllClassSpells(Player* player)
{
    ChrClassesEntry const* classEntry = sChrClassesStore.LookupEntry(player->getClass());
    if (!classEntry)
        return;

    uint32 family = classEntry->spellfamily;

    for (uint32 i = 0; i < sSkillLineAbilityStore.GetNumRows(); ++i)
    {
        SkillLineAbilityEntry const* ability = sSkillLineAbilityStore.LookupEntry(i);
        if (!ability)
            continue;

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(ability->Spell);
        if (!spellInfo)
            continue;

        if (spellInfo->SpellLevel == 0)
            continue;

        if (!player->IsSpellFitByClassAndRace(spellInfo->Id))
            continue;

        if (spellInfo->SpellFamilyName != family)
            continue;

        uint32 firstRank = sSpellMgr->GetFirstSpellInChain(spellInfo->Id);
        if (GetTalentSpellCost(firstRank) > 0)
            continue;

        // Skip mount spells (paladin/warlock class mounts or similar)
        if (spellInfo->HasAura(SPELL_AURA_MOUNTED))
            continue;

        if (!SpellMgr::IsSpellValid(spellInfo))
            continue;

        player->learnSpell(spellInfo->Id, false, true);
    }
}

void RestoreResources(Player* player)
{
    player->SetFullHealth();

    for (uint8 powerIndex = 0; powerIndex < MAX_POWERS; ++powerIndex)
    {
        Powers powerType = static_cast<Powers>(powerIndex);
        uint32 maxPower = player->GetMaxPower(powerType);
        if (maxPower == 0 && player->GetPower(powerType) == 0)
            continue;

        player->SetPower(powerType, maxPower);
    }
}

void PreparePlayer(Player* player)
{
    if (!player)
        return;

    uint32 desiredLevel = GetConfiguredStartLevel();

    if (player->GetLevel() != desiredLevel)
        player->GiveLevel(static_cast<uint8>(desiredLevel));

    player->SetUInt32Value(PLAYER_XP, 0);

    player->InitStatsForLevel(true);
    player->InitTalentForLevel();
    player->LearnDefaultSkills();
    player->LearnCustomSpells();
    player->learnQuestRewardedSpells();
    LearnAllClassSpells(player);
    player->UpdateSkillsToMaxSkillsForLevel();
    player->UpdateAllStats();
    RestoreResources(player);

    LOG_INFO("module", "ConquestPlayerStart: Initialized player {} at level {} with full spell/skill book",
        player->GetName(), desiredLevel);
}

WorldLocation GetStartLocation()
{
    return WorldLocation(START_MAP_ID, START_X, START_Y, START_Z, START_O);
}

void InitializeStartLocation(Player* player)
{
    if (!player)
        return;

    WorldLocation startLoc = GetStartLocation();

    if (player->GetMapId() != START_MAP_ID)
    {
        if (Map* newMap = sMapMgr->CreateMap(START_MAP_ID, player))
        {
            if (player->GetMap())
                player->ResetMap();

            player->SetMap(newMap);
        }
    }

    player->Relocate(startLoc);
    player->SetHomebind(startLoc, START_AREA_ID);
    player->SaveRecallPosition();
}

void TeleportPlayerToStart(Player* player)
{
    if (!player)
        return;

    WorldLocation startLoc = GetStartLocation();

    // Teleport only if the player is not already near the target spot
    if (player->GetMapId() != START_MAP_ID ||
        player->GetExactDist2d(START_X, START_Y) > 1.0f ||
        fabs(player->GetPositionZ() - START_Z) > 1.0f)
    {
        player->TeleportTo(START_MAP_ID, START_X, START_Y, START_Z, START_O);
    }

    player->SaveRecallPosition();
    player->SetHomebind(startLoc, START_AREA_ID);
    player->SavePositionInDB(START_MAP_ID, START_X, START_Y, START_Z, START_O, START_AREA_ID, player->GetGUID());
}

} // namespace

class ConquestPlayerStart : public PlayerScript
{
public:
    ConquestPlayerStart() : PlayerScript("ConquestPlayerStart") { }

    void OnPlayerCreate(Player* player) override
    {
        if (!IsEnabled())
            return;

        PreparePlayer(player);
        InitializeStartLocation(player);
    }

    void OnPlayerFirstLogin(Player* player) override
    {
        if (!IsEnabled())
            return;

        PreparePlayer(player);
        LOG_INFO("module", "ConquestPlayerStart: Teleporting player {} to custom start location on first login", player->GetName());
        TeleportPlayerToStart(player);
    }

private:
    static bool IsEnabled()
    {
        return sConfigMgr->GetOption<bool>("ConquestCore.PlayerStart.Enable", true);
    }
};

void AddConquestPlayerStartScripts()
{
    new ConquestPlayerStart();
}

