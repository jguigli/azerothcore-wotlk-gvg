/*
 * Conquest Frontline — Wallet joueur (Points de Conquête + Points de Bataille)
 *
 * API simple en namespace, lecture/écriture directes en DB (acore_characters).
 * Notification chat automatique sur Add quand le joueur est en ligne.
 */

#ifndef CONQUEST_POINTS_H
#define CONQUEST_POINTS_H

#include "Define.h"

class Player;

namespace ConquestPoints
{
    // Lectures (DB sync — usage modéré, ex: ouverture UI vendor)
    uint32 GetConquestPoints(uint32 guid);
    uint32 GetBattlePoints(uint32 guid);

    // Ajouts (DB async, notification chat verte/orange)
    void AddConquestPoints(Player* player, uint32 amount);
    void AddBattlePoints(Player* player, uint32 amount);

    // Dépenses (renvoie false si solde insuffisant)
    bool SpendConquestPoints(Player* player, uint32 amount);
    bool SpendBattlePoints(Player* player, uint32 amount);
}

#endif // CONQUEST_POINTS_H
