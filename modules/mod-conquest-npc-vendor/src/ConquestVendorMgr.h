/*
 * Conquest Vendor Mgr — manager DB-driven pour 15 NPCs vendor lvl 60.
 *
 * Chargé au boot, scan `conquest_vendor_npc` + `conquest_vendor_items`.
 * Le CreatureScript générique consulte ce mgr lors des hooks gossip.
 *
 * Devise par item : PB (Points de Bataille), PC (Points de Conquête), GOLD.
 */

#ifndef _CONQUEST_VENDOR_MGR_H_
#define _CONQUEST_VENDOR_MGR_H_

#include "Define.h"
#include <string>
#include <unordered_map>
#include <vector>

enum class ConquestVendorCurrency
{
    PB,
    PC,
    GOLD
};

struct ConquestVendorItem
{
    uint32 itemId = 0;
    uint32 price = 0;
    ConquestVendorCurrency currency = ConquestVendorCurrency::PB;
    std::string slotLabel;
    uint32 displayOrder = 100;
};

class ConquestVendorMgr
{
public:
    static ConquestVendorMgr* instance();

    /// Charge npc list + items depuis la DB. Appelé au boot worldserver.
    void Load();

    /// Retourne la liste d'items vendus par cet NPC, triée par display_order.
    std::vector<ConquestVendorItem> const& GetItems(uint32 npcEntry) const;

    /// Vérifie si un NPC est un vendor Conquête.
    bool IsVendor(uint32 npcEntry) const;

    /// Liste ordonnée des sections (slot_label distincts) du NPC.
    std::vector<std::string> GetSections(uint32 npcEntry) const;

    /// Sub-entry pour (main_npc_entry, slot_label). Utilisé par SendListInventory.
    /// Retourne 0 si pas trouvé.
    uint32 GetSubEntry(uint32 mainEntry, std::string const& slotLabel) const;

    /// Pour le hook buy : trouve l'item dans nos données via le sub_entry.
    /// Retourne nullptr si pas trouvé.
    ConquestVendorItem const* GetItemBySubEntry(uint32 subEntry, uint32 itemId) const;

    size_t GetVendorCount() const { return _items.size(); }
    size_t GetSubEntryCount() const { return _subEntryToMain.size(); }

private:
    ConquestVendorMgr() = default;
    std::unordered_map<uint32, std::vector<ConquestVendorItem>> _items;
    std::vector<ConquestVendorItem> _empty; // pour retour const-ref si entry inconnu

    // (main_npc, slot_label) → sub_entry
    std::unordered_map<uint64, uint32> _sectionToSubEntry;
    // sub_entry → main_npc (pour reverse lookup côté hook buy)
    std::unordered_map<uint32, uint32> _subEntryToMain;
    // sub_entry → slot_label
    std::unordered_map<uint32, std::string> _subEntryToSection;
};

#define sConquestVendorMgr ConquestVendorMgr::instance()

#endif // _CONQUEST_VENDOR_MGR_H_
