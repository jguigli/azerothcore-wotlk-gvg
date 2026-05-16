/*
 * Script pour le Kit de recrutement
 * Permet à un joueur de recruter jusqu'à 3 PNJs de sa guilde pour le suivre
 */

#include "ConquestRecruitmentKit.h"
#include "ConquestBuildCommon.h"
#include "Item.h"
#include "Player.h"
#include "Creature.h"
#include "MotionMaster.h"
#include "Chat.h"
#include "ScriptMgr.h"
#include "ItemScript.h"
#include "Guild.h"
#include "GuildMgr.h"
#include "ObjectAccessor.h"
#include "World.h"
#include "AllCreatureScript.h"
#include <unordered_map>
#include <vector>
#include <algorithm>

// Entry de l'item
#define ITEM_RECRUITMENT_KIT 80041

// Limite de recrues
#define MAX_RECRUITS 3

// Distance de follow
#define FOLLOW_DISTANCE 3.0f

// Angles pour les positions décalées (en radians)
// Les 3 recrues seront positionnées autour du joueur avec des angles différents
#define RECRUIT_ANGLE_0 3.141593f       // 180 degrés (M_PI) - Derrière le joueur
#define RECRUIT_ANGLE_1 2.094395f       // 120 degrés (2*PI/3)
#define RECRUIT_ANGLE_2 4.188790f       // 240 degrés (4*PI/3)

// Map globale pour stocker les recrues par joueur (PlayerGUID -> vector de CreatureGUID)
static std::unordered_map<ObjectGuid, std::vector<ObjectGuid>> s_playerRecruits;
// Map pour stocker le dernier état de mouvement du joueur (pour éviter les repositionnements inutiles)
static std::unordered_map<ObjectGuid, bool> s_playerWasMoving;
// Map pour stocker le timer de repositionnement (pour éviter les vérifications trop fréquentes)
static std::unordered_map<ObjectGuid, uint32> s_recruit0RepositionTimer;
// Map pour stocker la dernière position cible du PNJ en position 0 (pour éviter les repositionnements inutiles)
static std::unordered_map<ObjectGuid, Position> s_recruit0LastTargetPos;
// Map pour stocker si les recrues sont arrêtées (PlayerGUID -> bool)
static std::unordered_map<ObjectGuid, bool> s_recruitsStopped;

// Fonction pour obtenir l'angle selon l'index de la recrue (0, 1, 2)
float GetRecruitAngle(uint8 index)
{
    switch (index)
    {
        case 0: return RECRUIT_ANGLE_0; // Derrière le joueur
        case 1: return RECRUIT_ANGLE_1;
        case 2: return RECRUIT_ANGLE_2;
        default: return 0.0f;
    }
}

// Fonction pour obtenir le nombre de recrues actives d'un joueur
uint8 GetRecruitCount(Player* player)
{
    if (!player)
        return 0;

    auto it = s_playerRecruits.find(player->GetGUID());
    if (it == s_playerRecruits.end())
        return 0;

    // Nettoyer les recrues invalides
    std::vector<ObjectGuid>& recruits = it->second;
    Player* playerPtr = player; // Capture pour la lambda
    recruits.erase(
        std::remove_if(recruits.begin(), recruits.end(),
            [playerPtr](ObjectGuid const& guid) {
                Creature* creature = ObjectAccessor::GetCreature(*playerPtr, guid);
                return !creature || !creature->IsInWorld() || !creature->IsAlive();
            }),
        recruits.end()
    );

    return recruits.size();
}

// Fonction pour vérifier si une créature appartient à la guilde du joueur
bool IsCreatureFromPlayerGuild(Player* player, Creature* creature)
{
    if (!player || !creature)
        return false;

    // Vérifier si le joueur est dans une guilde
    if (player->GetGuildId() == 0)
        return false;

    // Obtenir la guilde du joueur
    Guild* guild = sGuildMgr->GetGuildById(player->GetGuildId());
    if (!guild)
        return false;

    // Vérifier si la créature a un custom subname (nom de guilde)
    if (!creature->HasCustomSubName())
        return false;

    // Comparer le nom de la guilde avec le custom subname de la créature
    std::string guildName = guild->GetName();
    std::string creatureSubName = creature->GetCustomSubName();

    return guildName == creatureSubName;
}

// Fonction pour recruter une créature
bool RecruitCreature(Player* player, Creature* creature)
{
    if (!player || !creature)
        return false;

    // Vérifier si la créature appartient à la guilde du joueur
    if (!IsCreatureFromPlayerGuild(player, creature))
    {
        ChatHandler(player->GetSession()).PSendSysMessage("Cette créature n'appartient pas à votre guilde.");
        return false;
    }

    // Vérifier si le joueur a déjà 3 recrues
    uint8 recruitCount = GetRecruitCount(player);
    if (recruitCount >= MAX_RECRUITS)
    {
        ChatHandler(player->GetSession()).PSendSysMessage("Vous avez déjà {} recrues. Libérez-en une pour en recruter une nouvelle.", MAX_RECRUITS);
        return false;
    }

    // Vérifier si la créature est déjà recrutée
    auto it = s_playerRecruits.find(player->GetGUID());
    if (it != s_playerRecruits.end())
    {
        for (ObjectGuid const& guid : it->second)
        {
            if (guid == creature->GetGUID())
            {
                ChatHandler(player->GetSession()).PSendSysMessage("Cette créature vous suit déjà.");
                return false;
            }
        }
    }

    // Ajouter la créature à la liste des recrues
    s_playerRecruits[player->GetGUID()].push_back(creature->GetGUID());

    // Obtenir l'index de la recrue (0, 1, ou 2)
    uint8 recruitIndex = recruitCount;
    float angle = GetRecruitAngle(recruitIndex);

    // Pour la position 0 (devant), on n'utilise pas de follow strict pour permettre le mouvement libre
    // Le repositionnement sera géré par AllCreatureScript quand il s'arrête
    // Pour les autres positions, on utilise le follow normal
    if (recruitIndex == 0)
    {
        // Position 0 : pas de follow strict, le PNJ peut se déplacer librement
        // Il sera repositionné automatiquement quand il s'arrête
        // On le positionne juste devant le joueur initialement
        Position pos = player->GetPosition();
        bool playerIsMoving = player->HasUnitState(UNIT_STATE_MOVING);
        float x, y, z;
        
        if (playerIsMoving)
        {
            // Joueur en mouvement : positionner derrière
            float behindAngle = player->GetOrientation() + M_PI;
            x = pos.GetPositionX() + FOLLOW_DISTANCE * cos(behindAngle);
            y = pos.GetPositionY() + FOLLOW_DISTANCE * sin(behindAngle);
        }
        else
        {
            // Joueur arrêté : positionner devant
            x = pos.GetPositionX() + FOLLOW_DISTANCE * cos(player->GetOrientation());
            y = pos.GetPositionY() + FOLLOW_DISTANCE * sin(player->GetOrientation());
        }
        z = pos.GetPositionZ();
        
        // Initialiser l'état de mouvement dans la map
        s_playerWasMoving[player->GetGUID()] = playerIsMoving;
        
        creature->GetMotionMaster()->MovePoint(0, x, y, z);
    }
    else
    {
        // Positions 1 et 2 : follow normal
        creature->GetMotionMaster()->MoveFollow(player, FOLLOW_DISTANCE, angle);
    }

    ChatHandler(player->GetSession()).PSendSysMessage("Vous avez recruté {}. ({}/{})", creature->GetName(), recruitCount + 1, MAX_RECRUITS);
    return true;
}

// Fonction pour réassigner les positions de toutes les recrues
void ReassignRecruitPositions(Player* player)
{
    if (!player)
        return;

    auto it = s_playerRecruits.find(player->GetGUID());
    if (it == s_playerRecruits.end())
        return;

    std::vector<ObjectGuid>& recruits = it->second;
    
    // Réassigner les positions pour chaque recrue restante
    for (size_t i = 0; i < recruits.size(); ++i)
    {
        Creature* recruit = ObjectAccessor::GetCreature(*player, recruits[i]);
        if (recruit && recruit->IsInWorld() && recruit->IsAlive())
        {
            if (i == 0)
            {
                // Position 0 : repositionner derrière le joueur (pas de follow strict)
                Position pos = player->GetPosition();
                float behindAngle = player->GetOrientation() + M_PI;
                float x = pos.GetPositionX() + FOLLOW_DISTANCE * cos(behindAngle);
                float y = pos.GetPositionY() + FOLLOW_DISTANCE * sin(behindAngle);
                float z = pos.GetPositionZ();
                recruit->GetMotionMaster()->MovePoint(0, x, y, z);
            }
            else
            {
                // Positions 1 et 2 : follow normal
                float angle = GetRecruitAngle(i);
                recruit->GetMotionMaster()->MoveFollow(player, FOLLOW_DISTANCE, angle);
            }
        }
    }
}

// Fonction pour libérer une créature
bool ReleaseCreature(Player* player, Creature* creature)
{
    if (!player || !creature)
        return false;

    auto it = s_playerRecruits.find(player->GetGUID());
    if (it == s_playerRecruits.end())
        return false;

    std::vector<ObjectGuid>& recruits = it->second;
    auto recruitIt = std::find(recruits.begin(), recruits.end(), creature->GetGUID());
    
    if (recruitIt == recruits.end())
    {
        ChatHandler(player->GetSession()).PSendSysMessage("Cette créature ne vous suit pas.");
        return false;
    }

    // Arrêter le mouvement proprement
    creature->StopMoving();
    creature->GetMotionMaster()->Clear();
    creature->GetMotionMaster()->MoveIdle();
    creature->StopMovingOnCurrentPos();

    // Retirer de la liste
    recruits.erase(recruitIt);

    // Réassigner les positions de toutes les recrues restantes
    ReassignRecruitPositions(player);

    // Si la liste est vide, retirer le joueur de la map
    if (recruits.empty())
    {
        s_playerRecruits.erase(it);
    }

    ChatHandler(player->GetSession()).PSendSysMessage("Vous avez libéré {}.", creature->GetName());
    return true;
}

// Item script pour le Kit de recrutement
class item_conquest_recruitment_kit : public ItemScript
{
public:
    item_conquest_recruitment_kit() : ItemScript("ConquestRecruitmentKit") { }

    bool OnUse(Player* player, Item* item, SpellCastTargets const& targets) override
    {
        // Vérifier si le joueur est dans une guilde
        if (player->GetGuildId() == 0)
        {
            ChatHandler(player->GetSession()).PSendSysMessage("Vous devez être dans une guilde pour utiliser cet objet.");
            return false;
        }

        // Vérifier si le joueur a une cible
        Unit* target = player->GetSelectedUnit();
        if (!target)
        {
            // Pas de cible : vérifier si le joueur a des recrues
            uint8 recruitCount = GetRecruitCount(player);
            if (recruitCount > 0)
            {
                // Toggle l'état arrêté/follow des recrues
                bool& stopped = s_recruitsStopped[player->GetGUID()];
                stopped = !stopped;

                auto it = s_playerRecruits.find(player->GetGUID());
                if (it != s_playerRecruits.end())
                {
                    std::vector<ObjectGuid>& recruits = it->second;
                    for (ObjectGuid const& guid : recruits)
                    {
                        Creature* recruit = ObjectAccessor::GetCreature(*player, guid);
                        if (recruit && recruit->IsInWorld() && recruit->IsAlive())
                        {
                            if (stopped)
                            {
                                // Arrêter la recrue sur place
                                recruit->StopMoving();
                                recruit->GetMotionMaster()->Clear();
                                recruit->GetMotionMaster()->MoveIdle();
                                recruit->StopMovingOnCurrentPos();
                            }
                            else
                            {
                                // Remettre en follow
                                // Trouver l'index de la recrue
                                int recruitIndex = -1;
                                for (size_t i = 0; i < recruits.size(); ++i)
                                {
                                    if (recruits[i] == guid)
                                    {
                                        recruitIndex = i;
                                        break;
                                    }
                                }

                                if (recruitIndex == 0)
                                {
                                    // Position 0 : repositionner derrière
                                    Position pos = player->GetPosition();
                                    float behindAngle = player->GetOrientation() + M_PI;
                                    float x = pos.GetPositionX() + FOLLOW_DISTANCE * cos(behindAngle);
                                    float y = pos.GetPositionY() + FOLLOW_DISTANCE * sin(behindAngle);
                                    float z = pos.GetPositionZ();
                                    recruit->GetMotionMaster()->MovePoint(0, x, y, z);
                                }
                                else if (recruitIndex > 0)
                                {
                                    // Positions 1 et 2 : follow normal
                                    float angle = GetRecruitAngle(recruitIndex);
                                    recruit->GetMotionMaster()->MoveFollow(player, FOLLOW_DISTANCE, angle);
                                }
                            }
                        }
                    }
                }

                if (stopped)
                {
                    ChatHandler(player->GetSession()).PSendSysMessage("Vos recrues sont maintenant arrêtées sur place.");
                }
                else
                {
                    ChatHandler(player->GetSession()).PSendSysMessage("Vos recrues vous suivent à nouveau.");
                }
                return true;
            }
            else
            {
                ChatHandler(player->GetSession()).PSendSysMessage("Vous devez cibler un PNJ de votre guilde.");
                return false;
            }
        }

        // Vérifier si la cible est une créature
        Creature* creature = target->ToCreature();
        if (!creature)
        {
            ChatHandler(player->GetSession()).PSendSysMessage("Vous devez cibler un PNJ.");
            return false;
        }

        // Vérifier si la créature est vivante
        if (!creature->IsAlive())
        {
            ChatHandler(player->GetSession()).PSendSysMessage("Vous ne pouvez pas recruter une créature morte.");
            return false;
        }

        // Vérifier si la créature est dans la même map
        if (creature->GetMap() != player->GetMap())
        {
            ChatHandler(player->GetSession()).PSendSysMessage("La créature doit être dans la même zone que vous.");
            return false;
        }

        // Vérifier si la créature est déjà recrutée
        auto it = s_playerRecruits.find(player->GetGUID());
        bool isAlreadyRecruited = false;
        if (it != s_playerRecruits.end())
        {
            for (ObjectGuid const& guid : it->second)
            {
                if (guid == creature->GetGUID())
                {
                    isAlreadyRecruited = true;
                    break;
                }
            }
        }

        if (isAlreadyRecruited)
        {
            // Libérer la créature
            ReleaseCreature(player, creature);
        }
        else
        {
            // Recruter la créature
            RecruitCreature(player, creature);
        }

        return true;
    }
};

// AllCreatureScript pour gérer le repositionnement automatique du PNJ en position 0
// et remettre les recrues en follow après le combat
class ConquestRecruitmentAllScript : public AllCreatureScript
{
public:
    ConquestRecruitmentAllScript() : AllCreatureScript("ConquestRecruitmentAllScript") { }

    void OnAllCreatureUpdate(Creature* creature, uint32 diff) override
    {
        if (!creature || !creature->IsAlive() || !creature->IsInWorld())
            return;

        // Vérifier si cette créature est une recrue
        for (auto& playerPair : s_playerRecruits)
        {
            std::vector<ObjectGuid>& recruits = playerPair.second;
            if (recruits.empty())
                continue;

            // Trouver l'index de cette recrue
            int recruitIndex = -1;
            for (size_t i = 0; i < recruits.size(); ++i)
            {
                if (recruits[i] == creature->GetGUID())
                {
                    recruitIndex = i;
                    break;
                }
            }

            if (recruitIndex == -1)
                continue;

            Player* player = ObjectAccessor::GetPlayer(*creature, playerPair.first);
            if (!player || !player->IsInWorld() || player->GetMap() != creature->GetMap())
                continue;

            MovementGeneratorType currentType = creature->GetMotionMaster()->GetCurrentMovementGeneratorType();
            bool isInCombat = creature->IsInCombat();
            bool isMoving = creature->HasUnitState(UNIT_STATE_MOVING);

            // Vérifier si les recrues sont arrêtées
            bool recruitsStopped = s_recruitsStopped[player->GetGUID()];
            if (recruitsStopped)
            {
                // Les recrues sont arrêtées, ne pas les repositionner
                continue;
            }

            if (recruitIndex == 0)
            {
                // Position 0 : gestion spéciale avec timer pour éviter les repositionnements trop fréquents
                uint32& timer = s_recruit0RepositionTimer[creature->GetGUID()];
                if (timer < 1000) // Vérifier toutes les secondes
                {
                    timer += diff;
                }
                else
                {
                    timer = 0;

                    // Si la créature n'est pas en combat, la repositionner derrière le joueur
                    if (!isInCombat)
                    {
                        Position currentPlayerPos = player->GetPosition();
                        float behindAngle = player->GetOrientation() + M_PI;
                        float x = currentPlayerPos.GetPositionX() + FOLLOW_DISTANCE * cos(behindAngle);
                        float y = currentPlayerPos.GetPositionY() + FOLLOW_DISTANCE * sin(behindAngle);
                        float z = currentPlayerPos.GetPositionZ();

                        // Calculer la distance du PNJ à la position cible
                        Position creaturePos = creature->GetPosition();
                        float distToTarget = creaturePos.GetExactDist(x, y, z);

                        // Vérifier si la position cible a changé significativement
                        Position targetPos;
                        targetPos.m_positionX = x;
                        targetPos.m_positionY = y;
                        targetPos.m_positionZ = z;

                        auto lastTargetIt = s_recruit0LastTargetPos.find(creature->GetGUID());
                        bool targetPosChanged = true;
                        if (lastTargetIt != s_recruit0LastTargetPos.end())
                        {
                            float distToLastTarget = targetPos.GetExactDist(&lastTargetIt->second);
                            targetPosChanged = distToLastTarget > 3.0f; // Seuil de 3 yards pour considérer que la position a changé
                        }

                        // Repositionner seulement si :
                        // 1. Le PNJ n'est pas encore initialisé (pas en mouvement vers un point)
                        // 2. OU le PNJ est très loin de sa position cible (> 8 yards) ET est arrêté
                        // 3. OU la position cible a changé significativement ET le PNJ est arrêté
                        bool shouldReposition = false;
                        if (currentType != POINT_MOTION_TYPE)
                        {
                            shouldReposition = true; // Pas encore initialisé
                        }
                        else if (distToTarget > 8.0f && creature->IsStopped() && !isMoving)
                        {
                            shouldReposition = true; // Très loin de la cible
                        }
                        else if (targetPosChanged && creature->IsStopped() && !isMoving)
                        {
                            shouldReposition = true; // Position cible a changé
                        }

                        if (shouldReposition)
                        {
                            creature->GetMotionMaster()->MovePoint(0, x, y, z);
                            s_recruit0LastTargetPos[creature->GetGUID()] = targetPos; // Mettre à jour la dernière position cible
                        }
                    }
                }
            }
            else
            {
                // Positions 1 et 2 : remettre en follow si nécessaire
                if (!isInCombat && currentType != FOLLOW_MOTION_TYPE && currentType != POINT_MOTION_TYPE)
                {
                    float angle = GetRecruitAngle(recruitIndex);
                    creature->GetMotionMaster()->MoveFollow(player, FOLLOW_DISTANCE, angle);
                }
            }
        }
    }
};

void AddSC_conquest_recruitment_kit()
{
    new item_conquest_recruitment_kit();
    new ConquestRecruitmentAllScript();
}

