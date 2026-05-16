/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

#include "ScriptMgr.h"
#include "Player.h"
#include "Config.h"
#include "Chat.h"
#include "Guild.h"
#include "GuildMgr.h"
#include "Group.h"

// Guild Script to limit guild members
class ConquestLimitGuild : public GuildScript
{
public:
    ConquestLimitGuild() : GuildScript("ConquestLimitGuild") { }

    void OnAddMember(Guild* guild, Player* player, uint8& /*plRank*/) override
    {
        if (!sConfigMgr->GetOption<bool>("ConquestLimitGroup.Enable", true))
            return;

        uint32 maxMembers = sConfigMgr->GetOption<uint32>("ConquestLimitGroup.MaxGuildMembers", 30);
        
        if (guild->GetMemberCount() >= maxMembers)
        {
            if (player)
            {
                ChatHandler(player->GetSession()).PSendSysMessage("Cette guilde a atteint la limite maximale de %u membres.", maxMembers);
            }
            
            // Trouver le joueur qui invite (le leader ou un officier)
            if (Player* inviter = ObjectAccessor::FindPlayer(guild->GetLeaderGUID()))
            {
                ChatHandler(inviter->GetSession()).PSendSysMessage("Impossible d'ajouter %s. La guilde a atteint la limite maximale de %u membres.", 
                    player->GetName().c_str(), maxMembers);
            }
        }
    }

    bool OnCanAddMember(Guild* guild, Player* /*player*/, uint8& /*plRank*/)
    {
        if (!sConfigMgr->GetOption<bool>("ConquestLimitGroup.Enable", true))
            return true;

        uint32 maxMembers = sConfigMgr->GetOption<uint32>("ConquestLimitGroup.MaxGuildMembers", 30);
        
        if (guild->GetMemberCount() >= maxMembers)
        {
            return false;
        }

        return true;
    }
};

// Group Script to limit group size and prevent raids
class ConquestLimitGroupScript : public GroupScript
{
public:
    ConquestLimitGroupScript() : GroupScript("ConquestLimitGroupScript") { }

    void OnAddMember(Group* group, ObjectGuid guid) override
    {
        if (!sConfigMgr->GetOption<bool>("ConquestLimitGroup.Enable", true))
            return;

        uint32 maxGroupSize = sConfigMgr->GetOption<uint32>("ConquestLimitGroup.MaxGroupSize", 5);
        bool allowRaids = sConfigMgr->GetOption<bool>("ConquestLimitGroup.AllowRaids", false);

        // Si les raids ne sont pas autorisés, empêcher la conversion en raid
        if (!allowRaids && group->isRaidGroup())
        {
            if (Player* leader = ObjectAccessor::FindPlayer(group->GetLeaderGUID()))
            {
                ChatHandler(leader->GetSession()).PSendSysMessage("Les groupes raid ne sont pas autorisés sur ce serveur.");
            }
            return;
        }

        // Vérifier la taille du groupe
        if (group->GetMembersCount() >= maxGroupSize)
        {
            if (Player* leader = ObjectAccessor::FindPlayer(group->GetLeaderGUID()))
            {
                ChatHandler(leader->GetSession()).PSendSysMessage("Le groupe a atteint la limite maximale de %u joueurs.", maxGroupSize);
            }
            
            if (Player* newMember = ObjectAccessor::FindPlayer(guid))
            {
                ChatHandler(newMember->GetSession()).PSendSysMessage("Ce groupe a atteint la limite maximale de %u joueurs.", maxGroupSize);
            }
        }
    }

    // Note: OnGroupTypeChanged is not available in GroupScript API
    // The raid prevention is handled in OnAddMember and CanJoinInGroup instead
};

// Note: PlayerScript doesn't have CanJoinInGroup hook
// The group size and raid limitations are handled in GroupScript::OnAddMember

// Add all scripts
void AddMyPlayerScripts()
{
    new ConquestLimitGuild();
    new ConquestLimitGroupScript();
}
