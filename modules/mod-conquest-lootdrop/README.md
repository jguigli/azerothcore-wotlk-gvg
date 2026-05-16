# GvG Loot Drop Module

## Description

Ce module permet de looter les corps des joueurs morts. Quand un joueur meurt, un sac apparaît à sa position contenant tous ses items avec leurs propriétés complètes (enchantements, gemmes, durabilité).

## Fonctionnalités

- **Spawn automatique d'un sac** : Quand un joueur meurt, un gameobject "sac" (entry 400002) spawn à sa position
- **Contenu du sac** : Le sac contient TOUS les items du joueur mort :
  - Items équipés (armes, armure, bijoux)
  - Items dans le sac à dos
  - Items dans tous les sacs d'inventaire
- **Conservation des propriétés** : Tous les items conservent leurs propriétés complètes :
  - **Enchantements** : Tous les enchantements permanents et temporaires
  - **Gemmes** : Toutes les gemmes serties dans les items
  - **Durabilité** : La durabilité actuelle de l'item
  - **Propriétés aléatoires** : Les propriétés aléatoires et suffixes
- **Durée de vie** : 
  - Le sac reste visible pendant 15 minutes puis disparaît automatiquement
  - **Si tout est looté** : Le sac disparaît immédiatement après que tous les items ont été pris
- **Loot ouvert** : N'importe qui peut ouvrir le sac et looter les items
- **Interface familière** : Le sac s'ouvre comme un coffre classique avec une interface de loot
- **Réouverture possible** : Le sac peut être rouvert plusieurs fois tant qu'il reste des items
- **Sacs équipés exclus** : Les sacs équipés (dans les slots de sacs) ne sont pas lootables, seulement leur contenu

## Configuration

Le module peut être configuré dans le fichier `gvg_lootdrop.conf.dist`:

```
GvGLootDrop.Enable = 1
```

- **1** : Module activé (défaut)
- **0** : Module désactivé

## Restrictions

- Le système ne fonctionne **PAS** dans les champs de bataille (battlegrounds)
- Le système ne fonctionne **PAS** dans les arènes

## Installation

1. Compiler le serveur avec le module activé
2. Copier le fichier de configuration `conf/gvg_lootdrop.conf.dist` vers votre répertoire de configuration
3. S'assurer que le gameobject 400002 existe dans votre base de données (voir `data/sql/db-world/gvg_lootdrop_gameobject.sql`)
4. Redémarrer le serveur

## Détails techniques

### Gameobject utilisé

- **Entry** : 400002
- **Type** : GAMEOBJECT_TYPE_CHEST (3)
- **DisplayId** : 323 (Money Bag)
- **Nom** : "Sac de butin du joueur" / "Player Loot Bag"

### Hooks utilisés

- `OnPlayerJustDied` : Détecte la mort du joueur et spawn le sac
- `OnGossipHello` : Gère l'ouverture du sac et l'affichage du loot
- `OnLootStateChanged` : Gère le despawn automatique quand tout est looté
- `OnPlayerLootItem` : Copie les propriétés complètes des items (enchantements, gemmes, durabilité)

### Loot

Le loot est généré dynamiquement et contient :
- Tous les items avec leurs propriétés complètes :
  - Enchantements permanents et temporaires
  - Gemmes serties
  - Durabilité actuelle
  - Propriétés aléatoires et suffixes
- Les stacks d'items
- Les montures sous forme d'items

## Notes

- Si le gameobject 400002 n'existe pas dans votre base de données, le module ne fonctionnera pas
- Le sac apparaît exactement à la position où le joueur est mort
- Le loot est en mode "free for all" - tout le monde peut looter
- Les sacs équipés eux-mêmes ne sont pas lootables (seulement leur contenu)
- Les montures sont ajoutées comme items de monture (ex: "Reins of the Black War Steed")
- Le sac disparaît automatiquement dès que tous les items sont lootés
- Le sac peut être rouvert plusieurs fois tant qu'il reste des items à looter

## Support

Pour tout problème ou suggestion, veuillez créer une issue sur le dépôt du projet.

## License

Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license
