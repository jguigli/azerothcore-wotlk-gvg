/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

#include "ScriptMgr.h"
#include "Player.h"
#include "Config.h"
#include "Chat.h"
#include "Creature.h"
#include "Item.h"
#include "ItemTemplate.h"
#include "ObjectMgr.h"
#include "Log.h"
#include "ScriptedGossip.h"
#include "World.h"
#include "DatabaseEnv.h"
#include "WorldDatabase.h"
#include "DBCStores.h"
#include "GameTime.h"
#include <string>
#include <map>
#include <vector>

// NPC entry
#define GEAR_MANAGER_NPC_ENTRY 400100

// Simple JSON parser for our specific format
class SimpleJsonParser
{
public:
    struct SlotData
    {
        uint32 item = 0;
        uint32 enchant = 0;
        std::map<uint8, uint32> gems; // gem slot -> gem item id
    };

    static std::string TrimWhitespace(const std::string& str)
    {
        size_t first = str.find_first_not_of(" \t\n\r");
        if (first == std::string::npos)
            return "";
        size_t last = str.find_last_not_of(" \t\n\r");
        return str.substr(first, (last - first + 1));
    }

    static uint32 ExtractNumber(const std::string& str, size_t start, size_t& end)
    {
        // Skip whitespace
        while (start < str.length() && (str[start] == ' ' || str[start] == '\t' || str[start] == '\n' || str[start] == '\r'))
            start++;
        
        end = start;
        while (end < str.length() && str[end] >= '0' && str[end] <= '9')
            end++;
        
        if (end > start)
        {
            std::string numStr = str.substr(start, end - start);
            return static_cast<uint32>(std::stoi(numStr));
        }
        return 0;
    }

    static bool ParseGearData(const std::string& jsonContent, std::map<uint8, SlotData>& slots)
    {
        LOG_INFO("module", "ConquestGearManager: ParseGearData START - content size: {} bytes", jsonContent.length());
        
        std::string content = jsonContent;

        LOG_INFO("module", "ConquestGearManager: Parsing gear data, size: {} bytes", content.length());

        // Find slots section
        LOG_INFO("module", "ConquestGearManager: Searching for 'slots' section");
        size_t slotsPos = content.find("\"slots\"");
        if (slotsPos == std::string::npos)
        {
            LOG_ERROR("module", "ConquestGearManager: 'slots' section not found in file");
            return false;
        }

        LOG_INFO("module", "ConquestGearManager: Found 'slots' section at position {}", slotsPos);

        // Find the opening brace after "slots"
        size_t slotsBraceStart = content.find("{", slotsPos);
        if (slotsBraceStart == std::string::npos)
        {
            LOG_ERROR("module", "ConquestGearManager: Opening brace not found after 'slots'");
            return false;
        }
        LOG_INFO("module", "ConquestGearManager: Found opening brace after 'slots' at position {}", slotsBraceStart);

        // Parse each slot - look for "slot_number": { ... }
        // Start from after the opening brace of "slots"
        size_t pos = slotsBraceStart + 1;
        
        // Find the closing brace for the "slots" object
        size_t braceCount = 1;
        size_t slotsEnd = slotsBraceStart + 1;
        while (braceCount > 0 && slotsEnd < content.length())
        {
            if (content[slotsEnd] == '{')
                braceCount++;
            else if (content[slotsEnd] == '}')
                braceCount--;
            slotsEnd++;
        }
        LOG_INFO("module", "ConquestGearManager: Found closing brace for 'slots' at position {}, braceCount={}", slotsEnd, braceCount);

        LOG_INFO("module", "ConquestGearManager: Starting slot parsing loop");
        uint32 slotLoopCount = 0;
        while (pos < slotsEnd)
        {
            slotLoopCount++;
            LOG_INFO("module", "ConquestGearManager: Slot loop iteration {}", slotLoopCount);
            
            // Find next slot number (format: "1": { or "10": {)
            LOG_INFO("module", "ConquestGearManager: Searching for slot quote at pos={}", pos);
            size_t slotQuoteStart = content.find("\"", pos);
            if (slotQuoteStart == std::string::npos || slotQuoteStart >= slotsEnd)
            {
                LOG_INFO("module", "ConquestGearManager: No more slots found (slotQuoteStart={}, slotsEnd={})", slotQuoteStart, slotsEnd);
                break;
            }
            LOG_INFO("module", "ConquestGearManager: Found slot quote at position {}", slotQuoteStart);

            size_t slotQuoteEnd = content.find("\"", slotQuoteStart + 1);
            if (slotQuoteEnd == std::string::npos)
            {
                LOG_ERROR("module", "ConquestGearManager: Slot quote end not found");
                break;
            }
            LOG_INFO("module", "ConquestGearManager: Found slot quote end at position {}", slotQuoteEnd);

            std::string slotStr = content.substr(slotQuoteStart + 1, slotQuoteEnd - slotQuoteStart - 1);
            LOG_INFO("module", "ConquestGearManager: Extracted slot string: '{}'", slotStr);
            
            uint8 slotNum = 0;
            try
            {
                slotNum = static_cast<uint8>(std::stoi(slotStr));
                LOG_INFO("module", "ConquestGearManager: Parsed slot number: {}", slotNum);
            }
            catch (const std::exception& e)
            {
                LOG_ERROR("module", "ConquestGearManager: Exception parsing slot number '{}': {}", slotStr, e.what());
                break;
            }
            catch (...)
            {
                LOG_ERROR("module", "ConquestGearManager: Unknown exception parsing slot number '{}'", slotStr);
                break;
            }

            // Find the opening brace for this slot
            LOG_INFO("module", "ConquestGearManager: Searching for opening brace after slot quote");
            size_t slotBraceStart = content.find("{", slotQuoteEnd);
            if (slotBraceStart == std::string::npos)
            {
                LOG_ERROR("module", "ConquestGearManager: Opening brace not found for slot {}", slotNum);
                break;
            }
            LOG_INFO("module", "ConquestGearManager: Found opening brace at position {}", slotBraceStart);

            // Find the closing brace for this slot
            LOG_INFO("module", "ConquestGearManager: Finding closing brace");
            size_t braceCount = 1;
            size_t slotBraceEnd = slotBraceStart + 1;
            while (braceCount > 0 && slotBraceEnd < content.length())
            {
                if (content[slotBraceEnd] == '{')
                    braceCount++;
                else if (content[slotBraceEnd] == '}')
                    braceCount--;
                slotBraceEnd++;
            }
            LOG_INFO("module", "ConquestGearManager: Found closing brace at position {}, braceCount={}", slotBraceEnd, braceCount);

            std::string slotContent = content.substr(slotBraceStart, slotBraceEnd - slotBraceStart);
            LOG_INFO("module", "ConquestGearManager: Extracted slot content, length={}", slotContent.length());

            SlotData slotData;

            // Find item
            size_t itemPos = slotContent.find("\"item\"");
            if (itemPos != std::string::npos)
            {
                size_t itemColon = slotContent.find(":", itemPos);
                if (itemColon != std::string::npos)
                {
                    size_t itemEnd;
                    slotData.item = ExtractNumber(slotContent, itemColon + 1, itemEnd);
                    LOG_INFO("module", "ConquestGearManager: Slot {} - Item: {}", slotNum, slotData.item);
                }
            }

            // Find enchant
            size_t enchantPos = slotContent.find("\"enchant\"");
            if (enchantPos != std::string::npos)
            {
                size_t enchantColon = slotContent.find(":", enchantPos);
                if (enchantColon != std::string::npos)
                {
                    size_t enchantEnd;
                    slotData.enchant = ExtractNumber(slotContent, enchantColon + 1, enchantEnd);
                    LOG_INFO("module", "ConquestGearManager: Slot {} - Enchant: {}", slotNum, slotData.enchant);
                }
            }

            // Find gems
            size_t gemsPos = slotContent.find("\"gems\"");
            if (gemsPos != std::string::npos)
            {
                size_t gemsBraceStart = slotContent.find("{", gemsPos);
                if (gemsBraceStart != std::string::npos)
                {
                    size_t gemsBraceEnd = slotContent.find("}", gemsBraceStart);
                    if (gemsBraceEnd != std::string::npos)
                    {
                        std::string gemsContent = slotContent.substr(gemsBraceStart, gemsBraceEnd - gemsBraceStart);
                        size_t gemPos = 0;
                        while ((gemPos = gemsContent.find("\"", gemPos)) != std::string::npos)
                        {
                            size_t gemQuoteEnd = gemsContent.find("\"", gemPos + 1);
                            if (gemQuoteEnd == std::string::npos)
                                break;

                            std::string gemSlotStr = gemsContent.substr(gemPos + 1, gemQuoteEnd - gemPos - 1);
                            uint8 gemSlot = static_cast<uint8>(std::stoi(gemSlotStr));

                            size_t gemColon = gemsContent.find(":", gemQuoteEnd);
                            if (gemColon != std::string::npos)
                            {
                                size_t gemEnd;
                                uint32 gemId = ExtractNumber(gemsContent, gemColon + 1, gemEnd);
                                slotData.gems[gemSlot] = gemId;
                                LOG_INFO("module", "ConquestGearManager: Slot {} - Gem slot {}: {}", slotNum, gemSlot, gemId);
                            }

                            gemPos = gemQuoteEnd + 1;
                        }
                    }
                }
            }

            LOG_INFO("module", "ConquestGearManager: Storing slot {} data", slotNum);
            slots[slotNum] = slotData;
            pos = slotBraceEnd;
            LOG_INFO("module", "ConquestGearManager: Slot {} processed, moving pos to {}", slotNum, pos);
        }

        LOG_INFO("module", "ConquestGearManager: Slot parsing loop ended after {} iterations", slotLoopCount);
        LOG_INFO("module", "ConquestGearManager: Parsed {} slots from file", slots.size());
        LOG_INFO("module", "ConquestGearManager: ParseGearFile END - success={}", !slots.empty());
        return !slots.empty();
    }
};

// Class to specialization mapping
struct ClassSpec
{
    std::string className;
    std::string specName;
    std::string fileName;
};

static const std::map<uint8, std::vector<ClassSpec>> CLASS_SPECS = {
    {1, {{"warrior", "arms", "arms"}, {"warrior", "fury", "fury"}, {"warrior", "protection", "protection"}}},
    {2, {{"paladin", "holy", "holy"}, {"paladin", "protection", "protection"}, {"paladin", "retribution", "retribution"}}},
    {3, {{"hunter", "beast_mastery", "beast_mastery"}, {"hunter", "marksmanship", "marksmanship"}, {"hunter", "survival", "survival"}}},
    {4, {{"rogue", "assassination", "assassination"}, {"rogue", "combat", "combat"}, {"rogue", "subtlety", "subtlety"}}},
    {5, {{"priest", "discipline", "discipline"}, {"priest", "holy", "holy"}, {"priest", "shadow", "shadow"}}},
    {6, {{"dk", "blood", "blood"}, {"dk", "frost", "frost"}, {"dk", "unholy", "unholy"}}},
    {7, {{"shaman", "elemental", "elemental"}, {"shaman", "enhancement", "enhancement"}, {"shaman", "restoration", "restoration"}}},
    {8, {{"mage", "arcane", "arcane"}, {"mage", "fire", "fire"}, {"mage", "frost", "frost"}}},
    {9, {{"warlock", "affliction", "affliction"}, {"warlock", "demonology", "demonology"}, {"warlock", "destruction", "destruction"}}},
    {11, {{"druid", "balance", "balance"}, {"druid", "feral", "feral"}, {"druid", "restoration", "restoration"}}}
};

class ConquestGearManagerNPC : public CreatureScript
{
public:
    ConquestGearManagerNPC() : CreatureScript("ConquestGearManagerNPC") { }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        if (!sConfigMgr->GetOption<bool>("ConquestGearManager.Enable", true))
            return false;

        // Get player class
        uint8 playerClass = player->getClass();
        auto it = CLASS_SPECS.find(playerClass);
        if (it == CLASS_SPECS.end())
        {
            ChatHandler(player->GetSession()).SendSysMessage("Votre classe n'est pas supportée.");
            return true;
        }

        // Clear previous menu
        ClearGossipMenuFor(player);

        // Add specialization options
        AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Choisissez votre spécialisation:", GOSSIP_SENDER_MAIN, 0, "", 0, false);
        AddGossipItemFor(player, GOSSIP_ICON_CHAT, "---", GOSSIP_SENDER_MAIN, 0, "", 0, false);

        uint32 actionId = 1;
        for (const auto& spec : it->second)
        {
            std::string specDisplayName = spec.specName;
            // Capitalize first letter
            if (!specDisplayName.empty())
                specDisplayName[0] = std::toupper(specDisplayName[0]);
            
            AddGossipItemFor(player, GOSSIP_ICON_TALK, specDisplayName, GOSSIP_SENDER_MAIN, actionId++, "", 0, false);
        }

        SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
    {
        LOG_INFO("module", "ConquestGearManager: OnGossipSelect called - action={}, player={}", action, player->GetName());
        
        if (action == 0)
        {
            LOG_INFO("module", "ConquestGearManager: Action 0, calling OnGossipHello");
            OnGossipHello(player, creature);
            return true;
        }

        LOG_INFO("module", "ConquestGearManager: Getting player class");
        // Get player class
        uint8 playerClass = player->getClass();
        LOG_INFO("module", "ConquestGearManager: Player class={}", playerClass);
        
        auto it = CLASS_SPECS.find(playerClass);
        if (it == CLASS_SPECS.end())
        {
            LOG_ERROR("module", "ConquestGearManager: Class {} not supported", playerClass);
            ChatHandler(player->GetSession()).SendSysMessage("Votre classe n'est pas supportée.");
            CloseGossipMenuFor(player);
            return true;
        }

        LOG_INFO("module", "ConquestGearManager: Class found, getting specialization");
        // Get selected specialization
        uint32 specIndex = action - 1;
        LOG_INFO("module", "ConquestGearManager: specIndex={}, available specs={}", specIndex, it->second.size());
        
        if (specIndex >= it->second.size())
        {
            LOG_ERROR("module", "ConquestGearManager: Invalid specIndex {} >= {}", specIndex, it->second.size());
            ChatHandler(player->GetSession()).SendSysMessage("Spécialisation invalide.");
            CloseGossipMenuFor(player);
            return true;
        }

        const ClassSpec& spec = it->second[specIndex];
        LOG_INFO("module", "ConquestGearManager: Selected spec - className={}, specName={}, fileName={}", spec.className, spec.specName, spec.fileName);

        // Load gear data from database
        LOG_INFO("module", "ConquestGearManager: Loading gear data from database for class {} spec {}", spec.className, spec.fileName);
        
        QueryResult result = WorldDatabase.Query("SELECT json_data FROM conquest_gear_data WHERE season = 's5' AND class_name = '{}' AND specialization = '{}'", 
            spec.className, spec.fileName);
        
        if (!result)
        {
            LOG_ERROR("module", "ConquestGearManager: No gear data found in database for class {} spec {}", spec.className, spec.fileName);
            ChatHandler(player->GetSession()).SendSysMessage("Impossible de charger l'équipement pour cette spécialisation depuis la base de données.");
            CloseGossipMenuFor(player);
            return true;
        }
        
        Field* fields = result->Fetch();
        std::string jsonData = fields[0].Get<std::string>();
        LOG_INFO("module", "ConquestGearManager: Loaded JSON data from database, size: {} bytes", jsonData.length());

        // Parse gear data
        std::map<uint8, SimpleJsonParser::SlotData> slots;
        LOG_INFO("module", "ConquestGearManager: Calling ParseGearData");
        if (!SimpleJsonParser::ParseGearData(jsonData, slots))
        {
            LOG_ERROR("module", "ConquestGearManager: Failed to parse gear data from database for class {} spec {}", spec.className, spec.fileName);
            ChatHandler(player->GetSession()).SendSysMessage("Erreur lors du parsing des données d'équipement.");
            CloseGossipMenuFor(player);
            return true;
        }

        LOG_INFO("module", "ConquestGearManager: Parsed {} slots from file", slots.size());

        // Save specialization to database
        CharacterDatabase.Execute("REPLACE INTO conquest_player_specialization (player_guid, class_name, specialization, last_updated) VALUES ({}, '{}', '{}', {})",
            player->GetGUID().GetCounter(), spec.className, spec.fileName, static_cast<uint32>(GameTime::GetGameTime().count()));
        LOG_INFO("module", "ConquestGearManager: Saved specialization {} for class {} to database for player {}", spec.fileName, spec.className, player->GetName());

        // Equip items
        LOG_INFO("module", "ConquestGearManager: Starting EquipGearSet");
        try
        {
            EquipGearSet(player, slots);
            LOG_INFO("module", "ConquestGearManager: EquipGearSet completed successfully");
            ChatHandler(player->GetSession()).SendSysMessage("Équipement appliqué avec succès!");
        }
        catch (const std::exception& e)
        {
            LOG_ERROR("module", "ConquestGearManager: Exception in EquipGearSet: {}", e.what());
            ChatHandler(player->GetSession()).SendSysMessage("Erreur lors de l'application de l'équipement.");
        }
        catch (...)
        {
            LOG_ERROR("module", "ConquestGearManager: Unknown exception in EquipGearSet");
            ChatHandler(player->GetSession()).SendSysMessage("Erreur inconnue lors de l'application de l'équipement.");
        }

        LOG_INFO("module", "ConquestGearManager: Closing gossip menu");
        CloseGossipMenuFor(player);
        LOG_INFO("module", "ConquestGearManager: OnGossipSelect returning true");
        return true;
    }

private:
    void UnequipAllItems(Player* player)
    {
        LOG_INFO("module", "ConquestGearManager: UnequipAllItems START - player={}", player->GetName());
        
        // Unequip all equipment slots
        for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
        {
            Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
            if (item)
            {
                LOG_INFO("module", "ConquestGearManager: Unequipping item {} from slot {}", item->GetEntry(), slot);
                uint16 pos = (INVENTORY_SLOT_BAG_0 << 8) | slot;
                InventoryResult result = player->CanUnequipItem(pos, false);
                if (result == EQUIP_ERR_OK)
                {
                    player->RemoveItem(INVENTORY_SLOT_BAG_0, slot, true);
                    LOG_INFO("module", "ConquestGearManager: Item {} removed from slot {}", item->GetEntry(), slot);
                }
                else
                {
                    LOG_ERROR("module", "ConquestGearManager: Cannot unequip item {} from slot {}: {}", item->GetEntry(), slot, result);
                }
            }
        }
        
        LOG_INFO("module", "ConquestGearManager: UnequipAllItems END");
    }

    void EquipGearSet(Player* player, const std::map<uint8, SimpleJsonParser::SlotData>& slots)
    {
        LOG_INFO("module", "ConquestGearManager: EquipGearSet START - player={}, slots count={}", player->GetName(), slots.size());
        
        // First, unequip all items to avoid stacking
        LOG_INFO("module", "ConquestGearManager: Unequipping all items first");
        UnequipAllItems(player);
        
        // Slot mapping: JSON slot numbers to EQUIPMENT_SLOT
        // 1=head, 2=neck, 3=shoulder, 5=chest, 6=waist, 7=legs, 8=feet, 9=wrist, 10=hands
        // 11=finger1, 12=finger2, 13=trinket1, 14=trinket2, 15=back, 16=mainhand, 17=offhand, 18=ranged
        static const std::map<uint8, uint8> SLOT_MAPPING = {
            {1, EQUIPMENT_SLOT_HEAD},
            {2, EQUIPMENT_SLOT_NECK},
            {3, EQUIPMENT_SLOT_SHOULDERS},
            {5, EQUIPMENT_SLOT_CHEST},
            {6, EQUIPMENT_SLOT_WAIST},
            {7, EQUIPMENT_SLOT_LEGS},
            {8, EQUIPMENT_SLOT_FEET},
            {9, EQUIPMENT_SLOT_WRISTS},
            {10, EQUIPMENT_SLOT_HANDS},
            {11, EQUIPMENT_SLOT_FINGER1},
            {12, EQUIPMENT_SLOT_FINGER2},
            {13, EQUIPMENT_SLOT_TRINKET1},
            {14, EQUIPMENT_SLOT_TRINKET2},
            {15, EQUIPMENT_SLOT_BACK},
            {16, EQUIPMENT_SLOT_MAINHAND},
            {17, EQUIPMENT_SLOT_OFFHAND},
            {18, EQUIPMENT_SLOT_RANGED}
        };

        LOG_INFO("module", "ConquestGearManager: Starting loop through {} slots", slots.size());
        uint32 slotIndex = 0;
        for (const auto& [jsonSlot, slotData] : slots)
        {
            slotIndex++;
            LOG_INFO("module", "ConquestGearManager: Processing slot {}/{} - jsonSlot={}, item={}", slotIndex, slots.size(), jsonSlot, slotData.item);
            
            auto slotIt = SLOT_MAPPING.find(jsonSlot);
            if (slotIt == SLOT_MAPPING.end())
            {
                LOG_INFO("module", "ConquestGearManager: jsonSlot {} not in SLOT_MAPPING, skipping", jsonSlot);
                continue;
            }

            uint8 equipmentSlot = slotIt->second;
            LOG_INFO("module", "ConquestGearManager: Mapped jsonSlot {} to equipmentSlot {}", jsonSlot, equipmentSlot);

            // Check if item exists
            LOG_INFO("module", "ConquestGearManager: Looking up item template for item {}", slotData.item);
            ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(slotData.item);
            if (!itemTemplate)
            {
                LOG_ERROR("module", "ConquestGearManager: Item {} not found", slotData.item);
                continue;
            }
            LOG_INFO("module", "ConquestGearManager: Item template found for item {} - name={}, class={}, subclass={}, inventoryType={}", 
                     slotData.item, itemTemplate->Name1, itemTemplate->Class, itemTemplate->SubClass, itemTemplate->InventoryType);

            // Check if slot is already occupied
            Item* existingItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, equipmentSlot);
            if (existingItem)
            {
                LOG_WARN("module", "ConquestGearManager: Slot {} already occupied by item {}, removing it first", equipmentSlot, existingItem->GetEntry());
                player->RemoveItem(INVENTORY_SLOT_BAG_0, equipmentSlot, true);
            }

            // Create item first, then check if it can be equipped
            LOG_INFO("module", "ConquestGearManager: Creating item {} for slot {}", slotData.item, equipmentSlot);
            Item* newItem = Item::CreateItem(slotData.item, 1, player);
            if (!newItem)
            {
                LOG_ERROR("module", "ConquestGearManager: Failed to create item {}", slotData.item);
                continue;
            }
            
            // Explicitly remove binding to ensure items are not soulbound
            newItem->SetBinding(false);
            
            LOG_INFO("module", "ConquestGearManager: Item {} created successfully", slotData.item);

            // Check if player can equip this item (using the created item)
            uint16 dest = 0;
            InventoryResult canEquipResult = player->CanEquipItem(equipmentSlot, dest, newItem, true, true);
            if (canEquipResult != EQUIP_ERR_OK)
            {
                LOG_WARN("module", "ConquestGearManager: CanEquipItem returned error {} for item {} in slot {}, trying anyway", 
                        canEquipResult, slotData.item, equipmentSlot);
                // Continue anyway - some checks might be too strict for gear manager
            }
            else
            {
                LOG_INFO("module", "ConquestGearManager: CanEquipItem check passed for item {} in slot {}", slotData.item, equipmentSlot);
            }

            // Use EquipItem instead of EquipNewItem since we already created the item
            uint16 pos = (INVENTORY_SLOT_BAG_0 << 8) | equipmentSlot;
            LOG_INFO("module", "ConquestGearManager: About to call EquipItem - item={}, slot={}, pos={}", slotData.item, equipmentSlot, pos);
            
            Item* item = nullptr;
            try
            {
                item = player->EquipItem(pos, newItem, true);
                LOG_INFO("module", "ConquestGearManager: EquipItem returned, item pointer={}", (void*)item);
                
                // Ensure binding is disabled after equip (in case it was re-applied)
                if (item)
                    item->SetBinding(false);
            }
            catch (const std::exception& e)
            {
                LOG_ERROR("module", "ConquestGearManager: Exception in EquipItem: {}", e.what());
                delete newItem;
                continue;
            }
            catch (...)
            {
                LOG_ERROR("module", "ConquestGearManager: Unknown exception in EquipItem");
                delete newItem;
                continue;
            }
            
            if (!item)
            {
                LOG_ERROR("module", "ConquestGearManager: Failed to equip item {} in slot {} - EquipItem returned nullptr", slotData.item, equipmentSlot);
                delete newItem;
                continue;
            }
            
            // Verify item is actually equipped
            Item* verifyItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, equipmentSlot);
            if (!verifyItem || verifyItem->GetEntry() != slotData.item)
            {
                LOG_ERROR("module", "ConquestGearManager: Item {} not properly equipped in slot {} (verifyItem={}, entry={})", 
                         slotData.item, equipmentSlot, (void*)verifyItem, verifyItem ? verifyItem->GetEntry() : 0);
                continue;
            }
            LOG_INFO("module", "ConquestGearManager: Item {} verified as equipped in slot {}", slotData.item, equipmentSlot);
            
            // Use the equipped item for enchantments
            item = verifyItem;

            LOG_INFO("module", "ConquestGearManager: Item {} equipped successfully, item GUID={}", slotData.item, item->GetGUID().ToString());

            // Apply enchantment
            if (slotData.enchant > 0)
            {
                LOG_INFO("module", "ConquestGearManager: Applying enchantment {} to item {}", slotData.enchant, slotData.item);
                try
                {
                    item->SetEnchantment(PERM_ENCHANTMENT_SLOT, slotData.enchant, 0, 0, player->GetGUID());
                    LOG_INFO("module", "ConquestGearManager: SetEnchantment completed");
                    
                    player->ApplyEnchantment(item, PERM_ENCHANTMENT_SLOT, true);
                    LOG_INFO("module", "ConquestGearManager: ApplyEnchantment completed");
                    
                    item->SetState(ITEM_CHANGED, player);
                    LOG_INFO("module", "ConquestGearManager: Item state set to ITEM_CHANGED");
                }
                catch (const std::exception& e)
                {
                    LOG_ERROR("module", "ConquestGearManager: Exception applying enchantment: {}", e.what());
                }
                catch (...)
                {
                    LOG_ERROR("module", "ConquestGearManager: Unknown exception applying enchantment");
                }
            }
            else
            {
                LOG_INFO("module", "ConquestGearManager: No enchantment for item {}", slotData.item);
            }

            // Apply gems
            LOG_INFO("module", "ConquestGearManager: Processing {} gems for item {}", slotData.gems.size(), slotData.item);
            uint32 gemIndex = 0;
            for (const auto& [gemSlot, gemItemId] : slotData.gems)
            {
                gemIndex++;
                LOG_INFO("module", "ConquestGearManager: Processing gem {}/{} - gemSlot={}, gemItemId={}", gemIndex, slotData.gems.size(), gemSlot, gemItemId);
                
                // Map gem slot: 0=SOCK_ENCHANTMENT_SLOT, 1=SOCK_ENCHANTMENT_SLOT_2, 2=SOCK_ENCHANTMENT_SLOT_3
                EnchantmentSlot enchantSlot = SOCK_ENCHANTMENT_SLOT;
                if (gemSlot == 1)
                    enchantSlot = SOCK_ENCHANTMENT_SLOT_2;
                else if (gemSlot == 2)
                    enchantSlot = SOCK_ENCHANTMENT_SLOT_3;

                LOG_INFO("module", "ConquestGearManager: Mapped gemSlot {} to enchantSlot {}", gemSlot, enchantSlot);

                // Convert gem item ID to enchantment ID
                LOG_INFO("module", "ConquestGearManager: Looking up gem template for gem item {}", gemItemId);
                ItemTemplate const* gemTemplate = sObjectMgr->GetItemTemplate(gemItemId);
                if (!gemTemplate || !gemTemplate->GemProperties)
                {
                    LOG_ERROR("module", "ConquestGearManager: Invalid gem item {} for slot {} (template={}, GemProperties={})", 
                             gemItemId, gemSlot, (void*)gemTemplate, gemTemplate ? gemTemplate->GemProperties : 0);
                    continue;
                }
                LOG_INFO("module", "ConquestGearManager: Gem template found, GemProperties={}", gemTemplate->GemProperties);

                LOG_INFO("module", "ConquestGearManager: Looking up gem properties entry {}", gemTemplate->GemProperties);
                GemPropertiesEntry const* gemProperty = sGemPropertiesStore.LookupEntry(gemTemplate->GemProperties);
                if (!gemProperty || !gemProperty->spellitemenchantement)
                {
                    LOG_ERROR("module", "ConquestGearManager: Invalid gem properties for gem item {} (property={}, enchant={})", 
                             gemItemId, (void*)gemProperty, gemProperty ? gemProperty->spellitemenchantement : 0);
                    continue;
                }

                uint32 enchantId = gemProperty->spellitemenchantement;
                LOG_INFO("module", "ConquestGearManager: Found enchant ID {} for gem item {}", enchantId, gemItemId);
                LOG_INFO("module", "ConquestGearManager: Applying gem enchant {} (from gem item {}) to item {} in gem slot {}", enchantId, gemItemId, slotData.item, gemSlot);
                
                try
                {
                    item->SetEnchantment(enchantSlot, enchantId, 0, 0, player->GetGUID());
                    LOG_INFO("module", "ConquestGearManager: SetEnchantment for gem completed");
                    
                    player->ApplyEnchantment(item, enchantSlot, true);
                    LOG_INFO("module", "ConquestGearManager: ApplyEnchantment for gem completed");
                    
                    item->SetState(ITEM_CHANGED, player);
                    LOG_INFO("module", "ConquestGearManager: Item state set to ITEM_CHANGED after gem");
                }
                catch (const std::exception& e)
                {
                    LOG_ERROR("module", "ConquestGearManager: Exception applying gem: {}", e.what());
                }
                catch (...)
                {
                    LOG_ERROR("module", "ConquestGearManager: Unknown exception applying gem");
                }
            }
            LOG_INFO("module", "ConquestGearManager: Finished processing item {} (slot {})", slotData.item, jsonSlot);
        }
        LOG_INFO("module", "ConquestGearManager: EquipGearSet END - processed {} slots", slotIndex);
    }
};

void AddConquestGearManagerScripts()
{
    new ConquestGearManagerNPC();
}

