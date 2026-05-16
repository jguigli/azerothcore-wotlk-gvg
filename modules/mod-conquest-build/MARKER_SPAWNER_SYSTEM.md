# Système de Marqueur et Spawner GvG

## Description

Ce système permet de :
1. Utiliser un **Kit de marqueur** (item) pour spawner un **Marqueur de spawn** (NPC)
2. Le **Spawner** (NPC) vérifie toutes les 10 secondes s'il y a un marqueur à proximité
3. Si un marqueur est trouvé, le Spawner fait spawner **5 créatures aléatoirement** de la liste [400300 à 400305]
4. Les créatures spawnées se déplacent automatiquement vers le marqueur avec **pathfinding**

## Entries

- **Item Kit de marqueur** : `80040`
- **NPC Marqueur de spawn** : `400103` (displayid: 20577)
- **NPC Spawner** : `400102` (displayid: 7109)
- **Créatures à spawner** : `400300` à `400305`

## Installation

### 1. Exécuter le SQL

Exécuter le fichier SQL dans la base de données `acore_world` :

```bash
mysql -u root -p acore_world < modules/mod-gvg-build/data/sql/db-world/gvg_marker_spawner.sql
```

Ou via docker :

```bash
docker exec -i ac-database mysql -u root -ppassword acore_world < modules/mod-gvg-build/data/sql/db-world/gvg_marker_spawner.sql
```

### 2. Compiler le module

Le code C++ est déjà intégré dans `GvGMarkerSpawner.cpp`. Il suffit de compiler le module.

## Utilisation

### 1. Obtenir le Kit de marqueur

Le joueur doit avoir l'item `80040` (Kit de marqueur de spawn) dans son inventaire.

### 2. Spawner le Marqueur

- Utiliser l'item (clic droit)
- Le marqueur sera spawné à la position du joueur
- L'item sera consommé (usage unique)

### 3. Placer le Spawner

Le Spawner (NPC 400102) doit être spawné dans le monde via la base de données ou via commande GM :

```sql
-- Exemple de spawn du Spawner
INSERT INTO `creature` (`guid`, `id1`, `id2`, `id3`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`, `ScriptName`, `VerifiedBuild`, `CreateObject`, `Comment`) VALUES
(1, 400102, 0, 0, 571, 0, 0, 1, 1, 0, 2281.34, 5245.08, 11.3994, 0.174533, 300, 0, 0, 1, 0, 0, 0, 0, 0, '', 0, 0, NULL);
```

### 4. Fonctionnement automatique

Une fois le Spawner placé :
- Toutes les **10 secondes**, il vérifie s'il y a un marqueur dans un rayon de **100 yards**
- Si un marqueur est trouvé, il spawn **5 créatures aléatoirement** de la liste [400300 à 400305]
- Les créatures sont spawnées autour du Spawner (en cercle)
- Chaque créature se déplace automatiquement vers le marqueur avec **pathfinding**
- Si aucun marqueur n'est trouvé, le Spawner ne fait rien

## Configuration

Les paramètres peuvent être modifiés dans `GvGMarkerSpawner.cpp` :

```cpp
#define ITEM_MARKER_KIT           80040
#define NPC_MARKER                400103
#define NPC_SPAWNER               400102
#define NPC_SPAWN_MIN             400300
#define NPC_SPAWN_MAX             400305
#define SPAWN_INTERVAL            10000  // 10 secondes en millisecondes
#define SPAWN_COUNT_PER_WAVE      5
#define MARKER_SEARCH_RANGE       100.0f  // Rayon de recherche du marqueur
```

## Détails techniques

### Pathfinding

Les créatures utilisent le pathfinding pour se déplacer vers le marqueur :
- Évitent les obstacles
- Suivent le terrain
- Calculent le chemin optimal via NavMesh

### Spawn aléatoire

Les créatures sont choisies aléatoirement parmi :
- 400300 : Brute ogre
- 400301 : Ogre-mage
- 400302 : Ecraseur ogre
- 400303 : Chaman ogre
- 400304 : Massacreur ogre
- 400305 : Démoniste ogre

### Position de spawn

Les créatures sont spawnées en cercle autour du Spawner avec un rayon de 3 yards.

## Notes importantes

1. **Le Spawner doit être spawné manuellement** dans le monde (via SQL ou commande GM)
2. **Le marqueur est spawné via l'item** et peut être supprimé/détruit
3. **Si le marqueur est détruit**, le Spawner ne spawnera plus de créatures
4. **Les créatures spawnées se déplacent automatiquement** vers le marqueur
5. **Le système fonctionne en continu** tant qu'un marqueur est présent

## Exemple d'utilisation

1. Un joueur utilise le Kit de marqueur → Un marqueur apparaît
2. Le Spawner détecte le marqueur (toutes les 10 secondes)
3. Le Spawner fait spawner 5 créatures aléatoirement
4. Les créatures se déplacent vers le marqueur
5. Le processus se répète toutes les 10 secondes tant que le marqueur existe

