# Changelog - GvG Loot Drop Module

Toutes les modifications notables de ce module seront documentées dans ce fichier.

Le format est basé sur [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
et ce projet adhère au [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.1.0] - 2025-11-23

### Ajouté
- **Despawn automatique quand le sac est vide** : Le sac disparaît immédiatement après que tous les items ont été lootés
- **Support des montures** : Toutes les montures apprises par le joueur sont ajoutées au loot sous forme d'items de monture
- **Exclusion des sacs équipés** : Les sacs équipés (dans les slots de sacs) ne sont pas lootables, seulement leur contenu est copié

### Modifié
- Amélioration de la logique de filtrage des items pour exclure les sacs équipés
- Ajout d'un script GameObject pour détecter quand le loot est complètement vidé

## [1.0.0] - 2025-11-23

### Ajouté
- **Système de loot de corps de joueur** : Quand un joueur meurt, un sac spawn à sa position contenant tous ses items
- **Gameobject "Sac de butin"** (entry 184821) : Un sac cliquable qui affiche une interface de loot comme un coffre
- **Copie automatique de tous les items** :
  - Items équipés (armure, armes, bijoux, etc.)
  - Items dans le sac à dos principal
  - Items dans tous les sacs additionnels
- **Durée de vie de 15 minutes** : Le sac despawn automatiquement après 15 minutes
- **Mode Free-For-All** : N'importe qui peut ouvrir et looter le sac
- **Exclusion BG/Arena** : Le système ne fonctionne pas dans les champs de bataille et arènes
- **Configuration via fichier conf** : Option `GvGLootDrop.Enable` pour activer/désactiver le module
- **Préservation des propriétés des items** :
  - Propriétés aléatoires (ex: "of the Eagle")
  - Suffixes aléatoires
  - Quantités dans les stacks
- **Logs de débogage** : Information sur le nombre d'items ajoutés et les erreurs éventuelles

### Documentation
- **README.md** : Guide d'utilisation et installation du module
- **TESTING.md** : Guide de test complet avec 8 scénarios de test
- **TECHNICAL.md** : Documentation technique pour les développeurs
- **CHANGELOG.md** : Ce fichier

### Scripts SQL
- **gvg_lootdrop_gameobject.sql** : Création du gameobject template pour le sac de butin

### Technique
- **Hook PlayerScript::OnPlayerJustDied** : Détection de la mort du joueur
- **SummonGameObject** : Spawn dynamique du sac à la position du joueur mort
- **Loot dynamique** : Génération du loot sans template database
- **Itération complète des items** : Parcours de tous les slots d'inventaire du joueur

### Limitations Connues
- Les sacs eux-mêmes ne sont pas copiés dans le loot (seulement leur contenu)
- Certaines propriétés des items ne sont pas préservées :
  - Durabilité
  - Charges restantes
  - Enchantements temporaires
  - État de liaison (sera bind on pickup pour le nouveau propriétaire)

### Configuration
```ini
GvGLootDrop.Enable = 1  # Activé par défaut
```

### Compatibilité
- **AzerothCore** : master branch (WOTLK 3.3.5a)
- **Gameobject entry** : 184821 (doit exister dans la database)

---

## [Non publié] - Idées pour Versions Futures

### À Considérer pour v1.1.0
- [ ] Option pour rendre le loot personnel (seulement le tueur peut looter)
- [ ] Option pour filtrer les items de faible valeur
- [ ] Système de protection contre l'exploitation (suicide volontaire)
- [ ] Support de la durabilité des items
- [ ] Message de notification aux joueurs proches
- [ ] Statistiques de loot (nombre de sacs, items lootés, etc.)

### À Considérer pour v1.2.0
- [ ] Configuration de la durée de vie du sac (actuellement fixe à 15 min)
- [ ] Configuration du modèle de gameobject (actuellement fixe)
- [ ] Système de loot différé (copie à l'ouverture du sac plutôt qu'à la mort)
- [ ] Support des items dans la banque (optionnel)
- [ ] Filtres par type d'item (armes uniquement, etc.)

### À Considérer pour v2.0.0
- [ ] Système de coffre-fort personnel (au lieu de free-for-all)
- [ ] Intégration avec un système de guildes/factions
- [ ] Temps de décomposition basé sur la valeur des items
- [ ] Plusieurs sacs si trop d'items
- [ ] Interface de réclamation d'items (récupérer ses items depuis le sac)

---

## Format des Entrées

### Types de Changements
- **Ajouté** : Nouvelles fonctionnalités
- **Modifié** : Changements dans les fonctionnalités existantes
- **Déprécié** : Fonctionnalités qui seront retirées
- **Retiré** : Fonctionnalités retirées
- **Corrigé** : Corrections de bugs
- **Sécurité** : Vulnérabilités corrigées

### Exemple d'Entrée Future
```markdown
## [1.1.0] - YYYY-MM-DD

### Ajouté
- Option pour rendre le loot personnel

### Corrigé
- Bug où les stacks d'items n'étaient pas correctement copiés
```

---

## Support

Pour rapporter un bug ou suggérer une amélioration, veuillez créer une issue sur le dépôt du projet.

## License

Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license

