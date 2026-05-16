# Guide de Test - GvG Loot Drop Module

## Prérequis

1. Serveur AzerothCore compilé avec le module mod-gvg-lootdrop
2. Base de données world avec le gameobject 184821 (via le script SQL fourni)
3. Configuration activée : `GvGLootDrop.Enable = 1` dans `gvg_core.conf`
4. Au moins 2 comptes joueurs pour tester

## Procédure de Test

### Test 1 : Spawn du sac à la mort

1. **Connectez-vous** avec le joueur de test 1
2. **Équipez** quelques items sur le personnage (armes, armure, etc.)
3. **Placez** quelques items dans le sac à dos
4. **Tuez** le joueur (utilisez `.die` en GM ou faites-le tuer par un monstre/autre joueur)
5. **Vérifiez** qu'un sac apparaît à la position du joueur mort

**Résultat attendu :**
- Un gameobject "Sac de butin du joueur" apparaît à l'emplacement exact de la mort
- Le sac est visible et cliquable

### Test 2 : Contenu du sac

1. **Tuez** un joueur qui a :
   - Items équipés (armes, armure)
   - Items dans le sac à dos
   - Items dans les sacs additionnels
2. **Cliquez** sur le sac spawné
3. **Vérifiez** le contenu du loot

**Résultat attendu :**
- Tous les items du joueur mort sont présents dans le loot
- Les quantités (stacks) sont correctes
- Les propriétés des items (enchantements, etc.) sont préservées

### Test 3 : Accès au loot (Free for All)

1. **Tuez** le joueur 1
2. **Connectez** le joueur 2 (différent)
3. **Faites** cliquer le joueur 2 sur le sac
4. **Tentez** de looter les items

**Résultat attendu :**
- Le joueur 2 peut voir tous les items du sac
- Le joueur 2 peut looter tous les items sans restriction
- Le loot est en mode "free for all"

### Test 4 : Durée de vie du sac (15 minutes)

1. **Tuez** un joueur pour spawner un sac
2. **Notez** l'heure de spawn
3. **Attendez** 15 minutes (ou utilisez `.gobject delete` pour accélérer)
4. **Vérifiez** que le sac disparaît automatiquement

**Résultat attendu :**
- Le sac reste visible pendant exactement 15 minutes
- Après 15 minutes, le sac despawn automatiquement
- Les items non lootés disparaissent avec le sac

### Test 5 : Restrictions (Battlegrounds/Arenas)

1. **Entrez** dans un champ de bataille ou une arène
2. **Tuez** un joueur dans le BG/Arena
3. **Vérifiez** qu'aucun sac n'apparaît

**Résultat attendu :**
- Aucun sac ne spawn dans les battlegrounds
- Aucun sac ne spawn dans les arènes
- Le système ne fonctionne que dans le monde normal

### Test 6 : Items spéciaux

1. **Équipez** des items avec :
   - Enchantements
   - Gemmes (sockets)
   - Propriétés aléatoires ("of the Eagle", etc.)
2. **Mourez** avec ces items
3. **Vérifiez** que ces propriétés sont préservées dans le loot

**Résultat attendu :**
- Les enchantements sont préservés
- Les gemmes sont présentes
- Les propriétés aléatoires sont correctes

### Test 7 : Stacks d'items

1. **Placez** plusieurs stacks d'items dans l'inventaire (potions, flèches, etc.)
2. **Mourez**
3. **Vérifiez** que tous les stacks sont lootables

**Résultat attendu :**
- Tous les stacks apparaissent dans le loot
- Les quantités sont exactes

### Test 8 : Désactivation du module

1. **Configurez** `GvGLootDrop.Enable = 0`
2. **Redémarrez** le serveur
3. **Tuez** un joueur
4. **Vérifiez** qu'aucun sac n'apparaît

**Résultat attendu :**
- Quand le module est désactivé, aucun sac ne spawn
- Le comportement par défaut d'AzerothCore est restauré

## Commandes GM Utiles

```
.die                          // Tue le personnage ciblé
.gobject near                 // Liste les gameobjects proches
.gobject delete               // Supprime le gameobject sélectionné
.additem [itemID] [quantity]  // Ajoute un item pour le test
.level 80                     // Monte le niveau pour équiper plus d'items
```

## Problèmes Connus / Limitations

1. **Sacs vides** : Les sacs d'inventaire eux-mêmes ne sont pas lootables (seulement leur contenu)
2. **Items de quête** : Certains items de quête peuvent ne pas être lootables s'ils sont liés
3. **Performance** : Si un joueur a beaucoup d'items, la génération du loot peut prendre un instant

## Logs de Débogage

Pour activer les logs de débogage, ajoutez dans `worldserver.conf` :

```
Logger.module=4,Console Server
```

Cherchez dans les logs :
- `GvGLootDrop: Added X items to loot bag for player Y`
- `GvGLootDrop: Failed to spawn loot bag for player X`

## Rapport de Bug

Si vous trouvez un bug, veuillez inclure :
1. Version d'AzerothCore
2. Logs du serveur
3. Étapes pour reproduire le bug
4. Comportement attendu vs comportement observé
5. Configuration du module

## Notes

- Le gameobject entry 184821 doit exister dans la base de données
- Le module ne fonctionne que pour les joueurs, pas pour les PNJs
- Le loot est généré au moment de la mort, pas à l'ouverture du sac

