/*
 * Conquest Frontline — Hooks PvP unifiés (Phase 3 + Phase 5).
 *
 * OnPlayerPVPKill (cross-faction) :
 *   - +N Points de Bataille au killer (Phase 3)
 *   - Si victim était en On Fire / Prestige → Marques de Champion au killer (Phase 5 Path B)
 *   - Sinon : Path A — 1% chance globale de drop une Marque
 *   - Mise à jour du compteur de kill streak du killer (Phase 5)
 *
 * OnPlayerJustDied : reset du kill streak + retrait des auras visibles (Phase 5).
 */

#include "ConquestKillStreak.h"
#include "ConquestPoints.h"
#include "Chat.h"
#include "Config.h"
#include "Log.h"
#include "Player.h"
#include "ScriptMgr.h"

namespace
{
    constexpr uint32 ITEM_MARK_OF_CHAMPION = 400300;

    void AwardChampionMarks(Player* killer, uint32 count, char const* reason)
    {
        if (!killer || count == 0)
            return;
        ItemPosCountVec dest;
        InventoryResult msg = killer->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, ITEM_MARK_OF_CHAMPION, count);
        if (msg != EQUIP_ERR_OK)
        {
            ChatHandler(killer->GetSession()).PSendSysMessage(
                "|cffff8800{} Marque(s) de Champion non remise(s) (inventaire plein).|r", count);
            return;
        }
        if (Item* it = killer->StoreNewItem(dest, ITEM_MARK_OF_CHAMPION, true))
            killer->SendNewItem(it, count, true, false);
        LOG_INFO("conquest", "Marques de Champion: +{} a '{}' ({})",
                 count, killer->GetName(), reason);
    }
}

class ConquestPointsHooks : public PlayerScript
{
public:
    ConquestPointsHooks() : PlayerScript("ConquestPointsHooks") { }

    void OnPlayerPVPKill(Player* killer, Player* victim) override
    {
        if (!killer || !victim || killer == victim)
            return;
        if (killer->GetTeamId() == victim->GetTeamId())
            return; // duel same-faction n'octroie rien

        // --- Phase 3 : Points de Bataille ---
        uint32 reward = sConfigMgr->GetOption<uint32>("ConquestFrontline.BattlePointsPerKill", 1);
        ConquestPoints::AddBattlePoints(killer, reward);

        // --- Phase 5 : Marques de Champion ---
        // Path B (paliers) — on regarde l'état de la victime AVANT son éventuel reset.
        bool victimWasPrestige = ConquestKillStreak::IsPrestige(victim);
        bool victimWasOnFire   = ConquestKillStreak::IsOnFire(victim);

        if (victimWasPrestige)
        {
            uint32 marks = sConfigMgr->GetOption<uint32>("ConquestFrontline.MarksPerPrestigeKill", 3);
            AwardChampionMarks(killer, marks, "kill Prestige");
        }
        else if (victimWasOnFire)
        {
            uint32 marks = sConfigMgr->GetOption<uint32>("ConquestFrontline.MarksPerOnFireKill", 1);
            AwardChampionMarks(killer, marks, "kill On Fire");
        }
        else
        {
            // Path A : chance globale (1% par défaut)
            uint32 chancePct = sConfigMgr->GetOption<uint32>("ConquestFrontline.MarksRandomChancePct", 1);
            if (chancePct > 0 && urand(0, 99) < chancePct)
                AwardChampionMarks(killer, 1, "drop aleatoire");
        }

        // --- Phase 5 : update killstreak (en dernier, après checks sur victim) ---
        ConquestKillStreak::OnKill(killer);
    }

    void OnPlayerJustDied(Player* victim) override
    {
        ConquestKillStreak::OnDeath(victim);
    }
};

void AddSC_ConquestPointsHooks()
{
    new ConquestPointsHooks();
}
