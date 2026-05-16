# Architecture GvG (Guild vs Guild) — AzerothCore 3.3.5

Le projet est composé de **9 modules** totalisant ~12 300 lignes de C++. Chacun suit la convention AzerothCore standard : un `loader.cpp` qui expose `Add<ModuleName>Scripts()`, et des scripts C++ qui se greffent via les hooks (`PlayerScript`, `CreatureScript`, `GameObjectScript`, `GuildScript`, `UnitScript`, `WorldScript`, `AllCreatureScript`, `AllSpellScript`, `AllGameObjectScript`).

## Vue d'ensemble des modules

| Module | LOC | Rôle |
|---|---|---|
| [mod-gvg-core](modules/mod-gvg-core/) | ~930 | **Socle du système GvG** : PvP inter-guildes, relations, spawn/mort joueur, restrictions |
| [mod-gvg-build](modules/mod-gvg-build/) | ~5 700 | **Construction** : murs, tours, herses, forts, canons, balises, kit de recrutement |
| [mod-zone-control](modules/mod-zone-control/) | ~2 500 | **Capture de zones** via bannières + coffres de guilde partagés |
| [mod-gvg-gear-manager](modules/mod-gvg-gear-manager/) | ~700 | **NPC équipement** : kits PvP s5 par classe/spé (JSON en DB) |
| [mod-gvg-guard](modules/mod-gvg-guard/) | ~830 | **Gardes ogres** : 6 types de PNJ défenseurs de guilde |
| [mod-gvg-mounts](modules/mod-gvg-mounts/) | ~640 | **Montures véhicules** : chevaux, loups, shredder, mécano-tank, engins de siège |
| [mod-gvg-lootdrop](modules/mod-gvg-lootdrop/) | ~460 | **Loot bag** à la mort (sac GO entry 400002, 15 min) |
| [mod-gvg-npc-vendor](modules/mod-gvg-npc-vendor/) | ~200 | **Vendeur de véhicules** de siège selon la faction |
| [mod-gvg-limit-group](modules/mod-gvg-limit-group/) | ~110 | **Plafonds** : 30 membres/guilde, 5 joueurs/groupe, pas de raid |

Le dossier contient aussi `mod-transmog` (module externe classique non-GvG).

## Logique pivot : [mod-gvg-core](modules/mod-gvg-core/)

Tout le gameplay repose sur **4 décisions** de ce module :

### 1. Hostilité forcée par guilde — [GvgHostility.cpp](modules/mod-gvg-core/src/GvgHostility.cpp)

- Le serveur tourne en `GameType = 16` (FFA mondial). Le hook `IfNormalReaction` (UnitScript) réécrit la relation entre deux joueurs :
  - Même guilde → `REP_EXALTED` (amis forcés, court-circuite le FFA).
  - Guildes différentes hostiles → `REP_HATED`.
  - Par défaut, `AreGuildsHostile` traite **toute paire de guildes différentes comme hostile** — une paire peut être explicitement neutralisée/alliée via `gvg_guild_relations`.
- `OnPlayerUpdate` **force à chaque frame** `IsInFFAPvPArea = true` + flag `UNIT_BYTE2_FLAG_FFA_PVP` pour que le client ne puisse jamais sortir du mode FFA ([GvgHostility.cpp:174](modules/mod-gvg-core/src/GvgHostility.cpp#L174)).
- `GvG_Guild_Update` (GuildScript) force `ForceValuesUpdateAtIndex(UNIT_FIELD_BYTES_2/FACTIONTEMPLATE)` sur tous les joueurs visibles lors d'un `OnAddMember`/`OnRemoveMember`, pour rafraîchir la couleur/le drapeau PvP côté client immédiatement.

### 2. Relations guildes — [GvgCore.cpp](modules/mod-gvg-core/src/GvgCore.cpp)

- Singleton `GvgCore::Instance()` avec cache en mémoire.
- Clé composite `(guildA << 32) | guildB`, stockage symétrique à chaque `SetGuildRelation` (écrit les deux sens).
- Valeurs : `-1 hostile`, `0 neutre`, `1 allié`. Expiration possible via `expire_at`.
- Commandes `.gvg reload` / `.gvg relation set/list` ([GvgCommands.cpp](modules/mod-gvg-core/src/GvgCommands.cpp)).

### 3. Cycle de vie du personnage — [GvgPlayerStart.cpp](modules/mod-gvg-core/src/GvgPlayerStart.cpp) / [GvgPlayerDeath.cpp](modules/mod-gvg-core/src/GvgPlayerDeath.cpp)

- **Création** : level 80, apprend tous les sorts de classe (hors montures), téléport **GM Island (map 1, ~16201,16211,1.13)** défini comme homebind.
- **Mort** : après 1 s de délai (pour laisser le loot-drop copier), vide l'intégralité de l'inventaire **sauf la pierre de foyer** (déplacée en sac principal si besoin, recréée si perdue).

### 4. Restrictions de contenu — [GvgRestrictions.cpp](modules/mod-gvg-core/src/GvgRestrictions.cpp)

- Bloque : Outreterre (map 530), Norfendre (map 571), toutes les instances/raids, BG, arènes.
- Bloque l'utilisation d'items-monture classiques (`SPELL_AURA_MOUNTED`) → force le passage par `mod-gvg-mounts`.
- `OnPlayerStoreNewItem` → `item->SetBinding(false)` : **aucun item n'est soulbound**, cohérent avec le looting des cadavres.

## Les modules gameplay se branchent sur le core

### [mod-gvg-build](modules/mod-gvg-build/) — le plus gros

3 types d'objets construits + dérivés :
- **Items kits** (80000–80040+) → `ItemScript::OnUse` qui appelle `GvGBuildMgr::SpawnStructure(...)` avec les coordonnées de la cible du sort.
- **GameObjects persistés** dans la table custom `gvg_build_structures` (guid, player_guid, guild_id, entry, position, build_type, group_id).
- **Systèmes groupés** via `group_id` : une herse spawn 2 tours + porte + 2 leviers ; détruire un composant supprime tout le groupe ([GvGBuildGameObject.cpp:32-43](modules/mod-gvg-build/src/GvGBuildGameObject.cpp#L32-L43)).
- **Contrôle par guilde** : [go_gvg_build_gate_lever](modules/mod-gvg-build/src/GvGBuildGameObject.cpp#L63) vérifie `guild_id` du GO vs `player->GetGuildId()`.
- **Canons (entry 34944)** avec AI custom (`GvGBuildCannon::GvGBuildCannonAI`) injectée via `AllCreatureScript` : scan 50 yd, attaque tout joueur hors-guilde ([GvGBuildGameObject.cpp:263-440](modules/mod-gvg-build/src/GvGBuildGameObject.cpp#L263-L440)).
- **Récupération 15 min** après pose via un outil dédié.
- **Kit de recrutement** ([GvGRecruitmentKit.cpp](modules/mod-gvg-build/src/GvGRecruitmentKit.cpp)) : clic sur un PNJ dont le custom-subname = nom de guilde du joueur → l'enrôle comme follower (max 3, positions fixes à 120°/180°/240°).
- **Marker Spawner** ([GvGMarkerSpawner.cpp](modules/mod-gvg-build/src/GvGMarkerSpawner.cpp)) : GO balise qui fait spawn des vagues de PNJ qui path vers elle.

### [mod-zone-control](modules/mod-zone-control/)

- 4 entries de bannières (400010/11 disputée H/A, 400012/13 contrôlée H/A).
- Canalisation 60 s (`DEFAULT_CAPTURE_TIME_MS`) interrompue par les dégâts/déplacement. Un feu (GO 193411) matérialise la capture en cours.
- Récompense périodique à la guilde propriétaire : 10 or + 10 bois (items 80020/80021) toutes les 10 min.
- Sur `OnDisband` ou dernier membre qui quitte, `GvgZoneControlGuild::CleanupGuildBanners` remet en disputé + supprime l'entrée de `gvg_zone_control`.
- Le second fichier [GvgSharedChestGuildBankScript.cpp](modules/mod-zone-control/src/GvgSharedChestGuildBankScript.cpp) est **entièrement commenté** : la fonctionnalité de coffres partagés via "guildes système" (ID ≥ 1 000 000) a été désactivée.

### Modules satellites

- **mod-gvg-gear-manager** : PNJ 400100 → gossip classe/spé → lit `gvg_gear_data` (JSON par spé), équipe le joueur, enregistre la spé choisie dans `gvg_player_specialization`.
- **mod-gvg-guard** : items 80030–80035 → spawn un ogre (entries 400300–400305), `SetData(0, playerGuid)` + `SetData(1, guildId)` dans l'AI, sous-titre = nom de guilde. `AllCreatureScript::OnCreatureSelectLevel` re-force les HP ; `UnitScript::ModifySpellDamageTaken` donne +50% aux casters (mage/shaman/démoniste).
- **mod-gvg-mounts** : entries 400000–400003 (montures) + 400200–400211 (engins de siège). Faction forcée à 35 (FACTION_FRIENDLY), `HOME_MOTION_TYPE` annulé pour que la bête reste là où le joueur descend.
- **mod-gvg-lootdrop** : `OnPlayerJustDied` → crée un GO 400002 à la position exacte, génère un loot copié (enchantements, gemmes, durabilité, random property). Le sac disparaît dès que vide ou après 15 min.
- **mod-gvg-limit-group** : `GuildScript::OnCanAddMember` → plafond 30 ; `GroupScript::OnAddMember` → plafond 5 + refus des raids.
- **mod-gvg-npc-vendor** : PNJ 400101, gossip pour summoner un véhicule de siège (entries vanilla 34775/76/93/02, 35069/273) adapté à la faction.

## Tables DB custom (monde vs persos)

- **db-world** (contenu du module) : `gvg_guild_relations`, `gvg_global_config`, `gvg_build_structures`, `gvg_marker_spawner`, `gvg_recruitment_kit`, `gvg_gear_data`, `gvg_guard_*`, `gvg_mounts_creature`, `gvg_siege_engines`, `gvg_lootdrop_gameobject`, `gvg_vehicle_vendor_npc`, `gvg_zone_control_items/gameobjects/guild_chests` + tous les items 80000+ et GO 400000+.
- **db-characters** (état des joueurs) : `gvg_player_stats`, `gvg_guild_stats`, `gvg_player_specialization`, `gvg_zone_control`, `gvg_zone_control_despawned_creatures`, `gvg_shared_chest_guild_mapping`.

## Conventions et patterns partagés

1. **Plage d'IDs** : items custom `80000–80099`, créatures custom `400000–400305`, GameObjects custom `400000–400103` (+ réemploi de GO vanilla comme 190397/190398/179117 pour murs/tours/portes de base).
2. **Hook lourd** : `OnPlayerUpdate` est utilisé **sans timer** dans `GvgCore` pour reforcer FFA chaque frame — c'est volontaire (le client réinitialise constamment) mais coûteux.
3. **Sous-titres de créatures** = nom de guilde (via `HasCustomSubName()`/`GetCustomSubName()`) — c'est le lien entre PNJ ogres/recrues et leur guilde propriétaire. C'est cohérent avec le commit récent sur le spawner de sous-titres de guilde.
4. **Persistance double** : pour tout GO placé, une ligne dans `gvg_build_structures` ET un `SaveToDB()` AzerothCore natif, d'où les nettoyages manuels dans `OnDestroyed`.
5. **Zone de jeu** : toute la vie se passe sur map 1 (Kalimdor) autour de GM Island, puisque les autres continents et instances sont bloqués.

## Points d'attention / fragilité

- `ForceValuesUpdateAtIndex` dans chaque `OnPlayerUpdate` **et** sur chaque changement de guilde → charge côté réseau non négligeable si beaucoup de joueurs sur la map.
- `LOG_ERROR("server", "[GvGBuild] OnModifyHealth called for entry: ...")` dans [GvGBuildGameObject.cpp:53](modules/mod-gvg-build/src/GvGBuildGameObject.cpp#L53) et [:234](modules/mod-gvg-build/src/GvGBuildGameObject.cpp#L234) : log de niveau ERROR pour un événement de gameplay normal — du bruit dans les logs prod.
- `GvgCore::SaveRelationsToDB` fait un `REPLACE INTO` dans une boucle sans transaction et est rappelé **à chaque `SetGuildRelation`** — O(N) par écriture, peut devenir lent.
- Dans [mod-gvg-guard/GvGGuard.cpp:165-171](modules/mod-gvg-guard/src/GvGGuard.cpp#L165-L171), `new Creature()` + `Create(...)` + `map->AddToMap(creature)` : si `AddToMap` échoue, le `delete creature` plante car AzerothCore gère la destruction via le map. Pattern à vérifier.
- `GvgSharedChestGuildBankScript.cpp` est du code mort commenté (~590 lignes). Si la feature est abandonnée, autant supprimer le fichier.
