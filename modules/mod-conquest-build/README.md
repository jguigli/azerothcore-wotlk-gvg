# GvG Build Module 🏰

Module pour AzerothCore permettant aux joueurs de construire des structures défensives dans le monde de WoW. Conçu pour le PvP guilde vs guilde (GvG).

## 📋 Table des matières

- [Fonctionnalités](#fonctionnalités)
- [Items disponibles](#items-disponibles)
- [Installation](#installation)
- [Configuration](#configuration)
- [Utilisation](#utilisation)
- [Système de fortification](#système-de-fortification)
- [Base de données](#base-de-données)
- [Commandes utiles](#commandes-utiles)

---

## ✨ Fonctionnalités

### Structures disponibles

- ✅ **Murs** (50,000 PV) - Destructibles
- ✅ **Tours** (100,000 PV) - Destructibles  
- ✅ **Système de fortification** : 2 Tours + 1 Herse + 2 Leviers (200,000 PV)
- ✅ **Outil de récupération** - Récupère les structures placées (15 min max)

### Fonctionnalités avancées

- 🛡️ **Structures destructibles** - Peuvent être détruites par des dégâts (type Wintergrasp)
- 🔒 **Contrôle par guilde** - Seule la guilde propriétaire peut ouvrir/fermer la herse
- 🎯 **Placement précis** - Clic au sol pour placement jusqu'à 30 yards
- ⏰ **Récupération limitée** - Récupération possible pendant les 15 premières minutes uniquement
- 🗑️ **Suppression automatique** - Les structures détruites sont supprimées de la DB
- 📏 **Distance de sécurité** - Empêche de construire trop proche d'autres structures
- 🚫 **Limite par joueur** - Nombre maximum de structures configurables
- 🏗️ **Système de groupe** - Destruction d'une tour = destruction de toute la fortification

---

## 🎒 Items disponibles

| Item ID | Nom | Type | Fonction |
|---------|-----|------|----------|
| **80000** | Kit de Mur GvG | Kit de construction | Spawne un mur destructible (50k PV) |
| **80001** | Kit de Tour GvG | Kit de construction | Spawne une tour destructible (100k PV) |
| **80003** | Kit de Herse GvG | Kit de fortification | Spawne 2 tours + 1 herse + 2 leviers |
| **80002** | Outil de Récupération GvG | Outil permanent | Récupère les structures dans les 15 min |

### Commandes pour obtenir les items

```bash
.additem 80000  # Kit de Mur
.additem 80001  # Kit de Tour
.additem 80003  # Kit de Herse (Fortification)
.additem 80002  # Outil de Récupération
```

---

## 📦 Installation

### 1. Compilation

Le module se compile automatiquement avec AzerothCore. Assurez-vous que le dossier du module est dans `modules/mod-gvg-build/`.

```bash
cd azerothcore-wotlk
mkdir -p modules
cd modules
git clone <votre-repo> mod-gvg-build
cd ..
./acore.sh compiler all
```

### 2. Base de données

Importez les scripts SQL dans la base de données **World** :

```bash
# Base World
mysql -u root -p acore_world < modules/mod-gvg-build/data/sql/db-world/gvg_build_strings.sql
mysql -u root -p acore_world < modules/mod-gvg-build/data/sql/db-world/gvg_build_structures.sql
mysql -u root -p acore_world < modules/mod-gvg-build/data/sql/db-world/gvg_build_item.sql
mysql -u root -p acore_world < modules/mod-gvg-build/data/sql/db-world/gvg_build_gameobjects.sql
mysql -u root -p acore_world < modules/mod-gvg-build/data/sql/db-world/gvg_build_recovery_item.sql
mysql -u root -p acore_world < modules/mod-gvg-build/data/sql/db-world/gvg_build_gate_item.sql
mysql -u root -p acore_world < modules/mod-gvg-build/data/sql/db-world/gvg_build_gate_gameobject.sql
mysql -u root -p acore_world < modules/mod-gvg-build/data/sql/db-world/gvg_build_gate_lever.sql
```

### 3. Configuration

Copiez les fichiers de configuration :

```bash
cp conf/gvg_build.conf.dist env/dist/etc/modules/gvg_build.conf
cp conf/gvg_core.conf.dist env/dist/etc/modules/gvg_core.conf
```

Éditez `gvg_build.conf` pour personnaliser les paramètres :

```ini
[GvGBuild]
GvGBuild.Enable = 1
GvGBuild.WallGobId = 190397        # GameObject ID pour les murs
GvGBuild.TowerGobId = 190398       # GameObject ID pour les tours
GvGBuild.GateGobId = 179117        # GameObject ID pour la herse
GvGBuild.MinDistance = 10.0        # Distance minimum entre structures (yards)
GvGBuild.MaxStructuresPerPlayer = 50
```

### 4. Redémarrage

Redémarrez le serveur pour charger le module :

```bash
docker-compose restart ac-worldserver
# OU
./acore.sh restart worldserver
```

---

## 🎮 Utilisation

### Construire un Mur ou une Tour

1. Obtenez un **Kit de Mur (80000)** ou **Kit de Tour (80001)**
2. Faites **clic droit** sur l'item
3. **Cliquez au sol** où vous voulez placer la structure (max 30 yards)
4. La structure apparaît instantanément
5. L'item est consommé

### Construire une Fortification (Herse)

1. Obtenez un **Kit de Herse (80003)**
2. Faites **clic droit** sur l'item
3. **Cliquez au sol** où vous voulez placer le centre de la fortification
4. Le système spawn automatiquement :
   - **Tour Gauche** : 20 yards à gauche
   - **Herse Centrale** : À l'emplacement cliqué (pivotée à 90°)
   - **Tour Droite** : 20 yards à droite
   - **2 Leviers** : Sur le côté droit de la herse (5 yards, espacés de 6 yards)
5. L'item est consommé

### Ouvrir/Fermer la Herse

1. **Membres de la guilde** qui a placé la herse :
   - Cliquez sur l'un des **2 leviers**
   - La herse s'ouvre ou se ferme
   - Message : "Herse ouverte" ou "Herse fermée"

2. **Joueurs d'autres guildes** :
   - Clic sur le levier → "Cette herse appartient à une autre guilde!"
   - Impossible d'ouvrir/fermer

### Récupérer une Structure

1. Obtenez l'**Outil de Récupération (80002)** (permanent, ne se consomme pas)
2. Placez-vous **près de la structure** (max 10 yards)
3. Faites **clic droit** sur l'outil
4. ✅ Si **moins de 15 minutes** : Structure récupérée, kit restitué
5. ❌ Si **plus de 15 minutes** : "Cette structure ne peut plus être récupérée"

**Important** : 
- Pour une **fortification** : Récupère **tout le système** et rend **1 seul Kit de Herse**
- Pour un **mur/tour** : Récupère la structure et rend le kit correspondant

---

## 🏰 Système de Fortification

### Composition

Quand vous placez un **Kit de Herse**, le système spawne **5 GameObjects liés** :

```
Tour Gauche (100k PV)  ←----- 40 yards ---→  Tour Droite (100k PV)
                                |
                            Herse (centre)
                              (pivotée 90°)
                                |
                          Levier 1  Levier 2
                         (5y droite, espacés 6y)
```

### Caractéristiques

| Élément | Entry | Type | PV | Fonction |
|---------|-------|------|-----|----------|
| Tour Gauche | 190398 | 33 (Destructible) | 100,000 | Structure défensive |
| Tour Droite | 190398 | 33 (Destructible) | 100,000 | Structure défensive |
| Herse | 179117 | 0 (Door) | - | Porte ouvrable/fermable |
| Levier 1 | 400001 | 1 (Button) | - | Contrôle de la herse |
| Levier 2 | 400001 | 1 (Button) | - | Contrôle de la herse |

### Règles de destruction

⚠️ **Destruction en chaîne** :
- Si **une tour est détruite** → **Tout le système disparaît** automatiquement
- Les 2 tours + la herse + les 2 leviers sont supprimés

🗑️ **Récupération** :
- Utiliser l'outil de récupération **près de n'importe quel élément** du système
- Récupère **tout le système** et rend **1 seul Kit de Herse (80003)**

---

## 🗄️ Base de données

### Table : `gvg_build_structures`

Stocke toutes les structures construites par les joueurs.

| Colonne | Type | Description |
|---------|------|-------------|
| `guid` | BIGINT UNSIGNED | GUID du GameObject spawned |
| `player_guid` | BIGINT UNSIGNED | GUID du joueur constructeur |
| `guild_id` | INT UNSIGNED | ID de la guilde du joueur |
| `entry` | INT UNSIGNED | Entry du GameObject |
| `map` | SMALLINT UNSIGNED | ID de la map |
| `position_x/y/z` | FLOAT | Position dans le monde |
| `orientation` | FLOAT | Orientation |
| `build_type` | TINYINT UNSIGNED | 0=Mur, 1=Tour, 2=Gate |
| `build_time` | TIMESTAMP | Date/heure de construction |
| `group_id` | BIGINT UNSIGNED | ID de groupe (pour fortifications) |

**Note** : `group_id` lie tous les éléments d'une fortification ensemble (2 tours + 1 herse + 2 leviers).

### GameObjects configurés

| Entry | Nom | Type | ScriptName |
|-------|-----|------|------------|
| 190397 | Mur GvG | 33 | go_gvg_build_structure |
| 190398 | Tour GvG | 33 | go_gvg_build_structure |
| 179117 | Herse GvG | 0 (Door) | go_gvg_build_gate |
| 400001 | Levier de Herse | 1 (Button) | go_gvg_build_gate_lever |

---

## 🎯 Limites et Protections

### Distance de sécurité
- **10 yards minimum** entre deux structures (configurable)
- Empêche le spam de structures

### Limite par joueur
- **50 structures maximum** par joueur (configurable)
- Compte les murs, tours, et fortifications

### Placement
- **30 yards maximum** de distance de clic
- Vérification de terrain (pas dans les murs, etc.)

### Récupération
- **15 minutes** après placement : Structure récupérable
- **Après 15 minutes** : Structure permanente (non récupérable)

---

## 🛠️ Commandes utiles

### Commandes GM

```bash
# Donner les items
.additem 80000     # Kit de Mur
.additem 80001     # Kit de Tour
.additem 80003     # Kit de Herse (Fortification)
.additem 80002     # Outil de Récupération

# Recharger la configuration
.reload config

# Recharger les GameObjects
.reload gobject_template

# Supprimer un GameObject en le ciblant
.gobject delete
```

### Requêtes SQL utiles

```sql
-- Voir toutes les structures d'un joueur
SELECT * FROM gvg_build_structures WHERE player_guid = <PLAYER_GUID>;

-- Voir toutes les fortifications (avec groupe)
SELECT * FROM gvg_build_structures WHERE group_id IS NOT NULL;

-- Compter les structures par joueur
SELECT player_guid, COUNT(*) as total 
FROM gvg_build_structures 
GROUP BY player_guid;

-- Nettoyer toutes les structures GvG
DELETE FROM gvg_build_structures;
DELETE FROM gameobject WHERE id IN (190397, 190398, 179117, 400001);
```

---

## 🐛 Dépannage

### Les structures n'apparaissent pas

1. Vérifiez que les GameObjects sont correctement configurés :
```sql
SELECT entry, name, type, ScriptName FROM gameobject_template WHERE entry IN (190397, 190398, 179117, 400001);
```

2. Rechargez les templates :
```bash
.reload gobject_template
```

3. Vérifiez les logs du worldserver

### L'outil de récupération ne fonctionne pas

1. Vérifiez que vous êtes **à moins de 10 yards** de la structure
2. Vérifiez que **moins de 15 minutes** se sont écoulées
3. Vérifiez que la structure vous appartient

### La herse ne s'ouvre pas

1. Vérifiez que vous êtes dans la **même guilde** que le poseur
2. Cliquez sur les **leviers** (pas sur la herse directement)
3. Rechargez le script :
```bash
.reload scripts
```

---

## 📝 Notes importantes

- ⚠️ Les structures sont **permanentes** après 15 minutes (non récupérables)
- ⚠️ La destruction d'une tour dans une fortification **détruit tout le système**
- ⚠️ Les leviers sont **uniquement utilisables par la guilde propriétaire**
- ✅ Les structures peuvent être détruites par des **dégâts** (joueurs, engins de siège)
- ✅ L'outil de récupération est **permanent** (ne se consomme pas)

---

## 📄 License

Ce module est fourni "tel quel" sans garantie. Libre d'utilisation et de modification.

## 🤝 Support

Pour toute question ou problème :
- [AzerothCore Wiki](https://www.azerothcore.org/wiki)
- [AzerothCore Discord](https://discord.gg/gkt4y2x)

---

**Développé pour les serveurs AzerothCore GvG** 🏰⚔️
