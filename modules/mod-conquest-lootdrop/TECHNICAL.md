# Documentation Technique - GvG Loot Drop Module

## Architecture du Module

### Vue d'ensemble

Le module `mod-gvg-lootdrop` intercepte l'événement de mort d'un joueur et crée dynamiquement un gameobject contenant tous les items du joueur décédé. Le système utilise les APIs natives d'AzerothCore pour gérer le loot de manière standard.

### Composants Principaux

#### 1. GvGLootDropPlayer (PlayerScript)

**Fichier :** `src/GvGLootDrop.cpp`

Cette classe hérite de `PlayerScript` et hook l'événement `OnPlayerJustDied`.

**Flux d'exécution :**

```
OnPlayerJustDied(Player* player)
    ↓
Vérification de la configuration (GvGLootDrop.Enable)
    ↓
Vérification du contexte (pas de BG/Arena)
    ↓
SummonGameObject(LOOT_BAG_ENTRY, position, respawnTime)
    ↓
FillLootBagWithPlayerItems(player, lootBag)
    ↓
Itération sur tous les items du joueur
    ↓
AddItemToLoot(item, loot) pour chaque item
    ↓
Mise à jour du unlootedCount
```

#### 2. Fonction FillLootBagWithPlayerItems

Cette fonction est responsable de :
- Nettoyer le loot existant du gameobject
- Définir le propriétaire du loot (lootOwnerGUID)
- Itérer sur tous les slots d'items du joueur :
  - `EQUIPMENT_SLOT_START` à `EQUIPMENT_SLOT_END` : Items équipés
  - `INVENTORY_SLOT_ITEM_START` à `INVENTORY_SLOT_ITEM_END` : Sac à dos
  - `INVENTORY_SLOT_BAG_START` à `INVENTORY_SLOT_BAG_END` : Sacs additionnels
- Appeler `AddItemToLoot` pour chaque item

#### 3. Fonction AddItemToLoot

Cette fonction crée un `LootItem` pour chaque item du joueur :

**Structure LootItem :**
```cpp
struct LootItem {
    uint32  itemid;              // ID de l'item
    uint32  itemIndex;           // Index dans la liste de loot
    uint32  randomSuffix;        // Suffixe aléatoire (ex: "of the Eagle")
    int32   randomPropertyId;    // Propriété aléatoire
    uint8   count;               // Quantité
    bool    is_looted;           // Déjà looté?
    bool    is_blocked;          // Bloqué pour roll?
    bool    freeforall;          // Lootable par tous
    bool    is_underthreshold;   // Sous le seuil de qualité
    bool    is_counted;          // Compté dans unlootedCount
    bool    needs_quest;         // Item de quête
    bool    follow_loot_rules;   // Suit les règles de loot de groupe
    uint8   groupid;             // ID de groupe (pour loot groupé)
    ConditionList conditions;    // Conditions additionnelles
    AllowedLooterSet allowedGUIDs; // GUIDs autorisés à looter
    ObjectGuid rollWinnerGUID;   // Gagnant du roll
};
```

**Configuration du LootItem :**
- `freeforall = true` : Tout le monde peut looter
- `follow_loot_rules = false` : Ne suit pas les règles de groupe
- `needs_quest = false` : N'est pas un item de quête
- `is_counted = true` : Compté dans le total d'items

## Détails Techniques

### Spawn du Gameobject

```cpp
GameObject* lootBag = player->SummonGameObject(
    LOOT_BAG_ENTRY,    // 184821
    x, y, z, o,        // Position du joueur mort
    0, 0, 0, 0,        // Quaternions de rotation (pas de rotation)
    LOOT_BAG_DESPAWN_TIME  // 900 secondes (15 minutes)
);
```

**Note :** `SummonGameObject` avec un `respawnTime` crée un gameobject temporaire qui despawn automatiquement après le délai spécifié.

### Gestion du Loot

Le loot est géré via la structure `Loot` native d'AzerothCore :

```cpp
class Loot {
    std::vector<LootItem> items;      // Items lootables
    std::vector<LootItem> quest_items; // Items de quête
    ObjectGuid lootOwnerGUID;          // Propriétaire du loot
    uint32 unlootedCount;              // Nombre d'items non lootés
    // ...
};
```

**Accès au loot :**
- Le loot est accessible via `gameObject->loot`
- Le `lootOwnerGUID` est défini sur le joueur mort pour le tracking
- `unlootedCount` est mis à jour pour afficher le nombre d'items dans le sac

### Itération sur les Items du Joueur

**Slots d'équipement :**
```cpp
EQUIPMENT_SLOT_START = 0
EQUIPMENT_SLOT_END = 19
// Inclut : tête, cou, épaules, etc.
```

**Slots d'inventaire (sac à dos) :**
```cpp
INVENTORY_SLOT_ITEM_START = 23
INVENTORY_SLOT_ITEM_END = 39
// 16 slots du sac à dos principal
```

**Slots de sacs :**
```cpp
INVENTORY_SLOT_BAG_START = 19
INVENTORY_SLOT_BAG_END = 23
// 4 sacs additionnels
```

Pour chaque sac, on itère sur `bag->GetBagSize()` slots.

### Exclusions

**Items non copiés :**
- Les sacs eux-mêmes (via `item->IsBag()`)
- Les items null ou invalides

**Raison :** Les sacs ne peuvent pas être correctement représentés dans le système de loot standard sans leur contenu.

## Configuration

### Fichier de configuration : `conf/gvg_core.conf.dist`

```ini
GvGLootDrop.Enable = 1
```

**Lecture dans le code :**
```cpp
if (!sConfigMgr->GetOption<bool>("GvGLootDrop.Enable", true))
    return;
```

### Base de données : Gameobject Template

Le gameobject 184821 doit exister dans `gameobject_template` :

```sql
INSERT INTO `gameobject_template` (
    `entry`, 
    `type`,      -- 3 = GAMEOBJECT_TYPE_CHEST
    `displayId`, -- 5424 = Model de sac
    `name`,
    -- ...
) VALUES (184821, 3, 5424, 'Sac de butin du joueur', ...);
```

**Type CHEST (3) :** Permet au gameobject d'être utilisé comme conteneur de loot, similaire aux coffres.

## Hooks AzerothCore Utilisés

### PlayerScript::OnPlayerJustDied

**Appelé par :** `Unit::DealDamage` → `Unit::JustDied` → `ScriptMgr::OnPlayerJustDied`

**Paramètres :**
- `Player* player` : Le joueur qui vient de mourir

**Moment d'appel :** Immédiatement après que le joueur meurt, avant la création du corpse.

## Limitations et Considérations

### 1. Performance

**Impact :** Pour un joueur avec beaucoup d'items (équipement complet + 4 sacs pleins), la création peut prendre ~1ms.

**Optimisation possible :**
- Pré-réserver la capacité du vecteur : `loot.items.reserve(MAX_EXPECTED_ITEMS)`
- Filtrer les items de faible valeur si nécessaire

### 2. Propriétés des Items

**Préservées :**
- `randomPropertyId` : Propriétés aléatoires
- `randomSuffix` : Suffixe aléatoire
- `count` : Quantité dans le stack

**Non préservées :**
- Durabilité des items
- Charges restantes
- Enchantements temporaires
- État de liaison (bind)

**Note :** Pour préserver ces propriétés, il faudrait utiliser un système de stockage d'items personnalisé plutôt que le système de loot.

### 3. Sécurité

**Exploitation potentielle :**
- Un joueur pourrait se suicider pour transférer des items à un autre joueur
- Solution : Ajouter une vérification de "vraie mort en combat"

**Exemple de vérification :**
```cpp
// Vérifier si le joueur a été tué par un autre joueur
if (player->GetKiller() && player->GetKiller()->IsPlayer()) {
    // C'est une vraie mort PvP
}
```

### 4. Compatibilité

**Versions testées :**
- AzerothCore master branch (WOTLK 3.3.5a)

**Modules compatibles :**
- Compatible avec la plupart des modules
- Peut entrer en conflit avec des modules qui modifient le système de loot

## Extension du Module

### Ajouter des Filtres

Pour filtrer certains items (ex: items de quête) :

```cpp
void AddItemToLoot(Item* item, Loot& loot)
{
    // Skip quest items
    if (item->GetTemplate()->Class == ITEM_CLASS_QUEST)
        return;
    
    // Votre code existant...
}
```

### Changer le Type de Loot

Pour rendre le loot personnel (non free-for-all) :

```cpp
lootItem.freeforall = false;
lootItem.follow_loot_rules = true;

// Ajouter le joueur comme seul looter autorisé
if (Player* killer = player->GetKiller()->ToPlayer())
{
    lootItem.allowedGUIDs.insert(killer->GetGUID());
}
```

### Modifier la Durée de Vie

Pour changer le temps de despawn :

```cpp
#define LOOT_BAG_DESPAWN_TIME 1800  // 30 minutes au lieu de 15
```

### Ajouter un Message

Pour notifier les joueurs proches :

```cpp
// Après le spawn du sac
std::list<Player*> nearbyPlayers;
player->GetPlayerListInGrid(nearbyPlayers, 50.0f);

for (Player* nearbyPlayer : nearbyPlayers)
{
    ChatHandler(nearbyPlayer->GetSession()).PSendSysMessage(
        "Le corps de %s a été pillé!",
        player->GetName().c_str()
    );
}
```

## Débogage

### Logs Utiles

```cpp
LOG_DEBUG("module", "GvGLootDrop: Player {} died at ({}, {}, {})",
    player->GetName(), x, y, z);

LOG_DEBUG("module", "GvGLootDrop: Added item {} (count: {}) to loot",
    item->GetEntry(), item->GetCount());
```

### Commandes GM pour Test

```bash
# Spawner le gameobject manuellement
.gobject add 184821

# Vérifier le loot
.gobject info

# Supprimer le gameobject
.gobject delete
```

## Références

### AzerothCore APIs

- `Player::SummonGameObject()` : Spawn un gameobject temporaire
- `Player::GetItemByPos()` : Récupère un item à un slot donné
- `Player::GetBagByPos()` : Récupère un sac à un slot donné
- `GameObject::loot` : Structure de loot du gameobject
- `Loot::items` : Liste des items lootables

### Structures de Données

- `LootItem` : `src/server/game/Loot/LootMgr.h`
- `Loot` : `src/server/game/Loot/LootMgr.h`
- `GameObject` : `src/server/game/Entities/GameObject/GameObject.h`
- `Player` : `src/server/game/Entities/Player/Player.h`

## Changelog

### Version 1.0
- Implémentation initiale
- Support des items équipés, inventaire et sacs
- Mode free-for-all
- Despawn automatique après 15 minutes
- Exclusion BG/Arena

