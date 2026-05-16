# Conquest Server — Design Document

> Serveur privé AzerothCore + Playerbots, axé **PvP open world Horde vs Alliance**, guerre persistante entre factions.

## 1. Vision

Créer un serveur où **la guerre faction vs faction est le cœur du gameplay**. Pas de PvE structuré, pas d'économie complexe, pas de politique guilde. Juste : la map, deux camps, des objectifs à capturer, des bots qui peuplent le monde pour donner l'illusion d'un serveur vivant.

### Principes directeurs

- **Boucle de jeu addictive et simple** : se logger → trouver un front → fighter → défendre/capturer → recommencer
- **Objectifs PvP permanents** : toujours quelque chose à attaquer ou défendre
- **Monde vivant via bots** : 150+ bots équipés en PvP qui simulent une population active
- **Lignes de front dynamiques** : les zones changent de main, l'équilibre territorial bouge
- **Sensation de guerre persistante** : pas d'instances, pas de queues, tout se passe en open world

### À éviter

- ❌ Systèmes économiques avancés (réputation, factions tierces, marchés)
- ❌ Politique de guildes complexe
- ❌ Mécaniques MMO lourdes (raids, professions, leveling forcé)
- ❌ Bots qui font le gameplay à la place du joueur

---

## 2. Règles de gameplay custom

### Personnages

- **Race spawn** : chaque race spawn dans sa **capitale respective** (Stormwind, Ironforge, Darnassus, Exodar / Orgrimmar, Undercity, Thunder Bluff, Silvermoon)
- **Respawn à la mort** : retour à la **capitale de la race**, pas au cimetière le plus proche
- **Pas d'XP** (servir uniquement la vision PvP — tous les joueurs et bots sont à un niveau fixe)
- **PvP forcé** (`GameType = 1`) : faction vs faction, même faction se respecte

### Groupe et social

- **Plafond groupe = 5 joueurs** (pas de raid 10/25/40, gameplay de petites escouades)
- **Pas de BG / Arène** (le PvP est uniquement en open world)
- **Pas de LFG / donjon** (les bots ne sont pas distraits par le PvE)

### Restrictions de monde — Azeroth uniquement

- ✅ **Accessible** : Eastern Kingdoms (map 0) + Kalimdor (map 1)
- ❌ **Bloqué** : Outreterre (map 530), Norfendre (map 571), toutes les instances et raids
- 🚫 **Tentative d'entrée** : si un joueur essaie de rentrer dans une map bloquée (portail, hearthstone vers Dalaran, etc.) → erreur `TRANSFER_ABORT_MAP_NOT_ALLOWED` et téléport à la capitale de race
- 🎯 **Pourquoi** : concentre 100% du gameplay sur les zones contestées d'Azeroth, évite que les joueurs aillent farmer du PvE en instance, garde tout le monde sur le même terrain de bataille
- ⚙️ **Implémentation** : hook `OnPlayerCanEnterMap` qui refuse si `mapId IN (530, 571)` ou si `entry->IsDungeon() || entry->IsRaid()` (similaire à ce que faisait l'ancien `mod-conquest-restrictions`, à réactiver de manière ciblée)

### Glyphes — Accessibles dès level 60

Pour cohérence avec le design **"level 60 + mécaniques WotLK"** :

- ✅ Modifier en DB le `RequiredLevel` de **tous les glyphes lvl 61-80** pour qu'ils deviennent `RequiredLevel = 60`
- 🎯 Effet : à level 60, un joueur peut utiliser **les 6 slots glyphes (3 majeurs + 3 mineurs)** au lieu de seulement 2 (lvl 25/50)
- ⚖️ **Équilibrage** : permet aux classes de récupérer une partie du power perdu en n'ayant que 41 talent points
- 📝 SQL approximatif :

```sql
UPDATE acore_world.item_template 
SET RequiredLevel = 60 
WHERE class = 16  -- ITEM_CLASS_GLYPH
  AND RequiredLevel BETWEEN 61 AND 80;
```

⚠️ Note : il faut aussi vérifier que **les slots glyphes** eux-mêmes (Major #2, Major #3, Minor #2, Minor #3) sont **débloqués à level 60**. Par défaut ils sont liés au level via la fonction `Player::InitGlyphsForLevel`. Si nécessaire, patch C++ custom dans `Player::InitGlyphsForLevel` pour considérer level 60 comme "max" (débloque tous les slots).

---

### Montures Conquest — Ownership et anti-vol

#### Mécanique d'ownership

- Les montures Conquest (entries 400000-400003 — chevaux, loups, shredder, mécano-tank custom) sont des **NPC montables**, pas des items de monture classiques
- Quand un joueur invoque sa monture (via item / sort custom), le NPC spawné est **lié à ce joueur** (ownership stocké en mémoire ou dans une table custom `conquest_mount_owner`)
- **Règle d'accès** :
  - ✅ **Le propriétaire** peut toujours monter dessus
  - ✅ **Un joueur de la faction ennemie** peut monter sur la monture de n'importe quel joueur (capture / vol assumé en PvP — cohérent lore-friendly : "j'ai pris la monture d'un Alliance après l'avoir tué")
  - ❌ **Un joueur de la même faction (allié)** ne peut PAS monter la monture d'un autre joueur de sa faction → **anti-troll / anti-abus** entre alliés

#### Implémentation technique

- Modifier `mod-conquest-mounts` :
  - Au spawn de la monture : enregistrer `owner_guid` et `owner_team` (Alliance/Horde) dans une map en mémoire ou en DB
  - Hook `OnPlayerEnterVehicle` (ou équivalent CreatureScript) : vérifier
    ```cpp
    if (player->GetGUID() == owner_guid) → allow
    if (player->GetTeamId() != owner_team) → allow  // faction ennemie peut voler
    else → deny  // même faction, pas propriétaire = bloqué
    ```
  - Message refus : "Cette monture appartient à un membre de votre faction."

#### Cas d'usage et effets émergents

| Scénario | Résultat |
|---|---|
| Joueur Alliance invoque sa monture | Lui seul peut monter dessus, ses alliés Alliance ne peuvent pas → pas d'abus |
| Joueur Alliance tue un Horde monté → veut la monture | ✅ Peut monter dessus (vol PvP autorisé) |
| 2 Alliance veulent partager une monture | ❌ Bloqué → chacun doit invoquer la sienne |
| Joueur Horde fuit un fight, abandonne sa monture | Un Alliance qui passe peut la prendre (récompense de pursuit) |

➡️ **Avantage gameplay** : crée des moments PvP supplémentaires (monture abandonnée = item à récupérer), évite les abus inter-alliés (un troll qui prend la monture d'un coéquipier au mauvais moment).

---

### Équipement bots et joueurs

- **Bots et joueurs utilisent les montures Conquest** custom (entries 400000-400003 du module mod-conquest-mounts), pas les montures classiques de race
- **Level** : **60 fixe** pour tout le monde (joueurs + bots)
- **iLvl cap** : **92** (= T3 Naxx40, peak vanilla level 60)
- **Quality** : Epic max (`RandomGearQualityLimit = 4`)
- **Préférences** : armure adaptée à la classe (`PreferClassArmorType = 1`) + armes adaptées à la spec (`PreferredSpecWeapons = 1`)
- **Persistence** : équipement figé une fois optimisé (`EquipmentPersistence = 1`)
- **`LimitGearExpansion = 1`** : pas d'items TBC/WotLK qui pollueraient la pool

> ⚠️ **TODO (2026-05-16)** : le stuff actuel des bots et les stats des items mis en vente par les NPC vendeurs Conquest doivent être retravaillés. Côté bots, beaucoup de pièces équipées sortent encore du pool RandomItemMgr classique sans calage précis sur le tier visé (R10 entrée / R13-R14 epic / T3 PvE). Côté vendeurs (`conquest_vendor_items`), les items vendus restent ceux du vanilla (Marshal's/Warlord's/T3) sans révision des stats, des prix PB/PC, ni des effets/procs au regard du level 60 cap iLvl 92. À faire : passe complète stuff bots (script de mass-equip par tier/class/race) + audit stats vendeurs + ré-équilibrage PB/PC.

#### Progression PvP attendue

| Tier | Source | Items concernés (vanilla R10-R14) |
|---|---|---|
| **Rare PvP** (entrée) | Donné au start ou bas prix | R10-R12 : Marshal's / Champion's (blue) — armor + arme blues |
| **Epic PvP** (cible kills) | Achat avec Points de Bataille | R13-R14 : Field Marshal's / Warlord's (armor epic) + Grand Marshal's / High Warlord's (armes epic) |
| **PvE accessoires** | Achat avec Points de Conquête | T3 Naxx40 epics : anneaux, trinkets, capes, cous + sets PvE complets |
| **Légendaire** (endgame) | Marques de Champion (cf section 3bis) | Set custom + arme custom événementielle |

#### ⚠️ Convention naming vanilla — Faction lock implicite

**Règle critique** à retenir pour tout équipement PvP vanilla :

| Préfixe nom de l'item | Faction | Tier approximatif |
|---|---|---|
| **Marshal's** | 🔵 **Alliance** | R10 (blue) — Belt/Legs/Feet/Wrists/Hands |
| **General's** | 🔴 **Horde** | R10 (blue) — Belt/Legs/Feet/Wrists/Hands |
| **Field Marshal's** | 🔵 **Alliance** | R13/R14 (epic) — Head/Shoulders/Chest |
| **Warlord's** | 🔴 **Horde** | R13/R14 (epic) — Head/Shoulders/Chest |
| **Grand Marshal's** | 🔵 **Alliance** | R14 (epic) — Armes |
| **High Warlord's** | 🔴 **Horde** | R14 (epic) — Armes |

Bien que `AllowableRace = -1` (= toutes races) en DB, ces items ont un **faction lock implicite** appliqué par le moteur AzerothCore au login. Si on équipe un item "General's" sur un perso Alliance via SQL direct, le worldserver le **déséquipera silencieusement** au prochain login du bot/joueur.

**Conséquence dev** : tout script SQL de mass-equip doit mapper correctement la faction :
- Alliance bots → items "Marshal's" / "Field Marshal's" / "Grand Marshal's"
- Horde bots → items "General's" / "Warlord's" / "High Warlord's"

⚠️ **Edge case Shaman Alliance (Draenei)** : la classe est arrivée en Burning Crusade avec Draenei. La gamme Marshal's Mail est **incomplète** :
- ✅ Marshal's Mail Leggings, Boots, Gauntlets existent
- ❌ Marshal's Mail Belt, Bracers **n'existent pas**

**Solution appliquée** : utiliser les versions Horde (`General's Mail Waistband` = 16575, `General's Mail Bracers` = 16576) en débloquant leur faction lock via :

```sql
UPDATE acore_world.item_template 
SET requiredhonorrank = 0 
WHERE entry IN (16575, 16576);
```

Sans risque de cross-contamination car ces items ont `AllowableClass = 64` (Shaman uniquement), donc aucun Warrior/Pala/etc. Alliance ne peut soudainement les porter.

#### 🔧 Mécanisme du faction lock — Important pour le dev

Le faction lock implicite n'est PAS lié à `AllowableRace` (qui est `-1` partout). C'est le champ **`requiredhonorrank`** d'`item_template` qui le matérialise :

- `requiredhonorrank = 0` → item sans restriction de faction
- `requiredhonorrank = N` (où N est un code de rang vanilla) → item lié à la faction qui possède ce rang
- Les rangs PvP vanilla sont **faction-spécifiques** (Marshal = Alliance R10, General = Horde R10, etc.), donc deux items "miroir" comme `Marshal's Plate Girdle` (entry 16482, Alliance) et `General's Plate Girdle` (entry 16547, Horde) ont **tous les deux** `requiredhonorrank = 16` mais le moteur vérifie le rang du joueur dans **sa** faction.

**Pour débloquer ponctuellement un item** (ex: faire porter du gear Horde à un Alliance) :
```sql
UPDATE item_template SET requiredhonorrank = 0 WHERE entry = X;
```

À utiliser **uniquement** sur des items déjà class-restricted (sinon cross-faction wear non-intentionnel). Et **prudemment** : un item débloqué reste débloqué pour tous (Horde Shaman aussi pourront les utiliser sans restriction de rang).

⚠️ **Edge case DK** : pas de set PvP vanilla — sets custom à créer (cf section dédiée plus bas).

#### Customisation des items vanilla (équilibrage WotLK)

Les items PvP vanilla R10-R14 n'ont pas de **résilience** (stat WotLK qui n'existait pas en classic). Pour équilibrer le burst des classes WotLK :

- **+30-50 Resilience Rating** par pièce d'armor (en utilisant `stat_type` libre)
- **+30-50 Stamina** supplémentaire par pièce
- **À répartir progressivement** : moins sur le rare R10-R12, plus sur l'epic R13-R14, max sur le légendaire custom
- Modification SQL dans `item_template` (slots `stat_type8-10` souvent libres)

#### Set PvP Death Knight — Création de 2 sets custom dédiés

Les DK n'ont **aucun set PvP vanilla** (la classe a été ajoutée en WotLK). On crée **2 sets PvP custom dédiés** au DK pour boucler proprement la progression.

##### Sets à créer

| Tier | Set DK custom Alliance | Set DK custom Horde | Pièces |
|---|---|---|---|
| **Rare** (start) | "Knight-Lieutenant's Dreadplate" | "Blood Guard's Dreadplate" | 8 (HSC + Belt + Legs + Feet + Wrists + Hands) |
| **Epic** (cible kills) | "Lieutenant Commander's Dreadplate" | "Champion's Dreadplate" | 8 (HSC + Belt + Legs + Feet + Wrists + Hands) |

##### Stats par pièce (à tuner)

**Set Rare** :
- ~30 Strength
- ~40 Stamina (+20 bonus customisation = ~60 total)
- ~30 Resilience Rating
- Armor (Plate)

**Set Epic** :
- ~50 Strength
- ~60 Stamina (+40 bonus customisation = ~100 total)
- ~60 Resilience Rating
- Armor (Plate)
- Bonus de set 2/4/6 pièces (à définir : +crit, +haste, ou compétence DK comme Frost Presence)

##### Armes DK

Création de 2 armes 2H custom (matching le style DK) :
- **Rare** : "Knight-Lieutenant's Greatsword" / "Blood Guard's Greatsword"
- **Epic** : "Lieutenant Commander's Reaper" / "Champion's Reaper"

##### Méthode de création technique

1. **Choisir des entries d'items custom libres** : ex 80100-80115 pour les sets DK rare/epic (8+8 pièces × 2 factions = 32 entries + 4 armes = 36 entries)
2. **DisplayId** : utiliser les modèles 3D des sets Warrior R14 vanilla (cosmétiquement proches du DK) ou les modèles T7/T8 DK natifs (à voir selon la pool de displayId dispo)
3. **AllowableClass** : `32` (DK seul) — pas de partage avec Warrior
4. **AllowableRace** : faction-spécifique
   - Sets Alliance : `AllowableRace = 1101` (Human + Dwarf + NightElf + Gnome + Draenei)
   - Sets Horde : `AllowableRace = 690` (Orc + Undead + Tauren + Troll + BloodElf)
5. **RequiredLevel** : 60
6. **Quality** : 3 (rare) ou 4 (epic) selon le tier
7. **ItemLevel** : ~71-78 pour rare, ~85-92 pour epic (matche les R12 et R14 des autres classes)
8. **SetId** : créer 4 nouvelles entrées dans `item_set` (une par combinaison tier/faction) pour les bonus de set
9. **SQL ScripT** : `data/sql/db-world/conquest_dk_sets.sql` à versionner

##### Stockage des données

Tout en SQL custom du module `mod-conquest-frontline` (ou nouveau module `mod-conquest-dk-sets` si on veut isoler) :

```
data/sql/db-world/
  ├── conquest_dk_sets_rare_alliance.sql      (8 pièces armor + 1 arme)
  ├── conquest_dk_sets_rare_horde.sql         (8 pièces armor + 1 arme)
  ├── conquest_dk_sets_epic_alliance.sql      (8 pièces armor + 1 arme)
  ├── conquest_dk_sets_epic_horde.sql         (8 pièces armor + 1 arme)
  └── conquest_dk_set_bonuses.sql             (item_set entries)
```

##### Effort estimé

- Création des 36 entries item_template : **~1h de SQL** (gabarit + copy-paste avec ajustements)
- Création des 4 item_set bonuses : **~30 min**
- Test en jeu (équiper un DK et vérifier que tout fonctionne) : **~30 min**
- **Total : ~2h** pour la création complète des sets DK

### Kill streak visuel — Drapeau "On Fire"

Feature purement **esthétique** pour valoriser les joueurs qui enchaînent les kills :

- 🔥 **Kill streak de 5** : applique un sort qui spawn un **drapeau visible** au-dessus/sur le joueur
- ⚡ **Kill streak de 10** : drapeau remplacé par une **version plus élaborée** (modèle visuel plus prestigieux)
- 💀 **Mort** : le drapeau disparaît (perdu)
- ❌ **Aucun buff stat** : c'est uniquement un marqueur visuel pour identifier qui est "on fire" dans la mêlée

**Effets de gameplay** :
- Crée un **incitatif à snowballer** : "j'ai 8 kills, plus que 2 avant le drapeau prestigieux"
- Crée une **cible désignée** : les joueurs adverses voient qui est dangereux, peuvent décider de focus
- Renforce le **côté arène spectacle** : un joueur "on fire" devient une figure visible du combat

**Implémentation technique** :
- Hook `OnPlayerPVPKill` → incrémente le compteur de streak du killer
- À 5 kills consécutifs → cast d'un sort avec aura visuelle qui spawn un GameObject "drapeau" attaché au joueur
- À 10 kills → remplace par un sort différent avec GameObject plus orné
- Hook `OnPlayerJustDied` → reset le compteur + dispel l'aura (drapeau retiré)
- Stockage en mémoire (pas en DB, c'est transitoire à la session)

**Items / Sorts à choisir** :
- Possibilité d'utiliser les drapeaux existants (Alterac Valley flags, Wintergrasp banners, custom GameObjects)
- À mapper : 2 sorts custom (kill5_flag, kill10_flag) + 2 displayId visuels

---

### Mort et drop

- **Drop bag à la mort** : à la mort d'un joueur, un sac de loot apparaît sur son cadavre (`mod-conquest-lootdrop` réactivé)
- **Contenu du sac** : **uniquement le contenu des sacs** du joueur (consommables, items de quête, currencies, points...)
- ❌ **L'équipement (gear porté) reste sur le joueur** — pas de loss de stuff R14/T3
- ✅ Risque ciblé : les **points gagnés** (currency items stackables) sont perdus si tu meurs avant de les déposer
- ⏱️ Le sac reste visible 15 min puis disparaît, n'importe qui peut looter
- **Modifications nécessaires de `mod-conquest-lootdrop`** :
  - Modifier la fonction qui copie les items pour **ne copier que les slots de bag** (slots 23+ et bags > 0), pas les slots d'équipement (0-22)

> ⚠️ **TODO (2026-05-16)** : actuellement le module spawne bien un sac sur le cadavre mais **ne vide pas réellement l'inventaire du joueur mort** — les items restent dans ses sacs après la mort, donc le risque "perdre ses points si tu meurs avant de déposer" n'est pas en place. Il faut compléter le hook pour `DestroyItemCount` / `MoveItemFromInventory` sur les slots concernés au moment du spawn du loot bag.

---

## 3. Système de capture de zones — Cœur du gameplay

### Zones initiales (MVP)

| Zone | Continent | Position approximative |
|---|---|---|
| **Crossroads** | Kalimdor (Horde-friendly) | Carrefour |
| **Astranaar** | Kalimdor (Alliance-friendly) | Ashenvale |
| **Southshore** | Eastern Kingdoms (Alliance-friendly) | Hillsbrad |
| **Tarren Mill** | Eastern Kingdoms (Horde-friendly) | Hillsbrad |
| **Gadgetzan** | Kalimdor (Neutre) | Tanaris |

L'idée : démarrer avec **5 zones bien réparties** entre les continents, certaines historiquement liées à une faction (intérêt RP) et une neutre (Gadgetzan) pour donner un "no man's land" disputable par tout le monde.

### Composants par zone

Chaque zone capturable est définie par **un seul GameObject "bannière de capture" custom de type 29 (`GAMEOBJECT_TYPE_CAPTURE_POINT`)** spawné manuellement par l'admin dans le monde Azeroth via `.gob add <entry>`. Le système est **plug & play** : dès qu'une instance du GO apparaît dans le monde, le module l'enregistre automatiquement et il devient capturable sans config manuelle.

#### Choix : **GO générique unique** (recommandé)

On utilise **une seule entry custom (ex: 400010 "Bannière de Capture Conquest")** pour TOUTES les zones, peu importe leur emplacement. Une seule ligne dans `gameobject_template` à maintenir, et on spawn autant d'instances qu'on veut.

- 🏳️ **GameObject type 29** (capture point natif AzerothCore — déclenche la UI gauge client automatiquement)
- 📍 **Zone de capture** : rayon défini dans le template (~40-50 yd), surchargeable au spawn
- 🛡️ **3 états** : `Neutre` / `Alliance` / `Horde` (gérés par le framework `OPvPCapturePoint`)
- ⚔️ **Gardes spawnés** quand capturé (PNJ de la faction propriétaire) — voir détails plus bas
- 📊 **Score de domination** persistant en DB (qui possède, depuis quand, capturé combien de fois)

#### Pourquoi un GO générique plutôt qu'un par zone

Toutes les informations de zone sont **dérivables au runtime** depuis n'importe quelle instance de GameObject :

```cpp
go->GetSpawnId();    // unique par instance (clé pour les stats / scores)
go->GetMap();        // continent
go->GetZoneId();     // ID zone (17 = Hillsbrad, 1377 = Silithus, etc.)
go->GetAreaId();     // sous-zone précise
sAreaTableStore.LookupEntry(zoneId)->area_name[locale];  // "Hillsbrad Foothills" en français
go->GetPositionX/Y/Z();  // coords exactes pour logs/annonces
```

→ Pour une annonce monde type *"[Capture] Joelguigli a capturé Le Crossroads"*, on peut tout obtenir via le GO + `Player*` qui a déclenché. Pas besoin d'entry pré-nommée.

#### Avantages du GO unique vs per-zone

| Aspect | GO unique (retenu) | Per-zone |
|---|---|---|
| Lignes DB `gameobject_template` | 1 | 5-20+ |
| Ajouter une nouvelle zone | `.gob add 400010` n'importe où | Créer nouvel entry SQL + ScriptName |
| Info zone (nom, coords, area) | Dérivée auto via `GetZoneId/AreaId` | Pré-stockée dans le template (rigide) |
| Plug & play | ✅ Total | ⚠️ Partiel (faut config par zone) |
| Stats par zone | Identifiable via `spawn_id` ou `(map, area)` | Identifiable via `entry` |

#### Auto-détection au spawn

Le module hook `OnGameObjectAddWorld` :

```cpp
void OnGameObjectAddWorld(GameObject* go) override {
    if (go->GetEntry() != CONQUEST_CAPTURE_POINT_ENTRY)  // 400010
        return;
    
    // Auto-register dans conquest_zones
    uint32 zoneId, areaId;
    go->GetMap()->GetZoneAndAreaId(zoneId, areaId, 
        go->GetPositionX(), go->GetPositionY(), go->GetPositionZ());
    
    char const* areaName = sAreaTableStore.LookupEntry(areaId)->area_name[locale];
    
    // Enregistre dans la DB conquest_zones avec spawn_id, zone_id, area_id, name dérivé
    // Crée un OPvPCapturePoint associé à ce GO
    sOutdoorPvPConquest->RegisterCapturePoint(go);
    
    LOG_INFO("conquest", "Auto-registered capture point at {} (zone {}, area {})",
        areaName, zoneId, areaId);
}
```

→ Workflow admin : `.gob add 400010` → automatiquement capturable, UI gauge fonctionne avec le bon nom, logs structurés, stats par zone.

#### Annonces / logs structurés

Avec ce système, à chaque événement on a contexte complet :

```
[Conquest] Player "Joelguigli" (Alliance) a capturé "Le Crossroads" 
      (map=Kalimdor, area_id=380, coords=2030,-2740,90, spawn_id=42)
[Conquest] Zone "Le Crossroads" est passée de NEUTRE à ALLIANCE
[Conquest] Joueurs contributeurs : Joelguigli, BotAlpha, BotBeta (3 présents au moment du fill)
```

### Mécanique de capture (style Eye of the Storm / Wintergrasp)

**Pas de canalisation au clic**. Le système est **basé sur la présence dans la zone** :

#### Logique de progression

```
État courant : NEUTRE
  ├─ Si 1+ joueur Alliance dans la zone et aucun Horde
  │    → Progression Alliance + 1 / tick
  │    → À 100% : NEUTRE → ALLIANCE (capture validée)
  │
  ├─ Si 1+ joueur Horde dans la zone et aucun Alliance
  │    → Progression Horde + 1 / tick
  │    → À 100% : NEUTRE → HORDE (capture validée)
  │
  └─ Si joueurs des 2 factions dans la zone
       → Aucune progression (état contesté, statu quo)

État courant : ALLIANCE
  └─ Si Horde dans la zone et aucun Alliance
       → Progression Horde + 1 / tick
       → À 100% : ALLIANCE → NEUTRE (pas de récompense pour Horde)

État courant : HORDE
  └─ Symétrique : Alliance peut faire passer à NEUTRE
```

#### Règles clés

- 🎯 **Récompense uniquement sur capture NEUTRE → faction** : seuls les joueurs qui font passer la zone de neutre à leur faction reçoivent les **Points de Conquête**
- ⚖️ **Pas de récompense pour passer ennemi → neutre** : c'est un "pré-requis" du gameplay (faut neutraliser avant de capturer)
- 🔄 **Cycle naturel** : Alliance hold → Horde arrive → Alliance partent → zone repasse neutre → Horde la prend → reçoivent les points
- ⏱️ **Vitesse de progression** : ~30-60 secondes pour passer d'un état à l'autre (à tuner)
- 👥 **Plusieurs joueurs présents ne speedupent PAS** : la progression reste linéaire (limite anti-zerg, force la défense plutôt que le push numérique)

#### Tick de mise à jour

- Le module check toutes les **1-2 secondes** quels joueurs sont dans le rayon de chaque bannière
- Si déséquilibre faction unilatéral → incrémente la jauge d'1 cran
- Quand la jauge atteint 100 (ou la valeur cible) → transition d'état

### Visuels et effets monde

#### État `NEUTRE`
- Seule la bannière principale est visible (modèle disputed/gris)
- Aucun PNJ autour

#### État `Alliance` (ou Horde — symétrique)
- Bannière prend le modèle faction (lion bleu / loup rouge)
- **Spawn automatique de gardes PNJ** de la faction autour de la bannière (3-5 gardes selon design)
- Buff zone-wide visible "Présence Alliance/Horde" pour les joueurs alliés présents
- Bannière émet un effet lumineux/particle pour signaler la possession

#### Transition d'état
- **Capture (neutre → faction)** :
  - Annonce monde : `⚔️ [zone] vient d'être capturée par la [Faction] !`
  - Spawn instantané des gardes PNJ
  - Joueurs ayant contribué à la capture (présents dans la zone au moment du fill) reçoivent les Points de Conquête
- **Perte (faction → neutre)** :
  - Annonce monde : `⚠️ [zone] est perdue par la [Faction] !`
  - Despawn des gardes PNJ
  - Aucune récompense distribuée

### Gardes PNJ (spawnés à la capture)

- **3 à 5 gardes** par bannière (configurable par zone)
- **Faction** : matche la faction propriétaire (Stormwind Guard pour Alliance, Orgrimmar Grunt pour Horde, ou équivalents génériques)
- **Position** : disposés en cercle autour de la bannière (rayon 10-15 yd)
- **Comportement** : aggressifs envers les joueurs de la faction adverse qui entrent dans la zone
- **Durée de vie** : tant que la zone reste de leur faction (despawn automatique sur perte)
- **Niveau** : niveau 60, gear suffisant pour challenger un joueur seul mais tombable à 2-3 joueurs (équilibrage à tuner)

### Aspects "plug & play" pour spawn manuel

L'admin doit pouvoir simplement faire `.gob add 400010` n'importe où dans le monde Azeroth, et la bannière devient automatiquement capturable. Cela implique :

1. **Auto-détection** : un hook `OnGameObjectAddWorld` qui détecte les nouveaux GO du type bannière et les enregistre dans la table `conquest_zones`
2. **Pas de config par fichier** : tout est dérivé de la position du GO et de son entry
3. **Nom et zone-id** : récupérés via `GetZoneAreaName(x, y, z)` au moment du spawn
4. **Rayon de capture** : valeur par défaut configurable globalement (`AiConquest.DefaultCaptureRadius = 45`), surchargable par zone via une commande GM

### 🏗️ Implémentation technique recommandée — Framework OutdoorPvP natif

L'AzerothCore fournit un framework natif `OutdoorPvP` + `OPvPCapturePoint` parfaitement adapté à notre besoin (utilisé nativement par les tours d'Eastern Plaguelands, Wintergrasp, Hellfire, etc.). On le réutilise plutôt que de tout recoder.

#### Architecture C++

```cpp
// Dans mod-conquest-frontline

class ConquestCapturePoint : public OPvPCapturePoint {
    // Une instance par bannière capturable
    bool Update(uint32 diff) override;            // Tick — calcule progression
    void ChangeState() override;                  // Transition d'état
    void ChangeTeam(TeamId oldTeam) override;     // Hook sur changement de propriétaire
    bool HandlePlayerEnter(Player* player) override; // Joueur entre dans le rayon
    void HandlePlayerLeave(Player* player) override; // Joueur sort
};

class OutdoorPvPConquest : public OutdoorPvP {
    bool SetupOutdoorPvP() override;              // Création des capture points
    void HandlePlayerEnterZone(Player*, uint32) override;
    void HandlePlayerLeaveZone(Player*, uint32) override;
    bool Update(uint32 diff) override;            // Tick global
};

class OutdoorPvPScript_Conquest : public OutdoorPvPScript {
    OutdoorPvPScript_Conquest() : OutdoorPvPScript("outdoorpvp_conquest") {}
    OutdoorPvP* GetOutdoorPvP() const override { return new OutdoorPvPConquest(); }
};

void AddConquestFrontlineOutdoorPvPScripts() {
    new OutdoorPvPScript_Conquest();
}
```

#### Avantages du framework

- ✅ **Logique de capture** déjà testée et stable (présence-based, progression linéaire, transitions d'état)
- ✅ **Tick automatique** : le framework appelle `Update(diff)` régulièrement
- ✅ **Tracking des joueurs** dans le rayon : `_activePlayers[2]` (Alliance + Horde) déjà géré
- ✅ **UI gauge automatique** sur écran client via `SendUpdateWorldState(field, value)`
- ✅ **Hook spawn auto** : `OnGameObjectAddWorld` + `SetCapturePointData()` pour enregistrer un nouveau GO comme point

#### ⚠️ Limitation worldstates client — UI gauge

Les **worldstates** que le client WoW 3.3.5 sait afficher pour le HUD sont **hard-codés** dans l'exe client. On ne peut pas créer de nouveaux worldstates "Crossroads Banner" — le client ne saurait pas quoi en faire.

**Solutions disponibles** :

| Option | Approche | Compromis |
|---|---|---|
| **A** | Réutiliser worldstates Eastern Plaguelands (Crown Guard, North Pass, Eastwall, Plaguewood Towers — 4 dispos) | UI native ✓ mais noms "Crown Guard Tower" affichés sur écran |
| **B** | Réutiliser worldstates Hellfire Peninsula (Overlook, Stadium, Broken Hill — 3 dispos) | Idem |
| **C** | Worldstates Wintergrasp (les plus visuellement adaptés) | Limité à 4-5 capture points max |
| **D** | Pas d'UI native, broadcast chat custom à chaque tick (`[Crossroads] Capture Alliance: 65%`) | Plus de UI gauge, mais nommage propre |

**Recommandation MVP** : **Option A** (worldstates EP) — on accepte les libellés "Crown Guard Tower" sur le UI au début, ça donne l'UI gauge gratuitement. Mapping :

| Bannière Conquest | Worldstate emprunté à EP |
|---|---|
| Crossroads | Crown Guard Tower (CGT) |
| Astranaar | Northpass Tower (NPT) |
| Southshore | Eastwall Tower (EWT) |
| Tarren Mill | Plaguewood Tower (PWT) |
| Gadgetzan | Pas de 5e worldstate EP — fallback Option D pour celui-ci |

Pour 5+ bannières simultanées, mixer EP + Hellfire + Nagrand pour avoir assez de worldstates uniques.

**Évolution V2** : addon custom (Lua client-side) qui réinterprète les worldstates EP comme nos noms réels. Effort ~1 journée de dev addon, pas critique.

#### Comparaison Frontière personnalisée vs Framework natif

| Aspect | Custom (notre `mod-conquest-frontline`) | OutdoorPvP natif réutilisé |
|---|---|---|
| Effort dev | ~500 lignes C++ | ~150 lignes C++ |
| Stabilité | À tester | Battle-tested |
| UI gauge | Doit faire un addon | Gratuit (worldstates) |
| Annonces monde | Custom (flexible) | Custom (flexible) |
| Hooks dispo | À créer | Déjà tous prévus |
| Limitation noms UI | Aucune | Worldstates EP / Hellfire imposés (V2 fixable via addon) |

➡️ **Conclusion** : utiliser le framework natif, accepter le compromis UI temporaire (libellés EP), polir avec un addon en V2 si besoin.

### 🏕️ Tentes goblins — Supply hubs neutres

Pour fluidifier le PvP en open world et donner aux joueurs des **points de ravitaillement** entre deux fights, on dispose des **tentes de goblins marchands** dans certaines zones (idéalement contestées ou stratégiques).

#### Concept

- 🛖 **GameObject "tente"** (custom entry, ex: 400060) — structure visuelle décorative
- 🟢 **PNJ goblin vendeur** spawné devant ou dans la tente (faction Steamwheedle Cartel = neutre pour les deux camps)
- 💰 **Vend des consommables PvP** contre de l'**or** (pas de currency custom — garde la boucle simple)

#### Pourquoi des goblins ?

- **Neutralité lore-friendly** : les goblins (Cartel de Steamwheedle) sont historiquement neutres dans WoW, marchands opportunistes
- **Vendable aux 2 factions** sans contradiction de design
- **Ambiance "zone de guerre"** : des marchands opportunistes au milieu du chaos = très WoW Classic flavor

#### Inventaire des goblins (suggestion)

| Catégorie | Items typiques |
|---|---|
| **Potions de soin** | Major Healing Potion, Major Mana Potion |
| **Potions stats** | Mighty Rage Potion, Free Action Potion, Living Action Potion |
| **Élixirs / Flasks** | Flask of Petrification, Elixir of Brute Force, Elixir of the Mongoose |
| **Bandages** | Heavy Runecloth Bandage |
| **Food / Drink** | Tender Wolf Steak, Mage food haute regen |
| **Enchantements** | Scrolls d'enchant temporaires (armor scroll, weapon oil — Wizard Oil, Brilliant Mana Oil, Sharpening Stones) |
| **Consommables PvP custom** | Token de TP rapide vers capitale (5 min CD), invisibilité courte, etc. — V2 |

#### Localisation des tentes (MVP)

3-4 tentes réparties dans les zones contestées, **proches mais hors du rayon de capture** des bannières :

| Zone | Localisation tente | Goblin |
|---|---|---|
| **Hillsbrad Foothills** | Entre Southshore et Tarren Mill (mi-chemin) | Trader Ratz |
| **Ashenvale** | Près d'Astranaar mais en lisière | Trader Slingz |
| **Stranglethorn / Booty Bay** | Près de Booty Bay, étendre l'influence goblin | Trader Roxx |
| **Tanaris** | Près de Gadgetzan (déjà neutre — renforce le thème) | Trader Bekk |

Possibilité d'ajouter une **tente à proximité de chaque capitale** pour le gameplay early-game (un joueur sort de capitale, achète ses pots, va au front).

#### Plug & play

Comme les bannières de capture, les tentes goblins sont **simplement spawn-and-forget** :
- `.gob add 400060` → tente apparaît
- Spawn manuel ou via SQL d'un goblin neutre à côté (entry custom ex: 400060 NPC)
- Le goblin a un vendor template standard AzerothCore (table `npc_vendor`)

#### Risques et mitigations

| Risque | Mitigation |
|---|---|
| Les goblins deviennent OP (joueur farm permanent les pots) | Limiter le stock disponible (restock 5 min), prix élevés pour les consommables fort impact |
| Les tentes sont attaquées par les bots (faction confusion) | Faction neutre stricte (35) sur les goblins, immune aux dégâts joueurs (`UNIT_FLAG_IMMUNE_TO_PC`) |
| Pas de raison d'avoir ces tentes vs aller en capitale | Tentes plus proches du front, gain de temps (capitale = TP long) |

#### Extensions V2 (idées pour plus tard)

- **Tentes destructibles** : les bots/joueurs peuvent attaquer une tente → goblin part, plus de service pendant X heures → impact stratégique sur la guerre
- **Tentes de faction (alternative aux neutres)** : tentes Alliance/Horde dédiées qui spawnent quand une zone proche est capturée par cette faction (renforce la possession)
- **Quartier-maître mobile** : un goblin caravane qui se déplace entre zones, créant un objectif de protection/raid

### Score et progression globale

- Chaque zone capturée rapporte des **points de domination** à la faction (score collectif)
- Score affiché en HUD permanent ou via `.conquest score` chat command
- Quand une faction atteint un seuil (ex: 1000 points), événement de fin de saison déclenché

### Système de points (currencies — boucle économique du serveur)

Deux types de **points stackables en inventaire** (des items custom, comme les Marques d'Honneur vanilla) :

#### 🏆 Points de Conquête (zone capture → gear PvE accessoires)

- **Source unique** : transition **NEUTRE → ta faction** sur une bannière de zone (50 pts par joueur présent dans la zone au moment de la capture)
- ❌ **Pas de récompense** quand on fait passer une zone ennemie → neutre (c'est un travail de "blocage" valorisé indirectement via l'absence de score adverse)
- ❌ **Pas de tick** de zone tenue dans le MVP (peut être ajouté plus tard si la boucle est trop lente)
- **Utilité** : acheter les **accessoires PvE niveau 60** (anneaux T3, trinkets Naxx40, capes, cous, sets PvE complets — tout le iLvl 92 PvE)
- **Item ID custom** : entry 80020 (déjà existant, "Or Conquest" — à renommer "Marque de Conquête")
- **Logique** : récompense le **gameplay objectif/tactique** (présence en zone, défense des conditions de capture) — pas la sweat individuelle

#### ⚔️ Points de Bataille (PvP kills → gear PvP epic)

- **Source** : kill d'un joueur ennemi (10 pts) ou bot ennemi (2 pts pour éviter le farm)
- **Utilité** : acheter les **sets PvP epic vanilla R13-R14** (Field Marshal's / Warlord's pour l'armor + Grand Marshal's / High Warlord's pour les armes)
- **Item ID custom** : entry 80021 (déjà existant, "Bois Conquest" — à renommer "Marque de Bataille")
- **Logique** : récompense la **skill individuelle de combat** — pas le farm AFK

#### Pourquoi deux currencies ?

- **Force la diversité de gameplay** : un joueur qui ne fait que kill peut habiller son set PvP visible (R14), mais pour les accessoires (anneaux, trinkets — qui font la différence stats), il doit participer aux objectifs
- **Empêche le farm pur** : sans kills, pas de set PvP. Sans captures, pas de stats fines
- **Crée des choix de gameplay** : un joueur skilled-DPS peut farmer les Points de Bataille pour son set R14. Un joueur teamplay capture les zones pour combler ses slots non-set

#### Vendeurs

| NPC | Localisation | Vend |
|---|---|---|
| **Maître d'Armes** (PvP epic) | Une instance dans chaque capitale | Sets PvP R13-R14 (Field Marshal's / Warlord's + armes Grand Marshal's / High Warlord's) par classe |
| **Quartier-maître** (PvE accessoires) | Idem capitale | Anneaux T3, trinkets Naxx40, capes, cous, sets PvE iLvl 92 |
| **Forgeron des Légendes** (Légendaire) | Idem capitale | Set armor légendaire (cf section 3bis) |

⚠️ **Note start joueur** : un nouveau joueur reçoit gratuitement un set PvP rare (R10-R12 blues) en arrivant en capitale (via un NPC welcome ou auto-equip). Il faut farmer les Points de Bataille pour upgrade en R13-R14 epic.

---

## 3bis. Tier légendaire — Endgame de prestige

Au-dessus du tier epic (R14 PvP / T10 PvE), un **tier légendaire** custom qui constitue la fin de progression. Power gap maîtrisé (+10% stats max vs epic) — la différenciation est surtout **visuelle et symbolique** : porter du légendaire = "j'ai investi des centaines d'heures dans ce serveur".

### 🏆 Set armor légendaire — Marques de Champion

Pour obtenir une pièce légendaire, il faut **10 Marques de Champion** chez le Forgeron des Légendes (capitale).

#### Path A — Le Marathon (drop aléatoire sur kill PvP)

- À chaque kill PvP, **1 chance sur 100** de drop une "Marque de Champion" (item stackable)
- ⏱️ Estimation : ~1 000 kills par pièce, ~8 000-10 000 kills pour le set complet
- 🎯 Public visé : joueurs réguliers, accumulation passive
- ⚙️ Implémentation : hook `OnPlayerPVPKill` + `urand(1, 100) == 1` → AddItem(MARQUE_CHAMPION)

#### Path B — Le Chasseur de Prime (kill de drapeau on fire)

- **Killer un joueur portant le drapeau on fire 5** → **+1 Marque de Champion**
- **Killer un joueur portant le drapeau on fire 10** → **+3 Marques de Champion**
- 🎯 Public visé : joueurs skilled, ganking ciblé des cibles "on fire"
- 💡 **Effet émergent** : les joueurs on fire deviennent des cibles narratives ("la prime est sur lui"), encourage l'overextend des killers tryhards
- ⚙️ Implémentation : extension du système on fire (hook quand on tue un joueur avec aura kill5/kill10) → AddItem en fonction du tier

#### Récap des chemins

| Path | Source | Rendement | Style |
|---|---|---|---|
| A | Tout kill PvP | 1% chance / kill | Grindy passif |
| B | Kill drapeau on fire 5 | 1 Marque garantie | Skill / ciblé |
| B | Kill drapeau on fire 10 | 3 Marques garanties | High skill / risky |

Les deux paths sont **cumulatifs** : un joueur skilled qui chasse les primes ET farme les kills progresse plus vite. Mais un joueur "objectif" qui kill juste beaucoup pourra aussi terminer son set, ça prend juste plus long.

### ⚔️ Armes légendaires — Coffre Mystique (Pattern 1)

Pour démarrer le système d'event, **Pattern 1 — Coffre Mystique** uniquement (les autres patterns Courier et Boss hebdo viendront plus tard).

#### Mécanique

- Toutes les **2-3 heures** (interval aléatoire), un **Coffre Légendaire** spawn dans une zone contestée aléatoire (parmi les 5 du MVP)
- **Annonce monde** :
  > ⚔️ **Un trésor légendaire est apparu à Tarren Mill !** ⏱️ 30 minutes pour le récupérer.
- Le coffre est **visible sur le minimap** des joueurs proches (compass marker)
- Pour ouvrir le coffre : **canalisation de 5 minutes** (fenêtre pour fighter)
- Le coffre **ne s'ouvre que si 3+ joueurs adverses sont présents dans la zone** (anti-camping solo)
- Si personne n'ouvre dans les 30 min → le coffre disparaît
- Drop : **1 arme légendaire random** dans un pool de 5-10 armes custom (1 par archétype : 1H sword, 2H, staff, bow, dagger, etc.)

#### Stats légendaires (à équilibrer)

- ItemLevel ~95-100 (vs 92 pour T3 Naxx40)
- Stats classiques + bonus visuel (effet glow, particles)
- 1-2 stats légères "OP" : haste rating, crit, etc.

#### Pool d'items custom

À créer (10 entries item_template) :
- "Lame Brisée de Lothar" (1H sword Alliance)
- "Hache de Sang de Saurfang" (2H axe Horde)
- "Bâton du Conseil Disparu" (staff caster)
- "Arc de l'Ombre Éternelle" (bow)
- "Dague de l'Assassin Légendaire" (dagger)
- ... (5 autres à définir selon les types d'armes manquants)

### ⚠️ Considérations balance

| Risque | Mitigation |
|---|---|
| Top players monopolisent le légendaire → écart grandissant | Cap stats légendaire à +10% vs epic. Le légendaire est principalement **visuel + petit edge** |
| Le coffre est trop facile à camper | Le check "3+ adverses présents" force le PvP |
| Le Path A est trop grindy → frustration | Pity timer : après 500 kills sans Marque, drop garanti au prochain kill |
| Les armes légendaires créent une meta unique | Diversifier le pool (5-10 armes différentes) pour éviter qu'une seule soit BiS |

✅ **Décision actée** : serveur en **level 60** pour tout le monde (joueurs + bots), gear classic vanilla R10-R14 + T3 PvE. Les items vanilla sont **customisés en DB** (+resilience +stamina) pour équilibrer le burst des classes WotLK. Pas de gear lvl 70/80 (sets S6/S7/S8/T7-T10 exclus).

---

## 4. Le rôle des playerbots

### Objectif

Donner l'impression d'un serveur peuplé, créer des fronts naturels, garantir qu'il y a toujours du PvP même la nuit.

### Comportement souhaité

#### Distribution
- ~150 bots actifs simultanément (75 Alliance + 75 Horde)
- Répartis sur les zones contestées et capitales
- Les bots de la faction propriétaire **défendent** la zone
- Les bots de la faction adverse **attaquent** périodiquement

#### IA stratégique
- **Mode défense** : bot reste dans le rayon de sa zone, attaque les ennemis qui rentrent
- **Mode attaque** : bot voyage vers une zone contestée (groupe de 3-5 bots pour une vague)
- **Mode patrouille** : bot circule sur les routes entre capitale et zones

#### Equipement
- Tous les bots level 60, gear PvP R14 (déjà setup)
- Montures Conquest custom pour les déplacements
- Sorts/talents PvP via `PremadeSpecName` du module Playerbot

### Pour éviter le côté artificiel

1. **Variation de comportement** : pas tous les bots ne font la même chose. Probabilité de différents states (defend / attack / patrol / idle).
2. **Pas de coordination omnisciente** : un bot ne sait pas où va son équipe. Ils répondent à l'aggro local.
3. **Pertes de la guerre** : les bots peuvent mourir et stay down quelques minutes. Pas d'invincibilité.
4. **Visibility limited** : un bot ne devrait pas "voir" un joueur de l'autre faction à 200 yards et le viser. Detection aggro normale.
5. **Chat émergent** : `RandomBotTalk = 1` active du chat aléatoire qui crée de l'ambiance.
6. **Niveau et gear cohérents** : pas de bot lvl 60 avec un baton de bois (déjà fixé).

---

## 5. Architecture technique

### Modules actifs (état 2026-05-12)

| Module | Rôle | Status |
|---|---|---|
| `mod-conquest-frontline` | **Cœur du gameplay** : capture banners, wallet PC/PB, kill streak, drops Marques, ForcePvP, EquipBots | ✅ |
| `mod-conquest-respawn-capital` | Respawn race → capitale via `OnPlayerCanRepopAtGraveyard` (ResurrectPlayer direct) | ✅ |
| `mod-conquest-group-cap` | Cap groupe à 5 (refuse `OnAddMember` au-delà, bloque raids) | ✅ |
| `mod-conquest-map-restrict` | Azeroth uniquement + whitelist Eversong/Azuremyst (Outland/Northrend bloqués) | ✅ |
| `mod-conquest-gear-manager` | Distribution BIS gear par classe/spé (utilisable par joueurs via NPC 400100) | ✅ |
| `mod-conquest-npc-vendor` | 3 NPCs vendors gossip (400200/400201/400202) avec balance check PC/PB/Marques | ✅ |
| `mod-conquest-lootdrop` | Drop bag-only à la mort (équipement préservé) | ✅ |
| `mod-playerbots` | Les bots + patch `ConquestZoneSpawn` (dispatch capitales raciales via `getRace()`) | ✅ |
| `mod-transmog` | Customisation visuelle joueurs | ✅ |

### Modules désactivés

| Module | Raison |
|---|---|
| `mod-conquest-zone-control` | Remplacé fonctionnellement par `mod-conquest-frontline` (capture via OutdoorPvP framework natif) |
| `mod-conquest-build` | Construction de structures — pas dans le MVP |
| `mod-conquest-guard` | PNJ gardes personnels — pas pertinent |
| `mod-conquest-limit-group` | Remplacé par `mod-conquest-group-cap` |
| `mod-conquest-mounts` | ⏳ Backlog : ownership + anti-vol monture |
| `mod-conquest-core` | Logique respawn/cap déplacée vers modules dédiés |
| `ConquestVehicleVendor.cpp.disabled` | Vendeur de siege engines — pas dans le MVP |

### Architecture détaillée — `mod-conquest-frontline`

C'est le module central. Contient :

- **`OutdoorPvPConquest`** : framework OutdoorPvP custom (TypeId 8), enregistre 4 zones via `RegisterZone(17, 331, 267, 440)`, singleton `g_conquestInstance` pour le hook d'auto-détection
- **`ConquestCapturePoint`** : extends `OPvPCapturePoint`, override `Update`/`HandlePlayerEnter`/`ChangeState`/`ChangeTeam`/`BroadcastProgress`/`BroadcastZoneAnnounce`/`AwardCaptureRewards`
- **`ConquestFrontlineAuto`** : `AllGameObjectScript::OnGameObjectAddWorld` hook qui auto-register les GOs 400010 (avec dédup par position arrondie pour éviter loop infini)
- **`ConquestPoints`** : namespace API wallet (`Get/Add/Spend{Conquest,Battle}Points`), persistance via table `character_conquest_points`
- **`ConquestPointsHooks`** : `PlayerScript` avec `OnPlayerPVPKill` (rewards PB + drops Marques) et `OnPlayerJustDied` (reset streak)
- **`ConquestKillStreak`** : namespace API tracker, persiste `character_conquest_killstreak`, applique auras 23333/23335 aux paliers 5/10
- **`ConquestForcePvP`** : `PlayerScript` qui force le byte flag PvP via `UpdatePvP(true, true) + SetPvP(true)` au login/map/zone change (GM exempt)
- **`ConquestEquipBots`** : `PlayerScript` qui équipe le set R14 rare BIS au login (lvl 60+, détection bot via prefix compte `rndbot%`)

### Patch core AzerothCore

| Fichier | Modification | Pourquoi |
|---|---|---|
| `src/server/game/OutdoorPvP/OutdoorPvP.h` | `OUTDOOR_PVP_CONQUEST = 8` + `MAX_OUTDOORPVP_TYPES = 9` | L'enum vanilla ne supportait pas de TypeId custom > 7 |

---

## 6. Schéma de données

### Tables nouvelles (DB `acore_characters`)

```sql
-- État des zones contestables
CREATE TABLE conquest_zones (
    zone_id INT UNSIGNED PRIMARY KEY,
    name VARCHAR(64) NOT NULL,
    map_id SMALLINT NOT NULL,
    banner_x FLOAT NOT NULL,
    banner_y FLOAT NOT NULL,
    banner_z FLOAT NOT NULL,
    capture_radius FLOAT DEFAULT 50.0,
    capture_time_seconds INT DEFAULT 60,
    current_owner TINYINT DEFAULT 0,  -- 0=neutral, 1=alliance, 2=horde
    captured_at INT UNSIGNED DEFAULT 0,
    capture_count_alliance INT DEFAULT 0,
    capture_count_horde INT DEFAULT 0
);

-- Historique des captures (pour stats / saisons)
CREATE TABLE conquest_capture_log (
    id INT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
    zone_id INT UNSIGNED NOT NULL,
    captured_by_faction TINYINT NOT NULL,
    captured_by_player BIGINT UNSIGNED,
    captured_at INT UNSIGNED NOT NULL,
    held_duration_seconds INT,
    KEY idx_zone (zone_id),
    KEY idx_player (captured_by_player)
);

-- Score faction global (1 ligne par faction, ou élargi en saisons)
CREATE TABLE conquest_faction_score (
    faction TINYINT PRIMARY KEY,  -- 1=alliance, 2=horde
    points INT UNSIGNED DEFAULT 0,
    zones_held TINYINT UNSIGNED DEFAULT 0,
    season_id INT UNSIGNED DEFAULT 1
);

-- (Optionnel V2) Stats joueur PvP
CREATE TABLE conquest_player_stats (
    player_guid BIGINT UNSIGNED PRIMARY KEY,
    captures INT UNSIGNED DEFAULT 0,
    defenses INT UNSIGNED DEFAULT 0,
    kills_in_zone INT UNSIGNED DEFAULT 0,
    deaths_in_zone INT UNSIGNED DEFAULT 0
);
```

### Réutilisation du schéma existant

- `conquest_zone_control` (de `mod-conquest-zone-control`) déjà existant — à conserver ou migrer
- `playerbots_random_bots` — pour piloter les bots via leur module
- `character_homebind` — à hijacker pour le respawn capitale

---

## 7. Fonctionnement détaillé des captures

### Cycle de vie d'une bannière

```
[NEUTRE / DISPUTÉE]
       │
       │ Joueur ennemi entame canal (30-60s)
       ▼
[EN COURS DE CAPTURE — alliance ou horde]
       │
       ├─ Canal interrompu (dégâts/mort) → retour à l'état précédent
       │
       │ Canal terminé sans interruption
       ▼
[CAPTURÉE PAR FACTION]
       │
       │ Faction adverse arrive et capture à son tour
       ▼
[CAPTURÉE PAR FACTION ADVERSE]
```

### Effets visuels et serveur

- **Bannière** change de modèle/displayId selon le propriétaire (déjà géré par `mod-conquest-zone-control` avec 4 entries)
- **PNJ de la zone** : les marchands et gardes changent (via spawn_group ou phasing)
- **Effets sonores** : sons de bataille en arrière-plan
- **Annonces** :
  - `[ALLIANCE] Astranaar est tombé aux mains de la Horde !` (chat monde)
  - `[VOUS] Astranaar : capture en cours par la Horde ! 30s restantes` (broadcast joueurs de la faction propriétaire dans la zone)

### Récompenses

- **Capteur** : **+50 Points de Conquête** (item stackable en inventaire)
- **Tick de zone tenue** : tous les bots/joueurs de la faction propriétaire dans la zone reçoivent **+5 Points de Conquête / 10 min**
- **Faction** : points de score collectif (pour le leaderboard saison)
- **Défenseur** qui kill un capteur en cours : **+20 Points de Bataille**
- **Kill PvP standard** (joueur vs joueur) : **+10 Points de Bataille**
- **Kill bot ennemi** : **+2 Points de Bataille** (limité pour éviter le farm AFK)

⚠️ Risque : un joueur qui meurt avec ses points en bag les drop (cf section "Mort et drop"). Il faut **déposer les points au vendeur en capitale** pour les sécuriser → crée un dilemme stratégique "farmer encore vs aller dépenser".

---

## 8. Logique des bots autour des zones

### Algorithme général (côté `mod-conquest-frontline`)

```
Toutes les 5 minutes :
1. Lire la liste des zones (conquest_zones)
2. Pour chaque zone :
   - Identifier sa faction propriétaire
   - Compter les bots de chaque faction dans le rayon
   - Si déséquilibre majeur → dispatch des bots
3. Pour chaque bot non-affecté :
   - 60% : reste à proximité d'une zone amie
   - 30% : voyage vers une zone ennemie
   - 10% : idle en capitale
```

### Comment driver les bots

Le module Playerbot a son propre RandomTeleportForLevel (qu'on a déjà patché). On va étendre :

```cpp
// Nouvelle option de config
AiPlayerbot.UseFrontlineDispatcher = 1

// Si activé, RandomTeleportForLevel délègue à ConquestFrontlineMgr
// qui choisit une zone selon la faction du bot et l'état du monde
```

### Comportement de combat

- Les bots utilisent leur logique de combat native Playerbot
- Pas de scripting custom de combat
- Ils respectent les ranges, attaquent l'ennemi le plus proche, follow les groupes
- Seul leur **destination** est dictée par notre dispatcher

---

## 9. Roadmap MVP → V2 → V3

### MVP — Prototype jouable (1-2 semaines)

**Objectif** : tester la boucle de gameplay de base avec 5 zones, 150 bots, et la boucle économique (points → vendeurs).

#### Modules custom à créer / modifier
- [x] Créer `mod-conquest-respawn-capital` (respawn à la capitale de race) ✅ via `OnPlayerCanRepopAtGraveyard`
- [x] Créer `mod-conquest-group-cap` (cap groupe à 5) ✅
- [x] Créer `mod-conquest-map-restrict` (bloque Outreterre/Norfendre/instances/raids — version chirurgicale de l'ancien mod-conquest-restrictions) ✅ avec whitelist Eversong/Azuremyst
- [ ] Réactiver `mod-conquest-mounts` proprement — ⏳ backlog
- [x] **Réactiver `mod-conquest-lootdrop`** mais patcher pour ne drop QUE le contenu des sacs (pas l'équipement) ✅
- [x] **NOUVEAU** : `ConquestForcePvP` PlayerScript pour force le byte flag PvP sur maps 0/1 (sans ça `IsOutdoorPvPActive()` retourne false et le framework ignore le joueur) ✅
- [x] **NOUVEAU** : `ConquestEquipBots` PlayerScript pour auto-equip BIS R14/rare PvP au login (résiste aux re-randomize playerbots) ✅

#### Zones et bots
- [x] **Créer 1 GO custom générique** (entry 400010, type 29 GAMEOBJECT_TYPE_CAPTURE_POINT) — sera spawn-and-forget ✅
- [x] **Hook `OnGameObjectAddWorld`** dans `mod-conquest-frontline` : auto-détection + auto-registration des spawns ✅ avec dédup par position (anti-loop infini)
- [x] Spawn des 5 bannières via SQL à Crossroads / Astranaar / Southshore / Tarren Mill / Gadgetzan ✅
- [x] Patch `mod-playerbots` : `ConquestZoneSpawn` → dispatch capitales raciales via `getRace()` ✅
- [x] **NOUVEAU** : core patch `OUTDOOR_PVP_CONQUEST = 8` dans `OutdoorPvP.h` + `MAX_OUTDOORPVP_TYPES = 9` (l'enum vanilla ne supportait pas de TypeId custom > 7) ✅
- [x] **NOUVEAU** : `RegisterZone(17, 331, 267, 440)` dans `SetupOutdoorPvP` — sans ça `m_zoneScript` reste nullptr, `OnGameObjectCreate` ne fire pas, `_capturePoint` reste null, et `Update()` sort early sans tracker les joueurs ✅ (fix critique)
- [ ] **Spawn 3-4 tentes goblins** (Hillsbrad, Ashenvale, Booty Bay, Tanaris) avec vendor template — ⏳ backlog

#### Système économique (currencies + vendeurs)
- [x] **Système de points** : wallet `character_conquest_points` (DB) + API `ConquestPoints::Get/Add/Spend` (architectural pivot — ces points sont stockés en DB par GUID joueur, pas en items inventaire) ✅
- [x] **Distribution capture** : +5 PC distribués sur transition FULL LOCK uniquement (pas sur CHALLENGE intermédiaire) ✅
- [x] **Hook kill PvP** : `OnPlayerPVPKill` qui distribue +1 PB cross-faction ✅
- [x] Créer les **3 NPC vendeurs** (Maître d'Armes PvP / Quartier-maître PvE / Forgeron des Légendes) en capitales avec gossip menus ✅ (entries 400200/400201/400202, 6 spawns Stormwind + Orgrimmar)
- [~] **Renommer les items** 80020 / 80021 → ⚠️ remplacé par le wallet DB `character_conquest_points` (plus robuste, pas de perte sur drop bag à la mort)
- [x] **Créer l'item "Marque de Champion"** ✅ entry 400300 (rare, stack 200, displayid 18491)
- [ ] **NPC welcome capitale** : donne le set PvP rare R10-R12 gratuitement aux nouveaux personnages — ⏳ backlog (les bots ont leur set auto-equip via `ConquestEquipBots`, mais pas encore les nouveaux joueurs)

#### Customisation game-feel (level 60 + balance)
- [x] **SQL** glyphes : `UPDATE item_template SET RequiredLevel = 60 WHERE class = 16 AND RequiredLevel BETWEEN 61 AND 80` ✅
- [x] **SQL** : Buff stamina + resilience sur les items PvP classic R10-R14 ✅ — final tuning : **+50 stam / +15 résilience** par pièce (résilience réduite de 60 à 15 après test)
- [x] **SQL** : Buff étendu aux **rare ranks intermédiaires** (~270 items) ✅
  - Alliance : Knight-Captain / Knight-Lieutenant / Lieutenant Commander
  - Horde : Blood Guard / Legionnaire / Champion
- [x] **SQL** : Créer 4 sets PvP DK custom (Rare + Epic × Alliance + Horde) ✅ — 39 entries 80100-80138 + `AllowableClass = 32` (DK only)
- [ ] Set bonuses 2/4/6 pcs custom — ⏳ backlog (nécessite DBC patch client-side ou système C++ custom)
- [ ] Patch C++ optionnel : `Player::InitGlyphsForLevel` pour débloquer les 6 slots glyphes à lvl 60 — ⏳ backlog (le SQL seul suffit pour le RequiredLevel des glyphes eux-mêmes)

#### Mod-conquest-mounts — ownership et anti-vol
- [ ] Au spawn d'une monture custom : enregistrer `owner_guid` + `owner_team` — ⏳ backlog
- [ ] Hook `OnPlayerEnterVehicle` (ou équivalent) : check ownership selon les 3 règles — ⏳ backlog
  - ✅ Owner → allow
  - ✅ Faction ennemie → allow (vol PvP)
  - ❌ Même faction non-owner → deny avec message "Cette monture appartient à un membre de votre faction."

#### Kill streak visuel "On Fire"
- [x] Hook `OnPlayerPVPKill` : tracker la streak par joueur ✅ — persisté en DB via table `character_conquest_killstreak` (résiste reconnexion)
- [x] À 5 kills : cast d'un sort visible → spell 23333 (Silverwing Flag) "On Fire" ✅
- [x] À 10 kills : remplacement par drapeau Prestige → spell 23335 (Warsong Flag) ✅
- [x] Hook `OnPlayerJustDied` : reset streak + retire les auras drapeau ✅
- [x] Sorts visibles (utilisation de BG flag spells existants) ✅ — pas besoin de créer des sorts custom

#### Tier légendaire (set armor + arme via event)
- [x] Path A légendaire : hook `OnPlayerPVPKill` + `urand(0,99) < 1` (1% chance) → AddItem(400300) ✅
- [x] Path B légendaire : kill victime On Fire → +1 Marque ; kill victime Prestige → +3 Marques ✅
- [ ] **Pity timer Path A** : 500 kills sans drop → garanti — ⏳ backlog
- [ ] **Pattern 1 — Coffre Mystique** : event scheduler — ⏳ backlog
  - [ ] GameObject coffre custom
  - [ ] Canalisation 5 min + check "3+ adverses présents"
  - [ ] Pool de 5-10 armes légendaires custom
  - [ ] Annonce monde au spawn et à l'ouverture
- [x] **NOUVEAU** : `Forgeron des Légendes` (NPC 400202) vend déjà 5 items legendary contre Marques de Champion (Thunderfury, Sulfuras, Atiesh, Warglaive, Warp Slicer) — la voie vendor est en place, le coffre événementiel reste à faire ✅

#### Polish et tests
- [x] Annonces monde lors des captures (zone-wide chat via `BroadcastZoneAnnounce` filtré par `_zoneId`) ✅
- [x] Visual feedback : artKit swap (21 neutral / 2 alliance / 1 horde) sur ChangeState ✅
- [x] Chat feedback périodique : tick 5s pendant phase CHALLENGE, skip sur FULL LOCK ✅
- [~] Test en jeu : capture chain validée bout-en-bout (joueur GM peut capturer via `/pvp` manuel, +5 PC distribués, annonce zone, banner change couleur). Reste à tester : drop bag à la mort + vendor purchase flow par un joueur réel.
- [x] Vérifier les restrictions de monde (Outland/Northrend refusés via `OnPlayerCanEnterMap`) ✅
- [ ] Test event coffre — ⏳ backlog (pas encore implémenté)

**Critères de succès** :
- Les 5 zones se capturent en boucle pendant 1 heure d'observation passive
- 60%+ des bots sont visibles dans/près d'une zone à un moment donné
- Pas de bug bloquant (bot stuck, crash, conflit module)

### V2 — Profondeur de gameplay (4-6 semaines après MVP)

- [ ] Système de saison avec reset périodique des scores
- [ ] Titres et récompenses cosmétiques (transmog) en fonction du score faction
- [ ] Buffs zone-wide pour la faction propriétaire d'une zone (genre +5% XP, +10% speed dans la zone)
- [ ] Stats individuelles joueurs (captures, kills) avec leaderboard
- [ ] Variation de difficulté : certaines zones plus dures à capturer (timer plus long, plus de bots défenseurs)
- [ ] Améliorer l'IA bot : groupe coordonné, retraite quand HP bas

### V3 — Profondeur stratégique (modulaire, à ajouter au cas par cas)

- [ ] **Sièges de capitale** : si une faction perd toutes ses zones extérieures, sa capitale devient attaquable
- [ ] **Événements dynamiques** : "Invasion ! La Horde attaque Stormwind, +50 bots ennemis pendant 30 min"
- [ ] **IA stratégique** : un "commandant fantôme" par faction qui décide où concentrer les attaques (Markov / utility AI)
- [ ] **Routes de ravitaillement** : ajout de PNJ ravitailleurs qui voyagent entre capitale et zones — les tuer affaiblit la défense
- [ ] **Sous-objectifs dans une zone** : 3 sous-points à capturer avant de claim la bannière principale

---

## 10. Conseils pour éviter l'artificialité des bots

1. **Variabilité de comportement** : sans randomness dans les décisions des bots, ça devient mécanique. Utiliser des probabilités pondérées.

2. **Mort réelle** : un bot mort ne respawn pas instantanément. Délai de 60-180s comme un vrai joueur qui ferait son corpse run.

3. **Pas d'omniscience** : un bot ne sait pas qu'un joueur est en train de capturer 5 zones plus loin. Il ne réagit qu'à ce qu'il voit (aggro range standard).

4. **Pas de précision parfaite** : les bots peuvent miss leurs sorts, ne pas optimiser leur rotation, fail certains kills. Trop de skill = sensation d'arène e-sport, pas de monde MMO.

5. **Vie de groupe** : 2-3 bots qui se déplacent ensemble sur une route, c'est plus crédible que 50 bots solo.

6. **Chat passif** : `RandomBotTalk = 1` + scripts custom occasionnels ("ils arrivent par le sud !") simule la communication d'équipe.

7. **Pacing** : pas tous les bots en combat permanent. Certains craftent (fake), discutent près d'un PNJ, regardent autour.

8. **Capacité de fuite** : si un bot est seul face à 3 joueurs, il doit pouvoir fuir (limite l'effet "muraille de loot")

---

## 11. Risques identifiés et mitigations

| Risque | Mitigation |
|---|---|
| Bots qui spam-capturent en boucle sans joueurs | Ralentir la capacité de capture des bots (timer x2 vs joueurs) |
| Une faction domine en permanence | Underdog bonus : bots de la faction perdante gagnent +20% dégâts |
| Population réelle trop basse → tout devient bot vs bot ennuyeux | Réduire dynamiquement le nombre de bots quand pas de joueur connecté |
| Bots stuck dans une zone, pile sur la bannière | Patch IA pour qu'ils se déplacent en cercle dans le rayon de capture |
| Conflits entre modules custom | Tester chaque module isolément avant intégration |
| Performance avec 150 bots actifs | Profiler avec un seul container Docker monitoring, viser <50% CPU |

---

## 12. État actuel du serveur (réf 2026-05-12)

### Phase 1 — Setup serveur ✅

- ✅ AzerothCore migré sur fork Playerbot (branch Playerbot)
- ✅ `mod-conquest-respawn-capital` : race → capitale via `OnPlayerCanRepopAtGraveyard` + `ResurrectPlayer` + `SpawnCorpseBones` (évite double-tp graveyard/capital qui bloquait le loading screen)
- ✅ `mod-conquest-group-cap` : max 5 joueurs (refuse `OnAddMember` au-delà, bloque raids)
- ✅ `mod-conquest-map-restrict` : Azeroth uniquement + whitelist Eversong/Azuremyst pour BE/Draenei (Outland/Northrend bloqués via `OnPlayerCanEnterMap`)
- ✅ 150 bots playerbots lvl 60 (pool 655 chars dont 65 DK régénéré via `DeleteRandomBotAccounts=1`)
- ✅ PvP faction vs faction (`GameType = 1`) + `ConquestForcePvP` qui force le byte flag PvP sur maps 0/1 (GM exempt)

### Phase 2 — Balance ✅

- ✅ SQL glyphes 61-80 → `RequiredLevel = 60`
- ✅ `+50 stamina` sur 187 items vanilla R10-R14 PvP
- ✅ `+15 résilience` sur 272 items PvP (réduit de 60 après tuning)
- ✅ Buff stamina/résilience étendu aux **rare ranks** (~270 items) :
  - Alliance : `Knight-Captain's`, `Knight-Lieutenant's`, `Lieutenant Commander's`
  - Horde : `Blood Guard's`, `Legionnaire's`, `Champion's`
- ✅ 4 sets DK custom créés (39 entries 80100-80138) :
  - Rare Alliance : `Knight-Lieutenant's Dreadplate` (entries 80100-80108)
  - Rare Horde : `Blood Guard's Dreadplate` (entries 80110-80118)
  - Epic : `Sanctified Scourgelord` Alliance + Horde
  - `AllowableClass = 32` (DK only) ; faction lock via naming
- ⚠️ Set bonuses 2/4/6 pcs non implémentés (DBC patch ou C++ custom requis — backlog)

### Phase 3 — Boucle de capture ✅

- ✅ **Core patch** : `OUTDOOR_PVP_CONQUEST = 8` ajouté à [src/server/game/OutdoorPvP/OutdoorPvP.h](../src/server/game/OutdoorPvP/OutdoorPvP.h), `MAX_OUTDOORPVP_TYPES` bumpé à 9
- ✅ `mod-conquest-frontline` créé avec :
  - `OutdoorPvPConquest` framework (TypeId 8, singleton via `g_conquestInstance`)
  - `ConquestCapturePoint` (override `Update`/`HandlePlayerEnter`/`ChangeState`/`ChangeTeam`)
  - GameObject 400010 type 29 (`GAMEOBJECT_TYPE_CAPTURE_POINT`), radius 45y, neutralPercent 30, minTime 30, maxTime 60
  - 5 spawns SQL : Crossroads / Astranaar / Southshore / Tarren Mill / Gadgetzan (guid 5400010-5400014)
  - Auto-register via `AllGameObjectScript::OnGameObjectAddWorld` + dédup par position arrondie (anti-loop infini — sans ce dédup, `SetCapturePointData` re-trigger le hook → boucle infinie)
  - **Fix critique** : `RegisterZone(17, 331, 267, 440)` dans `SetupOutdoorPvP` pour activer `m_zoneScript` → sans ça `_capturePoint` reste nullptr et `Update()` sort early
  - `outdoorpvp_template` SQL row (TypeId=8, ScriptName=`outdoorpvp_conquest`)
- ✅ Wallet `character_conquest_points` (guid, conquest_points, battle_points, updated_at) + API `ConquestPoints::Get/Add/Spend{Conquest,Battle}Points`
- ✅ **Capture reward** : +5 PC distribués via `BroadcastZoneAnnounce` + `AwardCaptureRewards` **uniquement sur transition FULL LOCK** (`OBJECTIVESTATE_ALLIANCE`/`HORDE`), pas sur les phases CHALLENGE intermédiaires
- ✅ **Kill reward** : +1 PB via `OnPlayerPVPKill` cross-faction
- ✅ Chat feedback complet :
  - `HandlePlayerEnter` → "Tu es entré en zone de capture : [Zone]"
  - Tick 5s progression → `[Zone] Capture : 65% (Alliance)` (skip quand verrouillé pour éviter spam 100%)
  - Annonce zone-wide au lock → `[Conquête] Gadgetzan capturée par Alliance !`
- ✅ Visuel artKit swap dans `ChangeState` (21 neutral / 2 alliance / 1 horde)
- ✅ Format `fmt::format` ({} placeholders) appliqué partout dans les `PSendSysMessage`
- ✅ `Logger.conquest=4` + `Logger.outdoorpvp=4` dans worldserver.conf

### Phase 4 — NPCs vendors ✅

- ✅ `mod-conquest-npc-vendor` réactivé (ancien `ConquestVehicleVendor` désactivé en `.cpp.disabled`)
- ✅ 3 CreatureScripts custom avec gossip menu + balance check + refund si inventaire plein :
  - `ConquestQuartierMaitre` (entry 400200) — 6 accessoires PvP pour Points de Conquête
  - `ConquestSergentDArmes` (entry 400201) — 6 consommables (potions/élixirs) pour Points de Bataille
  - `ConquestForgeronLegendes` (entry 400202) — 5 items legendary pour Marques de Champion (Thunderfury, Sulfuras, Atiesh, Warglaives, Warp Slicer)
- ✅ 6 spawns SQL : 3 à Stormwind Trade District (Alliance) + 3 à Orgrimmar Valley of Strength (Horde)

### Phase 5 — Kill streak & Légendaire ✅ (mis à jour builds #64-68)

- ✅ Item **Marque de Champion** entry 400300 (qualité rare, stack 200, displayid 18491)
- ✅ Table `character_conquest_killstreak` (guid, current_streak, max_streak, updated_at)
- ✅ `ConquestKillStreak` tracker : `OnKill` incrémente, `OnDeath` reset à 0
- ✅ **Pennons racialisés** (visuels iconiques Argent Tournament) :

  | Race | Pennon 5 kills | Pennon Champion 10 kills |
  |---|---|---|
  | Humain | 66367 | 62727 |
  | Orc | 66369 | 63444 |
  | Nain | 66363 | 63440 |
  | Elfe nuit | 66368 | 63443 |
  | Mort-vivant | 66365 | 63441 |
  | Tauren | 66370 | 63445 |
  | Gnome | 66366 | 63442 |
  | Troll | 66371 | 63446 |
  | Elfe sang | 66360 | 63438 |
  | Draenei | 66362 | 63439 |

- ✅ **Aura compagne** 47292 castée AVEC le pennon aux paliers 5 et 10 (via `CastSpell`, les pennons via `AddAura`)
- ✅ **Tiers supérieurs** (build #67) :
  - 20 kills → aura 71188 "TUEUR D'ÉLITE"
  - 40 kills → aura 71193 "LÉGENDAIRE" (retire 71188)
  - 80 kills → aura 71195 "DIEU DE GUERRE" (retire 71193)
- ✅ **Drop Marques de Champion** dans `OnPlayerPVPKill` :
  - Victime portant aura On Fire (pennon racial) → +1 Marque au killer
  - Victime portant aura Prestige (pennon Champion) → +3 Marques au killer
  - Sinon → 1% chance globale (Path A)
- ✅ Override config possible : `KillStreakOnFireSpell=0`/`PrestigeSpell=0` → racial mapping ; valeur >0 → spell unique pour tout le monde
- ⚠️ Pity timer Path A (500 kills sans drop garanti) — backlog
- ⚠️ Pattern 1 Coffre Mystique — backlog

### Phase 6 — Bots équipement ✅

- ✅ `ConquestEquipBots` hook `OnPlayerLogin` (skip si lvl < 60)
- ✅ Détection bot via préfixe compte `rndbot%` (cache de 65 account IDs)
- ✅ **BIS table rare set hardcodée** par slot/faction :

  | Slot | Alliance | Horde |
  |---|---|---|
  | Head + Shoulders | Lieutenant Commander's | Champion's |
  | Chest + Legs | Knight-Captain's | Blood Guard's |
  | Hands + Feet | Knight-Lieutenant's | Legionnaire's |
  | Autres | Fallback R14 → R10 | Fallback R14 → R10 |

- ✅ DK utilise 80100-80108 (Alliance) / 80110-80118 (Horde) — set custom Dreadplate
- ✅ Shield filter : Warrior/Paladin/Shaman uniquement peuvent équiper offhand
- ✅ Restauration automatique du set à chaque login (résiste aux re-randomize playerbots)

### Phase 7 — Dispatch bots ✅

- ✅ Override `RandomTeleportForLevel` dans `mod-playerbots` :
  - `AiPlayerbot.ConquestZoneSpawn = 1` activé
  - `AiPlayerbot.ConquestCapitalPct = 100` (actuel : tous les bots aux capitales raciales)
  - Mapping `getRace()` → 8 capitales (Stormwind/Ironforge/Darnassus/Exodar pour Alliance, Orgrimmar/Undercity/Thunder Bluff/Silvermoon pour Horde)
- ✅ Intervalle teleport `MinRandomBotTeleportInterval = 60` / `Max = 300` secondes
- ✅ Bot pool wipe + recréation via `DeleteRandomBotAccounts=1` workflow (auto-stop serveur après wipe, restart avec =0 pour création)

### Modules legacy désactivés ✅

- ✅ `mod-conquest-zone-control` (remplacé fonctionnellement par `mod-conquest-frontline`)
- ✅ `ConquestVehicleVendor.cpp` → `.cpp.disabled` (siege engines pas dans MVP)
- ✅ Legacy SQL `gvg_vehicle_vendor_npc.sql` → `.disabled`
- ✅ SQL legacy nettoyée : retrait des colonnes obsolètes `scale`, `mechanic_immune_mask`, `spell_school_immune_mask` (renommées en `CreatureImmunitiesId` dans le schéma actuel)

### Phase 8 — Vendors étendus L60 ✅ (builds #70-77)

Architecture **DB-driven** pour gérer ~1000 items équipement L60 sur NPCs catégorisés.

**Schéma DB (`db-world`)** :
- `conquest_vendor_npc` (npc_entry, name, category, description) — catalogue des 15 NPC roles
- `conquest_vendor_items` (npc_entry, item_id, price, currency=PB/PC/GOLD, slot_label, display_order)

**15 NPCs (entries 400210-400252)** spawn dans les **8 capitales** (120 spawns) :

| Role | Entry | Catégorie | Currency |
|---|---|---|---|
| Sergent PvP Rare | 400210 | Lieutenant Commander / Blood Guard | PB |
| Maréchal PvP Épique | 400211 | Field Marshal / Warlord | PB |
| Armurier d'Armes PvP | 400212 | Grand Marshal / High Warlord | PB |
| Champion T1 (MC) | 400220 | Set Tier 1 + offset | PC |
| Champion T2 (BWL+Ony) | 400221 | Set Tier 2 | PC |
| Champion T2.5 (AQ40) | 400222 | Set Tier 2.5 | PC |
| Champion T3 (Naxx40) | 400223 | Set Tier 3 | PC |
| Bijoutier de Guerre | 400230 | Anneaux/colliers/talismans tous tiers | PC |
| Forgeron Armes T1-T3 | 400240-400243 | Armes par tier | PC |
| Apothicaire | 400250 | Potions / élixirs / flasques | GOLD |
| Cuistot | 400251 | Bouffe / drinks | GOLD |
| Quincailler | 400252 | Bandages / huiles / pierres / réactifs | GOLD |

**920 items remplis** via bulk SQL `INSERT...SELECT` depuis `item_template` filtrés par patterns de noms (sets vanilla nommés `Battlegear of Might`, `Lawbringer%`, `Nightslayer%` etc).

**UX : vraie fenêtre vendor WoW** (avec icônes + tooltips) au lieu de gossip-list :
1. Click NPC → gossip menu avec sections (slot_labels distincts)
2. Click section → `WorldSession::SendListInventory(creature_guid, sub_entry)` ouvre la **fenêtre vendor standard** filtrée
3. Sub-entries (410000+) générés dynamiquement au boot via `ObjectMgr::AddVendorItem(persist=false)` dans `OnStartup` (timing critique : APRÈS `LoadVendors` qui sinon écraserait notre store)
4. Achat : hook `PlayerScript::OnPlayerBeforeBuyItemFromVendor` intercepte, vérifie le solde PB/PC réel via `conquest_vendor_items`, débite, ou annule (`item = 0`)

**Fix critique** : `UNIT_NPC_FLAG_GOSSIP = 0x01 = 1` et `UNIT_NPC_FLAG_VENDOR = 0x80 = 128` (NOT 4096 qui est `BANKER`). npcflag final = **129**.

**Commande joueur** : `.conquest points` affiche solde PB / PC / killstreak.

### Phase 9 — Smart dispatcher + défense post-capture ✅ (builds #79-81)

Remplace le round-robin par un scoring intelligent basé sur l'état réel des bannières.

**Scoring** dans `RandomTeleportForLevel` pour chaque bannière same-continent :
- État : enemy fully-locked = 1.0 / contesté contre nous = 1.2 / contesté pour nous = 0.5 / neutre = 0.8 / ours fully-locked = skip
- Distance : `exp(-dist / 3000)` (decay exponentiel)
- Jitter : `frand(0.9, 1.1)` (anti-convergence)
- Sélection : score max gagne

**Source** : `OutdoorPvPConquest::GetAllBanners()` snapshot des banners LIVE (slider, max, position, zone). Filtre les **banners phantom** (GO supprimé via `.gob delete` mais `ConquestCapturePoint` encore en mémoire) via `cp->IsBannerAlive()`.

**Registry dynamique** `s_dynamicBannerSlots` : crée TravelDestination à la volée pour CHAQUE banner LIVE (keyed by position arrondie 5y), supportant donc les bannières placées par GM manuellement.

**Défense post-capture 30s** :
- À l'`atFullLock` : `ConquestScheduleDefenseRedispatch(botGuid, now + 30000)` au lieu de redispatch immédiat.
- PlayerScript `OnPlayerBeforeUpdate` check le timer chaque tick → quand expiré, `RandomTeleportForLevel(bot)`.
- Bot reste sur place 30s puis part vers une nouvelle zone via smart dispatch.
- Configurable : `ConquestFrontline.DefenseDurationMs = 30000`.

**Fix stuck final WP** : bot bloqué >15s sur le dernier WP (la bannière) → teleport direct dans rayon 25y → entre dans la zone de capture (45y).

### Phase 10 — Système de waypoints curés ✅ (builds #58-63)

Voir [bots-ai-roadmap.md](bots-ai-roadmap.md) Phase A.7 pour détails complets.

- GameObject 400100 "Conquest Waypoint" — displayId 6671 (Lightwell), phaseMask=2 (invisible aux joueurs), GM auto-phase=3.
- `ConquestWaypointMgr` : graphe scanned au boot, edges auto <500y, Dijkstra pour pathfinding bot→banner.
- Routing hybride dans `MoveToTravelTargetAction` : >300y du target = WP graph, <300y = navmesh direct, fallback sub-step 100y.
- Détour opportuniste : bot passant <250y d'une banner non-locked-for-us se détourne pour contester.
- Commandes GM : `.conquest waypoints reload`, `.conquest waypoints count`.

### Backlog (Phase 11+) ⏳

- ⏳ Anti-farm sur kills (cooldown même victime)
- ⏳ Pattern 1 Coffre Mystique (events scheduler + 5-10 armes légendaires custom)
- ⏳ Pity timer Path A légendaire (500 kills sans drop → garanti)
- ⏳ Scoreboard public max_streak (table + leaderboard chat command)
- ⏳ Welcome NPC starter (donne R10 PvP gear aux nouveaux persos)
- ⏳ Tentes goblins (3-4 vendors neutres en zones contestées avec consommables)
- ⏳ Mounts Conquest custom + ownership/anti-vol (`mod-conquest-mounts` à réactiver)
- ⏳ Set bonuses 2/4/6 pcs (DBC patch ou système C++ custom)
- ⏳ Hook randomize playerbots pour préserver le set custom entre cycles
- ⏳ Crowd-cap dans smart dispatcher (pénalité si trop de bots déjà sur la banner)
- ⏳ Defense behavior actif (wander random pendant les 30s au lieu de stay)
- ⏳ Vrais groupes WoW (Group::AddMember pour les bots arrivés en squad)

---

## 13. Prochaines actions concrètes (par ordre)

**Phase 1 — Restrictions de base et level 60 cohérent** ✅ TERMINÉE
1. ✅ **Créer `mod-conquest-map-restrict`** : bloquer Outreterre / Norfendre / instances / raids / BG / arènes
2. ✅ **Créer `mod-conquest-respawn-capital`** : respawn à la capitale de race (via `OnPlayerCanRepopAtGraveyard`)
3. ✅ **Créer `mod-conquest-group-cap`** : cap groupe à 5

**Phase 2 — Customisation level 60** ✅ TERMINÉE
4. ✅ **SQL** : remap glyphes 61-80 à RequiredLevel = 60
5. ✅ **SQL** : ajouter stamina (+50) + résilience (+15) sur les items R10-R14 PvP + rare ranks
6. ⏳ **Patch C++ optionnel** : `InitGlyphsForLevel` pour débloquer les 6 slots à lvl 60 (backlog, SQL seul suffit)
7. ✅ **Test en jeu** : balance vérifiée (résilience réduite de 60 à 15 après feedback)

**Phase 3 — Boucle économique de base** ✅ TERMINÉE
8. ✅ **`ConquestZoneSpawn`** (capitales raciales via `getRace()` switch, 100% capitales actuel)
9. ✅ **`mod-conquest-lootdrop`** patché pour drop uniquement les bags
10. ✅ **Spawn 5 bannières SQL** dans Crossroads/Astranaar/Southshore/Tarren Mill/Gadgetzan
11. ✅ **Système de points** : wallet DB `character_conquest_points` + hooks capture (+5 PC) + kill PvP (+1 PB)
12. ✅ **3 vendeurs** : Quartier-maître Conquête / Sergent d'Armes / Forgeron des Légendes (entries 400200-400202, 6 spawns)
13. ✅ **Core patch** `OUTDOOR_PVP_CONQUEST = 8` + `RegisterZone(4 zones)` (fix critique pour activer le tracking framework)
14. ✅ **`ConquestForcePvP`** : force le byte flag PvP sur maps 0/1 (sans ça `IsOutdoorPvPActive()` retourne false)
15. ✅ **`ConquestEquipBots`** : auto-equip BIS rare set au login lvl 60+ (résiste re-randomize)

**Phase 4 — Système on fire** ✅ TERMINÉE
16. ✅ **Hook OnPlayerPVPKill** : tracker streaks (table `character_conquest_killstreak`), cast sorts drapeau à 5 (spell 23333) / 10 (spell 23335)
17. ✅ **Hook OnPlayerJustDied** : reset streak, dispel drapeaux

**Phase 5 — Tier légendaire** ✅ PARTIELLE
18. ✅ **Item "Marque de Champion"** entry 400300 (rare, stack 200)
19. ✅ **Path A** : 1% chance drop sur kill PvP cross-faction
20. ✅ **Path B** : kill victime On Fire → +1 Marque ; kill Prestige → +3 Marques
21. ✅ **Forgeron des Légendes** : 5 items legendary contre Marques (Thunderfury/Sulfuras/Atiesh/Warglaive/Warp Slicer)
22. ⏳ **Pity timer Path A** (500 kills garanti) — backlog
23. ⏳ **Pattern 1 Coffre Mystique** (event scheduler + GO + pool armes custom) — backlog

**Phase 6 — Polish** ✅ PARTIELLE
24. ✅ **Annonces monde** : zone-wide chat à la capture (FULL LOCK uniquement), visual artKit swap, ticks progression 5s
25. ✅ **Format `fmt::format`** ({} placeholders) appliqué partout
26. ✅ **Logger.conquest=4** + Logger.outdoorpvp=4 (worldserver.conf)
27. ⏳ **Test bout-en-bout joueur réel** : reste à vérifier vendor purchase flow + drop bag à la mort
28. ⏳ **Tuning** : drop rates, prix vendeurs en fonction du feedback (en cours)

**Phase 7+ — Backlog** (ordre indicatif)
29. ⏳ Anti-farm sur kills (cooldown même victime)
30. ⏳ NPC welcome capitale donnant R10 PvP starter aux nouveaux persos
31. ⏳ Tentes goblins (3-4 vendors neutres en zones contestées)
32. ✅ Mounts Conquest custom + ownership/anti-vol (Phase 8 ci-dessous)
33. ⏳ Set bonuses 2/4/6 pcs DK (DBC patch ou système C++)
34. ⏳ Scoreboard public max_streak (leaderboard chat command)
35. ⏳ Coffre Mystique légendaire (Pattern 1)
36. ⏳ Hook randomize playerbots pour préserver le set custom

---

## 14. Consolidation Vendors PvP/PvE Conquest (2026-05-14)

Refactor majeur du système vendor : passé de **18 PNJs fragmentés** (3 gossip hardcodés legacy + 15 DB-driven) à **7 PNJs consolidés** avec un pattern gossip→vendor window unifié.

### Architecture vendor (`mod-conquest-npc-vendor`)

L'infrastructure [`ConquestVendorMgr`](../modules/mod-conquest-npc-vendor/src/ConquestVendorMgr.h) + [`ConquestVendorScript`](../modules/mod-conquest-npc-vendor/src/ConquestVendorScript.cpp) supporte le pattern :
1. Player click PNJ → `CanCreatureGossipHello` : affiche le solde PB/PC + liste des sections (`slot_label` distincts)
2. Click section → `SendListInventory(creature_guid, sub_entry)` ouvre la fenêtre vendor WoW classique avec icônes/tooltips, filtrée sur la section
3. Achat via mécanisme vanilla → `OnPlayerBeforeBuyItemFromVendor` hook intercepte, vérifie solde PB/PC, déduit, annule si insuffisant

**Table `conquest_vendor_items`** : `(npc_entry, item_id, price, currency, slot_label, display_order)`. Currency enum PB/PC/GOLD. Le `ConquestVendorMgr::Load()` génère des sub-entries (base 410000) par couple `(main_npc, slot_label)` et populate l'inventaire `npc_vendor` en mémoire via `sObjectMgr->AddVendorItem`.

### 7 PNJs (entries 400260-400266)

Tous : goblin femelle (`displayId 17819`), subname `Azeroth Conquest`, faction 35, `npcflag = 129` (VENDOR + GOSSIP), spawnés dans les 8 capitales.

| Entry | Nom | Sections (slot_label) | Devise |
|-------|-----|----------------------|--------|
| `400260` | Armes | T1 / T2 / T2.5 / T3 / PVP Rare / PVP Épique | PC (PvE) / PB (PvP) |
| `400261` | Armures | idem | idem |
| `400262` | Offpart | idem (capes, brassards, ceintures, bottes) | idem |
| `400263` | Offset | PVE / PVP Rare / PVP Épique (bijoux : anneaux, cous, trinkets) | idem |
| `400264` | Légendes | Légendaires (1000 PC) / Epics Iconiques (500 PC) | PC |
| `400265` | Consommables | Potions / Elixirs / Flasques / Nourriture | or (vendor BuyPrice) |
| `400266` | Réactifs | Bandages / Pierres / Huiles / Runes Mage | or |

#### Mapping `InventoryType` → PNJ
- Armures (400261) : 1 (head), 3 (shoulders), 5 (chest), 7 (legs), 10 (hands)
- Offpart (400262) : 6 (waist), 8 (feet), 9 (wrists), 16 (back)
- Offset (400263) : 2 (neck), 11 (finger), 12 (trinket)
- Armes (400260) : 13/14/15/17/21/22/23/25/26/28 (weapons + shields + off-hands)

#### Logique de pricing
- **Gratuit** (price = 0) : T1, T2, T2.5, PVP Rare (armures pré-max)
- **Pricé** : T3 (120-180 PC selon type), PVP Épique (60-250 PB selon type)
- **Légendaires** : 500-1000 PC (top-end vanilla)
- **Consommables/Réactifs** : BuyPrice item natif (or)

#### PVP Rare weapons élargi
Au lieu de seulement Lieutenant Commander / Blood Guard (2 items), patterns ajoutés : `Marshal's / Legionnaire's / Knight-Captain's / Stone Guard's / Champion's`, filtré `quality=3 + class=2/4 + InventoryType weapon/shield`. Résultat ~6+ items.

### SQL apply manuel requis

Le `dbimport` AzerothCore ne ré-applique pas automatiquement les fichiers SQL module `data/sql/db-world/` après modification (tracking par hash dans `updates` table mais entrée pas créée pour ces fichiers spécifiques). Workflow :

```bash
docker exec -i ac-database mysql -uroot -ppassword acore_world < <file.sql>
docker compose restart ac-worldserver  # recharger creature_template + ConquestVendorMgr en mémoire
```

`INSERT IGNORE INTO conquest_vendor_items` (au lieu de `INSERT INTO`) dans les fichiers items, pour gérer les overlaps de filtres (e.g., items à `ItemLevel 78` matchent T2 ET T2.5).

---

## 15. Système véhicules custom + GMK Vendor (2026-05-14/15)

Module `mod-conquest-mounts` réactivé. GMK est un vendor goblin (entry **400101**) qui vend 9 véhicules custom avec ownership, faction inheritance, et accessoires (turrets + chairs).

### GMK Vehicle Vendor (`mod-conquest-npc-vendor/src/ConquestVehicleVendor.cpp`)

#### Menu gossip faction-aware (pricing PC)

| Véhicule | Prix PC | Entry Alliance | Entry Horde |
|----------|---------|----------------|-------------|
| Pisteur M2 (scout rapide) | 50 | 400209 | 400310 |
| Baroudeur P-W8 (siege medium) | 80 | 400311 | 400200 |
| Destructeur B27 (siege lourd) | 120 | 400201 | 400312 |
| Léviathan 330 (premium, scale 0.25, speed 3) | 250 | 400314 | 400315 |
| Protecteur E800 (40k HP, partagé) | 100 | 400326 | 400326 |
| Catapulte (long range, partagé) | 30 | 400202 | 400202 |
| Démolisseur (mid range, partagé) | 50 | 400205 | 400205 |
| Lanceur de glaive violet (Alliance) | 40 | 400203 | (caché) |
| Lanceur de glaive jaune (Horde) | 40 | (caché) | 400204 |

#### Flow d'achat
1. `OnGossipHello` filtre les options par `bot->GetTeamId()` (l'entry de la faction opposée à 0 = option cachée). Affiche `Solde : X PC` + label `>> Nom - Prix PC`.
2. `OnGossipSelect` : check solde via `ConquestPoints::GetConquestPoints`, refuse si insuffisant, sinon `SpendConquestPoints(player, price)`.
3. `SummonCreature(entry, ...)` 5y devant le joueur, `TEMPSUMMON_TIMED_DESPAWN 300000` (5min).
4. `vehicle->SetPhaseMask(1, true)` force phase 1 (visible par tous, peu importe le phase du GM acheteur).
5. `ConquestRegisterVehicleOwner(vehicle, player)` (exporté par mod-conquest-mounts) :
   - `vehicle->SetFaction(owner->GetFaction())` → allié friendly, ennemi hostile
   - `RemoveUnitFlag(NON_ATTACKABLE | IMMUNE_TO_PC | IMMUNE_TO_NPC)` → ennemis peuvent attaquer
   - Stocke `(vehicleGUID, ownerGUID)` dans `s_vehicleOwners` map
6. Si spawn échoue, `AddConquestPoints` rembourse.

### Ownership et anti-vol via `PassengerBoarded` ([`ConquestMounts.cpp`](../modules/mod-conquest-mounts/src/ConquestMounts.cpp))

Le `ConquestSiegeEngineAI::PassengerBoarded(passenger, seatId, apply=true)` :
- Si le passager est un joueur ET pas le owner stocké → `passenger->ExitVehicle()` immédiat. Seul l'acheteur peut monter son véhicule.
- Allies de même faction voient le véhicule friendly mais ne peuvent pas le voler (kicked au boarding).
- Ennemis voient le véhicule hostile et peuvent l'attaquer mais pas le monter.

### Personnalisations par entry (C++ override au spawn)

`ConquestSiegeEngineAI::InitializeAI` + miroir `ConquestSiegeEngineVehicle::OnInstall` appliquent par entry :
- **Speed** (`SetSpeed MOVE_WALK / MOVE_RUN`) — M2 = 3, P-W8 = 2, B27 = 1, Léviathan = 3, Protecteur = 1, Catapulte = 3, Glaives = 2, Démolisseur = 2
- **Scale** (`SetObjectScale`) — M2 = 0.5, P-W8 = 0.75, B27 = 1, Léviathan = 0.25, Protecteur = 1
- **HP exact** (`SetMaxHealth/SetHealth`) via `GetSiegeEngineMaxHealth()` helper : M2 = 20000, P-W8 = 30000, B27 = 40000, Léviathan = 50000, Protecteur = 40000
- **Phase 1 force** (`SetPhaseMask(1, true)`) sur le parent + chaque passager au `OnInstall` (couvre les cas où le GM achète en phase 3)

### Accessoires (turrets + chairs + lance-flammes)

Toutes ont `VehicleId 436` (Siege Turret) → **mountable directement** (click → player rides), spellclick `67830` (Ride Vehicle), faction 35, flags appropriés.

| Entry | Nom | DisplayID | Spells | Utilisé sur |
|-------|-----|-----------|--------|-------------|
| `400211` | Canon massif | 29488 | 67461 + 66541 | B27 seat 7 (scale C++ 1.5) |
| `400316` | Tourelle Léviathan | 28875 | 67452 + 67461 | Léviathan seat 7 (scale 0.3) |
| `400317` | Tourelle pisteur M2 | 29489 | 67452 | M2 seat 7 (scale 0.75) |
| `400318` | Tourelle baroudeur P-W8 Alliance | 25301 | 67461 | P-W8 Alliance seat 7 |
| `400321` | Tourelle baroudeur P-W8 Horde | 28106 | 67461 | P-W8 Horde seat 7 |
| `400319` | Tourelle destructeur B27 Horde | 28106 | 66541 | B27 Horde seats 1+2 (scale 0.75) |
| `400320` | Tourelle destructeur B27 Alliance | 25301 | 66541 | B27 Alliance seats 1+2 (scale 0.75) |
| `400327` | Canon de protecteur E800 | 28526 | 66541 + 66186 | Protecteur seat 7 |
| `400328` | Tourelle de protecteur E800 | 29489 | 67452 | Protecteur seats 1+2 |
| `400313` | Siege chair | 29205 (Turkey Chair) | 46598 | P-W8 seats 1+2, Léviathan seats 1+2 (scale 0.3) |
| `400322` | Lance flamme P-W8 Alliance | 29424 | 66183 + 66186 | (deprecated v9, replaced by Siege chair) |
| `400323` | Lance flamme P-W8 Horde | 30080 | 66183 + 66186 | (deprecated v9) |
| `400324` | Lance-flamme Léviathan Alliance | 29424 | 66183 + 66186 | (deprecated v9) |
| `400325` | Lance-flamme Léviathan Horde | 30080 | 66183 + 66186 | (deprecated v9) |

#### Spells vanilla utilisés
- `67461 + 67462` : Siege Turret (Wintergrasp, mais 67462 retiré sur demande)
- `67452 + 66541` : Keep Cannon (heavy bombing)
- `66183 + 66186` : Flame Turret (clones 34778 Alliance / 36356 Horde)
- `46598` : Ride Vehicle generic (chairs)

### Bugs critiques rencontrés (et fixés)

#### 1. Conflit d'entries 400201 / 400211
- `mod-conquest-npc-vendor` SQL legacy redéfinissait 400201 ("Sergent d'Armes") et 400211 ("Marechal PvP Épique") → écrasait les définitions véhicule.
- Fix : suppression des claims vendor sur ces entries + relocate les vendors dans la plage 400260-400266.

#### 2. Schema SQL obsolète
- Les anciens `gvg_siege_engines.sql` utilisaient des colonnes `scale`, `trainer_*`, `mechanic_immune_mask`, `spell_school_immune_mask` qui n'existent plus dans le schéma AzerothCore actuel.
- Erreur silencieuse : `dbimport` échouait à appliquer, mais sans signal clair. Le user en jeu voyait "creature not found".
- Fix : réécriture en migration v2/v3/v6/v8/v10/v14 avec uniquement les colonnes du schéma courant, application manuelle via `mysql exec`.

#### 3. Léviathan vehicle non drivable
- `VehicleId 340` (Flame Leviathan boss vehicle) : tous les seats configurés comme accessory seats dans `vehicle.dbc`, aucun seat driver-capable exposé pour player entry.
- Fix : change `VehicleId 340 → 514` (Horde Siege Engine, seat 0 = driver fonctionnel). On garde le visuel Léviathan (display 28875). Trade-off : 4 seats au lieu de 5 (2 chairs + 1 turret + 1 driver, au lieu de 4 chairs).

#### 4. `npc_spellclick_spells` requirement
- [`ObjectMgr.cpp:4029`](../src/server/game/Globals/ObjectMgr.cpp#L4029) : le parent vehicle DOIT avoir une entrée dans `npc_spellclick_spells` pour que ses `vehicle_template_accessory` rows soient chargées au boot.
- Léviathan (cloné de 33113 boss) avait `npcflag=0` et pas de spellclick → tous ses 5 accessoires rejetés silencieusement.
- Fix : `INSERT npc_spellclick_spells (400314/400315, 46598+66245)` + `UPDATE creature_template SET npcflag=16777216` (UNIT_NPC_FLAG_SPELLCLICK).

#### 5. `minion = 1` brisait les accessoires existants
- Hypothèse initiale : vanilla 33113 utilise `minion=1` dans `vehicle_template_accessory`, donc fix Léviathan = appliquer minion=1 partout.
- Réalité : avec `minion=1`, les accessoires destruction turret qui fonctionnaient avant ont disparu (UNIT_MASK_ACCESSORY changeait leur comportement).
- Fix : revert `minion=0` sur toutes nos lignes. Le vrai bug Léviathan était ailleurs (npcflag + spellclick, cf. point 4).

#### 6. **`CREATURE_FLAG_EXTRA_GHOST_VISIBILITY` (0x400)** dans `flags_extra=344407930`
- **Bug racine d'invisibilité aux non-GM** : bit 10 du `flags_extra` rend la creature visible **uniquement aux joueurs morts (fantômes)**. GMs voient tout par override, joueurs vivants en phase 1 ne voient rien.
- Hérité du SQL legacy `344407930` du user. Appliqué sur 27 entries (tous les vehicles + accessoires).
- Fix : `UPDATE creature_template SET flags_extra = 344406906` (= `344407930 & ~0x400`).

#### 7. Auberdine coast point sur l'eau
- Coast landing TP utilisait `(6404, 514, 12)` qui est **exactement** la position d'une bannière Darkshore + au niveau de l'eau.
- Bots TPed dans l'eau → nageaient indéfiniment.
- Fix : coast point Auberdine déplacé à `(6488, 472, 22)` (plaza centrale, walkable plateforme bois).
- Plus globalement : suppression du fallback coast TP (cf. Phase A.9 dans `bots-ai-roadmap.md`). Seuls les boat docks TP restent.

### Fichiers clés

**SQL** (appliqués manuellement, pas via dbimport)
- [`modules/mod-conquest-mounts/data/sql/db-world/gvg_siege_engines_v15.sql`](../modules/mod-conquest-mounts/data/sql/db-world/gvg_siege_engines_v15.sql) — dernier état (v2 → v15 itératifs)
- [`modules/mod-conquest-npc-vendor/data/sql/db-world/conquest_vendor_creatures.sql`](../modules/mod-conquest-npc-vendor/data/sql/db-world/conquest_vendor_creatures.sql) — 4+3 PNJs vendors

**C++**
- [`modules/mod-conquest-mounts/src/ConquestMounts.cpp`](../modules/mod-conquest-mounts/src/ConquestMounts.cpp) — `ConquestSiegeEngineAI` + `VehicleScript` + `ConquestRegisterVehicleOwner` exporté
- [`modules/mod-conquest-npc-vendor/src/ConquestVehicleVendor.cpp`](../modules/mod-conquest-npc-vendor/src/ConquestVehicleVendor.cpp) — GMK gossip + pricing
- [`modules/mod-conquest-npc-vendor/src/ConquestVendorMgr.cpp`](../modules/mod-conquest-npc-vendor/src/ConquestVendorMgr.cpp) — vendor item loader + sub-entry generator
- [`modules/mod-conquest-npc-vendor/src/ConquestVendorScript.cpp`](../modules/mod-conquest-npc-vendor/src/ConquestVendorScript.cpp) — generic gossip→vendor window hook + buy hook

### À faire (backlog)

- ⏳ Display PB/PC sous l'icône d'item (refonte vers `ItemExtendedCost.dbc` ou refonte `ConquestPoints` en items 400301/400302 distribués dans l'inventaire — actuellement les solde est en chat header uniquement)
- ⏳ Switch Seat fonctionnel sur turrets (les turrets sont mountable directement avec `VehicleId 436` mais le seat-switching via vehicle UI semble limité — investigate `vehicle_seat.dbc` flags)
- ⏳ Vehicle Z-attachment fix Léviathan (les seats sont configurés selon le model 514 sur vehicle.dbc → positionnement seats parfois en l'air avec model Léviathan 28875, mitigé par scale 0.25)

Cette doc est un point d'ancrage. Toute évolution doit être ajoutée ici pour garder la vision claire.
