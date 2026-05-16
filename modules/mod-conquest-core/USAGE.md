# Utilisation du Module GvG Core

## Comment fonctionne le module ?

Le module permet le **PvP entre guildes peu importe la faction**. Par défaut, toutes les guildes différentes sont considérées comme hostiles.

## Comportement par défaut

### ✅ PvP autorisé entre :
- Joueurs de guildes différentes (même faction ou factions opposées)
- Le système force l'hostilité automatiquement

### ❌ PvP bloqué entre :
- Joueurs de la même guilde
- Joueurs sans guilde
- Si vous définissez des relations amicales (voir ci-dessous)

## Gestion des relations entre guildes

### Voir les relations
```
.gvg relation list
```

### Définir une relation
```
.gvg relation set <guildA_ID> <guildB_ID> <relation>
```

**Valeurs de relation :**
- `-1` = Hostile (guerre active)
- `0` = Neutre (pas d'hostilité forcée)
- `1` = Amical (alliés, pas d'attaque possible)

### Exemples

**Déclarer la guerre entre deux guildes :**
```
.gvg relation set 1 2 -1
```

**Créer une alliance (empêcher le PvP) :**
```
.gvg relation set 1 2 1
```

**Retirer toute relation spéciale :**
```
.gvg relation set 1 2 0
```

### Recharger les relations
```
.gvg reload
```
Force le rechargement des relations depuis la base de données.

## Statistiques

Le module track automatiquement :
- Les kills de chaque joueur en PvP inter-guilde
- Les morts de chaque joueur
- Les statistiques par guilde (prochainement)

## Configuration

Dans `gvg_core.conf` :
```
GvgCore.Enable = 1   # 1 pour activer, 0 pour désactiver
```

## Note importante

⚠️ **Pour que le PvP fonctionne, les joueurs doivent être dans des guildes différentes !**

Si vos deux personnages ne peuvent pas s'attaquer :
1. Vérifiez qu'ils sont bien dans des guildes différentes : `.guild info`
2. Vérifiez que le module est activé : vérifiez les logs au démarrage du serveur
3. Rechargez les relations : `.gvg reload`
4. Vérifiez qu'il n'y a pas de relation amicale : `.gvg relation list`

