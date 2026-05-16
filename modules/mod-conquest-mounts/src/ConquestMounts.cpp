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
#include "Vehicle.h"
#include "MotionMaster.h"
#include "ObjectMgr.h"
#include "VehicleDefines.h"
#include "Spell.h"
#include "VehicleScript.h"
#include "SpellMgr.h"
#include "SpellInfo.h"
#include "GameTime.h"
#include "ObjectAccessor.h"

#include <unordered_map>

// Ownership registry for GMK-summoned siege engines.
// Key = vehicle GUID raw, Value = owner player GUID raw.
// Single-threaded access from worldserver main loop, no lock needed.
namespace
{
    std::unordered_map<uint64, uint64> s_vehicleOwners;
}

// Exported across modules: vendor calls this right after SummonCreature.
// Inherits the summoner's faction (allies see friendly, enemies see hostile)
// and registers the owner so PassengerBoarded can reject foreign boarders.
void ConquestRegisterVehicleOwner(Creature* vehicle, Player* owner)
{
    if (!vehicle || !owner)
        return;

    uint64 vguid = vehicle->GetGUID().GetRawValue();
    s_vehicleOwners[vguid] = owner->GetGUID().GetRawValue();

    vehicle->SetFaction(owner->GetFaction());

    vehicle->RemoveUnitFlag(static_cast<UnitFlags>(
        UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC));

    LOG_INFO("module", "ConquestMounts: Registered owner {} (faction {}) for vehicle {} (entry {})",
             owner->GetName(), owner->GetFaction(), vguid, vehicle->GetEntry());
}

static bool ConquestIsOwnedBy(Creature* vehicle, Unit* unit)
{
    if (!vehicle || !unit)
        return false;
    auto it = s_vehicleOwners.find(vehicle->GetGUID().GetRawValue());
    if (it == s_vehicleOwners.end())
        return true;
    return it->second == unit->GetGUID().GetRawValue();
}

static void ConquestForgetVehicle(Creature* vehicle)
{
    if (!vehicle)
        return;
    s_vehicleOwners.erase(vehicle->GetGUID().GetRawValue());
}

// NPC entry for the mount creature
#define MOUNT_HORSE_ENTRY 400000
#define MOUNT_WOLF_ENTRY 400001
#define MOUNT_SHREDDER_ENTRY 400002
#define MOUNT_MECHANO_TANK_ENTRY 400003

// Siege engine entries (400200-400205)
#define SIEGE_ENGINE_RED      400200  // Baroudeur P-W8
#define SIEGE_ENGINE_BLUE     400201  // Destructeur B27
#define CATAPULT              400202
#define GLAIVE_THROWER_PURPLE 400203
#define GLAIVE_THROWER_ORANGE 400204
#define DEMOLISHER            400205
#define COMBAT_TURRET         400206
#define COMBAT_TURRET_2       400210
#define SCOUT_M2              400209  // Pisteur M2
#define DESTRUCTION_TURRET_RED 400207
#define DESTRUCTION_TURRET_BLUE 400208
#define DESTRUCTION_TURRET_MAIN 400211  // Main destruction turret with scale 2

// v3 : variants faction + Leviathan + Siege chair
#define M2_RED                400310  // Pisteur M2 (Horde) — display rouge
#define PW8_BLUE              400311  // Baroudeur P-W8 (Alliance) — display bleu
#define B27_RED               400312  // Destructeur B27 (Horde) — display rouge
#define SIEGE_CHAIR           400313  // PNJ chaise pour bloquer les seats
#define LEVIATHAN_ALLIANCE    400314
#define LEVIATHAN_HORDE       400315
#define LEVIATHAN_TURRET      400316  // tourelle Leviathan custom (clone 33139, faction 35)
#define TURRET_M2             400317  // tourelle Pisteur M2 (display 27101)
#define TURRET_PW8            400318  // tourelle Baroudeur P-W8 (display 29489)
#define TURRET_B27_HORDE      400319  // tourelle B27 cote Horde (display 28106)
#define TURRET_B27_ALLIANCE   400320  // tourelle B27 cote Alliance (display 25301)
#define PROTECTEUR_E800       400326  // Protecteur E800 (vehicle parent)

// Helper function to check if a creature is a siege engine (custom). Inclus
// les variants v3 (M2 rouge, P-W8 bleu, B27 rouge, Leviathan A/H).
inline bool IsSiegeEngine(uint32 entry)
{
    if ((entry >= SIEGE_ENGINE_RED && entry <= DEMOLISHER) || entry == SCOUT_M2)
        return true;
    return entry == M2_RED || entry == PW8_BLUE || entry == B27_RED
        || entry == LEVIATHAN_ALLIANCE || entry == LEVIATHAN_HORDE;
}

// HP cible exact par entry (override la formule level/class).
inline uint32 GetSiegeEngineMaxHealth(uint32 entry)
{
    switch (entry)
    {
        case SCOUT_M2:
        case M2_RED:               return 20000;
        case SIEGE_ENGINE_RED:
        case PW8_BLUE:             return 30000;
        case SIEGE_ENGINE_BLUE:
        case B27_RED:              return 40000;
        case LEVIATHAN_ALLIANCE:
        case LEVIATHAN_HORDE:      return 50000;
        case PROTECTEUR_E800:      return 40000;
        default:                   return 0; // 0 = pas d'override
    }
}

// Script to handle mount creature
class ConquestMountsCreature : public CreatureScript
{
public:
    ConquestMountsCreature() : CreatureScript("ConquestMountsCreature") { }

    struct ConquestMountsCreatureAI : public ScriptedAI
    {
        ConquestMountsCreatureAI(Creature* creature) : ScriptedAI(creature), m_homePositionUpdated(false) { }

        void InitializeAI() override
        {
            // Ensure the vehicle kit is installed if the creature has a VehicleId
            if (me->IsVehicle() && me->GetVehicleKit())
            {
                me->GetVehicleKit()->Install();
            }
            
            // Ensure faction is neutral (35 = FACTION_FRIENDLY) so mountable by all players
            // This ensures the creature is friendly to all factions
            if (me->GetFaction() != 35)
            {
                me->SetFaction(35);
                LOG_INFO("module", "ConquestMounts: Set faction to 35 (FACTION_FRIENDLY) for creature {}", me->GetEntry());
            }
        }

        void UpdateAI(uint32 /*diff*/) override
        {
            // Check if creature is trying to move home and prevent it
            if (m_homePositionUpdated && me->GetMotionMaster()->GetCurrentMovementGeneratorType() == HOME_MOTION_TYPE)
            {
                LOG_INFO("module", "ConquestMounts: Creature {} trying to return home, preventing movement", me->GetEntry());
                // Stop movement and force idle
                me->StopMoving();
                me->GetMotionMaster()->Clear();
                me->GetMotionMaster()->MoveIdle();
            }
        }

        void PassengerBoarded(Unit* passenger, int8 /*seatId*/, bool apply) override
        {
            // When player dismounts (apply = false), mark that we need to update home position
            // We'll do it in OnCharmed(false) which is called after RemoveCharmedBy
            if (!apply && passenger && passenger->IsPlayer())
            {
                LOG_INFO("module", "ConquestMounts: Player {} dismounted from creature {}", 
                    passenger->GetName(), me->GetEntry());
                m_homePositionUpdated = true;
            }
        }

        void OnCharmed(bool apply) override
        {
            // When charm is removed (apply = false), update home position to current position
            // This is called AFTER RemoveCharmedBy which calls InitDefault()
            // So we need to update home position and reset movement here
            if (!apply && m_homePositionUpdated)
            {
                LOG_INFO("module", "ConquestMounts: Charm removed from creature {}, updating home position to ({}, {}, {})", 
                    me->GetEntry(), me->GetPositionX(), me->GetPositionY(), me->GetPositionZ());
                
                // Update home position to current position
                me->SetHomePosition(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());
                
                // Stop any movement that might be in progress
                me->StopMoving();
                me->GetMotionMaster()->Clear();
                
                // Force idle state to prevent any movement
                me->GetMotionMaster()->MoveIdle();
                
                LOG_INFO("module", "ConquestMounts: Home position updated and movement forced to idle");
                m_homePositionUpdated = false;
            }
        }

        void JustReachedHome() override
        {
            // Override to prevent any default behavior when reaching home
            // The home position is already updated to current position in OnCharmed
        }

        void JustDied(Unit* /*killer*/) override
        {
            // Delete mount creature from database when it dies
            if (me->GetEntry() >= MOUNT_HORSE_ENTRY && me->GetEntry() <= MOUNT_MECHANO_TANK_ENTRY)
            {
                LOG_INFO("module", "ConquestMounts: Mount creature {} died, deleting from database", me->GetEntry());
                me->DeleteFromDB();
            }
        }

    private:
        bool m_homePositionUpdated; // Track if we've updated home position after dismount
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new ConquestMountsCreatureAI(creature);
    }
};

// Script to handle siege engines (delete from DB on death)
class ConquestSiegeEngine : public CreatureScript
{
public:
    ConquestSiegeEngine() : CreatureScript("ConquestSiegeEngine") { }

    struct ConquestSiegeEngineAI : public ScriptedAI
    {
        ConquestSiegeEngineAI(Creature* creature) : ScriptedAI(creature), m_turretGUID(), m_smallTurretsGUID(), m_homePositionUpdated(false), m_turretInstallTimer(0), m_turretCheckTimer(0), m_turretsInstalled(false), m_initialRebindTimer(2000), m_initialRebindDone(false) { }

        void InitializeAI() override
        {
            LOG_INFO("module", "ConquestMounts: [INIT] InitializeAI called for siege engine {}", me->GetEntry());

            // Force phase 1 (visible par tous les joueurs). GM qui achete un
            // vehicle en phase 3 -> sans ce force, le vehicle est invisible
            // pour les joueurs phase 1.
            me->SetPhaseMask(1, true);

            uint32 entry = me->GetEntry();

            // ============ Speed + scale par entry ============
            // v3 : variants faction partagent les valeurs de l'original.
            // B27 (bleu OU rouge) : scale 1.0 (etait 1.5)
            // M2  (bleu OU rouge) : scale 0.5 (etait 0.6)
            // P-W8 (bleu OU rouge) : scale 0.75
            // Leviathan (A/H) : scale 0.5
            if (entry == SIEGE_ENGINE_RED || entry == PW8_BLUE) // Baroudeur P-W8 (rouge ou bleu)
            {
                me->SetSpeed(MOVE_WALK, 1.0f, true);
                me->SetSpeed(MOVE_RUN, 2.0f, true);
                me->SetObjectScale(0.75f);
            }
            else if (entry == SIEGE_ENGINE_BLUE || entry == B27_RED) // Destructeur B27 (bleu ou rouge)
            {
                me->SetSpeed(MOVE_WALK, 1.2f, true);
                me->SetSpeed(MOVE_RUN, 1.0f, true);
                me->SetObjectScale(1.0f);
            }
            else if (entry == SCOUT_M2 || entry == M2_RED) // Pisteur M2 (bleu ou rouge)
            {
                me->SetSpeed(MOVE_WALK, 1.0f, true);
                me->SetSpeed(MOVE_RUN, 3.0f, true);
                me->SetObjectScale(0.5f);
            }
            else if (entry == LEVIATHAN_ALLIANCE || entry == LEVIATHAN_HORDE)
            {
                me->SetSpeed(MOVE_WALK, 1.0f, true);
                me->SetSpeed(MOVE_RUN, 3.0f, true);
                me->SetObjectScale(0.25f);
            }
            else if (entry == DEMOLISHER)
            {
                me->SetSpeed(MOVE_WALK, 1.0f, true);
                me->SetSpeed(MOVE_RUN, 2.0f, true);
            }
            else if (entry == CATAPULT)
            {
                me->SetSpeed(MOVE_WALK, 1.0f, true);
                me->SetSpeed(MOVE_RUN, 3.0f, true);
            }
            else if (entry == GLAIVE_THROWER_PURPLE || entry == GLAIVE_THROWER_ORANGE)
            {
                me->SetSpeed(MOVE_WALK, 1.0f, true);
                me->SetSpeed(MOVE_RUN, 2.0f, true);
            }

            // ============ HP cible exact ============
            if (uint32 targetHp = GetSiegeEngineMaxHealth(entry))
            {
                me->SetMaxHealth(targetHp);
                me->SetHealth(targetHp);
                LOG_INFO("module", "ConquestMounts: [INIT] entry {} HP set to {}", entry, targetHp);
            }

            // Install() est appelé automatiquement dans AddToWorld() APRÈS InitializeAI
            // InstallAllAccessories sera appelé dans VehicleScript::OnInstall
            m_turretInstallTimer = 100;
        }
        
        // Pas de OnSpellClick custom dans la version "qui marchait" (on laisse la mécanique native)
        void Reset() override
        {
            LOG_INFO("module", "ConquestMounts: [RESET] Reset called for siege engine {} - DO NOT call InstallAllAccessories here!", me->GetEntry());
            // DO NOT call InstallAllAccessories here - Reset() calls InstallAllAccessories which calls RemoveAllPassengers()
            // This would remove all turrets and cause them to despawn
            // Just call ScriptedAI::Reset() without calling Vehicle::Reset()
            // ScriptedAI::Reset() does not call Vehicle::Reset()
        }
        
        void UpdateAI(uint32 diff) override
        {
            // Install turrets after a short delay if OnInstall didn't work
            // Only install once to avoid conflicts
            if (m_turretInstallTimer > 0 && !m_turretsInstalled)
            {
                if (m_turretInstallTimer <= diff)
                {
                    m_turretInstallTimer = 0;
                    if (me->IsVehicle() && me->GetVehicleKit())
                    {
                        Vehicle* veh = me->GetVehicleKit();
                        if (veh)
                        {
                            // Check if turrets are already installed
                            bool hasTurrets = (veh->GetPassenger(1) != nullptr) || 
                                             (veh->GetPassenger(2) != nullptr) || 
                                             (veh->GetPassenger(7) != nullptr);
                            
                            if (!hasTurrets)
                            {
                                LOG_INFO("module", "ConquestMounts: [UPDATE] Installing accessories as fallback for siege engine {}", me->GetEntry());
                                veh->InstallAllAccessories(false);
                                m_turretsInstalled = true;
                                
                                // Wait a bit and check again
                                m_turretCheckTimer = 500; // Check after 500ms
                            }
                            else
                            {
                                LOG_INFO("module", "ConquestMounts: [UPDATE] Turrets already installed, skipping");
                                m_turretsInstalled = true;
                            }
                        }
                    }
                }
                else
                {
                    m_turretInstallTimer -= diff;
                }
            }
            
            // Check turrets after installation
            if (m_turretCheckTimer > 0)
            {
                if (m_turretCheckTimer <= diff)
                {
                    m_turretCheckTimer = 0;
                    if (me->IsVehicle() && me->GetVehicleKit())
                    {
                        Vehicle* veh = me->GetVehicleKit();
                        LOG_INFO("module", "ConquestMounts: [UPDATE] Checking turrets after installation for siege engine {}", me->GetEntry());
                        
                        // Check if accessories were installed
                        if (Unit* turret1 = veh->GetPassenger(1))
                            LOG_INFO("module", "ConquestMounts: [UPDATE] Seat 1 has passenger: {}", turret1->GetEntry());
                        else
                            LOG_INFO("module", "ConquestMounts: [UPDATE] Seat 1 is empty");
                        
                        if (Unit* turret2 = veh->GetPassenger(2))
                            LOG_INFO("module", "ConquestMounts: [UPDATE] Seat 2 has passenger: {}", turret2->GetEntry());
                        else
                            LOG_INFO("module", "ConquestMounts: [UPDATE] Seat 2 is empty");
                        
                        if (Unit* turret7 = veh->GetPassenger(7))
                        {
                            LOG_INFO("module", "ConquestMounts: [UPDATE] Seat 7 has passenger: {}", turret7->GetEntry());
                            
                            // Canon massif scale 1.5 sur B27 bleu (400201) OU rouge (400312)
                            if ((me->GetEntry() == SIEGE_ENGINE_BLUE || me->GetEntry() == B27_RED)
                                && turret7->GetEntry() == DESTRUCTION_TURRET_MAIN)
                            {
                                if (Creature* turret = turret7->ToCreature())
                                {
                                    turret->SetObjectScale(1.5f);
                                }
                            }
                            // Tourelle M2 (400317) scale 0.75 sur M2 bleu (400209) OU rouge (400310)
                            else if ((me->GetEntry() == SCOUT_M2 || me->GetEntry() == M2_RED)
                                     && turret7->GetEntry() == TURRET_M2)
                            {
                                if (Creature* turret = turret7->ToCreature())
                                {
                                    turret->SetObjectScale(0.75f);
                                }
                            }
                        }
                        else
                            LOG_INFO("module", "ConquestMounts: [UPDATE] Seat 7 is empty");
                    }
                }
                else
                {
                    m_turretCheckTimer -= diff;
                }
            }
            
            // Re-apply scale 0.75 sur Canon massif / M2 combat turret apres rebind.
            if (m_initialRebindDone && me->IsVehicle() && me->GetVehicleKit())
            {
                uint32 vEntry = me->GetEntry();
                Vehicle* veh = me->GetVehicleKit();
                if (Unit* turret7 = veh->GetPassenger(7))
                {
                    Creature* turret = turret7->ToCreature();
                    if (turret)
                    {
                        if ((vEntry == SIEGE_ENGINE_BLUE || vEntry == B27_RED)
                            && turret7->GetEntry() == DESTRUCTION_TURRET_MAIN
                            && turret->GetObjectScale() != 1.5f)
                        {
                            turret->SetObjectScale(1.5f);
                        }
                        else if ((vEntry == SCOUT_M2 || vEntry == M2_RED)
                                 && turret7->GetEntry() == TURRET_M2
                                 && turret->GetObjectScale() != 0.75f)
                        {
                            turret->SetObjectScale(0.75f);
                        }
                    }
                }
            }
            
            // Initial rebind once after 2 seconds (only if no player is in any turret)
            if (!m_initialRebindDone && m_initialRebindTimer > 0)
            {
                if (m_initialRebindTimer <= diff)
                {
                    m_initialRebindTimer = 0;
                    if (me->IsVehicle() && me->GetVehicleKit())
                    {
                        Vehicle* veh = me->GetVehicleKit();
                        
                        // Check if any player is currently in any turret
                        bool hasPlayer = false;
                        for (int8 seat = 0; seat < 8; ++seat)
                        {
                            if (Unit* passenger = veh->GetPassenger(seat))
                            {
                                if (passenger->IsPlayer())
                                {
                                    hasPlayer = true;
                                    break;
                                }
                            }
                        }
                        
                        // Only rebind if no player is in any turret
                        if (!hasPlayer)
                        {
                            LOG_INFO("module", "ConquestMounts: [UPDATE] Performing initial rebind for siege engine {}", me->GetEntry());
                            
                            // Rebind all turrets
                            for (int8 seat = 1; seat < 8; ++seat)
                            {
                                if (Unit* turret = veh->GetPassenger(seat))
                                {
                                    if (turret->IsCreature())
                                    {
                                        LOG_INFO("module", "ConquestMounts: [UPDATE] Rebinding turret {} at seat {}", turret->GetEntry(), seat);
                                        turret->ExitVehicle();
                                        turret->EnterVehicleUnattackable(me, seat);
                                    }
                                }
                            }
                            
                            m_initialRebindDone = true;
                        }
                        else
                        {
                            LOG_INFO("module", "ConquestMounts: [UPDATE] Player in turret, skipping initial rebind");
                            m_initialRebindDone = true; // Don't retry if player is using it
                        }
                    }
                }
                else
                {
                    m_initialRebindTimer -= diff;
                }
            }
            
            // Check if creature is trying to move home and prevent it
            if (m_homePositionUpdated && me->GetMotionMaster()->GetCurrentMovementGeneratorType() == HOME_MOTION_TYPE)
            {
                me->StopMoving();
                me->GetMotionMaster()->Clear();
                me->GetMotionMaster()->MoveIdle();
            }
        }
        void PassengerBoarded(Unit* passenger, int8 /*seatId*/, bool apply) override
        {
            // When player dismounts (apply = false), mark that we need to update home position
            if (!apply && passenger && passenger->IsPlayer())
            {
                m_homePositionUpdated = true;
            }

            // When a player boards, enforce ownership: only the summoner can ride.
            // Foreign boarders are ejected immediately.
            if (apply && passenger && passenger->IsPlayer())
            {
                if (!ConquestIsOwnedBy(me, passenger))
                {
                    LOG_INFO("module", "ConquestMounts: Rejecting non-owner {} from vehicle {}",
                             passenger->GetName(), me->GetEntry());
                    passenger->ExitVehicle();
                    return;
                }

                // Store turret GUIDs when player boards
                if (Unit* turret1 = me->GetVehicleKit()->GetPassenger(2))
                    if (turret1->IsCreature() && turret1->GetEntry() == COMBAT_TURRET)
                        m_smallTurretsGUID.push_back(turret1->GetGUID());
                if (Unit* turret2 = me->GetVehicleKit()->GetPassenger(1))
                    if (turret2->IsCreature() && turret2->GetEntry() == COMBAT_TURRET_2)
                        m_smallTurretsGUID.push_back(turret2->GetGUID());
                if (Unit* turret = me->GetVehicleKit()->GetPassenger(7))
                    if (turret->IsCreature() && (turret->GetEntry() == DESTRUCTION_TURRET_RED || turret->GetEntry() == DESTRUCTION_TURRET_BLUE))
                        m_turretGUID = turret->GetGUID();
            }
        }

        void OnCharmed(bool apply) override
        {
            // When charm is removed (apply = false), update home position to current position
            if (!apply && m_homePositionUpdated)
            {
                LOG_INFO("module", "ConquestMounts: Charm removed from siege engine {}, updating home position to ({}, {}, {})", 
                    me->GetEntry(), me->GetPositionX(), me->GetPositionY(), me->GetPositionZ());
                
                // Update home position to current position
                me->SetHomePosition(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());
                
                // Stop any movement that might be in progress
                me->StopMoving();
                me->GetMotionMaster()->Clear();
                
                // Force idle state to prevent any movement
                me->GetMotionMaster()->MoveIdle();
                
                LOG_INFO("module", "ConquestMounts: Home position updated and movement forced to idle");
                m_homePositionUpdated = false;
            }
        }

        void JustReachedHome() override
        {
            // Override to prevent any default behavior when reaching home
        }

        void JustDied(Unit* killer) override
        {
            // Delete main turret if it exists
            if (m_turretGUID)
            {
                if (Creature* turret = ObjectAccessor::GetCreature(*me, m_turretGUID))
                {
                    turret->DespawnOrUnsummon();
                }
                m_turretGUID.Clear();
            }
            
            // Delete all small turrets
            for (ObjectGuid const& guid : m_smallTurretsGUID)
            {
                if (Creature* turret = ObjectAccessor::GetCreature(*me, guid))
                {
                    turret->DespawnOrUnsummon();
                }
            }
            m_smallTurretsGUID.clear();
            
            // Delete siege engine from database when it dies
            if (IsSiegeEngine(me->GetEntry()))
            {
                LOG_INFO("module", "ConquestMounts: Siege engine {} died, deleting from database", me->GetEntry());

                // Update achievement criteria for killing blows (like npc_four_car_garage)
                if (killer)
                {
                    if (Player* player = killer->GetCharmerOrOwnerPlayerOrPlayerItself())
                    {
                        player->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_GET_KILLING_BLOWS, 1, 0, me);
                    }
                }

                me->DeleteFromDB();
            }

            ConquestForgetVehicle(me);
        }
        
        ObjectGuid m_turretGUID; // GUID of the destruction turret
        std::vector<ObjectGuid> m_smallTurretsGUID; // GUIDs of the combat turrets
        bool m_homePositionUpdated; // Track if we've updated home position after dismount
        uint32 m_turretInstallTimer; // Timer to install turrets as fallback if OnInstall doesn't work
        uint32 m_turretCheckTimer; // Timer to check if turrets were installed after a delay
        bool m_turretsInstalled; // Track if turrets have been installed to avoid multiple calls
        uint32 m_initialRebindTimer; // Timer for initial rebind (2 seconds)
        bool m_initialRebindDone; // Track if initial rebind has been done
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new ConquestSiegeEngineAI(creature);
    }
};

// VehicleScript for siege engines - installs turrets automatically
// Uses the same ScriptName as CreatureScript so it gets called for the same creatures
class ConquestSiegeEngineVehicle : public VehicleScript
{
public:
    ConquestSiegeEngineVehicle() : VehicleScript("ConquestSiegeEngine") { }

    void OnInstall(Vehicle* veh) override
    {
        if (!veh || !veh->GetBase()->IsCreature())
        {
            LOG_ERROR("module", "ConquestMounts: [VEHICLE] OnInstall invalid");
            return;
        }

        Creature* creature = veh->GetBase()->ToCreature();
        uint32 entry = creature->GetEntry();

        // ============ Miroir de InitializeAI : speeds + scales par entry ============
        if (entry == SIEGE_ENGINE_RED || entry == PW8_BLUE)
        {
            creature->SetSpeed(MOVE_WALK, 1.0f, true);
            creature->SetSpeed(MOVE_RUN, 2.0f, true);
            creature->SetObjectScale(0.75f);
        }
        else if (entry == SIEGE_ENGINE_BLUE || entry == B27_RED)
        {
            creature->SetSpeed(MOVE_WALK, 1.2f, true);
            creature->SetSpeed(MOVE_RUN, 1.0f, true);
            creature->SetObjectScale(1.0f);
        }
        else if (entry == SCOUT_M2 || entry == M2_RED)
        {
            creature->SetSpeed(MOVE_WALK, 1.0f, true);
            creature->SetSpeed(MOVE_RUN, 3.0f, true);
            creature->SetObjectScale(0.5f);
        }
        else if (entry == LEVIATHAN_ALLIANCE || entry == LEVIATHAN_HORDE)
        {
            creature->SetSpeed(MOVE_WALK, 1.0f, true);
            creature->SetSpeed(MOVE_RUN, 3.0f, true);
            creature->SetObjectScale(0.25f);
        }
        else if (entry == DEMOLISHER)
        {
            creature->SetSpeed(MOVE_WALK, 1.0f, true);
            creature->SetSpeed(MOVE_RUN, 2.0f, true);
        }
        else if (entry == CATAPULT)
        {
            creature->SetSpeed(MOVE_WALK, 1.0f, true);
            creature->SetSpeed(MOVE_RUN, 3.0f, true);
        }
        else if (entry == GLAIVE_THROWER_PURPLE || entry == GLAIVE_THROWER_ORANGE)
        {
            creature->SetSpeed(MOVE_WALK, 1.0f, true);
            creature->SetSpeed(MOVE_RUN, 2.0f, true);
        }

        // ============ HP exact ============
        if (uint32 targetHp = GetSiegeEngineMaxHealth(entry))
        {
            creature->SetMaxHealth(targetHp);
            creature->SetHealth(targetHp);
        }

        // Install turrets pour les engins qui ont des accessoires definis
        // (B27, P-W8, M2, Leviathan -- variants bleus et rouges).
        bool hasAccessories = (entry == SIEGE_ENGINE_RED || entry == SIEGE_ENGINE_BLUE
                            || entry == PW8_BLUE || entry == B27_RED
                            || entry == SCOUT_M2 || entry == M2_RED
                            || entry == LEVIATHAN_ALLIANCE || entry == LEVIATHAN_HORDE
                            || entry == PROTECTEUR_E800);
        if (hasAccessories)
        {
            bool hasTurrets = (veh->GetPassenger(1) != nullptr) ||
                              (veh->GetPassenger(2) != nullptr) ||
                              (veh->GetPassenger(7) != nullptr);
            if (!hasTurrets)
            {
                LOG_INFO("module", "ConquestMounts: [VEHICLE] InstallAllAccessories for entry {}", entry);
                veh->InstallAllAccessories(false);
            }
        }

        // Force phase 1 sur le parent ET tous les passagers (accessoires)
        // pour garantir la visibilite cross-phase (GM on/off / joueurs).
        creature->SetPhaseMask(1, true);
        LOG_INFO("module", "ConquestMounts: [PHASE] vehicle entry {} phaseMask={}",
                 entry, creature->GetPhaseMask());
        for (int8 seat = 0; seat < 8; ++seat)
        {
            if (Unit* p = veh->GetPassenger(seat))
            {
                p->SetPhaseMask(1, true);
                LOG_INFO("module", "ConquestMounts: [PHASE] accessory seat {} entry {} phaseMask={}",
                         (int)seat, p->GetEntry(), p->GetPhaseMask());
            }
        }
    }
    
    void OnInstallAccessory(Vehicle* veh, Creature* accessory) override
    {
        if (!veh || !veh->GetBase() || !accessory)
            return;

        Creature* vehicle = veh->GetBase()->ToCreature();
        if (!vehicle)
            return;

        // Force phase 1 sur l'accessoire (cf. ConquestRegisterVehicleOwner qui
        // force aussi phase 1 sur le parent). Sans ca, un GM en phase 3 qui
        // achete un vehicle voit les accessoires mais les joueurs phase 1 non.
        accessory->SetPhaseMask(1, true);

        uint32 vehicleEntry = vehicle->GetEntry();
        uint32 accessoryEntry = accessory->GetEntry();

        // Canon massif (400211) sur B27 (bleu ou rouge) seat 7 : scale 1.5
        if ((vehicleEntry == SIEGE_ENGINE_BLUE || vehicleEntry == B27_RED)
            && accessoryEntry == DESTRUCTION_TURRET_MAIN)
        {
            accessory->SetObjectScale(1.5f);
            LOG_INFO("module", "ConquestMounts: Canon massif scale 1.5 on B27 entry {}", vehicleEntry);
        }
        // Tourelle pisteur M2 (400317) sur M2 (bleu ou rouge) seat 7 : scale 0.75
        else if ((vehicleEntry == SCOUT_M2 || vehicleEntry == M2_RED)
                 && accessoryEntry == TURRET_M2)
        {
            accessory->SetObjectScale(0.75f);
            LOG_INFO("module", "ConquestMounts: Tourelle M2 scale 0.75 on entry {}", vehicleEntry);
        }
    }
};

// Add all scripts
void AddConquestMountsScripts()
{
    new ConquestMountsCreature();
    new ConquestSiegeEngine();
    new ConquestSiegeEngineVehicle();
}


