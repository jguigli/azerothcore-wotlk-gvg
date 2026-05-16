/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

#include "ScriptMgr.h"
#include "Player.h"
#include "Config.h"
#include "Chat.h"
#include "Creature.h"
#include "CreatureAI.h"
#include "ScriptedCreature.h"
#include "ItemScript.h"
#include "Item.h"
#include "Map.h"
#include "GuildMgr.h"
#include "Guild.h"
#include "ObjectMgr.h"
#include "AllCreatureScript.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Cell.h"
#include "CellImpl.h"
#include "ReputationMgr.h"
#include "DBCStores.h"
#include "WorldPacket.h"
#include "Opcodes.h"
#include "CreatureData.h"
#include "SpellMgr.h"
#include "Random.h"
#include "UnitScript.h"

// Guard creature entries
#define GUARD_BRUTE_OGRE       400300
#define GUARD_OGRE_MAGE        400301
#define GUARD_ECRASEUR_OGRE    400302
#define GUARD_CHAMAN_OGRE      400303
#define GUARD_MASSACREUR_OGRE  400304
#define GUARD_DEMONISTE_OGRE   400305

// Spawn item entries
#define ITEM_BRUTE_OGRE        80030
#define ITEM_OGRE_MAGE         80031
#define ITEM_ECRASEUR_OGRE     80032
#define ITEM_CHAMAN_OGRE       80033
#define ITEM_MASSACREUR_OGRE   80034
#define ITEM_DEMONISTE_OGRE    80035

// Helper function to get creature entry from item entry
uint32 GetCreatureEntryFromItem(uint32 itemEntry)
{
    switch (itemEntry)
    {
        case ITEM_BRUTE_OGRE:        return GUARD_BRUTE_OGRE;
        case ITEM_OGRE_MAGE:         return GUARD_OGRE_MAGE;
        case ITEM_ECRASEUR_OGRE:     return GUARD_ECRASEUR_OGRE;
        case ITEM_CHAMAN_OGRE:        return GUARD_CHAMAN_OGRE;
        case ITEM_MASSACREUR_OGRE:    return GUARD_MASSACREUR_OGRE;
        case ITEM_DEMONISTE_OGRE:     return GUARD_DEMONISTE_OGRE;
        default:                      return 0;
    }
}

// Helper function to equip weapons based on item entry
void EquipWeaponsForCreature(Creature* creature, uint32 itemEntry)
{
    if (!creature)
        return;

    uint32 mainHand = 0;
    uint32 offHand = 0;
    uint32 ranged = 0;

    switch (itemEntry)
    {
        case ITEM_BRUTE_OGRE:        // Dual wield one-hand axes
            mainHand = 13952;
            offHand = 13952;
            break;
        case ITEM_OGRE_MAGE:         // Staff
            mainHand = 17191;
            break;
        case ITEM_ECRASEUR_OGRE:     // Two-hand mace
            mainHand = 11921;
            break;
        case ITEM_CHAMAN_OGRE:       // One-hand mace
            mainHand = 13028;
            break;
        case ITEM_MASSACREUR_OGRE:   // Two-hand axe
            mainHand = 12769;
            break;
        case ITEM_DEMONISTE_OGRE:    // Staff
            mainHand = 13937;
            break;
        default:
            return;
    }

    // Equip weapons (slot 0 = main hand, slot 1 = off hand, slot 2 = ranged)
    creature->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID + 0, mainHand);
    creature->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID + 1, offHand);
    creature->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID + 2, ranged);

    LOG_INFO("module", "ConquestGuard: Equipped weapons for creature {} - Main: {}, Off: {}, Ranged: {}", 
        creature->GetEntry(), mainHand, offHand, ranged);
}

// Helper function to get HP for creature entry
uint32 GetCreatureHP(uint32 creatureEntry)
{
    switch (creatureEntry)
    {
        case GUARD_BRUTE_OGRE:
        case GUARD_OGRE_MAGE:
            return 50000;
        case GUARD_ECRASEUR_OGRE:
        case GUARD_CHAMAN_OGRE:
            return 100000;
        case GUARD_MASSACREUR_OGRE:
        case GUARD_DEMONISTE_OGRE:
            return 200000;
        default:
            return 0;
    }
}

// Item script to handle spawning guards
class ConquestGuardItem : public ItemScript
{
public:
    ConquestGuardItem() : ItemScript("ConquestGuardItem") { }

    bool OnUse(Player* player, Item* item, SpellCastTargets const& /*targets*/) override
    {
        if (!player || !item)
            return false;

        // Check if player has a guild
        if (player->GetGuildId() == 0)
        {
            ChatHandler(player->GetSession()).PSendSysMessage("Vous devez être dans une guilde pour utiliser cet objet.");
            return false;
        }

        uint32 creatureEntry = GetCreatureEntryFromItem(item->GetEntry());
        if (!creatureEntry)
        {
            LOG_ERROR("module", "ConquestGuard: Invalid item entry {}", item->GetEntry());
            return false;
        }

        // Get player position
        float x = player->GetPositionX();
        float y = player->GetPositionY();
        float z = player->GetPositionZ();
        float o = player->GetOrientation();

        // Spawn the creature
        Map* map = player->GetMap();
        if (!map)
        {
            LOG_ERROR("module", "ConquestGuard: Failed to get map for player {}", player->GetName());
            return false;
        }

        Creature* creature = new Creature();
        if (!creature->Create(map->GenerateLowGuid<HighGuid::Unit>(), map, PHASEMASK_NORMAL, creatureEntry, 0, x, y, z, o))
        {
            LOG_ERROR("module", "ConquestGuard: Failed to create creature entry {}", creatureEntry);
            delete creature;
            return false;
        }

        // Store the spawner's GUID and guild ID in the creature's AI
        // We'll use a custom data storage for this
        creature->SetLevel(80);
        creature->SetHomePosition(x, y, z, o);

        // Set faction to player's faction (makes creature appear friendly)
        // Hostility to non-guild members will be handled via forced reputation
        creature->SetFaction(player->GetFaction());

        // Add to map
        if (!map->AddToMap(creature))
        {
            LOG_ERROR("module", "ConquestGuard: Failed to add creature to map");
            delete creature;
            return false;
        }

        // Set HP based on creature entry (after AddToMap to ensure creature is fully initialized)
        uint32 maxHP = GetCreatureHP(creatureEntry);
        if (maxHP > 0)
        {
            creature->SetCreateHealth(maxHP);
            creature->SetMaxHealth(maxHP);
            creature->SetHealth(maxHP);
            creature->UpdateMaxHealth();
        }

        // Equip weapons based on item entry
        EquipWeaponsForCreature(creature, item->GetEntry());

        // Store spawner info in AI (must be done after AddToMap)
        if (CreatureAI* ai = creature->AI())
        {
            // Store spawner GUID and guild ID in AI data
            ai->SetData(0, player->GetGUID().GetCounter()); // Store spawner GUID low
            ai->SetData(1, player->GetGuildId()); // Store guild ID
            // SetData will handle setting the custom subname with the correct guild name
        }

        LOG_INFO("module", "ConquestGuard: Player {} spawned creature {} at ({}, {}, {})", 
            player->GetName(), creatureEntry, x, y, z);

        // Consume the item (single use)
        player->DestroyItemCount(item->GetEntry(), 1, true);

        return true;
    }
};

// Creature script to handle faction and behavior
class ConquestGuard : public CreatureScript
{
public:
    ConquestGuard() : CreatureScript("ConquestGuard") { }

    struct ConquestGuardAI : public ScriptedAI
    {
        ConquestGuardAI(Creature* creature) : ScriptedAI(creature), m_spawnerGUID(0), m_spawnerGuildId(0), m_initialized(false), m_guildName(""), m_checkPlayersTimer(0), m_hpSetTimer(100), m_hpCheckTimer(5000), m_spellTimer(3000) { }

        void InitializeAI() override
        {
            // Initialize will be called after spawn
            UpdateFaction();
            
            // Set HP based on creature entry (ensure HP are correct after initialization)
            // Use a small delay to ensure SelectLevel has been called
            m_hpSetTimer = 100; // Set HP after 100ms
            
            // Immediately check nearby players and apply reputation
            if (m_spawnerGuildId > 0)
            {
                std::list<Player*> players;
                Acore::AnyPlayerInObjectRangeCheck checker(me, 50.0f);
                Acore::PlayerListSearcher<Acore::AnyPlayerInObjectRangeCheck> searcher(me, players, checker);
                Cell::VisitObjects(me, searcher, 50.0f);
                
                if (FactionTemplateEntry const* factionTemplate = me->GetFactionTemplateEntry())
                {
                    if (FactionEntry const* factionEntry = sFactionStore.LookupEntry(factionTemplate->faction))
                    {
                        for (Player* player : players)
                        {
                            if (player && player->IsAlive())
                            {
                                // Check if spawner is still in guild (if spawner left guild, they should be hostile)
                                bool spawnerInGuild = IsSpawnerInGuild();
                                
                                // Player is friendly only if:
                                // 1. They are in the spawner's guild AND the spawner is still in the guild
                                // 2. OR they are the spawner AND the spawner is still in the guild
                                bool isGuildMember = false;
                                if (spawnerInGuild)
                                {
                                    isGuildMember = IsInSpawnerGuild(player) || (IsSpawner(player) && spawnerInGuild);
                                }
                                
                                if (isGuildMember)
                                {
                                    player->GetReputationMgr().ApplyForceReaction(factionEntry->ID, REP_FRIENDLY, true);
                                    player->GetReputationMgr().SendForceReactions();
                                }
                                else
                                {
                                    player->GetReputationMgr().ApplyForceReaction(factionEntry->ID, REP_HOSTILE, true);
                                    player->GetReputationMgr().SendForceReactions();
                                }
                            }
                        }
                    }
                }
            }
        }
        
        void UpdateHP()
        {
            uint32 maxHP = GetCreatureHP(me->GetEntry());
            if (maxHP > 0)
            {
                uint32 currentMaxHP = me->GetMaxHealth();
                if (currentMaxHP != maxHP)
                {
                    me->SetCreateHealth(maxHP);
                    me->SetMaxHealth(maxHP);
                    me->SetHealth(maxHP);
                    me->UpdateMaxHealth();
                    me->ResetPlayerDamageReq();
                    
                    // Also update the modifier value to ensure it's correct
                    me->SetModifierValue(UNIT_MOD_HEALTH, BASE_VALUE, (float)maxHP);
                    
                    LOG_INFO("module", "ConquestGuard: UpdateHP - Set HP to {} for creature entry {} (was {})", 
                        maxHP, me->GetEntry(), currentMaxHP);
                }
            }
        }

        void Reset() override
        {
            if (!m_initialized)
            {
                // Try to get spawner info from stored data
                // This will be set by the item script
                m_initialized = true;
            }
            
            // Reset HP timer to set HP again after reset
            m_hpSetTimer = 100;
        }

        void UpdateFaction()
        {
            // Find the spawner player
            Player* spawner = nullptr;
            if (m_spawnerGUID > 0)
            {
                spawner = ObjectAccessor::FindPlayer(ObjectGuid::Create<HighGuid::Player>(m_spawnerGUID));
            }

            if (!spawner)
            {
                // Keep current faction (set by item script)
                return;
            }

            // Store spawner info
            m_spawnerGUID = spawner->GetGUID().GetCounter();
            m_spawnerGuildId = spawner->GetGuildId();

            // Faction is already set to spawner's faction (friendly appearance)
            // We'll force hostile reputation for non-guild members in UpdateAI
        }

        bool IsInSpawnerGuild(Player* player) const
        {
            if (!player || m_spawnerGuildId == 0)
                return false;

            return player->GetGuildId() == m_spawnerGuildId;
        }

        bool IsSpawner(Player* player) const
        {
            if (!player || m_spawnerGUID == 0)
                return false;

            return player->GetGUID().GetCounter() == m_spawnerGUID;
        }

        bool IsSpawnerInGuild() const
        {
            // Check if the spawner is still in the guild
            if (m_spawnerGUID == 0 || m_spawnerGuildId == 0)
                return false;

            Player* spawner = ObjectAccessor::FindPlayer(ObjectGuid::Create<HighGuid::Player>(m_spawnerGUID));
            if (!spawner)
                return false;

            return spawner->GetGuildId() == m_spawnerGuildId;
        }

        void SendSubNameUpdateToNearbyPlayers()
        {
            // Send creature query response packet to all nearby players to update subname
            CreatureTemplate const* ci = sObjectMgr->GetCreatureTemplate(me->GetEntry());
            if (!ci)
                return;

            std::string Name = ci->Name;
            std::string Title = me->HasCustomSubName() ? me->GetCustomSubName() : ci->SubName;

            // Find nearby players
            std::list<Player*> players;
            Acore::AnyPlayerInObjectRangeCheck checker(me, 100.0f);
            Acore::PlayerListSearcher<Acore::AnyPlayerInObjectRangeCheck> searcher(me, players, checker);
            Cell::VisitObjects(me, searcher, 100.0f);

            for (Player* player : players)
            {
                if (!player || !player->IsInWorld())
                    continue;

                // Build creature query response packet
                WorldPacket data(SMSG_CREATURE_QUERY_RESPONSE, 100);
                data << uint32(me->GetEntry());                    // creature entry
                data << Name;
                data << uint8(0) << uint8(0) << uint8(0);          // name2, name3, name4, always empty
                data << Title;                                      // subname (with custom guild name if set)
                data << ci->IconName;                              // "Directions" for guard, string for Icons 2.3.0
                data << uint32(ci->type_flags);                    // flags
                data << uint32(ci->type);                          // CreatureType.dbc
                data << uint32(ci->family);                       // CreatureFamily.dbc
                data << uint32(ci->rank);                          // Creature Rank (elite, boss, etc)
                data << uint32(ci->KillCredit[0]);                // new in 3.1, kill credit
                data << uint32(ci->KillCredit[1]);                 // new in 3.1, kill credit
                if (ci->GetModelByIdx(0))
                    data << uint32(ci->GetModelByIdx(0)->CreatureDisplayID); // Modelid1
                else
                    data << uint32(0);                             // Modelid1
                if (ci->GetModelByIdx(1))
                    data << uint32(ci->GetModelByIdx(1)->CreatureDisplayID); // Modelid2
                else
                    data << uint32(0);                             // Modelid2
                if (ci->GetModelByIdx(2))
                    data << uint32(ci->GetModelByIdx(2)->CreatureDisplayID); // Modelid3
                else
                    data << uint32(0);                             // Modelid3
                if (ci->GetModelByIdx(3))
                    data << uint32(ci->GetModelByIdx(3)->CreatureDisplayID); // Modelid4
                else
                    data << uint32(0);                             // Modelid4
                data << float(ci->ModHealth);                      // dmg/hp modifier
                data << float(ci->ModMana);                        // dmg/mana modifier
                data << uint8(ci->RacialLeader);

                CreatureQuestItemList const* items = sObjectMgr->GetCreatureQuestItemList(me->GetEntry());
                if (items)
                    for (std::size_t i = 0; i < MAX_CREATURE_QUEST_ITEMS; ++i)
                        data << (i < items->size() ? uint32((*items)[i]) : uint32(0));
                else
                    for (std::size_t i = 0; i < MAX_CREATURE_QUEST_ITEMS; ++i)
                        data << uint32(0);

                data << uint32(ci->movementId);                    // CreatureMovementInfo.dbc

                // Send packet to player
                player->SendDirectMessage(&data);
            }
        }

        void UpdateAI(uint32 diff) override
        {
            // Set HP after a short delay to ensure SelectLevel has been called
            if (m_hpSetTimer > 0)
            {
                if (m_hpSetTimer <= diff)
                {
                    m_hpSetTimer = 0;
                    UpdateHP();
                }
                else
                {
                    m_hpSetTimer -= diff;
                }
            }
            else
            {
                // Periodically verify HP are still correct (in case something recalculates them)
                if (m_hpCheckTimer <= diff)
                {
                    m_hpCheckTimer = 5000; // Check every 5 seconds
                    UpdateHP(); // This will only update if HP are wrong
                }
                else
                {
                    m_hpCheckTimer -= diff;
                }
            }
            
            // Periodically check nearby players and manage reputation
            if (m_checkPlayersTimer <= diff)
            {
                m_checkPlayersTimer = 2000; // Check every 2 seconds
                
                if (m_spawnerGuildId > 0)
                {
                    // Find nearby players
                    std::list<Player*> players;
                    Acore::AnyPlayerInObjectRangeCheck checker(me, 50.0f);
                    Acore::PlayerListSearcher<Acore::AnyPlayerInObjectRangeCheck> searcher(me, players, checker);
                    Cell::VisitObjects(me, searcher, 50.0f);
                    
                    if (FactionTemplateEntry const* factionTemplate = me->GetFactionTemplateEntry())
                    {
                        if (FactionEntry const* factionEntry = sFactionStore.LookupEntry(factionTemplate->faction))
                        {
                            for (Player* player : players)
                            {
                                if (player && player->IsAlive())
                                {
                                    // Check if spawner is still in guild (if spawner left guild, they should be hostile)
                                    bool spawnerInGuild = IsSpawnerInGuild();
                                    
                                    // Player is friendly only if:
                                    // 1. They are in the spawner's guild AND the spawner is still in the guild
                                    // 2. OR they are the spawner AND the spawner is still in the guild
                                    bool isGuildMember = false;
                                    if (spawnerInGuild)
                                    {
                                        isGuildMember = IsInSpawnerGuild(player) || (IsSpawner(player) && spawnerInGuild);
                                    }
                                    
                                    // Check if player currently has forced reaction
                                    ReputationRank const* forcedRank = player->GetReputationMgr().GetForcedRankIfAny(factionTemplate);
                                    
                                    if (isGuildMember)
                                    {
                                        // Force friendly reputation for guild members (creature appears friendly)
                                        // Only apply if not already forced to friendly
                                        if (!forcedRank || *forcedRank != REP_FRIENDLY)
                                        {
                                            player->GetReputationMgr().ApplyForceReaction(factionEntry->ID, REP_FRIENDLY, true);
                                            player->GetReputationMgr().SendForceReactions(); // Update client immediately
                                        }
                                    }
                                    else
                                    {
                                        // Player is not in guild (or spawner left guild) - remove forced friendly reaction if it exists
                                        // This allows the creature to attack them
                                        if (forcedRank && *forcedRank == REP_FRIENDLY)
                                        {
                                            // Remove forced friendly reaction
                                            player->GetReputationMgr().ApplyForceReaction(factionEntry->ID, REP_FRIENDLY, false);
                                            player->GetReputationMgr().SendForceReactions(); // Update client immediately
                                        }
                                        // Force hostile reputation for non-guild members (creature appears hostile to them)
                                        // Only apply if not already forced to hostile
                                        if (!forcedRank || *forcedRank != REP_HOSTILE)
                                        {
                                            player->GetReputationMgr().ApplyForceReaction(factionEntry->ID, REP_HOSTILE, true);
                                            player->GetReputationMgr().SendForceReactions(); // Update client immediately
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                m_checkPlayersTimer -= diff;
            }

            if (!UpdateVictim())
                return;

            // Try to cast spells from creature_template_spell
            if (m_spellTimer <= diff)
            {
                m_spellTimer = 2000; // Check every 2 seconds for spell casting
                
                // Only cast if not already casting and have a victim
                if (!me->HasUnitState(UNIT_STATE_CASTING) && me->GetVictim())
                {
                    Unit* victim = me->GetVictim();
                    if (!victim)
                        return;
                    
                    // Try to cast spells directly from m_spells array
                    // This bypasses SelectSpell's restrictive checks
                    // First, collect all available spells
                    std::vector<uint32> availableSpells;
                    for (uint8 i = 0; i < MAX_CREATURE_SPELLS; ++i)
                    {
                        uint32 spellId = me->m_spells[i];
                        if (!spellId)
                            continue;
                        
                        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
                        if (!spellInfo)
                            continue;
                        
                        // Check if spell is on cooldown
                        if (me->HasSpellCooldown(spellId))
                            continue;
                        
                        // Check if we have enough power (mana/energy/etc)
                        bool hasEnoughPower = true;
                        if (spellInfo->PowerType == POWER_MANA)
                        {
                            uint32 manaCost = spellInfo->ManaCost;
                            if (manaCost > 0 && me->GetPower(POWER_MANA) < manaCost)
                            {
                                // If creature has less than 10% mana, skip this spell
                                // Otherwise, try to cast anyway (might have mana regen)
                                if (me->GetMaxPower(POWER_MANA) > 0 && 
                                    (me->GetPower(POWER_MANA) * 100 / me->GetMaxPower(POWER_MANA)) < 10)
                                {
                                    hasEnoughPower = false;
                                }
                            }
                        }
                        
                        if (!hasEnoughPower)
                            continue;
                        
                        // Check range
                        float dist = me->GetDistance(victim);
                        float maxRange = me->GetSpellMaxRangeForTarget(victim, spellInfo);
                        float minRange = me->GetSpellMinRangeForTarget(victim, spellInfo);
                        
                        bool inRange = false;
                        // For melee spells, check if in melee range
                        if (maxRange <= NOMINAL_MELEE_RANGE)
                        {
                            if (dist <= maxRange && dist >= minRange)
                                inRange = true;
                        }
                        // For ranged spells, check if in range and LOS
                        else if (dist <= maxRange && dist >= minRange)
                        {
                            // Check LOS for ranged spells (but be lenient for close range)
                            if (me->IsWithinLOSInMap(victim) || dist < 15.0f)
                                inRange = true;
                        }
                        
                        if (inRange)
                            availableSpells.push_back(spellId);
                    }
                    
                    // If we have available spells, cast one randomly
                    if (!availableSpells.empty())
                    {
                        uint32 spellToCast = availableSpells[urand(0, availableSpells.size() - 1)];
                        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellToCast);
                        if (spellInfo)
                        {
                            DoCastVictim(spellToCast);
                            m_spellTimer = spellInfo->RecoveryTime ? spellInfo->RecoveryTime : 3000;
                        }
                    }
                }
            }
            else
            {
                m_spellTimer -= diff;
            }

            DoMeleeAttackIfReady();
        }

        void JustDied(Unit* /*killer*/) override
        {
            // Delete guard creature from database when it dies
            uint32 entry = me->GetEntry();
            if (entry >= GUARD_BRUTE_OGRE && entry <= GUARD_DEMONISTE_OGRE)
            {
                LOG_INFO("module", "ConquestGuard: Guard creature {} died, deleting from database", entry);
                me->DeleteFromDB();
            }
        }

        void SetData(uint32 type, uint32 data) override
        {
            if (type == 0) // Spawner GUID low
            {
                m_spawnerGUID = data;
            }
            else if (type == 1) // Guild ID
            {
                m_spawnerGuildId = data;
                
                // Get guild name when guild ID is set and update custom subname
                if (m_spawnerGuildId > 0)
                {
                    if (Guild* guild = sGuildMgr->GetGuildById(m_spawnerGuildId))
                    {
                        m_guildName = guild->GetName();
                        // Set custom subname (this is per-creature, not per-template)
                        if (!m_guildName.empty())
                        {
                            me->SetCustomSubName(m_guildName);
                            LOG_INFO("module", "ConquestGuard: Updated subname to '{}' for creature {} (guild ID: {})", 
                                m_guildName, me->GetEntry(), m_spawnerGuildId);
                            // Force update subname for nearby players
                            SendSubNameUpdateToNearbyPlayers();
                        }
                        else
                        {
                            // Clear subname if guild name is empty
                            me->SetCustomSubName("");
                            SendSubNameUpdateToNearbyPlayers();
                        }
                    }
                    else
                    {
                        // Guild not found, clear subname
                        m_guildName = "";
                        me->SetCustomSubName("");
                        LOG_WARN("module", "ConquestGuard: Guild ID {} not found for creature {}", m_spawnerGuildId, me->GetEntry());
                        SendSubNameUpdateToNearbyPlayers();
                    }
                }
                else
                {
                    // No guild ID, clear subname
                    m_guildName = "";
                    me->SetCustomSubName("");
                    SendSubNameUpdateToNearbyPlayers();
                }
            }
        }

        std::string GetGuildName() const { return m_guildName; }

        bool CanAIAttack(Unit const* target) const override
        {
            if (!target)
                return false;

            // Don't attack the spawner
            if (target->IsPlayer())
            {
                Player* player = const_cast<Player*>(target->ToPlayer());
                
                // Check if spawner is still in guild
                bool spawnerInGuild = IsSpawnerInGuild();
                
                // Player is friendly only if:
                // 1. They are in the spawner's guild AND the spawner is still in the guild
                // 2. OR they are the spawner AND the spawner is still in the guild
                bool isFriendly = false;
                if (spawnerInGuild)
                {
                    isFriendly = IsInSpawnerGuild(player) || (IsSpawner(player) && spawnerInGuild);
                }
                
                if (isFriendly)
                {
                    return false; // Don't attack spawner or guild members (if spawner is still in guild)
                }
            }

            return ScriptedAI::CanAIAttack(target);
        }

        private:
            uint64 m_spawnerGUID;
            uint32 m_spawnerGuildId;
            bool m_initialized;
            std::string m_guildName;
            uint32 m_checkPlayersTimer;
            uint32 m_hpSetTimer;
            uint32 m_hpCheckTimer;
            uint32 m_spellTimer; // Timer for spell casting
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new ConquestGuardAI(creature);
    }
};

// AllCreatureScript to handle HP setting after SelectLevel
class ConquestGuardAllCreature : public AllCreatureScript
{
public:
    ConquestGuardAllCreature() : AllCreatureScript("ConquestGuardAllCreature") { }

    void OnCreatureSelectLevel(const CreatureTemplate* /*cinfo*/, Creature* creature) override
    {
        if (!creature)
            return;

        // Check if this is one of our guard creatures
        uint32 entry = creature->GetEntry();
        if (entry >= GUARD_BRUTE_OGRE && entry <= GUARD_DEMONISTE_OGRE)
        {
            uint32 maxHP = GetCreatureHP(entry);
            if (maxHP > 0)
            {
                // Set HP immediately after SelectLevel calculates them
                creature->SetCreateHealth(maxHP);
                creature->SetMaxHealth(maxHP);
                creature->SetHealth(maxHP);
                creature->UpdateMaxHealth();
                creature->ResetPlayerDamageReq();
                
                // Also update the modifier value to ensure it's correct
                creature->SetModifierValue(UNIT_MOD_HEALTH, BASE_VALUE, (float)maxHP);
                
                LOG_INFO("module", "ConquestGuard: OnCreatureSelectLevel - Set HP to {} for creature entry {} (was {})", 
                    maxHP, entry, creature->GetMaxHealth());
            }
        }
    }
};

// AllUnitScript to increase spell damage for caster creatures (mage, shaman, warlock)
class ConquestGuardAllUnit : public UnitScript
{
public:
    ConquestGuardAllUnit() : UnitScript("ConquestGuardAllUnit", true, {UNITHOOK_MODIFY_SPELL_DAMAGE_TAKEN}) { }

    void ModifySpellDamageTaken(Unit* target, Unit* attacker, int32& damage, SpellInfo const* spellInfo) override
    {
        if (!attacker || !spellInfo || damage <= 0)
            return;

        // Check if attacker is one of our caster guard creatures
        if (attacker->IsCreature())
        {
            uint32 entry = attacker->GetEntry();
            
            // Increase spell damage by 50% for caster creatures
            if (entry == GUARD_OGRE_MAGE || entry == GUARD_CHAMAN_OGRE || entry == GUARD_DEMONISTE_OGRE)
            {
                // Multiply damage by 1.5 (50% increase)
                int32 originalDamage = damage;
                damage = int32(damage * 1.5f);
                LOG_DEBUG("module", "ConquestGuard: Increased spell damage from {} to {} for creature entry {} (spell: {})", 
                    originalDamage, damage, entry, spellInfo->Id);
            }
        }
    }
};

// Add all scripts
void AddConquestGuardScripts()
{
    new ConquestGuardItem();
    new ConquestGuard();
    new ConquestGuardAllCreature();
    new ConquestGuardAllUnit();
}

