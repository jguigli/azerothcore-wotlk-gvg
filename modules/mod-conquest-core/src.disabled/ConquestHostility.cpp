/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

#include "ConquestHostility.h"
#include "ConquestCore.h"
#include "Player.h"
#include "Guild.h"
#include "Config.h"
#include "Log.h"
#include "Map.h"

Conquest_PvP_Override::Conquest_PvP_Override()
    : PlayerScript("Conquest_PvP_Override")
{
}

void Conquest_PvP_Override::OnPlayerPVPKill(Player* killer, Player* killed)
{
    if (!sConfigMgr->GetOption<bool>("ConquestCore.Enable", true))
        return;

    if (!killer || !killed)
        return;

    // Check guild membership
    uint32 killerGuild = killer->GetGuildId();
    uint32 killedGuild = killed->GetGuildId();

    if (killerGuild == 0 || killedGuild == 0)
        return;

    if (killerGuild == killedGuild)
        return;

    // Check if guilds are hostile
    if (!ConquestCore::Instance().AreGuildsHostile(killerGuild, killedGuild))
        return;

    // Track the kill
    ConquestCore::Instance().OnPlayerKill(killer->GetGUID().GetCounter(), killed->GetGUID().GetCounter());
}

void Conquest_PvP_Override::OnPlayerIsFFAPvP(Player* player, bool& result)
{
    if (!sConfigMgr->GetOption<bool>("ConquestCore.Enable", true))
        return;

    // In FFA mode (GameType = 16), we need to allow FFA but protect same guild
    // The FFA flag is already set by the server, we just keep it
    // Protection happens via IfNormalReaction hook instead
    
    if (player->GetGuildId() > 0)
    {
        result = true; // Keep FFA active for guild members
    }
}

void Conquest_PvP_Override::OnPlayerUpdateArea(Player* player, uint32 /*oldArea*/, uint32 newArea)
{
    if (!sConfigMgr->GetOption<bool>("ConquestCore.Enable", true))
        return;

    // Force FFA PvP state when changing area (to prevent game from disabling it)
    if (player->GetGuildId() > 0)
    {
        // Force the area state BEFORE the game processes it
        player->pvpInfo.IsInFFAPvPArea = true;
        
        // Immediately force the FFA flag
        player->SetByteFlag(UNIT_FIELD_BYTES_2, 1, UNIT_BYTE2_FLAG_FFA_PVP);
        
        // Force UpdateFFAPvPState to reprocess with our forced value
        player->UpdateFFAPvPState(false);
        
        LOG_INFO("module", "Conquest: Area change - FFA forced for player {} (Guild {}) in area {}", 
                 player->GetName(), player->GetGuildId(), newArea);
    }
}

void Conquest_PvP_Override::OnPlayerFfaPvpStateUpdate(Player* player, bool /*result*/)
{
    if (!sConfigMgr->GetOption<bool>("ConquestCore.Enable", true))
        return;

    // CRITICAL: Force FFA PvP to stay active even when the game tries to disable it
    if (player->GetGuildId() > 0)
    {
        // Force the area state to FFA (this is what the game checks)
        player->pvpInfo.IsInFFAPvPArea = true;
        
        // Ensure the FFA flag is set
        if (!player->IsFFAPvP())
        {
            player->SetByteFlag(UNIT_FIELD_BYTES_2, 1, UNIT_BYTE2_FLAG_FFA_PVP);
        }
    }
}

// ================== NEW: Hostility Override ==================

Conquest_Hostility_Override::Conquest_Hostility_Override()
    : UnitScript("Conquest_Hostility_Override", true, { UNITHOOK_IF_NORMAL_REACTION })
{
}

bool Conquest_Hostility_Override::IfNormalReaction(Unit const* unit, Unit const* target, ReputationRank& repRank)
{
    if (!sConfigMgr->GetOption<bool>("ConquestCore.Enable", true))
        return true; // Continue with normal reaction

    // Only apply to players
    if (!unit->IsPlayer() || !target->IsPlayer())
        return true;

    Player const* player1 = unit->ToPlayer();
    Player const* player2 = target->ToPlayer();

    // Check guild membership
    uint32 guild1 = player1->GetGuildId();
    uint32 guild2 = player2->GetGuildId();

    // If either player is not in a guild, use normal reaction
    if (guild1 == 0 || guild2 == 0)
        return true;

    // Same guild = FORCE EXALTED (strongest friendly status to override FFA)
    if (guild1 == guild2)
    {
        repRank = REP_EXALTED; // Maximum friendly level
        return false; // Override to make them friendly
    }

    // Different guilds = check if they are hostile
    if (ConquestCore::Instance().AreGuildsHostile(guild1, guild2))
    {
        repRank = REP_HATED; // Force maximum hostile
        return false; // Override normal reaction
    }

    // Not hostile = continue with normal reaction
    return true;
}

// ================== NEW: Auto-enable PvP for guild enemies ==================

class Conquest_PvP_Flag_Manager : public PlayerScript
{
public:
    Conquest_PvP_Flag_Manager() : PlayerScript("Conquest_PvP_Flag_Manager") { }

    // When a player logs in, enable FFA PvP mode for ALL players
    // NOTE: Server is in GameType = 16 (FFA global), so everyone should have FFA
    void OnPlayerLogin(Player* player) override
    {
        if (!sConfigMgr->GetOption<bool>("ConquestCore.Enable", true))
            return;

        // Force PvP mode for all players
        player->SetPvP(true);
        player->UpdatePvP(true, true);
        
        // Force FFA flag
        player->pvpInfo.IsInFFAPvPArea = true;
        player->SetByteFlag(UNIT_FIELD_BYTES_2, 1, UNIT_BYTE2_FLAG_FFA_PVP);
        
        LOG_INFO("module", "Conquest: Player {} (Guild {}) - FFA PvP mode activated on login", 
                 player->GetName(), player->GetGuildId());
    }

    // Keep PvP active AND force FFA PvP for ALL players
    // CRITICAL: This MUST run every frame to override the game's constant resets
    // NOTE: We force FFA for everyone since GameType = 16 (FFA global server)
    void OnPlayerUpdate(Player* player, uint32 /*diff*/) override
    {
        if (!sConfigMgr->GetOption<bool>("ConquestCore.Enable", true))
            return;

        // NO TIMER - Force FFA every single frame
        // The game resets IsInFFAPvPArea constantly, we must be faster
        
        // Force FFA PvP area state (this allows attacking same faction)
        player->pvpInfo.IsInFFAPvPArea = true;
        
        // Ensure PvP flag stays active
        if (!player->HasPlayerFlag(PLAYER_FLAGS_IN_PVP))
        {
            player->SetPvP(true);
        }
        
        // Force FFA PvP flag every frame
        if (!player->IsFFAPvP())
        {
            player->SetByteFlag(UNIT_FIELD_BYTES_2, 1, UNIT_BYTE2_FLAG_FFA_PVP);
        }
    }
};

// ================== NEW: Guild Change Handler ==================

Conquest_Guild_Update::Conquest_Guild_Update() : GuildScript("Conquest_Guild_Update") { }

void Conquest_Guild_Update::OnAddMember(Guild* /*guild*/, Player* player, uint8& /*plRank*/)
{
    if (!sConfigMgr->GetOption<bool>("ConquestCore.Enable", true))
        return;

    if (!player || !player->IsInWorld())
        return;

    LOG_INFO("module", "Conquest: Player {} joined guild {} - forcing visibility update", player->GetName(), player->GetGuildId());

    // Force update of the player's faction and bytes fields (like Group::BroadcastGroupUpdate)
    player->ForceValuesUpdateAtIndex(UNIT_FIELD_BYTES_2);
    player->ForceValuesUpdateAtIndex(UNIT_FIELD_FACTIONTEMPLATE);

    // Get all players in the same map
    Map::PlayerList const& players = player->GetMap()->GetPlayers();
    for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
    {
        Player* otherPlayer = itr->GetSource();
        if (!otherPlayer || otherPlayer == player || !otherPlayer->IsInWorld())
            continue;

        // Check if players are in visibility range
        if (!player->IsWithinDistInMap(otherPlayer, player->GetSightRange()))
            continue;

        // Force update of other players' fields too
        otherPlayer->ForceValuesUpdateAtIndex(UNIT_FIELD_BYTES_2);
        otherPlayer->ForceValuesUpdateAtIndex(UNIT_FIELD_FACTIONTEMPLATE);
    }
}

void Conquest_Guild_Update::OnRemoveMember(Guild* guild, Player* player, bool /*isDisbanding*/, bool /*isKicked*/)
{
    if (!sConfigMgr->GetOption<bool>("ConquestCore.Enable", true))
        return;

    if (!player || !player->IsInWorld())
        return;

    uint32 oldGuildId = guild ? guild->GetId() : 0;
    LOG_INFO("module", "Conquest: Player {} left guild {} - forcing visibility update", player->GetName(), oldGuildId);

    // Force guild ID to 0 NOW to make the player appear hostile
    player->SetUInt32Value(PLAYER_GUILDID, 0);

    // Force update of the player's faction and bytes fields (like Group::BroadcastGroupUpdate)
    player->ForceValuesUpdateAtIndex(UNIT_FIELD_BYTES_2);
    player->ForceValuesUpdateAtIndex(UNIT_FIELD_FACTIONTEMPLATE);

    // Get all players in the same map who were in the same guild
    Map::PlayerList const& players = player->GetMap()->GetPlayers();
    
    for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
    {
        Player* otherPlayer = itr->GetSource();
        if (!otherPlayer || otherPlayer == player || !otherPlayer->IsInWorld())
            continue;

        // Check if players are in visibility range and were in the same guild
        if (otherPlayer->GetGuildId() == oldGuildId && 
            player->IsWithinDistInMap(otherPlayer, player->GetSightRange()))
        {
            // Force update of former guildmates' fields too
            otherPlayer->ForceValuesUpdateAtIndex(UNIT_FIELD_BYTES_2);
            otherPlayer->ForceValuesUpdateAtIndex(UNIT_FIELD_FACTIONTEMPLATE);
        }
    }
}

// Add all hostility scripts
void AddConquestHostilityScripts()
{
    new Conquest_PvP_Override();
    new Conquest_Hostility_Override();
    new Conquest_PvP_Flag_Manager();
    new Conquest_Guild_Update();
}
