/*
 * Conquest Vendor Script — gossip → vendor window standard avec icônes.
 *
 * Workflow :
 *  1. Player clic NPC → gossip menu : liste des sections (slot_labels)
 *  2. Click sur une section → SendListInventory(creature_guid, sub_entry)
 *     ouvre la fenêtre vendor WoW classique avec icônes et tooltips, filtrée
 *     sur les items de la section.
 *  3. Achat via le mécanisme vanilla → notre PlayerScript hook intercepte
 *     l'achat, vérifie le solde PB/PC, déduit, et autorise/refuse.
 */

#include "ConquestPoints.h"
#include "ConquestVendorMgr.h"
#include "AllCreatureScript.h"
#include "Chat.h"
#include "Creature.h"
#include "Log.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "PlayerScript.h"
#include "ScriptedGossip.h"
#include "WorldScript.h"
#include "WorldSession.h"

#include <algorithm>

namespace
{
    char const* CurrencyLabel(ConquestVendorCurrency c)
    {
        switch (c)
        {
            case ConquestVendorCurrency::PB:   return "PB";
            case ConquestVendorCurrency::PC:   return "PC";
            case ConquestVendorCurrency::GOLD: return "g";
        }
        return "?";
    }

    uint32 GetPlayerBalance(Player* p, ConquestVendorCurrency c)
    {
        if (!p) return 0;
        uint32 guid = p->GetGUID().GetCounter();
        switch (c)
        {
            case ConquestVendorCurrency::PB:   return ConquestPoints::GetBattlePoints(guid);
            case ConquestVendorCurrency::PC:   return ConquestPoints::GetConquestPoints(guid);
            case ConquestVendorCurrency::GOLD: return p->GetMoney() / 10000;
        }
        return 0;
    }

    bool DebitPlayer(Player* p, ConquestVendorCurrency c, uint32 amount)
    {
        if (!p) return false;
        switch (c)
        {
            case ConquestVendorCurrency::PB:
                return ConquestPoints::SpendBattlePoints(p, amount);
            case ConquestVendorCurrency::PC:
                return ConquestPoints::SpendConquestPoints(p, amount);
            case ConquestVendorCurrency::GOLD:
                if (p->GetMoney() < amount * 10000ULL) return false;
                p->ModifyMoney(-static_cast<int32>(amount * 10000));
                return true;
        }
        return false;
    }

    // Track currently selected sub-entry per player (pour le hook buy)
    static std::unordered_map<uint64, uint32> s_playerSubEntry;
}

class conquest_vendor_generic : public AllCreatureScript
{
public:
    conquest_vendor_generic() : AllCreatureScript("conquest_vendor_generic") {}

    bool CanCreatureGossipHello(Player* player, Creature* creature) override
    {
        if (!sConquestVendorMgr->IsVendor(creature->GetEntry()))
            return false;

        ClearGossipMenuFor(player);

        // Header solde
        uint32 pb = GetPlayerBalance(player, ConquestVendorCurrency::PB);
        uint32 pc = GetPlayerBalance(player, ConquestVendorCurrency::PC);
        AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1,
            "Solde : " + std::to_string(pb) + " PB | " + std::to_string(pc) + " PC",
            GOSSIP_SENDER_MAIN, 0);

        // Sections cliquables
        auto sections = sConquestVendorMgr->GetSections(creature->GetEntry());
        for (size_t i = 0; i < sections.size(); ++i)
        {
            AddGossipItemFor(player, GOSSIP_ICON_VENDOR,
                             ">> " + sections[i],
                             GOSSIP_SENDER_MAIN,
                             1000 + static_cast<uint32>(i));
        }

        SendGossipMenuFor(player, 1, creature->GetGUID());
        return true;
    }

    bool CanCreatureGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
    {
        if (!sConquestVendorMgr->IsVendor(creature->GetEntry()))
            return false;

        if (action < 1000)
        {
            CloseGossipMenuFor(player);
            return true;
        }

        uint32 sectionIdx = action - 1000;
        auto sections = sConquestVendorMgr->GetSections(creature->GetEntry());
        if (sectionIdx >= sections.size())
        {
            CloseGossipMenuFor(player);
            return true;
        }

        // Mémorise le sub_entry pour le hook de buy
        uint32 subEntry = sConquestVendorMgr->GetSubEntry(creature->GetEntry(), sections[sectionIdx]);
        LOG_INFO("conquest", "Vendor: '{}' click section '{}' on entry {} → sub_entry={}",
                 player->GetName(), sections[sectionIdx], creature->GetEntry(), subEntry);
        if (!subEntry)
        {
            ChatHandler(player->GetSession()).PSendSysMessage("|cffff0000Section non disponible.|r");
            CloseGossipMenuFor(player);
            return true;
        }
        s_playerSubEntry[player->GetGUID().GetRawValue()] = subEntry;

        // Diagnostic complet
        bool hasVendorFlag = creature->HasNpcFlag(UNIT_NPC_FLAG_VENDOR);
        VendorItemData const* vlist = sObjectMgr->GetNpcVendorItemList(subEntry);
        size_t itemCount = vlist ? vlist->GetItemCount() : 0;
        LOG_INFO("conquest", "Vendor: creature HasVENDORflag={} sub_entry={} item_list_size={}",
                 hasVendorFlag, subEntry, itemCount);

        // Pattern standard AC : Close puis SendListInventory
        CloseGossipMenuFor(player);
        player->GetSession()->SendListInventory(creature->GetGUID(), subEntry);
        return true;
    }
};

// Hook PlayerScript pour intercepter les achats sur nos sub-vendors
class conquest_vendor_buy_hook : public PlayerScript
{
public:
    conquest_vendor_buy_hook() : PlayerScript("conquest_vendor_buy_hook") {}

    void OnPlayerBeforeBuyItemFromVendor(Player* player, ObjectGuid /*vendorguid*/,
                                         uint32 /*vendorslot*/, uint32& item,
                                         uint8 count, uint8 /*bag*/, uint8 /*slot*/) override
    {
        if (!player || !item)
            return;

        auto it = s_playerSubEntry.find(player->GetGUID().GetRawValue());
        if (it == s_playerSubEntry.end())
            return; // pas notre vendor

        uint32 subEntry = it->second;
        ConquestVendorItem const* v = sConquestVendorMgr->GetItemBySubEntry(subEntry, item);
        if (!v)
            return; // item pas dans nos données → laisse passer (sécurité)

        // Vérifie solde
        uint32 cost = v->price * std::max<uint8>(count, 1);
        uint32 balance = GetPlayerBalance(player, v->currency);
        if (balance < cost)
        {
            ChatHandler(player->GetSession()).PSendSysMessage(
                "|cffff0000Solde insuffisant ({} {} requis, tu as {}).|r",
                cost, CurrencyLabel(v->currency), balance);
            item = 0; // annule l'achat (item invalide → buy refusé downstream)
            return;
        }

        // Débite PB/PC. Le vendor vanilla déduira en plus le gold du buyprice item_template,
        // mais comme les items vendus ici sont issus de raids (BuyPrice = 0 ou très bas),
        // l'impact gold est négligeable.
        if (!DebitPlayer(player, v->currency, cost))
        {
            ChatHandler(player->GetSession()).PSendSysMessage("|cffff0000Erreur paiement.|r");
            item = 0;
            return;
        }

        ChatHandler(player->GetSession()).PSendSysMessage(
            "|cff00ff00-{} {} (acheté %s)|r", cost, CurrencyLabel(v->currency));
    }
};

// WorldScript pour charger la table au boot. On utilise OnStartup (après
// ObjectMgr::LoadVendors) pour que notre AddVendorItem in-memory ne soit pas
// écrasé par le LoadVendors qui charge npc_vendor de la DB.
class conquest_vendor_worldscript : public WorldScript
{
public:
    conquest_vendor_worldscript() : WorldScript("conquest_vendor_worldscript") {}

    void OnStartup() override
    {
        sConquestVendorMgr->Load();
    }
};

void AddSC_ConquestVendorScript()
{
    LOG_INFO("conquest", "VendorScript: AddSC called");
    new conquest_vendor_generic();
    new conquest_vendor_buy_hook();
    new conquest_vendor_worldscript();
}
