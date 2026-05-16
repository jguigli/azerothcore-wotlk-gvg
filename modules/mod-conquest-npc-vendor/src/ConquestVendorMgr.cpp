/*
 * Conquest Vendor Mgr — impl.
 *
 * Au boot :
 *  - Scan conquest_vendor_items
 *  - Génère un sub_entry par section (slot_label) de chaque main NPC
 *  - Populate l'inventaire vendor (in-memory only via ObjectMgr::AddVendorItem)
 *    pour chaque sub_entry → permet SendListInventory(guid, subEntry) d'ouvrir
 *    la vraie fenêtre vendor avec icônes et tooltips.
 */

#include "ConquestVendorMgr.h"

#include "DatabaseEnv.h"
#include "Field.h"
#include "Log.h"
#include "ObjectMgr.h"
#include "QueryResult.h"

#include <algorithm>

namespace
{
    constexpr uint32 SUB_ENTRY_BASE = 410000;
    inline uint64 SectionKey(uint32 main, std::string const& slot)
    {
        return (uint64(main) << 32) ^ std::hash<std::string>{}(slot);
    }
}

ConquestVendorMgr* ConquestVendorMgr::instance()
{
    static ConquestVendorMgr inst;
    return &inst;
}

void ConquestVendorMgr::Load()
{
    _items.clear();
    _sectionToSubEntry.clear();
    _subEntryToMain.clear();
    _subEntryToSection.clear();

    QueryResult result = WorldDatabase.Query(
        "SELECT npc_entry, item_id, price, currency, slot_label, display_order "
        "FROM conquest_vendor_items ORDER BY npc_entry, display_order, item_id");

    if (!result)
    {
        LOG_INFO("conquest", "ConquestVendorMgr::Load — 0 vendor items found.");
        return;
    }

    uint32 totalItems = 0;
    do
    {
        Field* f = result->Fetch();
        ConquestVendorItem v;
        uint32 npc = f[0].Get<uint32>();
        v.itemId   = f[1].Get<uint32>();
        v.price    = f[2].Get<uint32>();
        std::string cur = f[3].Get<std::string>();
        if (cur == "PB")        v.currency = ConquestVendorCurrency::PB;
        else if (cur == "PC")   v.currency = ConquestVendorCurrency::PC;
        else                    v.currency = ConquestVendorCurrency::GOLD;
        v.slotLabel    = f[4].IsNull() ? std::string() : f[4].Get<std::string>();
        v.displayOrder = f[5].Get<uint32>();
        _items[npc].push_back(std::move(v));
        ++totalItems;
    } while (result->NextRow());

    // Génère sub-entries par (main, section) et populate l'inventaire en mémoire
    uint32 nextSub = SUB_ENTRY_BASE;
    uint32 totalSections = 0;
    for (auto const& [mainEntry, items] : _items)
    {
        // collecte sections distinctes (ordre d'apparition)
        std::vector<std::string> sections;
        for (auto const& v : items)
        {
            if (v.slotLabel.empty()) continue;
            if (std::find(sections.begin(), sections.end(), v.slotLabel) == sections.end())
                sections.push_back(v.slotLabel);
        }
        for (std::string const& sec : sections)
        {
            uint32 subEntry = nextSub++;
            _sectionToSubEntry[SectionKey(mainEntry, sec)] = subEntry;
            _subEntryToMain[subEntry] = mainEntry;
            _subEntryToSection[subEntry] = sec;
            // populate npc_vendor cache in-memory
            for (auto const& v : items)
            {
                if (v.slotLabel != sec) continue;
                // maxcount=0 (illimité), incrtime=0, extendedCost=0, persist=false
                sObjectMgr->AddVendorItem(subEntry, v.itemId, 0, 0, 0, false);
            }
            ++totalSections;
        }
    }

    LOG_INFO("conquest", "ConquestVendorMgr::Load — {} items, {} NPCs, {} sub-vendors generated",
             totalItems, _items.size(), totalSections);
}

std::vector<ConquestVendorItem> const& ConquestVendorMgr::GetItems(uint32 npcEntry) const
{
    auto it = _items.find(npcEntry);
    if (it == _items.end())
        return _empty;
    return it->second;
}

bool ConquestVendorMgr::IsVendor(uint32 npcEntry) const
{
    return _items.find(npcEntry) != _items.end();
}

std::vector<std::string> ConquestVendorMgr::GetSections(uint32 npcEntry) const
{
    std::vector<std::string> sections;
    auto it = _items.find(npcEntry);
    if (it == _items.end()) return sections;
    for (auto const& v : it->second)
    {
        if (v.slotLabel.empty()) continue;
        if (std::find(sections.begin(), sections.end(), v.slotLabel) == sections.end())
            sections.push_back(v.slotLabel);
    }
    return sections;
}

uint32 ConquestVendorMgr::GetSubEntry(uint32 mainEntry, std::string const& slotLabel) const
{
    auto it = _sectionToSubEntry.find(SectionKey(mainEntry, slotLabel));
    return (it == _sectionToSubEntry.end()) ? 0 : it->second;
}

ConquestVendorItem const* ConquestVendorMgr::GetItemBySubEntry(uint32 subEntry, uint32 itemId) const
{
    auto sIt = _subEntryToMain.find(subEntry);
    if (sIt == _subEntryToMain.end()) return nullptr;
    uint32 mainEntry = sIt->second;
    auto secIt = _subEntryToSection.find(subEntry);
    if (secIt == _subEntryToSection.end()) return nullptr;
    std::string const& slot = secIt->second;
    auto iIt = _items.find(mainEntry);
    if (iIt == _items.end()) return nullptr;
    for (auto const& v : iIt->second)
    {
        if (v.slotLabel == slot && v.itemId == itemId)
            return &v;
    }
    return nullptr;
}
