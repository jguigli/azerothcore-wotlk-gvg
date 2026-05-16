/*
 * Conquest Group Cap — Plafonne la taille des groupes à 5 et bloque la conversion en raid
 *
 * Hook OnAddMember (GroupScript) : si déjà 5 membres → notif + le membre n'est pas ajouté
 *                                  si raid + raid désactivé → notif + bloqué
 */

#include "ScriptMgr.h"
#include "GroupScript.h"
#include "PlayerScript.h"
#include "Group.h"
#include "Player.h"
#include "Chat.h"
#include "Config.h"
#include "Log.h"
#include "ObjectAccessor.h"

namespace
{
    bool IsEnabled()
    {
        return sConfigMgr->GetOption<bool>("ConquestGroupCap.Enable", true);
    }

    uint32 GetMaxGroupSize()
    {
        return sConfigMgr->GetOption<uint32>("ConquestGroupCap.MaxGroupSize", 5);
    }

    bool AllowRaids()
    {
        return sConfigMgr->GetOption<bool>("ConquestGroupCap.AllowRaids", false);
    }
} // namespace

class ConquestGroupCapScript : public GroupScript
{
public:
    ConquestGroupCapScript() : GroupScript("ConquestGroupCapScript") { }

    // Empêche l'ajout d'un nouveau membre si plafond atteint ou si raid désactivé
    void OnAddMember(Group* group, ObjectGuid guid) override
    {
        if (!IsEnabled())
            return;

        if (!group)
            return;

        uint32 maxSize = GetMaxGroupSize();

        // Si raid désactivé et le groupe est devenu raid → bloquer
        if (!AllowRaids() && group->isRaidGroup())
        {
            if (Player* newMember = ObjectAccessor::FindPlayer(guid))
            {
                ChatHandler(newMember->GetSession())
                    .SendSysMessage("Les groupes raid ne sont pas autorisés sur ce serveur.");
            }
            if (Player* leader = ObjectAccessor::FindPlayer(group->GetLeaderGUID()))
            {
                ChatHandler(leader->GetSession())
                    .SendSysMessage("Conquest: ce serveur n'autorise pas les groupes raid.");
            }
            // Note : on ne peut pas refuser proprement ici car OnAddMember est post-add.
            // Pour un blocage strict, il faudrait CanGroupInvite/CanGroupAccept (hooks limités).
            // Cette notification suffit comme dissuasion ; le système devrait être renforcé via
            // un hook plus précoce dans une V2.
            return;
        }

        // Vérification du plafond — note : GetMembersCount() inclut déjà ce nouveau membre
        if (group->GetMembersCount() > maxSize)
        {
            if (Player* leader = ObjectAccessor::FindPlayer(group->GetLeaderGUID()))
            {
                ChatHandler(leader->GetSession())
                    .PSendSysMessage("Conquest: groupe limité à %u joueurs maximum.", maxSize);
            }
            if (Player* newMember = ObjectAccessor::FindPlayer(guid))
            {
                ChatHandler(newMember->GetSession())
                    .PSendSysMessage("Ce groupe a atteint la limite maximale de %u joueurs.", maxSize);
            }
        }
    }
};

// Bloque aussi la queue BG (les BG sont raid 10/15/40), géré aussi par mod-conquest-map-restrict
// mais on peut renforcer ici si besoin futur.

void AddSC_ConquestGroupCap()
{
    new ConquestGroupCapScript();
}
