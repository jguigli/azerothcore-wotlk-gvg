/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

#include "ConquestBuildCommon.h"
#include "GameObjectAI.h"
#include "Log.h"
#include "Creature.h"
#include "CreatureAI.h"
#include "ScriptedCreature.h"
#include "Guild.h"
#include "GuildMgr.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "CellImpl.h"
#include "ReputationMgr.h"
#include "DBCStores.h"
#include "AllCreatureScript.h"
#include <cfloat>

// GameObject script to handle destruction (for walls and towers)
class go_conquest_build_structure : public GameObjectScript
{
public:
    go_conquest_build_structure() : GameObjectScript("go_conquest_build_structure") { }

    bool OnGossipHello(Player* /*player*/, GameObject* /*go*/) override
    {
        return true;
    }

    void OnDestroyed(GameObject* go, Player* /*player*/) override
    {
        // Remove fires before removing structure
        ConquestBuildMgr::instance()->RemoveFiresForStructure(go->GetSpawnId(), go->GetMap());
        
        // Remove from database when destroyed (including entire group if part of one)
        ConquestBuildMgr::instance()->RemoveStructure(go->GetSpawnId(), go->GetMap());
        
        // Delete from world (already handled by RemoveStructure, but be safe)
        go->SetRespawnTime(0);
        go->Delete();
    }

    void OnDamaged(GameObject* go, Player* /*player*/) override
    {
        // Check if structure health is below 50% and spawn fires if needed
        ConquestBuildMgr::instance()->CheckAndSpawnFires(go);
    }

    void OnModifyHealth(GameObject* go, Unit* /*attackerOrHealer*/, int32& change, SpellInfo const* /*spellInfo*/) override
    {
        LOG_ERROR("server", "[ConquestBuild] OnModifyHealth called for entry: {}, change: {}", go ? go->GetEntry() : 0, change);
        // Check health after modification (called every time health changes, BEFORE health is updated)
        // This is needed because OnDamaged is only called when state changes to DAMAGED,
        // but if Data5=0, the object goes directly from INTACT to DESTROYED
        // Pass the health change so we can calculate the new health
        ConquestBuildMgr::instance()->CheckAndSpawnFires(go, change);
    }
};

// GameObject script for gate levers (controls opening/closing the gate)
class go_conquest_build_gate_lever : public GameObjectScript
{
public:
    go_conquest_build_gate_lever() : GameObjectScript("go_conquest_build_gate_lever") { }

    bool OnGossipHello(Player* player, GameObject* lever) override
    {
        ConquestBuildConfig config = ConquestBuildMgr::instance()->GetConfig();

        if (!config.enabled)
        {
            ChatHandler(player->GetSession()).PSendSysMessage(CONQUEST_BUILD_DISABLED);
            return false;
        }

        // Find the gate associated with this lever (same group_id)
        ObjectGuid::LowType leverSpawnId = lever->GetSpawnId();
        QueryResult result = WorldDatabase.Query(
            "SELECT group_id, guild_id FROM conquest_build_structures WHERE guid = {}", 
            leverSpawnId
        );

        if (!result)
        {
            ChatHandler(player->GetSession()).SendSysMessage("Erreur: Herse associée non trouvée.");
            return true;
        }

        Field* fields = result->Fetch();
        uint64 groupId = fields[0].Get<uint64>();
        uint32 gateGuildId = fields[1].Get<uint32>();

        // Check if player is in the same guild
        uint32 playerGuildId = player->GetGuildId();
        
        if (gateGuildId != 0 && playerGuildId != gateGuildId)
        {
            ChatHandler(player->GetSession()).SendSysMessage("Cette herse appartient à une autre guilde!");
            return true; // Prevent interaction
        }

        // Find the gate GameObject in this group (check big gate, small gate, and grand fort gate entries)
        // Find the closest gate to the lever to ensure we get the right gate for ramparts
        float leverX = lever->GetPositionX();
        float leverY = lever->GetPositionY();
        
        QueryResult gateResult = WorldDatabase.Query(
            "SELECT guid, position_x, position_y FROM conquest_build_structures WHERE group_id = {} AND entry IN ({}, {}, {})", 
            groupId, config.gateGobId, 400026, 400025
        );

        if (!gateResult)
        {
            ChatHandler(player->GetSession()).SendSysMessage("Erreur: Herse non trouvée dans le groupe.");
            return true;
        }

        // Find the closest gate to the lever
        uint64 gateGuid = 0;
        float minDistance = FLT_MAX;
        
        do
        {
            Field* gateFields = gateResult->Fetch();
            uint64 currentGateGuid = gateFields[0].Get<uint64>();
            float gateX = gateFields[1].Get<float>();
            float gateY = gateFields[2].Get<float>();
            
            float distance = sqrt(pow(leverX - gateX, 2) + pow(leverY - gateY, 2));
            if (distance < minDistance)
            {
                minDistance = distance;
                gateGuid = currentGateGuid;
            }
        } while (gateResult->NextRow());
        
        if (gateGuid == 0)
        {
            ChatHandler(player->GetSession()).SendSysMessage("Erreur: Herse non trouvée dans le groupe.");
            return true;
        }

        // Find the gate GameObject in the world
        GameObject* gate = nullptr;
        auto bounds = player->GetMap()->GetGameObjectBySpawnIdStore().equal_range(gateGuid);
        for (auto itr = bounds.first; itr != bounds.second; ++itr)
        {
            gate = itr->second;
            break;
        }

        if (!gate)
        {
            ChatHandler(player->GetSession()).SendSysMessage("Erreur: Herse non trouvée dans le monde.");
            return true;
        }

        // Toggle gate state (open/close)
        if (gate->GetGoState() == GO_STATE_READY)
        {
            gate->SetGoState(GO_STATE_ACTIVE);
            ChatHandler(player->GetSession()).SendSysMessage("Herse ouverte.");
        }
        else
        {
            gate->SetGoState(GO_STATE_READY);
            ChatHandler(player->GetSession()).SendSysMessage("Herse fermée.");
        }

        return true; // Prevent default interaction
    }
};

// GameObject script for gates (with guild access control for opening/closing)
class go_conquest_build_gate : public GameObjectScript
{
public:
    go_conquest_build_gate() : GameObjectScript("go_conquest_build_gate") { }

    bool OnGossipHello(Player* player, GameObject* go) override
    {
        // Get the guild ID of the player who placed this gate
        ObjectGuid::LowType spawnId = go->GetSpawnId();
        QueryResult result = WorldDatabase.Query(
            "SELECT guild_id FROM conquest_build_structures WHERE guid = {}", 
            spawnId
        );

        if (!result)
        {
            // No data found, allow access
            return false;
        }

        Field* fields = result->Fetch();
        uint32 gateGuildId = fields[0].Get<uint32>();

        // Check if player is in the same guild
        uint32 playerGuildId = player->GetGuildId();
        
        if (gateGuildId != 0 && playerGuildId != gateGuildId)
        {
            ChatHandler(player->GetSession()).SendSysMessage("Cette herse appartient à une autre guilde!");
            return true; // Prevent interaction
        }

        // Allow the player to use the gate (open/close)
        return false; // Allow normal interaction
    }

    void OnDestroyed(GameObject* go, Player* /*player*/) override
    {
        // Remove fires before removing structure
        ConquestBuildMgr::instance()->RemoveFiresForStructure(go->GetSpawnId(), go->GetMap());
        
        // Remove from database when destroyed (including entire group if part of one)
        ConquestBuildMgr::instance()->RemoveStructure(go->GetSpawnId(), go->GetMap());
        
        // Delete from world (already handled by RemoveStructure, but be safe)
        go->SetRespawnTime(0);
        go->Delete();
    }

    void OnDamaged(GameObject* go, Player* /*player*/) override
    {
        // Check if structure health is below 50% and spawn fires if needed
        ConquestBuildMgr::instance()->CheckAndSpawnFires(go);
    }

    void OnModifyHealth(GameObject* go, Unit* /*attackerOrHealer*/, int32& change, SpellInfo const* /*spellInfo*/) override
    {
        LOG_ERROR("server", "[ConquestBuild] OnModifyHealth called for entry: {}, change: {}", go ? go->GetEntry() : 0, change);
        // Check health after modification (called every time health changes, BEFORE health is updated)
        // This is needed because OnDamaged is only called when state changes to DAMAGED,
        // but if Data5=0, the object goes directly from INTACT to DESTROYED
        // Pass the health change so we can calculate the new health
        ConquestBuildMgr::instance()->CheckAndSpawnFires(go, change);
    }
};

// World script to initialize the system
class conquest_build_world : public WorldScript
{
public:
    conquest_build_world() : WorldScript("conquest_build_world") { }

    void OnAfterConfigLoad(bool /*reload*/) override
    {
        ConquestBuildMgr::instance()->LoadConfig();
    }

    void OnStartup() override
    {
        // Note: Cleanup of orphaned structures is handled by the OnDestroyed hook
        // We cannot directly JOIN between characters and world databases
    }
};

// Creature script for cannons (auto-attack enemies not in owner's guild)
// Note: The AI is actually assigned via AllCreatureScript to avoid conflicts with the original cannon script
class ConquestBuildCannon
{
public:
    struct ConquestBuildCannonAI : public ScriptedAI
    {
        ConquestBuildCannonAI(Creature* creature) : ScriptedAI(creature), m_ownerGuildId(0), m_initialized(false), m_checkEnemiesTimer(2000) { }

        void InitializeAI() override
        {
            // Load guild ID from database
            LoadGuildId();
            
            // Immediately check nearby enemies
            if (m_ownerGuildId > 0)
            {
                CheckAndAttackEnemies();
            }
        }

        void LoadGuildId()
        {
            // Get guild ID from conquest_build_structures table
            ObjectGuid::LowType spawnId = me->GetSpawnId();
            QueryResult result = WorldDatabase.Query(
                "SELECT guild_id FROM conquest_build_structures WHERE guid = {} AND entry = 34944",
                spawnId
            );

            if (result)
            {
                Field* fields = result->Fetch();
                m_ownerGuildId = fields[0].Get<uint32>();
                m_initialized = true;
                
                LOG_INFO("module", "ConquestBuildCannon: Loaded guild ID {} for cannon spawn ID {}", m_ownerGuildId, spawnId);
            }
            else
            {
                LOG_WARN("module", "ConquestBuildCannon: Could not find guild ID for cannon spawn ID {}", spawnId);
            }
        }

        bool IsInOwnerGuild(Player* player) const
        {
            if (!player || m_ownerGuildId == 0)
                return false;

            return player->GetGuildId() == m_ownerGuildId;
        }

        void CheckAndAttackEnemies()
        {
            if (m_ownerGuildId == 0)
                return;

            // Find nearby enemy players
            std::list<Player*> players;
            Acore::AnyPlayerInObjectRangeCheck checker(me, 50.0f);
            Acore::PlayerListSearcher<Acore::AnyPlayerInObjectRangeCheck> searcher(me, players, checker);
            Cell::VisitObjects(me, searcher, 50.0f);

            Player* targetEnemy = nullptr;
            float closestDistance = 50.0f;

            for (Player* player : players)
            {
                if (!player || !player->IsAlive() || !player->IsInWorld())
                    continue;

                // Check if player is not in owner's guild
                if (!IsInOwnerGuild(player))
                {
                    float distance = me->GetDistance(player);
                    if (distance < closestDistance)
                    {
                        closestDistance = distance;
                        targetEnemy = player;
                    }
                }
            }

            // Attack the closest enemy if found
            if (targetEnemy && !me->IsInCombat())
            {
                me->Attack(targetEnemy, true);
                me->GetMotionMaster()->MoveChase(targetEnemy);
                LOG_DEBUG("module", "ConquestBuildCannon: Cannon {} attacking enemy player {}", me->GetSpawnId(), targetEnemy->GetName());
            }
        }

        void UpdateAI(uint32 diff) override
        {
            if (!m_initialized)
            {
                LoadGuildId();
                m_initialized = true;
            }

            // Periodically check for enemies
            if (m_checkEnemiesTimer <= diff)
            {
                m_checkEnemiesTimer = 2000; // Check every 2 seconds

                if (m_ownerGuildId > 0)
                {
                    // If not in combat, check for enemies
                    if (!me->IsInCombat())
                    {
                        CheckAndAttackEnemies();
                    }
                }
            }
            else
            {
                m_checkEnemiesTimer -= diff;
            }

            // Update combat AI
            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }

        bool CanAIAttack(Unit const* target) const override
        {
            if (!target)
                return false;

            // Only attack players not in owner's guild
            if (target->IsPlayer())
            {
                Player* player = const_cast<Player*>(target->ToPlayer());
                if (IsInOwnerGuild(player))
                {
                    return false; // Don't attack guild members
                }
            }

            return ScriptedAI::CanAIAttack(target);
        }

    private:
        uint32 m_ownerGuildId;
        bool m_initialized;
        uint32 m_checkEnemiesTimer;
    };
};

// AllCreatureScript to assign AI to Conquest cannons
class ConquestBuildCannonAllCreature : public AllCreatureScript
{
public:
    ConquestBuildCannonAllCreature() : AllCreatureScript("ConquestBuildCannonAllCreature") { }

    CreatureAI* GetCreatureAI(Creature* creature) const override
    {
        // Check if this is a cannon (entry 34944) and if it's a Conquest cannon (has guild_id in database)
        if (creature && creature->GetEntry() == 34944)
        {
            // Check if this cannon is tracked in conquest_build_structures (meaning it's a Conquest cannon)
            ObjectGuid::LowType spawnId = creature->GetSpawnId();
            QueryResult result = WorldDatabase.Query(
                "SELECT guild_id FROM conquest_build_structures WHERE guid = {} AND entry = 34944",
                spawnId
            );

            if (result)
            {
                // This is a Conquest cannon, return our custom AI
                return new ConquestBuildCannon::ConquestBuildCannonAI(creature);
            }
        }
        
        // Return nullptr to let the default script handle it
        return nullptr;
    }
};

// Add game object scripts
void AddConquestBuildGameObjectScripts()
{
    new go_conquest_build_structure();
    new go_conquest_build_gate_lever();
    new go_conquest_build_gate();
    new conquest_build_world();
    new ConquestBuildCannonAllCreature(); // Add AllCreatureScript for cannons (assigns AI to Conquest cannons)
}

