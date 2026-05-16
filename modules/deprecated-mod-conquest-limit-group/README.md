# ![logo](https://raw.githubusercontent.com/azerothcore/azerothcore.github.io/master/images/logo-github.png) AzerothCore

## mod-gvg-limit-group

Module pour AzerothCore qui permet de limiter la taille des guildes et des groupes de joueurs.

## Description

Ce module implémente deux fonctionnalités principales :

1. **Limitation des guildes** : Limite le nombre maximum de membres dans une guilde
2. **Limitation des groupes** : Limite le nombre maximum de joueurs dans un groupe et permet d'empêcher la création de raids

## Fonctionnalités

### Limitation des Guildes

- Définit un nombre maximum de membres par guilde (par défaut : 30)
- Empêche l'ajout de nouveaux membres une fois la limite atteinte
- Affiche des messages informatifs aux joueurs concernés

### Limitation des Groupes

- Définit un nombre maximum de joueurs par groupe (par défaut : 5)
- Option pour désactiver complètement les groupes raid
- Empêche les joueurs de rejoindre des groupes complets
- Affiche des messages d'erreur clairs

## Installation

1. Placez le module dans le dossier `modules/` de votre serveur AzerothCore
2. Recompilez le serveur
3. Configurez les paramètres dans `gvg_core.conf`

## Configuration

Le module se configure via le fichier `gvg_core.conf.dist` (à copier en `gvg_core.conf`) :

```conf
########################################
# GvG Limit Group Module Configuration
########################################

# Active ou désactive le module
GvGLimitGroup.Enable = 1

# Nombre maximum de membres dans une guilde
GvGLimitGroup.MaxGuildMembers = 30

# Nombre maximum de joueurs dans un groupe
GvGLimitGroup.MaxGroupSize = 5

# Autoriser les groupes raid (0 = non, 1 = oui)
GvGLimitGroup.AllowRaids = 0
```

### Paramètres de configuration

| Paramètre | Type | Défaut | Description |
|-----------|------|--------|-------------|
| `GvGLimitGroup.Enable` | bool | 1 | Active/désactive le module |
| `GvGLimitGroup.MaxGuildMembers` | uint32 | 30 | Nombre maximum de membres par guilde |
| `GvGLimitGroup.MaxGroupSize` | uint32 | 5 | Nombre maximum de joueurs par groupe |
| `GvGLimitGroup.AllowRaids` | bool | 0 | Autorise ou non les groupes raid |

## Comportement

### Guildes

Quand un joueur tente de rejoindre ou d'être invité dans une guilde :
- Si la guilde a atteint la limite, l'invitation est refusée
- Le joueur invité reçoit un message : "Cette guilde a atteint la limite maximale de X membres."
- Le joueur qui invite reçoit un message : "Impossible d'ajouter [nom]. La guilde a atteint la limite maximale de X membres."

### Groupes

Quand un joueur tente de rejoindre un groupe :
- Si le groupe a atteint la limite, l'invitation est refusée
- Le joueur reçoit un message : "Ce groupe a atteint la limite maximale de X joueurs."
- Le leader du groupe reçoit également un message

Si les raids sont désactivés (`GvGLimitGroup.AllowRaids = 0`) :
- Les joueurs ne peuvent pas convertir un groupe en raid
- Les joueurs ne peuvent pas rejoindre un groupe raid
- Message d'erreur : "Les groupes raid ne sont pas autorisés sur ce serveur."

## Cas d'usage

Ce module est particulièrement utile pour :
- Les serveurs PvP avec focus sur les petits groupes
- Les serveurs GvG (Guild vs Guild) avec des guildes de taille limitée
- Les serveurs qui veulent encourager le contenu en petit groupe
- Les serveurs qui veulent équilibrer les combats entre guildes

## Compatibilité

- AzerothCore (branche master)
- Testé avec la version 3.3.5a (WotLK)

## Crédits

- Module développé pour GvG Server
- Basé sur AzerothCore Module Skeleton

## Licence

Ce module est sous licence [GNU AGPL v3](https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3)

## Support

Pour tout problème ou suggestion, veuillez ouvrir une issue sur le dépôt du projet.

