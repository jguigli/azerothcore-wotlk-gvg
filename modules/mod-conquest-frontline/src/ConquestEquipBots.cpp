/*
 * Conquest Frontline — Force-equip permanent du set R14 PvP sur les bots.
 *
 * Au login de chaque bot (compte prefix "rndbot"), on déséquipe le random gear
 * et on applique le set R14 PvP max custom :
 *   - DK : entries custom 80100-80108 (Alliance) ou 80110-80118 (Horde)
 *   - Autres classes : items vanilla "Grand Marshal's" / "High Warlord's" R14
 *     query'és à runtime via item_template (filtre AllowableClass + faction par name).
 *
 * Permanent : à chaque login le set est restauré, même si playerbots randomize
 * a chang\xC3\xA9 le gear entre-temps.
 */

#include "Chat.h"
#include "CharacterCache.h"
#include "Config.h"
#include "DatabaseEnv.h"
#include "Item.h"
#include "ItemTemplate.h"
#include "Log.h"
#include "LoginDatabase.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "WorldSession.h"
#include <array>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace
{
    // Slots d'équipement à remplir (les ranged sont conditionnels selon classe)
    constexpr std::array<uint8, 17> SLOTS = {
        EQUIPMENT_SLOT_HEAD, EQUIPMENT_SLOT_NECK, EQUIPMENT_SLOT_SHOULDERS,
        EQUIPMENT_SLOT_BACK, EQUIPMENT_SLOT_CHEST, EQUIPMENT_SLOT_WAIST,
        EQUIPMENT_SLOT_LEGS, EQUIPMENT_SLOT_FEET, EQUIPMENT_SLOT_WRISTS,
        EQUIPMENT_SLOT_HANDS, EQUIPMENT_SLOT_FINGER1, EQUIPMENT_SLOT_FINGER2,
        EQUIPMENT_SLOT_TRINKET1, EQUIPMENT_SLOT_TRINKET2, EQUIPMENT_SLOT_MAINHAND,
        EQUIPMENT_SLOT_OFFHAND, EQUIPMENT_SLOT_RANGED
    };

    // Mapping slot \xE2\x86\x92 InventoryType (utilis\xC3\xA9 pour filtrer dans item_template)
    uint32 SlotToInventoryType(uint8 slot)
    {
        switch (slot)
        {
            case EQUIPMENT_SLOT_HEAD:      return INVTYPE_HEAD;
            case EQUIPMENT_SLOT_NECK:      return INVTYPE_NECK;
            case EQUIPMENT_SLOT_SHOULDERS: return INVTYPE_SHOULDERS;
            case EQUIPMENT_SLOT_BACK:      return INVTYPE_CLOAK;
            case EQUIPMENT_SLOT_CHEST:     return INVTYPE_CHEST;
            case EQUIPMENT_SLOT_WAIST:     return INVTYPE_WAIST;
            case EQUIPMENT_SLOT_LEGS:      return INVTYPE_LEGS;
            case EQUIPMENT_SLOT_FEET:      return INVTYPE_FEET;
            case EQUIPMENT_SLOT_WRISTS:    return INVTYPE_WRISTS;
            case EQUIPMENT_SLOT_HANDS:     return INVTYPE_HANDS;
            case EQUIPMENT_SLOT_FINGER1:
            case EQUIPMENT_SLOT_FINGER2:   return INVTYPE_FINGER;
            case EQUIPMENT_SLOT_TRINKET1:
            case EQUIPMENT_SLOT_TRINKET2:  return INVTYPE_TRINKET;
            case EQUIPMENT_SLOT_MAINHAND:  return INVTYPE_WEAPON;
            case EQUIPMENT_SLOT_OFFHAND:   return INVTYPE_SHIELD;
            case EQUIPMENT_SLOT_RANGED:    return INVTYPE_RANGED;
        }
        return 0;
    }

    bool IsBotAccount(uint32 accountId)
    {
        // Cache de comptes bot pour ne pas requ\xC3\xAAter \xC3\xA0 chaque login
        static std::unordered_set<uint32> botAccountsCache;
        static bool cacheLoaded = false;
        if (!cacheLoaded)
        {
            QueryResult r = LoginDatabase.Query(
                "SELECT id FROM account WHERE username LIKE 'rndbot%%'");
            if (r)
            {
                do
                {
                    botAccountsCache.insert(r->Fetch()[0].Get<uint32>());
                } while (r->NextRow());
            }
            cacheLoaded = true;
            LOG_INFO("conquest", "ConquestEquipBots: cached {} bot accounts", botAccountsCache.size());
        }
        return botAccountsCache.count(accountId) > 0;
    }

    // Cache des entries d'items s\xC3\xA9lectionn\xC3\xA9s par (classe, faction, slot)
    struct EquipKey
    {
        uint8 classId; uint8 team; uint8 slot;
        bool operator==(EquipKey const& o) const { return classId==o.classId && team==o.team && slot==o.slot; }
    };
    struct EquipKeyHash
    {
        size_t operator()(EquipKey const& k) const {
            return (uint32(k.classId) << 16) ^ (uint32(k.team) << 8) ^ uint32(k.slot);
        }
    };

    // Classes pouvant \xC3\xA9quiper un bouclier
    bool CanUseShield(uint8 classId)
    {
        return classId == CLASS_WARRIOR || classId == CLASS_PALADIN || classId == CLASS_SHAMAN;
    }

    // Classes sans offhand utile (caster pur, hunter, druide, rogue gere via 2x mainhand)
    bool HasOffhand(uint8 classId)
    {
        // Warrior fury / Rogue / Shaman enh peuvent dual-wield, mais on n'a pas la spec ici.
        // Pour MVP, on \xC3\xA9quipe l'offhand uniquement pour les classes pouvant porter bouclier.
        return CanUseShield(classId);
    }

    // ====== Set rare starter — BIS hardcod\xC3\xA9 par slot (utilisateur design) ======
    // Alliance :
    //   - Head + Shoulders : Lieutenant Commander's
    //   - Chest + Legs     : Knight-Captain's
    //   - Hands + Feet     : Knight-Lieutenant's
    // Horde :
    //   - Head + Shoulders : Champion's
    //   - Chest + Legs     : Blood Guard's
    //   - Hands + Feet     : Legionnaire's
    // Autres slots : fallback R14 \xE2\x86\x92 R10 par ordre de qualit\xC3\xA9.
    std::string GetRareSetPrefix(uint8 team, uint8 slot)
    {
        if (team == TEAM_ALLIANCE)
        {
            switch (slot)
            {
                case EQUIPMENT_SLOT_HEAD:
                case EQUIPMENT_SLOT_SHOULDERS: return "Lieutenant Commander''s";
                case EQUIPMENT_SLOT_CHEST:
                case EQUIPMENT_SLOT_LEGS:      return "Knight-Captain''s";
                case EQUIPMENT_SLOT_HANDS:
                case EQUIPMENT_SLOT_FEET:      return "Knight-Lieutenant''s";
            }
        }
        else
        {
            switch (slot)
            {
                case EQUIPMENT_SLOT_HEAD:
                case EQUIPMENT_SLOT_SHOULDERS: return "Champion''s";
                case EQUIPMENT_SLOT_CHEST:
                case EQUIPMENT_SLOT_LEGS:      return "Blood Guard''s";
                case EQUIPMENT_SLOT_HANDS:
                case EQUIPMENT_SLOT_FEET:      return "Legionnaire''s";
            }
        }
        return ""; // pas de mapping rare pour ce slot, on fera fallback
    }

    // Fallback : R14 \xE2\x86\x92 R10 pour les slots non couverts par le set rare (neck/back/waist/wrists/etc.)
    std::vector<std::string> GetFallbackPrefixes(uint8 team)
    {
        if (team == TEAM_ALLIANCE)
            return { "Grand Marshal''s", "Field Marshal''s", "Marshal''s",
                     "Lieutenant Commander''s", "Commander''s",
                     "Knight-Captain''s", "Knight-Lieutenant''s" };
        return { "High Warlord''s", "Warlord''s", "General''s",
                 "Legionnaire''s", "Champion''s",
                 "Centurion''s", "Stone Guard''s", "Blood Guard''s" };
    }

    uint32 FindR14ItemForSlot(uint8 classId, uint8 team, uint8 slot)
    {
        static std::unordered_map<EquipKey, uint32, EquipKeyHash> cache;
        EquipKey key{ classId, team, slot };
        auto it = cache.find(key);
        if (it != cache.end())
            return it->second;

        uint32 entry = 0;

        // Filtrer les classes non \xC3\xA9ligibles pour SHIELD / OFFHAND
        if (slot == EQUIPMENT_SLOT_OFFHAND)
        {
            if (!HasOffhand(classId))
            {
                cache[key] = 0;
                return 0;
            }
        }
        // Ranged : skip pour les classes sans ranged (mage/priest/druid utilisent wand mais ok)
        // On laisse passer, le query renverra 0 si rien.

        uint32 invType = SlotToInventoryType(slot);
        if (!invType)
        {
            cache[key] = 0;
            return 0;
        }

        // Pour OFFHAND on cherche un shield (INVTYPE_SHIELD d\xC3\xA9j\xC3\xA0 set par SlotToInventoryType)
        // Pour les autres slots on garde l'invType standard.

        uint32 classBit = 1u << (classId - 1);

        // DK: override avec entries custom (80100-80108 Alliance, 80110-80118 Horde rare)
        if (classId == CLASS_DEATH_KNIGHT)
        {
            uint32 base = (team == TEAM_ALLIANCE) ? 80100 : 80110;
            uint32 maxEntry = base + 8;
            QueryResult r = WorldDatabase.Query(
                "SELECT entry FROM item_template "
                "WHERE entry BETWEEN {} AND {} AND InventoryType = {} LIMIT 1",
                base, maxEntry, invType);
            if (r)
                entry = r->Fetch()[0].Get<uint32>();
        }
        else
        {
            // 1. Essayer le set rare hardcod\xC3\xA9 (head/shoulders/chest/legs/hands/feet)
            std::string rarePrefix = GetRareSetPrefix(team, slot);
            if (!rarePrefix.empty())
            {
                QueryResult r = WorldDatabase.Query(
                    "SELECT entry FROM item_template "
                    "WHERE InventoryType = {} AND AllowableClass & {} > 0 "
                    "AND name LIKE '{} %' "
                    "ORDER BY ItemLevel DESC LIMIT 1",
                    invType, classBit, rarePrefix);
                if (r)
                    entry = r->Fetch()[0].Get<uint32>();
            }

            // 2. Fallback : essai successif des autres pr\xC3\xA9fixes (R14 \xE2\x86\x92 R10)
            //    pour les slots non couverts par le set rare ou si pas de match.
            if (!entry)
            {
                for (std::string const& prefix : GetFallbackPrefixes(team))
                {
                    QueryResult r = WorldDatabase.Query(
                        "SELECT entry FROM item_template "
                        "WHERE InventoryType = {} AND AllowableClass & {} > 0 "
                        "AND name LIKE '{} %' "
                        "ORDER BY ItemLevel DESC LIMIT 1",
                        invType, classBit, prefix);
                    if (r)
                    {
                        entry = r->Fetch()[0].Get<uint32>();
                        break;
                    }
                }
            }
        }

        cache[key] = entry;
        return entry;
    }

    void EquipR14Set(Player* player)
    {
        if (!player) return;
        uint8 classId = player->getClass();
        uint8 team = player->GetTeamId();

        uint32 equipped = 0;
        for (uint8 slot : SLOTS)
        {
            uint32 entry = FindR14ItemForSlot(classId, team, slot);
            if (!entry)
                continue;

            // Retire ce qui est d\xC3\xA9j\xC3\xA0 \xC3\xA9quip\xC3\xA9 (pour \xC3\xA9viter doublons / refus EquipNewItem)
            if (Item* old = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
            {
                if (old->GetEntry() == entry)
                    continue; // d\xC3\xA9j\xC3\xA0 bon
                player->DestroyItem(INVENTORY_SLOT_BAG_0, slot, true);
            }

            // \xC3\x89quipe le nouvel item dans le slot exact
            if (Item* newItem = player->EquipNewItem(slot, entry, true))
            {
                player->AutoUnequipOffhandIfNeed();
                ++equipped;
            }
        }

        if (equipped)
            LOG_INFO("conquest", "ConquestEquipBots: equipped {} R14 PvP pieces on '{}' (class={}, team={})",
                     equipped, player->GetName(), classId, team);
    }
}

class ConquestEquipBots : public PlayerScript
{
public:
    ConquestEquipBots() : PlayerScript("ConquestEquipBots") { }

    void OnPlayerLogin(Player* player) override
    {
        if (!sConfigMgr->GetOption<bool>("ConquestFrontline.AutoEquipBots", true))
            return;
        if (!player)
            return;
        if (!player->GetSession())
            return;
        if (!IsBotAccount(player->GetSession()->GetAccountId()))
            return;
        // Ne pas re-\xC3\xA9quiper les bots niveau < 60 (pas encore randomized)
        if (player->GetLevel() < 60)
            return;
        EquipR14Set(player);
    }
};

void AddSC_ConquestEquipBots()
{
    new ConquestEquipBots();
}
