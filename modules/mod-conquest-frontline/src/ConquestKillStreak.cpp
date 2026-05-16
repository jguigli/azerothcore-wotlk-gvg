/*
 * Conquest Frontline — Kill Streak tracker impl. (Phase 5)
 */

#include "ConquestKillStreak.h"
#include "CharacterDatabase.h"
#include "Chat.h"
#include "Config.h"
#include "Log.h"
#include "Player.h"
#include "SharedDefines.h"

namespace
{
    // Pennon de [Capitale] (5 kills) et Pennon de Champion de [Capitale] (10 kills).
    // IDs validés par user en jeu.
    struct PennonPair { uint32 onFireSpell; uint32 prestigeSpell; };

    PennonPair GetPennonsForRace(uint8 race)
    {
        switch (race)
        {
            case RACE_HUMAN:         return {66367, 62727}; // Stormwind
            case RACE_ORC:           return {66369, 63444}; // Orgrimmar
            case RACE_DWARF:         return {66363, 63440}; // Ironforge
            case RACE_NIGHTELF:      return {66368, 63443}; // Darnassus
            case RACE_UNDEAD_PLAYER: return {66365, 63441}; // Undercity
            case RACE_TAUREN:        return {66370, 63445}; // Thunder Bluff
            case RACE_GNOME:         return {66366, 63442}; // Gnomeregan
            case RACE_TROLL:         return {66371, 63446}; // Sen'jin
            case RACE_BLOODELF:      return {66360, 63438}; // Silvermoon
            case RACE_DRAENEI:       return {66362, 63439}; // Exodar
            default:                 return {66367, 62727}; // fallback Stormwind
        }
    }

    // Aura compagne appliquée AVEC les pennons (palier 5 et 10).
    constexpr uint32 PENNON_COMPANION_AURA = 47292;

    // Paliers supérieurs — auras non racialisées, simples.
    // À 20 kills → aura tier3, à 40 → tier4 (retire tier3), à 80 → tier5 (retire tier4).
    constexpr uint32 TIER3_AURA = 71188; // 20 kills
    constexpr uint32 TIER4_AURA = 71193; // 40 kills
    constexpr uint32 TIER5_AURA = 71195; // 80 kills

    uint32 OnFireThreshold()    { return sConfigMgr->GetOption<uint32>("ConquestFrontline.KillStreakOnFire",   5); }
    uint32 PrestigeThreshold()  { return sConfigMgr->GetOption<uint32>("ConquestFrontline.KillStreakPrestige", 10); }
    uint32 Tier3Threshold()     { return sConfigMgr->GetOption<uint32>("ConquestFrontline.KillStreakTier3",    20); }
    uint32 Tier4Threshold()     { return sConfigMgr->GetOption<uint32>("ConquestFrontline.KillStreakTier4",    40); }
    uint32 Tier5Threshold()     { return sConfigMgr->GetOption<uint32>("ConquestFrontline.KillStreakTier5",    80); }
    uint32 OnFireSpellFor(Player const* p)
    {
        if (!p) return 0;
        uint32 override_id = sConfigMgr->GetOption<uint32>("ConquestFrontline.KillStreakOnFireSpell", 0);
        if (override_id) return override_id;
        return GetPennonsForRace(p->getRace()).onFireSpell;
    }
    uint32 PrestigeSpellFor(Player const* p)
    {
        if (!p) return 0;
        uint32 override_id = sConfigMgr->GetOption<uint32>("ConquestFrontline.KillStreakPrestigeSpell", 0);
        if (override_id) return override_id;
        return GetPennonsForRace(p->getRace()).prestigeSpell;
    }

    void Persist(uint32 guid, uint32 current, uint32 maxStreak)
    {
        CharacterDatabase.Execute(
            "INSERT INTO character_conquest_killstreak (guid, current_streak, max_streak, updated_at) "
            "VALUES ({}, {}, {}, UNIX_TIMESTAMP()) "
            "ON DUPLICATE KEY UPDATE current_streak = {}, max_streak = GREATEST(max_streak, {}), updated_at = UNIX_TIMESTAMP()",
            guid, current, maxStreak, current, maxStreak);
    }
}

namespace ConquestKillStreak
{

uint32 GetCurrentStreak(uint32 guid)
{
    QueryResult r = CharacterDatabase.Query(
        "SELECT current_streak FROM character_conquest_killstreak WHERE guid = {}", guid);
    return r ? r->Fetch()[0].Get<uint32>() : 0;
}

uint32 GetMaxStreak(uint32 guid)
{
    QueryResult r = CharacterDatabase.Query(
        "SELECT max_streak FROM character_conquest_killstreak WHERE guid = {}", guid);
    return r ? r->Fetch()[0].Get<uint32>() : 0;
}

bool IsOnFire(Player const* player)
{
    return player && player->HasAura(OnFireSpellFor(player));
}

bool IsPrestige(Player const* player)
{
    return player && player->HasAura(PrestigeSpellFor(player));
}

void OnKill(Player* killer)
{
    if (!killer)
        return;

    uint32 guid = killer->GetGUID().GetCounter();
    uint32 current = GetCurrentStreak(guid) + 1;
    uint32 maxStreak = std::max(GetMaxStreak(guid), current);
    Persist(guid, current, maxStreak);

    uint32 onFire = OnFireThreshold();
    uint32 prestige = PrestigeThreshold();

    uint32 tier3 = Tier3Threshold();
    uint32 tier4 = Tier4Threshold();
    uint32 tier5 = Tier5Threshold();

    // Application aux paliers exacts (et non > pour éviter de re-cast à chaque kill au-dessus du seuil)
    // Note : on utilise AddAura (comme .aura) au lieu de CastSpell — les pennons
    // Argent Tournament sont des spells particuliers qui ne s'appliquent pas via
    // CastSpell(triggered=true), mais AddAura les pose direct.
    if (current == onFire)
    {
        if (uint32 spell = OnFireSpellFor(killer))
            killer->AddAura(spell, killer);
        killer->CastSpell(killer, PENNON_COMPANION_AURA, true); // aura compagne via CastSpell
        ChatHandler(killer->GetSession()).PSendSysMessage(
            "|cffff8800Tu portes le Pennon de ta capitale ! ({} kills consécutifs) — "
            "t'abattre rapporte 1 Marque de Champion.|r", current);
        LOG_INFO("conquest", "KillStreak: '{}' atteint OnFire/Pennon ({} kills, race {})",
                 killer->GetName(), current, killer->getRace());
    }
    else if (current == prestige)
    {
        // Retire le palier précédent (pennon basique) avant d'appliquer celui de prestige
        if (uint32 spell = OnFireSpellFor(killer))
            killer->RemoveAura(spell);
        if (uint32 spell = PrestigeSpellFor(killer))
            killer->AddAura(spell, killer);
        killer->CastSpell(killer, PENNON_COMPANION_AURA, true); // aura compagne via CastSpell (refresh)
        ChatHandler(killer->GetSession()).PSendSysMessage(
            "|cffff0000CHAMPION ! Pennon de Champion de ta capitale ({} kills consécutifs) — "
            "t'abattre rapporte 3 Marques de Champion.|r", current);
        LOG_INFO("conquest", "KillStreak: '{}' atteint Prestige/Champion ({} kills, race {})",
                 killer->GetName(), current, killer->getRace());
    }
    else if (current == tier3)
    {
        killer->AddAura(TIER3_AURA, killer);
        ChatHandler(killer->GetSession()).PSendSysMessage(
            "|cffff0044TUEUR D'\xC3\x89LITE ! ({} kills cons\xC3\xA9""cutifs)|r", current);
        LOG_INFO("conquest", "KillStreak: '{}' atteint Tier3 ({} kills)", killer->GetName(), current);
    }
    else if (current == tier4)
    {
        killer->RemoveAura(TIER3_AURA);
        killer->AddAura(TIER4_AURA, killer);
        ChatHandler(killer->GetSession()).PSendSysMessage(
            "|cffaa00ffL\xC3\x89GENDAIRE ! ({} kills cons\xC3\xA9""cutifs)|r", current);
        LOG_INFO("conquest", "KillStreak: '{}' atteint Tier4 ({} kills)", killer->GetName(), current);
    }
    else if (current == tier5)
    {
        killer->RemoveAura(TIER4_AURA);
        killer->AddAura(TIER5_AURA, killer);
        ChatHandler(killer->GetSession()).PSendSysMessage(
            "|cff00ffffDIEU DE GUERRE ! ({} kills cons\xC3\xA9""cutifs)|r", current);
        LOG_INFO("conquest", "KillStreak: '{}' atteint Tier5 ({} kills)", killer->GetName(), current);
    }
}

void OnDeath(Player* victim)
{
    if (!victim)
        return;

    uint32 guid = victim->GetGUID().GetCounter();
    Persist(guid, 0, GetMaxStreak(guid));

    if (uint32 spell = OnFireSpellFor(victim))
        victim->RemoveAura(spell);
    if (uint32 spell = PrestigeSpellFor(victim))
        victim->RemoveAura(spell);
    victim->RemoveAura(PENNON_COMPANION_AURA);
    victim->RemoveAura(TIER3_AURA);
    victim->RemoveAura(TIER4_AURA);
    victim->RemoveAura(TIER5_AURA);
}

} // namespace ConquestKillStreak
