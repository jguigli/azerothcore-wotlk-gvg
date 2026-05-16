/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 *
 * GMK vehicle vendor — summons custom siege engines + Leviathan with :
 *   - Faction-aware entry selection (Alliance/Horde variants).
 *   - Pricing en PC (Points de Conquete) via ConquestPoints::SpendConquestPoints.
 *   - Phase 1 force pour visibilite multi-faction.
 *   - Per-owner ownership via ConquestRegisterVehicleOwner (faction inheritance,
 *     only summoner can mount via PassengerBoarded hook).
 */

#include "ConquestPoints.h"
#include "ScriptMgr.h"
#include "Player.h"
#include "Config.h"
#include "Log.h"
#include "Creature.h"
#include "ScriptedGossip.h"
#include "Map.h"
#include "ObjectMgr.h"
#include "TemporarySummon.h"
#include "Chat.h"

#include <cmath>

// Provided by mod-conquest-mounts (ConquestMounts.cpp).
void ConquestRegisterVehicleOwner(Creature* vehicle, Player* owner);

#define VEHICLE_VENDOR_NPC_ENTRY 400101

namespace VehicleEntries
{
    // Veh originaux + variants v3
    constexpr uint32 PW8_RED              = 400200; // P-W8 Horde
    constexpr uint32 B27_BLUE             = 400201; // B27 Alliance
    constexpr uint32 CATAPULTE            = 400202; // partagee
    constexpr uint32 GLAIVE_VIOLET        = 400203; // Alliance
    constexpr uint32 GLAIVE_JAUNE         = 400204; // Horde (renomme ex-orange)
    constexpr uint32 DEMOLISSEUR          = 400205; // partagee
    constexpr uint32 M2_BLUE              = 400209; // M2 Alliance
    constexpr uint32 M2_RED               = 400310; // M2 Horde
    constexpr uint32 PW8_BLUE             = 400311; // P-W8 Alliance
    constexpr uint32 B27_RED              = 400312; // B27 Horde
    constexpr uint32 LEVIATHAN_ALLIANCE   = 400314;
    constexpr uint32 LEVIATHAN_HORDE      = 400315;
    constexpr uint32 PROTECTEUR_E800      = 400326; // partage 2 factions
}

namespace
{
    struct VehicleOption
    {
        uint32 entryAlliance; // 0 = option cachee pour Alliance
        uint32 entryHorde;    // 0 = option cachee pour Horde
        uint32 pricePC;       // Points de Conquete
        char const* label;
    };

    constexpr VehicleOption VEHICLE_MENU[] =
    {
        { VehicleEntries::M2_BLUE,            VehicleEntries::M2_RED,            50,  "Pisteur M2" },
        { VehicleEntries::PW8_BLUE,           VehicleEntries::PW8_RED,           80,  "Baroudeur P-W8" },
        { VehicleEntries::B27_BLUE,           VehicleEntries::B27_RED,           120, "Destructeur B27" },
        { VehicleEntries::LEVIATHAN_ALLIANCE, VehicleEntries::LEVIATHAN_HORDE,   250, "Leviathan 330" },
        { VehicleEntries::PROTECTEUR_E800,    VehicleEntries::PROTECTEUR_E800,   100, "Protecteur E800" },
        { VehicleEntries::CATAPULTE,          VehicleEntries::CATAPULTE,          30, "Catapulte" },
        { VehicleEntries::DEMOLISSEUR,        VehicleEntries::DEMOLISSEUR,        50, "Demolisseur" },
        { VehicleEntries::GLAIVE_VIOLET,      0,                                  40, "Lanceur de glaive violet" },
        { 0,                                  VehicleEntries::GLAIVE_JAUNE,       40, "Lanceur de glaive jaune" },
    };

    inline uint32 EntryForPlayer(VehicleOption const& opt, Player* p)
    {
        return (p->GetTeamId() == TEAM_ALLIANCE) ? opt.entryAlliance : opt.entryHorde;
    }
}

class ConquestVehicleVendorNPC : public CreatureScript
{
public:
    ConquestVehicleVendorNPC() : CreatureScript("ConquestVehicleVendorNPC") { }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        if (!sConfigMgr->GetOption<bool>("ConquestNpcVendor.Enable", true))
            return false;

        ClearGossipMenuFor(player);

        uint32 pc = ConquestPoints::GetConquestPoints(player->GetGUID().GetCounter());
        AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1,
            "Solde : " + std::to_string(pc) + " PC", GOSSIP_SENDER_MAIN, 0);

        for (uint32 i = 0; i < std::size(VEHICLE_MENU); ++i)
        {
            VehicleOption const& opt = VEHICLE_MENU[i];
            uint32 entry = EntryForPlayer(opt, player);
            if (entry == 0) continue; // option cachee pour cette faction
            if (!sObjectMgr->GetCreatureTemplate(entry))
            {
                LOG_ERROR("module", "ConquestVehicleVendor: template missing for entry {} ({})",
                          entry, opt.label);
                continue;
            }
            std::string label = std::string(">> ") + opt.label
                              + "  - " + std::to_string(opt.pricePC) + " PC";
            AddGossipItemFor(player, GOSSIP_ICON_VENDOR, label, GOSSIP_SENDER_MAIN, i + 1);
        }

        SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* /*creature*/, uint32 /*sender*/, uint32 action) override
    {
        if (!sConfigMgr->GetOption<bool>("ConquestNpcVendor.Enable", true))
            return false;

        ClearGossipMenuFor(player);

        if (action == 0 || action > std::size(VEHICLE_MENU))
        {
            CloseGossipMenuFor(player);
            return true;
        }

        VehicleOption const& opt = VEHICLE_MENU[action - 1];
        uint32 entry = EntryForPlayer(opt, player);
        if (entry == 0)
        {
            ChatHandler(player->GetSession()).SendSysMessage("Vehicule non disponible pour ta faction.");
            CloseGossipMenuFor(player);
            return true;
        }

        CreatureTemplate const* tmpl = sObjectMgr->GetCreatureTemplate(entry);
        if (!tmpl)
        {
            ChatHandler(player->GetSession()).SendSysMessage("Erreur: modele de vehicule introuvable.");
            CloseGossipMenuFor(player);
            return true;
        }

        // Verification + debit PC
        uint32 balance = ConquestPoints::GetConquestPoints(player->GetGUID().GetCounter());
        if (balance < opt.pricePC)
        {
            ChatHandler(player->GetSession()).PSendSysMessage(
                "|cffff0000Solde insuffisant (%u PC requis, tu as %u PC).|r",
                opt.pricePC, balance);
            CloseGossipMenuFor(player);
            return true;
        }
        if (!ConquestPoints::SpendConquestPoints(player, opt.pricePC))
        {
            ChatHandler(player->GetSession()).SendSysMessage("|cffff0000Erreur paiement PC.|r");
            CloseGossipMenuFor(player);
            return true;
        }

        // Spawn 5y devant le joueur, despawn auto apres 5 min.
        float x, y, z, o;
        player->GetPosition(x, y, z);
        o = player->GetOrientation();
        x += 5.0f * std::cos(o);
        y += 5.0f * std::sin(o);

        Creature* vehicle = player->SummonCreature(entry, x, y, z, o,
                                                   TEMPSUMMON_TIMED_DESPAWN, 300000);
        if (!vehicle)
        {
            ChatHandler(player->GetSession()).SendSysMessage("Erreur: impossible de creer le vehicule.");
            // Re-credit en cas d'echec spawn
            ConquestPoints::AddConquestPoints(player, opt.pricePC);
            CloseGossipMenuFor(player);
            return true;
        }

        // Force phase 1 -> visible par tous les joueurs (peu importe le phase
        // du GM qui aurait pu summon en phase 2/3).
        LOG_INFO("module", "ConquestVehicleVendor: BUY player phaseMask={} vehicle BEFORE SetPhaseMask={}",
                 player->GetPhaseMask(), vehicle->GetPhaseMask());
        vehicle->SetPhaseMask(1, true);
        LOG_INFO("module", "ConquestVehicleVendor: vehicle AFTER SetPhaseMask(1)={}",
                 vehicle->GetPhaseMask());

        ConquestRegisterVehicleOwner(vehicle, player);

        LOG_INFO("module", "ConquestVehicleVendor: {} bought {} (entry {}) for {} PC at ({:.1f}, {:.1f}, {:.1f})",
                 player->GetName(), opt.label, entry, opt.pricePC, x, y, z);

        ChatHandler(player->GetSession()).PSendSysMessage(
            "|cff00ff00Vehicule %s achete (-%u PC). Vous seul pouvez le monter.|r",
            opt.label, opt.pricePC);
        CloseGossipMenuFor(player);
        return true;
    }
};

void AddSC_ConquestVehicleVendor()
{
    new ConquestVehicleVendorNPC();
}
