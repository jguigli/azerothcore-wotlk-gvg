/*
 * Script pour créer un marqueur que les joueurs peuvent utiliser
 * pour faire spawner des créatures qui se déplacent vers le marqueur
 */

#include "ConquestMarkerSpawner.h"
#include "ConquestBuildCommon.h"
#include "GameObject.h"
#include "GameObjectAI.h"
#include "Player.h"
#include "Creature.h"
#include "CreatureAI.h"
#include "ScriptedCreature.h"
#include "Map.h"
#include "MotionMaster.h"
#include "ScriptedGossip.h"
#include "Log.h"
#include "ScriptMgr.h"
#include "ItemScript.h"
#include "Item.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Cell.h"
#include "CellImpl.h"
#include "Random.h"
#include "AllCreatureScript.h"
#include <unordered_map>

// Entries
#define ITEM_MARKER_KIT           80040
#define NPC_MARKER                400103
#define NPC_SPAWNER               400102
#define NPC_SPAWN_MIN             400300
#define NPC_SPAWN_MAX             400305
#define SPAWN_INTERVAL            10000  // 10 secondes en millisecondes
#define SPAWN_COUNT_PER_WAVE      5
#define SPAWN_DELAY_BETWEEN       1000   // 1 seconde entre chaque spawn
#define MARKER_SEARCH_RANGE       500.0f  // Rayon de recherche du marqueur (augmenté pour voir plus loin)
#define DISPERSION_RANGE          10.0f   // Rayon de dispersion autour du marqueur

// Map globale pour stocker les créatures qui doivent se disperser (GUID du marqueur)
static std::unordered_map<ObjectGuid, ObjectGuid> s_creaturesToDisperse;

// Configuration
#define MARKER_GOB_ENTRY 400100        // Entry du GameObject marqueur (à définir dans la DB)
#define CREATURE_ENTRY_TO_SPAWN 25219  // Entry de la créature à spawner (exemple: Borean Queue Trigger)
#define SPAWN_COUNT 5                  // Nombre de créatures à spawner
#define SPAWN_DISTANCE 5.0f            // Distance entre chaque créature spawnée
#define SPAWN_OFFSET_X 2.0f            // Offset X pour le spawn initial
#define SPAWN_OFFSET_Y 2.0f            // Offset Y pour le spawn initial

// Structure pour stocker les informations du marqueur
struct MarkerData
{
    Position spawnPoint;      // Point de spawn des créatures
    Position markerPoint;     // Position du marqueur (destination)
    ObjectGuid markerGUID;    // GUID du marqueur
    uint32 creatureCount;     // Nombre de créatures spawnées
};

// AI du GameObject marqueur
class go_conquest_marker_ai : public GameObjectAI
{
public:
    go_conquest_marker_ai(GameObject* go) : GameObjectAI(go)
    {
        _markerData.markerGUID = go->GetGUID();
        _markerData.markerPoint = go->GetPosition();
    }

    bool GossipHello(Player* player, bool /*code*/) override
    {
        ClearGossipMenuFor(player);
        
        // Menu de gossip pour le marqueur
        AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Définir le point de spawn", GOSSIP_SENDER_MAIN, 1);
        AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Spawner des créatures vers ce marqueur", GOSSIP_SENDER_MAIN, 2);
        AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Spawner avec pathfinding (recommandé)", GOSSIP_SENDER_MAIN, 3);
        AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Spawner en ligne droite", GOSSIP_SENDER_MAIN, 4);
        
        SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, me->GetGUID());
        return true;
    }

    bool GossipSelect(Player* player, uint32 /*sender*/, uint32 action) override
    {
        ClearGossipMenuFor(player);

        switch (action)
        {
            case 1: // Définir le point de spawn
            {
                _markerData.spawnPoint = player->GetPosition();
                ChatHandler(player->GetSession()).PSendSysMessage("Point de spawn défini à votre position actuelle.");
                CloseGossipMenuFor(player);
                return true;
            }
            case 2: // Spawner avec pathfinding (par défaut)
            case 3: // Spawner avec pathfinding
            {
                SpawnCreaturesToMarker(player, true);
                CloseGossipMenuFor(player);
                return true;
            }
            case 4: // Spawner en ligne droite
            {
                SpawnCreaturesToMarker(player, false);
                CloseGossipMenuFor(player);
                return true;
            }
        }

        CloseGossipMenuFor(player);
        return true;
    }

private:
    void SpawnCreaturesToMarker(Player* player, bool usePathfinding)
    {
        if (!player || !me)
            return;

        Map* map = me->GetMap();
        if (!map)
            return;

        // Si le point de spawn n'est pas défini, utiliser la position du joueur
        if (_markerData.spawnPoint.GetPositionX() == 0.0f && 
            _markerData.spawnPoint.GetPositionY() == 0.0f)
        {
            _markerData.spawnPoint = player->GetPosition();
        }

        Position markerPos = me->GetPosition();
        Position spawnPos = _markerData.spawnPoint;

        uint32 spawnedCount = 0;

        // Spawner les créatures en ligne avec un offset
        for (uint32 i = 0; i < SPAWN_COUNT; ++i)
        {
            // Calculer la position de spawn avec un offset
            float offsetX = spawnPos.GetPositionX() + (i * SPAWN_OFFSET_X);
            float offsetY = spawnPos.GetPositionY() + (i * SPAWN_OFFSET_Y);
            float offsetZ = spawnPos.GetPositionZ();

            // Vérifier que la position est valide
            if (!Acore::IsValidMapCoord(offsetX, offsetY, offsetZ))
            {
                LOG_ERROR("gvg.marker", "Position de spawn invalide: X={}, Y={}, Z={}", offsetX, offsetY, offsetZ);
                continue;
            }

            // Créer la créature
            Creature* creature = new Creature();
            if (!creature->Create(map->GenerateLowGuid<HighGuid::Unit>(), map, PHASEMASK_NORMAL, CREATURE_ENTRY_TO_SPAWN, 0, offsetX, offsetY, offsetZ, 0.0f))
            {
                LOG_ERROR("gvg.marker", "Impossible de créer la créature entry {}", CREATURE_ENTRY_TO_SPAWN);
                delete creature;
                continue;
            }

            // Configurer la créature
            creature->SetHomePosition(offsetX, offsetY, offsetZ, 0.0f);
            creature->SetFaction(player->GetFaction());
            
            // Ajouter la créature à la map
            if (!map->AddToMap(creature))
            {
                LOG_ERROR("gvg.marker", "Impossible d'ajouter la créature à la map");
                delete creature;
                continue;
            }

            // Faire déplacer la créature vers le marqueur
            // usePathfinding = true : utilise le pathfinding (évite les obstacles, suit le terrain)
            // usePathfinding = false : ligne droite (peut traverser les obstacles)
            creature->GetMotionMaster()->MovePoint(
                0,  // point ID
                markerPos.GetPositionX(), 
                markerPos.GetPositionY(), 
                markerPos.GetPositionZ(),
                FORCED_MOVEMENT_NONE,  // mouvement normal
                0.0f,  // vitesse (0 = vitesse par défaut)
                0.0f,  // orientation
                usePathfinding,  // generatePath : true = pathfinding, false = ligne droite
                true   // forceDestination : forcer la destination même si le chemin est bloqué
            );

            spawnedCount++;
        }

        if (spawnedCount > 0)
        {
            ChatHandler(player->GetSession()).PSendSysMessage(
                "{} créature(s) spawnée(s) et déplacée(s) vers le marqueur (Pathfinding: {})", 
                spawnedCount, usePathfinding ? "Oui" : "Non");
        }
        else
        {
            ChatHandler(player->GetSession()).SendSysMessage("Aucune créature n'a pu être spawnée.");
        }
    }

    MarkerData _markerData;
};

// Script du GameObject
class go_conquest_marker : public GameObjectScript
{
public:
    go_conquest_marker() : GameObjectScript("go_conquest_marker") { }

    GameObjectAI* GetAI(GameObject* go) const override
    {
        return new go_conquest_marker_ai(go);
    }
};

// ============================================
// Item script pour le kit de marqueur
// ============================================
class ConquestMarkerItem : public ItemScript
{
public:
    ConquestMarkerItem() : ItemScript("ConquestMarkerItem") { }

    bool OnUse(Player* player, Item* item, SpellCastTargets const& /*targets*/) override
    {
        if (!player || !item)
            return false;

        // Vérifier si le joueur est dans une guilde
        if (player->GetGuildId() == 0)
        {
            ChatHandler(player->GetSession()).PSendSysMessage("Vous devez être dans une guilde pour utiliser cet objet.");
            return false;
        }

        // Obtenir la position du joueur
        float x = player->GetPositionX();
        float y = player->GetPositionY();
        float z = player->GetPositionZ();
        float o = player->GetOrientation();

        // Spawner le marqueur
        Map* map = player->GetMap();
        if (!map)
        {
            LOG_ERROR("module", "ConquestMarker: Failed to get map for player {}", player->GetName());
            return false;
        }

        Creature* marker = new Creature();
        if (!marker->Create(map->GenerateLowGuid<HighGuid::Unit>(), map, PHASEMASK_NORMAL, NPC_MARKER, 0, x, y, z, o))
        {
            LOG_ERROR("module", "ConquestMarker: Failed to create marker creature entry {}", NPC_MARKER);
            delete marker;
            return false;
        }

        marker->SetLevel(80);
        marker->SetHomePosition(x, y, z, o);
        marker->SetFaction(player->GetFaction());

        // Ajouter à la map
        if (!map->AddToMap(marker))
        {
            LOG_ERROR("module", "ConquestMarker: Failed to add marker to map");
            delete marker;
            return false;
        }

        LOG_INFO("module", "ConquestMarker: Player {} spawned marker at ({}, {}, {})", 
            player->GetName(), x, y, z);

        // Consommer l'item (usage unique)
        player->DestroyItemCount(item->GetEntry(), 1, true);

        return true;
    }
};

// ============================================
// NPC Spawner AI
// ============================================
class npc_conquest_spawner : public CreatureScript
{
public:
    npc_conquest_spawner() : CreatureScript("ConquestSpawner") { }

    struct npc_conquest_spawnerAI : public ScriptedAI
    {
        npc_conquest_spawnerAI(Creature* creature) : ScriptedAI(creature), m_spawnTimer(SPAWN_INTERVAL), m_pendingSpawns(0), m_nextSpawnTimer(0), m_currentMarker(nullptr)
        {
        }

        void Reset() override
        {
            m_spawnTimer = SPAWN_INTERVAL;
            m_pendingSpawns = 0;
            m_nextSpawnTimer = 0;
            m_currentMarker = nullptr;
        }

        void UpdateAI(uint32 diff) override
        {
            // Vérifier si le marqueur actuel est toujours valide
            if (m_currentMarker && (!m_currentMarker->IsInWorld() || !m_currentMarker->IsAlive()))
            {
                // Le marqueur a été supprimé, arrêter les spawns
                m_currentMarker = nullptr;
                m_pendingSpawns = 0;
                m_nextSpawnTimer = 0;
            }

            // Gérer les spawns en attente avec délai de 1 seconde
            if (m_pendingSpawns > 0 && m_nextSpawnTimer <= diff)
            {
                m_nextSpawnTimer = SPAWN_DELAY_BETWEEN;
                SpawnSingleCreature();
                m_pendingSpawns--;
            }
            else if (m_nextSpawnTimer > 0)
            {
                m_nextSpawnTimer -= diff;
            }

            // Timer principal pour déclencher une nouvelle vague
            if (m_spawnTimer <= diff)
            {
                m_spawnTimer = SPAWN_INTERVAL; // Reset timer

                // Chercher un marqueur à proximité
                Creature* marker = FindNearbyMarker();
                if (!marker)
                {
                    // Pas de marqueur, arrêter les spawns en cours
                    m_currentMarker = nullptr;
                    m_pendingSpawns = 0;
                    m_nextSpawnTimer = 0;
                    return;
                }

                // Programmer le spawn de 5 créatures avec délai de 1 seconde entre chaque
                m_currentMarker = marker;
                m_pendingSpawns = SPAWN_COUNT_PER_WAVE;
                m_nextSpawnTimer = 0; // Spawn immédiat de la première

            }
            else
            {
                m_spawnTimer -= diff;
            }

            // Pas de combat pour ce NPC
            if (!UpdateVictim())
                return;
        }

    private:
        Creature* FindNearbyMarker()
        {
            std::list<Creature*> markers;
            Acore::AllCreaturesOfEntryInRange checker(me, NPC_MARKER, MARKER_SEARCH_RANGE);
            Acore::CreatureListSearcher<Acore::AllCreaturesOfEntryInRange> searcher(me, markers, checker);
            Cell::VisitObjects(me, searcher, MARKER_SEARCH_RANGE);

            if (markers.empty())
                return nullptr;

            // Retourner le premier marqueur trouvé (le plus proche)
            return markers.front();
        }

        void SpawnSingleCreature()
        {
            if (!m_currentMarker || !me)
                return;

            // Vérifier que le marqueur est toujours valide
            if (!m_currentMarker->IsInWorld() || !m_currentMarker->IsAlive())
            {
                // Le marqueur a été supprimé, arrêter les spawns
                m_currentMarker = nullptr;
                m_pendingSpawns = 0;
                m_nextSpawnTimer = 0;
                return;
            }

            Map* map = me->GetMap();
            if (!map)
                return;

            Position spawnerPos = me->GetPosition();
            Position markerPos = m_currentMarker->GetPosition();

            // Choisir une créature aléatoire
            uint32 creatureEntry = urand(NPC_SPAWN_MIN, NPC_SPAWN_MAX);

            // Calculer la position de spawn avec un offset autour du spawner
            float angle = frand(0.0f, 2.0f * M_PI);
            float offsetX = spawnerPos.GetPositionX() + 3.0f * cos(angle);
            float offsetY = spawnerPos.GetPositionY() + 3.0f * sin(angle);
            float offsetZ = spawnerPos.GetPositionZ();

            // Vérifier que la position est valide
            if (!Acore::IsValidMapCoord(offsetX, offsetY, offsetZ))
            {
                LOG_ERROR("module", "ConquestSpawner: Position de spawn invalide: X={}, Y={}, Z={}", offsetX, offsetY, offsetZ);
                return;
            }

            // Créer la créature
            Creature* creature = new Creature();
            if (!creature->Create(map->GenerateLowGuid<HighGuid::Unit>(), map, PHASEMASK_NORMAL, creatureEntry, 0, offsetX, offsetY, offsetZ, 0.0f))
            {
                LOG_ERROR("module", "ConquestSpawner: Impossible de créer la créature entry {}", creatureEntry);
                delete creature;
                return;
            }

            // Configurer la créature
            creature->SetHomePosition(offsetX, offsetY, offsetZ, 0.0f);
            creature->SetFaction(me->GetFaction());
            
            // Ajouter la créature à la map
            if (!map->AddToMap(creature))
            {
                LOG_ERROR("module", "ConquestSpawner: Impossible d'ajouter la créature à la map");
                delete creature;
                return;
            }

            // Stocker dans la map globale pour la dispersion
            s_creaturesToDisperse[creature->GetGUID()] = m_currentMarker->GetGUID();

            // Faire déplacer la créature vers le marqueur avec pathfinding
            // Utiliser point ID 0 pour identifier l'arrivée au marqueur
            creature->GetMotionMaster()->MovePoint(
                0,  // point ID (0 = arrivée au marqueur)
                markerPos.GetPositionX(), 
                markerPos.GetPositionY(), 
                markerPos.GetPositionZ(),
                FORCED_MOVEMENT_NONE,  // mouvement normal
                0.0f,  // vitesse (0 = vitesse par défaut)
                0.0f,  // orientation
                true,  // generatePath = true : utilise le pathfinding
                true   // forceDestination = true
            );

            LOG_DEBUG("module", "ConquestSpawner: Créature {} spawnée et déplacée vers le marqueur", creatureEntry);
        }

        uint32 m_spawnTimer;
        uint32 m_pendingSpawns;
        uint32 m_nextSpawnTimer;
        Creature* m_currentMarker;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_conquest_spawnerAI(creature);
    }
};

// ============================================
// AllCreatureScript pour gérer la dispersion des créatures spawnées
// ============================================
class ConquestSpawnedCreatureAllScript : public AllCreatureScript
{
public:
    ConquestSpawnedCreatureAllScript() : AllCreatureScript("ConquestSpawnedCreatureAllScript") { }

    void OnAllCreatureUpdate(Creature* creature, uint32 /*diff*/) override
    {
        if (!creature)
            return;

        // Vérifier si c'est une créature spawnée (400300-400305)
        uint32 entry = creature->GetEntry();
        if (entry < NPC_SPAWN_MIN || entry > NPC_SPAWN_MAX)
            return;

        // Nettoyer la map si la créature est morte
        if (!creature->IsAlive())
        {
            s_creaturesToDisperse.erase(creature->GetGUID());
            return;
        }

        // Vérifier si cette créature doit se disperser
        auto it = s_creaturesToDisperse.find(creature->GetGUID());
        if (it == s_creaturesToDisperse.end())
            return;

        // Vérifier si la créature est arrivée au marqueur (elle ne bouge plus et est proche du marqueur)
        if (creature->IsStopped() && !creature->HasUnitState(UNIT_STATE_ROAMING_MOVE))
        {
            // Vérifier la distance au marqueur
            Creature* marker = ObjectAccessor::GetCreature(*creature, it->second);
            if (marker && marker->IsInWorld() && marker->IsAlive() && creature->IsWithinDist(marker, 5.0f))
            {
                // La créature est arrivée, la disperser
                DisperseAroundMarker(creature, marker);
                s_creaturesToDisperse.erase(it);
            }
            else if (!marker || !marker->IsInWorld())
            {
                // Le marqueur a été supprimé, nettoyer la référence
                s_creaturesToDisperse.erase(it);
            }
        }
    }

    void OnCreatureRemoveWorld(Creature* creature) override
    {
        // Nettoyer la map quand la créature est retirée du monde
        if (creature)
            s_creaturesToDisperse.erase(creature->GetGUID());
    }

private:
    void DisperseAroundMarker(Creature* creature, Creature* marker)
    {
        if (!creature || !marker)
            return;

        Position markerPos = marker->GetPosition();

        // Calculer une position aléatoire autour du marqueur
        float angle = frand(0.0f, 2.0f * M_PI);
        float distance = frand(2.0f, DISPERSION_RANGE);
        
        float disperseX = markerPos.GetPositionX() + distance * cos(angle);
        float disperseY = markerPos.GetPositionY() + distance * sin(angle);
        float disperseZ = markerPos.GetPositionZ();

        // Vérifier que la position est valide
        if (!Acore::IsValidMapCoord(disperseX, disperseY, disperseZ))
        {
            // Si la position n'est pas valide, essayer une position plus proche
            distance = frand(2.0f, DISPERSION_RANGE / 2.0f);
            disperseX = markerPos.GetPositionX() + distance * cos(angle);
            disperseY = markerPos.GetPositionY() + distance * sin(angle);
        }

        // Se déplacer vers la position de dispersion
        creature->GetMotionMaster()->MovePoint(
            1,  // point ID différent pour la dispersion
            disperseX,
            disperseY,
            disperseZ,
            FORCED_MOVEMENT_NONE,
            0.0f,
            0.0f,
            true,  // pathfinding
            true
        );

        LOG_DEBUG("module", "ConquestSpawnedCreature: Créature {} dispersée vers ({}, {}, {})", creature->GetEntry(), disperseX, disperseY, disperseZ);
    }
};

// Fonction d'enregistrement
void AddSC_conquest_marker_spawner()
{
    new go_conquest_marker();
    new ConquestMarkerItem();
    new npc_conquest_spawner();
    new ConquestSpawnedCreatureAllScript();
}

