/*
 * Conquest Frontline — Kill Streak tracker (Phase 5).
 *
 * Compteur de kills consécutifs sans mourir. Persisté en DB pour survivre aux reconnexions
 * (et au max_streak du scoreboard à venir). Reset sur mort. À 5/10 kills, on applique
 * une aura visible "On Fire" / "Prestige" qui marque le joueur comme cible prioritaire.
 */

#ifndef CONQUEST_KILL_STREAK_H
#define CONQUEST_KILL_STREAK_H

#include "Define.h"

class Player;

namespace ConquestKillStreak
{
    // Appelé sur PVP kill cross-faction. Incrémente le streak, applique aura aux paliers.
    void OnKill(Player* killer);

    // Appelé sur mort du joueur. Reset le streak et retire les auras.
    void OnDeath(Player* victim);

    // Lecture (DB sync — usage modéré).
    uint32 GetCurrentStreak(uint32 guid);
    uint32 GetMaxStreak(uint32 guid);

    // True si la victime portait une aura "On Fire" / "Prestige" au moment du kill.
    // Permet à la couche Champion Marks de décider du nombre de drops.
    bool IsOnFire(Player const* player);
    bool IsPrestige(Player const* player);
}

#endif // CONQUEST_KILL_STREAK_H
