# Bots AI — Stratégie de dispatch et déplacement

> Roadmap d'amélioration progressive de l'IA des playerbots pour simuler un serveur vivant et stratégique. Objectif : des bots qui **savent quoi faire automatiquement** (défendre/attaquer les bannières), qui **se déplacent comme des joueurs** (pas que du teleport), et qui se **regroupent en escouades** plutôt que tous au même endroit.

## Vision

Les playerbots actuels (mod-playerbots) ont une IA de combat solide mais **aucune notion stratégique des objectifs Conquest**. Sans patch, ils errent au hasard ou se téléportent ponctuellement. On veut :

1. **Spawn capitale** comme état initial visuel (villes peuplées)
2. **Dispatch automatique** vers les 5 bannières (capture / défense)
3. **Squads de 5** au lieu de bots solitaires éparpillés
4. **Déplacement à pied** quand possible (immersion vs teleport)
5. **IA réactive** aux états de zone (défendre si banner ami contesté, attaquer si ennemi consolidé)

---

## État actuel (réf 2026-05-12)

### Phase A.0 — Dispatch capitale ✅
- `AiPlayerbot.ConquestZoneSpawn = 1` dans `RandomPlayerbotMgr::RandomTeleportForLevel`
- Mapping `bot->getRace()` → 8 capitales (4 Alliance + 4 Horde)
- `AiPlayerbot.ConquestCapitalPct` : % de chance pour qu'un bot teleport en capitale (vs zone banner)
- Intervalle teleport : 60-300s par bot (`MinRandomBotTeleportInterval` / `Max`)

### Phase A.1 — Squad dispatcher + walk same-continent ✅
- **Squad memory** par faction : variables statiques `s_allianceZoneIdx/s_allianceSquadCnt` et `s_hordeZoneIdx/s_hordeSquadCnt`
- Round-robin : N bots successifs vers la même bannière, puis rotation
- Taille squad configurable : `AiPlayerbot.ConquestSquadSize = 5`
- **Walk same-continent** : `AiPlayerbot.ConquestWalkSameContinent = 1`
  - Si `bot->GetMapId() == banner.map` → `MovePoint(banner)` au lieu de teleport
  - Pathfinding navmesh via `MotionMaster::MovePoint(generatePath=true)`
  - Cross-continent (différentes maps) → fallback teleport
- Jitter : 10y zones (effet squad) vs 20y capitales

### Cas couverts par Phase A.1

| Race | Capitale | Banners atteignables à pied (same-map) |
|---|---|---|
| Human | Stormwind (EK 0) | Southshore, Tarren Mill |
| Dwarf / Gnome | Ironforge (EK 0) | Southshore, Tarren Mill |
| Undead | Undercity (EK 0) | Southshore, Tarren Mill |
| Night Elf | Darnassus (Kal 1) | Astranaar, Crossroads, Gadgetzan |
| Orc / Troll | Orgrimmar (Kal 1) | Crossroads, Astranaar, Gadgetzan |
| Tauren | Thunder Bluff (Kal 1) | Crossroads, Astranaar, Gadgetzan |
| Draenei | Exodar (Outland 530) | aucun (teleport always) |
| Blood Elf | Silvermoon (Outland 530) | aucun (teleport always) |

**~75% des pairs sont same-continent walking.** Les 25% cross-continent (Outland-based races + cross-EK/Kalimdor) restent en teleport.

---

## Roadmap

### Phase A.2 — Walk long-distance via stepping incrémental ✅ (2026-05-12)

**Objectif** : tout bot (cross-zone même continent, voire cross-map) marche progressivement vers sa bannière au lieu de teleport, ou rester stationnaire.

#### Le problème initial
Les bots étaient bien assignés à une bannière (via `BannerDestination` injectée dans `travel target`) mais **ne marchaient pas physiquement**. `MoveTo()` retournait soit `canMove=false`, soit `canMove=true` sans changement de position en DB.

#### Phase de diagnostic — Build #48 (FORCED MovePoint bypass)
Ajout dans [MoveToTravelTargetAction.cpp](../modules/mod-playerbots/src/Ai/Base/Actions/MoveToTravelTargetAction.cpp) d'un bypass direct du `MotionMaster::MovePoint()` quand `canMove=false`, pour tester si le blocage venait du moteur (spline) ou des gates playerbots (`IsWaitingForLastMove`, `IsDuplicateMove`).

**Données récoltées via DB diff** :
| Bot | Avant | Après ~30s | Mouvement |
|---|---|---|---|
| Twaenlil | (-4833,-998) | (-4915,-949) | ~95y vers waypoint à 399y ✓ |
| Kilmis | (-627,-500) | (-670,-506) | ~43y ✓ |
| Eustere | (2776,-321), cible 10k yards | (2776,-321) | 0y ✗ |
| Feblellazz | cible 10k yards | +8y | quasi 0y ✗ |

#### Conclusion du diagnostic
**Le moteur spline fonctionne pour les bots** : `Player::Update` → `Unit::Update` → `UpdateSplineMovement` → `UpdateSplinePosition` met à jour la position. Le vrai problème : **un `MoveTo` vers une cible >450y produit un spline inexploitable** car le navmesh AC est capé à ~450y (`PATHFIND_NOPATH`).

Le `TravelNodeMap` (3781 nodes chargés après le fix `loadNodeStore()`) ne suffit pas seul : pour les trajets cross-Kalimdor de 10k yards, `getFullPath` retourne souvent un path à 2 points (start+end), donc aucun waypoint intermédiaire utile.

#### Le fix — Build #49 (step cap 250y + FORCED en primaire)

Dans `MoveToTravelTargetAction::Execute()` :

1. **Étape 1 — TravelNode opportuniste** : si `getFullPath` propose un waypoint sur la même map que le bot, on l'utilise comme cap intermédiaire (taxi nodes, ports). Filtre simplifié (suppression du `< 400y` qui rejetait trop).

2. **Étape 2 — Step cap à 250y** : avant tout déplacement, on recalcule une cible interpolée à 250y du bot, dans la direction de la destination (waypoint TravelNode ou cible finale).
```cpp
if (distNow > 250.0f) {
    float ratio = 250.0f / distNow;
    x = bot->GetPositionX() + dx * ratio;
    y = bot->GetPositionY() + dy * ratio;
    z = bot->GetPositionZ() + dz * ratio;
}
```
Le bot avance, l'action re-fire (CheckDelay), et un nouveau step de 250y est calculé. **Marche progressive par "pas" de 250y.**

3. **FORCED MovePoint en fallback** : si `MoveTo` est bloqué par les gates playerbots, on appelle directement `bot->GetMotionMaster()->MovePoint(0, x, y, z, FORCED_MOVEMENT_RUN, 0, 0, true, true)` qui contourne `IsWaitingForLastMove` / `IsDuplicateMove`. On force `canMove=true` après pour éviter que la TravelTarget s'auto-blacklist.

#### Patches complémentaires nécessaires (cumulés Phase A.2)

| Fichier | Patch | Raison |
|---|---|---|
| [TravelMgr.cpp](../modules/mod-playerbots/src/Mgr/Travel/TravelMgr.cpp) | `Init()` appelle `sTravelNodeMap.loadNodeStore()` | `LoadQuestTravelTable` orphelin → 0 nodes chargés |
| [TravelAction.cpp](../modules/mod-playerbots/src/Ai/Base/Actions/TravelAction.cpp) | Retrait `return false && ...` dans `isUseful` | Action désactivée silencieusement |
| [PlayerbotAI.cpp](../modules/mod-playerbots/src/Bot/PlayerbotAI.cpp) | `Reset(true)` préserve les `ConquestBannerDestination` | Téléport admin/respawn wipait la destination |
| [MovementActions.cpp](../modules/mod-playerbots/src/Ai/Base/Actions/MovementActions.cpp) | `DoMovePoint` : `FORCED_MOVEMENT_RUN` + `forceDestination=true` | Bots marchaient sans engager le spline run + navmesh imparfait |
| [MoveToTravelTargetAction.cpp](../modules/mod-playerbots/src/Ai/Base/Actions/MoveToTravelTargetAction.cpp) | TravelNode + step cap 250y + FORCED bypass | Cœur de la fix |
| [playerbots.conf](../env/dist/etc/modules/playerbots.conf) | `BotActiveAlone=100` | Sans, l'IA bot ne tourne que près d'un vrai joueur |
| [worldserver.conf](../env/dist/etc/worldserver.conf) | `Logger.playerbots=4,Console Server` | Capturer LOG_INFO pour diagnostics |

#### Validation
- **Logs** : `MoveToTravel: 'Mariondo' step=(-1668,-474)` → `step=(-1918,-462)` → `step=(-2168,-450)`, chaque tick avance le step de 250y dans la direction cible.
- **Visuel** : confirmation user "un joueur partir en ligne droite bien loin" pendant le live test.
- **Distance vitesse** : à `MOVE_RUN ≈ 7yds/sec`, un step 250y prend ~36s → traversée Kalimdor (10k yards) ≈ 4-5 minutes.

#### Limites connues (state Phase A.2 — résolues par Phase A.3/A.7)
- Trajets **cross-map** (ex: Stormwind EK → Crossroads Kalimdor) restent en teleport (bateau pas géré).
- Les Goblins / Outland (Exodar, Silvermoon, Dalaran) → bootstrap one-shot vers capitale Azeroth (Phase A.3).
- Stuck detection ajoutée en build #62.

### Phase A.3 — Robustesse ✅ (builds #51-#56)

| Patch | Détail |
|---|---|
| **No-TP en transit** (`ConquestKeep` log) | Si bot a `ConquestBannerDestination` active et pas arrivé (>60y), le periodic teleport est skipped → pas d'interruption en cours de trajet |
| **Re-dispatch à l'arrivée** | Bot < 60y de la banner → round-robin assigne une nouvelle banner (rotation) |
| **Outland → Azeroth bootstrap** | Draenei/BloodElf TP'd une seule fois vers Stormwind/Undercity puis marchent depuis Azeroth |
| **Capture trigger** | Build #53b : à `atFullLock` (banner fully locked pour faction X), on itère tous les bots de cette faction dans 200y et `RandomTeleportForLevel(bot)` → redispatch immédiat |
| **Late-arrival redispatch** | Build #57 : `HandlePlayerEnter` sur banner déjà locked-for-us → redispatch immédiat (évite camping) |
| **Stuck detection** | Build #62 : si bot pas bougé >5y en 60s avec banner active → force redispatch (sort des limbes après cross-continent TP) |
| **Same-continent preference** | Build #61b : si round-robin tire cible cross-map, on cherche la cible same-map la plus proche (≥80y pour éviter de boucler sur soi-même) |
| **Default `travel` strategy** | Build #54 : `AiFactory::AddDefaultNonCombatStrategies` ajoute `travel` si `ConquestZoneSpawn=1` → survit aux `ResetStrategies` |
| **Bypass HP/mana gate** | Build #52 : `MoveToTravelTargetAction::isUseful` ne checke plus `can move around` (qui exigeait `group ready` → HP > 80%) — remplace par `IsInCombat + HasStrategy("stay")` |
| **SmartScale OFF** | Config `botActiveAloneSmartScale = 0` — sinon les bots loin du vrai joueur étaient deactivated |

#### Validation
- 75% des 150 bots en transit/sur banner à chaque snapshot.
- Capture trigger fires confirmé : "Late-arrival redispatch: 'X' (banner already locked)" log.
- 0 bots stuck > 60s.

### Phase A.7 — Navigation curée par waypoint graphe ✅ (builds #58-#63)

Pour éviter "bots qui volent en ligne droite à travers le décor" sur les longues distances :

#### Architecture
- **GameObject 400100 "Conquest Waypoint"** : entry custom, type 5 GENERIC, displayId 6671 (Lightwell — globe lumineux bleu). PhaseMask=2 forcé à la création → invisible aux joueurs.
- **GM auto-phase** : `ConquestGMPhase` PlayerScript passe les GM en `SetPhaseMask(3)` à login → ils voient phase 1 (monde) + phase 2 (waypoints).
- **Graphe** : `ConquestWaypointMgr` (singleton) scan tous les GO 400100 au boot, auto-build les edges par proximité (`<500y same-map`).
- **Dijkstra** : `GetRoute(start, end)` retourne séquence ordonnée de WPs. Avec n<300, instant.
- **Cache route per-bot** : `s_routes[guid]` planifie une fois par changement de destination, invalidé via `.conquest waypoints reload`.

#### Workflow GM
```
.gm on                    ← Lightwells visibles via phase 3
.tele <lieu>
.gob add 400100           ← place un WP à ta position
.gobject near 30          ← vérifier WPs alentour
.gob move <guid>          ← repositionner
.conquest waypoints reload ← rescan + invalider caches bots
.conquest waypoints count  ← affiche WPs / edges en mémoire
```

#### Logique de routing hybride
À chaque tick `MoveToTravelTargetAction::Execute` :
1. Si `dist(bot, banner_target) > 300y` ET graphe non vide → `Dijkstra` → bot suit WPs un par un.
2. `dist < 300y` → navmesh direct (PathGenerator, fiable < 450y).
3. Sub-step fallback 100y si navmesh échoue.

#### Détour opportuniste (build #63)
À chaque tick, scan banners same-map dans 250y du bot. Si une banner n'est PAS fully-locked pour notre faction → override la cible immédiate vers cette banner. Bot Alliance qui passe à 125y d'une banner Horde se détourne pour la contester. La `TravelTarget` originale reste intacte → reprise auto après capture.

#### Validation
- 28 WPs placés Crossroads → Astranaar (avg 137y, min 22, max 197).
- Bots planifient 13 WPs sur ce trajet (chemin curé, pas ligne droite).

### Phase A.4 — Bots count + immersion (build #51-#67)

#### Pennons killstreak racialisés (build #64-#68)
À 5 et 10 kills consécutifs, application d'auras visuelles racialisées :

| Race | Pennon (5 kills) | Pennon Champion (10 kills) |
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

Aura compagne `47292` cast aux paliers 5 et 10 (via `CastSpell`, les pennons via `AddAura`).

Tiers supérieurs (build #67) :
- 20 kills → aura `71188` (TUEUR D'ÉLITE)
- 40 kills → aura `71193` + retire 71188 (LÉGENDAIRE)
- 80 kills → aura `71195` + retire 71193 (DIEU DE GUERRE)

À la mort : reset streak à 0 + retire toutes les auras.

**Découverte de debug (build #66-68)** :
- Spell `.aura X` (via AC core) utilise `Player::AddAura()` direct.
- `CastSpell(target, spell, true)` ne pose pas certains spells aura-only.
- Choix : pennons via `AddAura` ; aura compagne 47292 via `CastSpell` (chacun fonctionne différemment selon le spell).
- Config conf : `KillStreakOnFireSpell=0` / `PrestigeSpell=0` → bascule sur racial mapping.

Commande joueur : `.conquest points` → affiche solde PB / PC / killstreak actuel + record.

### Phase A.8 — Smart dispatcher orienté état (build #79-81) ✅

Remplace le round-robin par un scoring basé sur l'état réel des bannières.

#### Architecture
1. **`OutdoorPvPConquest::GetAllBanners()`** : retourne un snapshot de toutes les bannières (map, x/y/z, slider, max, zone). Filtre les **banners phantom** (`!cp->IsBannerAlive()`) — bannières dont le GO a été supprimé via `.gob delete` mais dont le ConquestCapturePoint reste en mémoire (build #80).
2. **Smart score** (build #79) appliqué à chaque banner same-continent du bot, pour chaque dispatch :
   - **État** : enemy fully-locked = 1.0 (attaque) ; contesté CONTRE nous = 1.2 (défense urgente) ; contesté POUR nous = 0.5 ; neutre = 0.8 ; ours fully-locked = skip
   - **Distance** : `exp(-dist / 3000)` (decay exponentiel, proche = priorité +)
   - **Jitter** : `frand(0.9, 1.1)` (anti-convergence)
3. **Sélection** : la bannière au score max. Logs : `ConquestSmart: 'Bot' → banner (x,y) slider=val/max score=X.XXX`.
4. **Fallback** : si `g_conquestInstance` indispo, retour à round-robin sur les hardcoded `allianceBanners[]/hordeBanners[]`.
5. **Registry dynamique TravelDestination** : `s_dynamicBannerSlots` (keyed by position arrondie) crée à la volée les `WorldPosition + BannerDestination` pour CHAQUE banner LIVE, plus seulement les hardcoded. Supporte donc les bannières placées par GM manuellement.

#### Défense post-capture 30s (build #81)
- Quand un bot capture une bannière (`atFullLock`), au lieu de redispatch immédiat, on **schedule** une redispatch à `now + 30000ms` via `ConquestScheduleDefenseRedispatch(botGuid, atMs)`.
- Pendant ces 30s, le bot reste à la bannière (status WORK, action ne fire pas).
- Un PlayerScript `conquest_defense_redispatch` hook `OnPlayerBeforeUpdate` check chaque tick si un bot a un schedule expiré → `sRandomPlayerbotMgr.RandomTeleportForLevel(p)`.
- Configurable : `ConquestFrontline.DefenseDurationMs = 30000`.

#### Fix stuck sur dernier waypoint (build #78)
- Avant : `ConquestRoute` détectait stuck >15s mais ne skippait que si `currentIdx + 1 < waypoints.size()`. Donc un bot bloqué sur la **dernière** étape (la bannière elle-même) restait stuck forever.
- Fix : si stuck >15s sur LAST WP → **teleport direct** à `(banner_x ± 25, banner_y ± 25)` pour le mettre dans le radius de capture (45y). Évite les agglomérats de bots tournant en rond.

### Phase A.9 — Cross-map travel via bateaux/zeppelins (2026-05-14)

Avant : les bots cross-map faisaient un TP direct vers la bannière (souvent en plein milieu de l'eau ou sur le banner lui-même → effet "flicker"). Le user voulait du réalisme : les bots prennent un dock, le serveur les TP au dock paire de l'autre côté, puis ils marchent du dock à la bannière.

#### Architecture
- **5 boat/zeppelin routes hardcodées** dans [`RandomPlayerbotMgr.cpp`](../modules/mod-playerbots/src/Bot/RandomPlayerbotMgr.cpp) :
  - Alliance : Stormwind ↔ Auberdine, Menethil ↔ Theramore
  - Horde : Orgrimmar ↔ Grom'gol, Orgrimmar ↔ Undercity
  - Both factions : Rut'theran ↔ Auberdine (résout Teldrassil sans TP magique), Darnassus_portal ↔ Auberdine (le portail Darnassus → Rut'theran n'ayant pas de navmesh walkable)
- **`BoatRoute` struct** : `{mapA, xA/yA/zA, mapB, xB/yB/zB, teamFilter, name}`. `teamFilter = ROUTE_ALLY (1) / ROUTE_HORDE (2) / ROUTE_ANY (0)`.
- **`FindBestBoatRoute(botMap, botPos, targetMap, targetPos, teamFilter)`** : cherche la route optimale (cost = dist(bot, src_dock) + dist(dst_dock, target)). Renvoie `{srcMap, srcPos, dstMap, dstPos}`.

#### Routing fork dans `RouteBotToBanner(bot, target)`
1. **Same map + same cluster** → walk pur (`SetBotTravelTo(banner)`, pas de TP).
2. **Cross-map ou cross-cluster + route faction-OK existe** → marche vers `srcDock` + stash `(finalBanner, srcDock, dstDock)` dans `s_stashedBoatTrips`.
3. **Pas de route** → `LOG_WARN` + walk-only (jamais de TP sur banner).

#### `ConquestCheckBoatArrival` (hook OnPlayerBeforeUpdate 1Hz)
- Si bot a un trip stashed ET est ≤ 15y du `srcDock` → `TeleportTo(dstDock + jitter ±5y)` + ré-arme la `TravelTarget` vers la bannière finale.
- Visuellement : bot marche jusqu'au dock → flash → bot apparaît au dock paire → marche jusqu'à la bannière.

#### Cluster TP (cas Teldrassil isolé)
- `ConquestClusterOf(zoneId)` enum `{ MAINLAND, TELDRASSIL, AZUREMYST }`. Zones 141 (Teldrassil), 1657 (Darnassus) → TELDRASSIL. Zones 3524 (Azuremyst), 3525 (Bloodmyst), 3557 (Exodar) → AZUREMYST.
- Si bot et target ont des clusters différents même map → le boat route Rut'theran↔Auberdine (same-map) résout le cas Teldrassil naturellement.

#### Coast landing fallback (deprecated v6)
- Avant : si aucun boat route applicable, TP au coast point le plus proche du banner. Bug : Auberdine coast point = `(6404, 514, 12)` = position **exacte** de la bannière Darkshore → TP onto banner.
- Fix : suppression du fallback. Cross-map sans route → walk-only (bot reste en place si impossible, mieux que TP visible sur banner).

#### Validation
- Logs : `ConquestBoat: 'X' walking to dock (X,Y) -> final (X,Y) map=N` puis `ConquestBoat: 'X' arrived at dock -> TP to (X,Y) map=N`.
- Ratio arrivées au dock après fix : ~95% (avant : 13/506 = 2.5% dû à coords Auberdine z=8 sur l'eau, fixé à z=22 plateforme centrale).

### Phase A.10 — Group-of-5 dispatch persistent (2026-05-14)

Au démarrage des bots, formation d'**escouades de 5 mono-faction** assignées **round-robin sur les bannières LIVE** au lieu du smart dispatch solo. Évite les bus de joueurs sur la même bannière et équilibre la première vague.

#### Registry dans [`RandomPlayerbotMgr.cpp`](../modules/mod-playerbots/src/Bot/RandomPlayerbotMgr.cpp)
- `s_groups` (leader GUID → `ConquestGroup{sealed, teamId, pendingRedispatch, createdMs, members[]}`)
- `s_botToLeader` (bot GUID → leader GUID)
- `s_pendingLeader[2]` : un pending group par faction
- `s_bannerCursor[2]` : curseur round-robin par faction

#### `HandleConquestGroupDispatch(bot)` (hook au début du Conquest block dans `RandomTeleportForLevel`)
1. Si bot **déjà connu** (in `s_botToLeader`) :
   - Leader + sealed + `pendingRedispatch=true` → `DispatchConquestGroup(leader)` (re-TP tout le squad)
   - Leader + sealed mais pas pending → return (squad reste sur la bannière)
   - Leader + pending (<5) + age > 20s → **timeout seal** + dispatch avec partial members (évite bots stuck en capitale)
   - Non-leader → return (le leader gère)
2. Sinon, ajout au pending de sa faction. Pending vide → bot devient leader. Pending atteint 5 → `sealed=true` + dispatch immédiat.

#### Coordination post-capture
- `OutdoorPvPConquest::Update` à `atFullLock` : scan **toute la zone** du banner (pas juste 200y) et schedule redispatch pour chaque bot de la team capturante (`ConquestScheduleDefenseRedispatch`).
- `OnPlayerBeforeUpdate` (PlayerScript) pop le trigger après 30s :
  - Si bot en groupe + non-leader → invoke leader RTfL (pour réveiller le leader même si lui-même hors zone) + return
  - Si leader → set `pendingRedispatch=true` + call own RTfL → `HandleConquestGroupDispatch` → `DispatchConquestGroup` → tous les 5 TP'd ensemble vers une nouvelle bannière (round-robin)
- Fonctions externes exposées par RandomPlayerbotMgr.cpp pour cross-module :
  - `uint64 ConquestGetGroupLeader(uint64 botGuid)`
  - `void ConquestSignalGroupRedispatch(uint64 leaderGuid)`
  - `bool ConquestCheckBoatArrival(Player* bot)`

#### Configuration
- `AiPlayerbot.ConquestGroupDispatch = 1` (default on)
- `AiPlayerbot.ConquestSquadSize = 5`
- `AiPlayerbot.ConquestWalkSameContinent = 1`
- Toutes ajoutées à [`playerbots.conf.dist`](../modules/mod-playerbots/conf/playerbots.conf.dist) pour éviter le log spam `> Config: Missing property`.

#### Validation
- Logs au démarrage : `ConquestGroup: new pending team=0 leader=1042` → `ConquestGroup: sealed team=0 leader=1042 -> dispatching` → `ConquestGroupDispatch: leader=1042 dispatched 5/5 -> (2313,-2530) map=1`.

### Phase A.11 — Banner auto-registration + admin command (2026-05-14)

Avant : les bannières placées via `.gob add 400010` en jeu (en plus des SQL spawns) n'étaient pas toujours actives. Les capture points étaient bien créés mais les joueurs entrant dans ces zones n'étaient pas rattachés à l'OutdoorPvPConquest (zone non enregistrée).

#### `EnsureZoneRegistered(zoneId)` ([`OutdoorPvPConquest.cpp`](../modules/mod-conquest-frontline/src/OutdoorPvPConquest.cpp))
- Idempotent : ajoute `zoneId` à `m_OutdoorPvPMap` si pas déjà attaché. Cache via `_registeredZones` set.
- Appelée depuis `RegisterCapturePoint` avec `go->GetZoneId()` (vraie zone calculée par position, pas le `gameobject.zoneId` qui peut être 0).

#### `ReattachPlayersInZone(map, zoneId)`
- Si la zone vient d'être attachée à chaud (banner ajouté via `.gob add`), itère `Map::GetPlayers()` et appelle `HandlePlayerEnterZone(p, zoneId)` pour chaque joueur déjà sur place. Sans ça, ils ne participent pas à la capture jusqu'à ce qu'ils sortent et reviennent dans la zone.

#### Commande `.conquest banners rescan` (SEC_GAMEMASTER)
- `OutdoorPvPConquest::RescanAllBanners(force=true)` itère `MapMgr::DoForAllMaps` + `Map::GetGameObjectBySpawnIdStore()` pour trouver tous les GO entry 400010 vivants et appeler `RegisterCapturePoint` sur chacun. Force-clear de `_registeredPositions` avant le scan pour permettre la re-registration des banners déjà vus.

### Phase A.12 — PvP flag permanent + ForcePvP (2026-05-14)

Avant : les bots / joueurs perdaient le flag PvP périodiquement (timer interne AC de désactivation), ce qui faisait que `OutdoorPvP::IsOutdoorPvPActive` retournait false et désactivait la capture.

#### [`ConquestForcePvP.cpp`](../modules/mod-conquest-frontline/src/ConquestForcePvP.cpp)
- Hook `OnPlayerLogin` + `OnPlayerMapChanged` + `OnPlayerUpdateZone` → `UpdatePvP(true, override=true)` + `SetPvP(true)` sur maps 0/1 hors GameMaster.
- Hook `OnPlayerBeforeUpdate` (1Hz throttled via `s_lastReapply`) : si `!IsPvP()` et bot/player éligible → re-apply silencieusement. Couvre les cas où un timer interne AC réinitialise le flag entre 2 events.

### Phase A.13 — Unstick post-respawn capitale + dock bootstrap (2026-05-16)

Symptôme : ~100 bots clusterisés en capitales (Stormwind/Ironforge/Orgrimmar) et au dock SW pour les Draenei/BloodElf, restant immobiles plusieurs minutes.

#### Hook `OnPlayerResurrect` ([`ConquestDefenseRedispatch.cpp`](../modules/mod-conquest-frontline/src/ConquestDefenseRedispatch.cpp))
- Quand `mod-conquest-respawn-capital` ressuscite un bot en capitale, on planifie un redispatch dans `nowMs + 1000` via `ConquestScheduleDefenseRedispatch`. Sans ça, le bot attend son prochain `RandomTeleportForLevel` périodique (60-300s) avant de bouger.
- Signature `OnPlayerResurrect(Player*, float, bool& applySickness)` (3e param est une référence, pas une valeur).

#### Fix garde "in transit" cross-map ([`RandomPlayerbotMgr.cpp`](../modules/mod-playerbots/src/Bot/RandomPlayerbotMgr.cpp))
- Bug : un bot avec une `ConquestBannerDestination` active sur map 1 + TP en capitale map 0 (post-mort) déclenchait l'early return `ConquestKeep: in transit` parce que le check `bot->GetMapId() == pos->GetMapId()` échouait silencieusement, laissant `arrivedAtBanner = false` et `hasActiveBanner = true`.
- Fix : si le banner cible est cross-map, on force `arrivedAtBanner = true` pour traverser la garde et déclencher un nouveau dispatch.

#### Redispatch auto post-bootstrap Outland dock
- Après le TP `map 530 → harbor SW / Undercity zep tower`, le bot n'avait pas de `TravelTarget` et restait au dock jusqu'au prochain dispatch périodique.
- On programme un `ConquestScheduleDefenseRedispatch(guid, nowMs + 2000)` après le TP bootstrap pour enchaîner immédiatement vers une bannière.
- Forward decl `extern void ConquestScheduleDefenseRedispatch(uint64, uint32)` ajoutée dans `RandomPlayerbotMgr.cpp`.

#### Fix `RescanAllBanners(force=true)` création de doublons
- Bug : la commande GM `.conquest banners rescan` vidait `_registeredPositions` puis re-iterait tous les GO. Chaque cp existant restait en mémoire avec son slider en cours (locked Alliance/Horde), et un **nouveau** cp neutre + ghost GO était créé à la même position — résultat : deux bannières visibles côte à côte par position.
- Fix : on reconstruit `_registeredPositions` à partir des `_conquestPoints` vivants (`IsBannerAlive() == true`) au lieu de vider sec. Les cp existants sont préservés, seuls les nouveaux GO orphelins (ex: `.gob add` post-boot) déclenchent une nouvelle registration.

### Phase A.4 — Vrais groupes WoW ⏳

Actuellement les squads de 5 sont juste un agglomérat visuel (mêmes coords). Pour de **vrais groupes WoW** :

- Hook `RandomTeleportForLevel` ou un dispatcher périodique
- Quand 5 bots arrivent successivement à une même zone, les regrouper via `Group::AddInvite` + `Group::AddMember`
- Le 1er bot du squad devient leader
- Avantage : `Group` partage XP/loot/follow, l'IA exploite déjà `master` (followLeader)
- Permet d'utiliser `RandomPlayerbotMgr::FormGroup()` si existe

**Effort estimé** : 1-2 jours

### Phase A.5 — IA réactive aux états de zone ⏳

Au-delà du dispatch, faire que les bots **réagissent intelligemment** :

- **Si banner ami contesté** (slider décroît) → bots libres marchent défendre
- **Si banner ennemi à 90% (juste avant lock)** → vague d'attaque coordonnée
- **Si toutes les zones amies sécurisées** → patrouilles, ou attaque opportuniste
- **Suivi de "front" mobile** : si une zone passe ennemi, les bots qui patrouillent dans la région réorientent vers la zone perdue

**Architecture proposée** :
- Module séparé `mod-conquest-bot-ai` qui s'exécute toutes les 30s
- Lit l'état des 5 banners via `g_conquestInstance`
- Score chaque zone par "urgence" (contestée, presque lock ennemi, etc.)
- Émet des "ordres" aux bots libres via leur AI Context

**Effort estimé** : 3-4 jours

### Phase A.6 — Pacing comportemental ⏳

Pour éviter le côté robotique :

- **Idle réaliste** : 10% des bots restent en ville (vendor browsing, discussion)
- **Patrouille** : déplacement aléatoire entre zone et waypoint hors capture
- **Repos** : après un fight, 60-180s de "down time" avant nouveau dispatch
- **Variation des routes** : pas tous le même path pour atteindre une zone

**Effort estimé** : 1-2 jours

---

## Métriques de succès

À implémenter en parallèle (logs Conquest) pour mesurer l'efficacité :

| Métrique | Cible | Outil |
|---|---|---|
| % bots en zone (vs capitale) | 60-70% en peak | SQL `SELECT zone, COUNT(*) FROM characters WHERE online=1` |
| Distribution sur 5 zones | Écart-type < 20% du moyenne | Idem |
| Ratio walk vs teleport | A.1 : ~50% (POC), A.2 : ~80% | Logs `MovePoint` vs `TeleportTo` |
| Bots stuck (pas bougé en 5 min) | < 5% | Position diff entre 2 polls |
| Squad cohésion (5 bots dans 20y) | > 60% des squads | Calcul barycentre zone |
| Réactivité défense (joueur attaque banner) | 3-5 bots arrivent < 2 min | Test scénario |

---

## Risques connus + mitigations

| Risque | Cause | Mitigation |
|---|---|---|
| **Pathfinding navmesh défaillant** | Map 3.3.5 imparfaite, falaises/eau/gates | A.3 stuck detection + timeout retry teleport |
| **Bot bloque au capital gate** | Geometry du portail city | Spawn waypoint à 50y hors-ville plutôt qu'inside |
| **Boat miss + 4min attente** | Timer bateaux fixe | A.2 : préfère portails Mage / TP scrolls quand dispo |
| **Bot tué en route → loop respawn** | Mort en transit, respawn capitale, redispatche | A.3 : lock destination 5 min après mort pour switch sur autre zone |
| **AI override MovePoint** | Strategies playerbots (Travel/Idle/Combat) interrompent | Forced=true sur MovePoint, ou activer strat `travel` explicitement |
| **Charge CPU élevée** | 150 bots × navmesh = stress | Limiter à N walks simultanés, fallback teleport au-dessus |
| **Tous bots stuck → zones désertes** | Cascade d'échecs | Watchdog : si zone X a 0 bots depuis 10 min, force-teleport N bots |

---

## Backlog (au-delà de A.6)

- **B.1 — Pull intelligente** : bot voit joueur isolé proche → pull à 30y au lieu de 5y, simule joueur
- **B.2 — Wave coordinator** : 1 dispatcher central qui décide "vague d'attaque sur Crossroads à T+5min" et coordonne 15 bots
- **B.3 — Personnalité bot** : 1/10 bots "agressif" (full attaque), 1/10 "défenseur" (reste sur banner ami), 1/10 "explorer" (visite zones aléatoires)
- **B.4 — RP chat** : bots disent "ils viennent par le sud !" en local quand un joueur ennemi entre dans leur range
- **B.5 — Boss-fight collaborative** : si un GO type Coffre Mystique spawn, dispatcher dirige X bots de chaque faction

---

## Notes implémentation

### Code touché (à date — fin Phase A.7)

**mod-playerbots**
- `src/Bot/RandomPlayerbotMgr.cpp` — `RandomTeleportForLevel`
  - Squad memory static round-robin (A.1)
  - 26 banners cibles (13/faction, A.2 builds #55-56 — banners ensuite suppr DB pour replacement manuel par GM)
  - No-TP transit-skip + arrived re-dispatch (A.3 build #51)
  - Outland → Azeroth bootstrap (A.3 build #51)
  - Capture trigger redispatch (A.3 build #53b)
  - Same-continent preference (A.3 build #61b)
  - Stuck detection 60s (A.3 build #62)
  - Smart dispatcher state×distance×jitter sur LIVE banners (A.8 build #79)
  - Registry dynamique `s_dynamicBannerSlots` TravelDestination (A.8 build #79)
- `src/Mgr/Travel/TravelMgr.cpp` — `Init()` charge `loadNodeStore` (A.2)
- `src/Ai/Base/Actions/TravelAction.cpp` — `isUseful` removed `return false &&` (A.2)
- `src/Ai/Base/Actions/MoveToTravelTargetAction.cpp` — routing hybride, navmesh+sub-step+FORCED, ConquestWaypointMgr Dijkstra, détour opportuniste (A.2 + A.7), **fix stuck final WP teleport** (A.8 build #78)
- `src/Ai/Base/Actions/MovementActions.cpp` — `DoMovePoint` forcedRun + forceDestination (A.2)
- `src/Bot/PlayerbotAI.cpp` — `Reset(true)` preserve ConquestBannerDestination (A.2)
- `src/Bot/Factory/AiFactory.cpp` — `AddDefaultNonCombatStrategies` adds `travel` if ConquestZoneSpawn (A.3 build #54)

**mod-conquest-frontline**
- `src/OutdoorPvPConquest.cpp` + `.h` — RegisterZone 21 zones, capture trigger, late-arrival redispatch, FindOpportunisticBanner, tracker `_conquestPoints`, **`GetAllBanners` + `IsBannerAlive`** (A.3 + A.7 + A.8), **`ConquestScheduleDefenseRedispatch`** (A.8 build #81)
- `src/ConquestFrontlineAuto.cpp` — auto-register waypoints 400100 en phase=2, GM auto-phase=3 (A.7)
- `src/ConquestWaypointMgr.h/cpp` — graphe waypoints + Dijkstra (A.7 build #58-60)
- `src/ConquestCommands.cpp` — `.conquest waypoints reload/count`, **`.conquest points`** (A.7-A.8)
- `src/ConquestKillStreak.cpp` — pennons racialisés + 47292 + tiers 20/40/80 (A.4 builds #64-68)
- `src/ConquestDefenseRedispatch.cpp` — **NEW** : PlayerScript `OnPlayerBeforeUpdate` qui fire la redispatch 30s post-capture (A.8 build #81)
- `data/sql/db-world/conquest_waypoint_template.sql` — entry 400100 GO Lightwell (A.7)
- ~~`conquest_banners_extended.sql`~~ — 22 banners auto-spawned (build #55) **supprimés** car user a opté pour placement manuel via `.gob add 400010`

**Config**
- `env/dist/etc/modules/playerbots.conf` :
  - `MinRandomBots = MaxRandomBots = 1000` (peut être réduit pour CPU)
  - `BotActiveAlone = 100` + `botActiveAloneSmartScale = 0`
  - `MinRandomBotTeleportInterval = 60`, `Max = 300`
  - `ConquestZoneSpawn = 1`, `ConquestSquadSize = 5`, `ConquestWalkSameContinent = 1`
- `env/dist/etc/modules/conquest_frontline.conf` :
  - `KillStreakOnFire = 5`, `Prestige = 10`, `Tier3 = 20`, `Tier4 = 40`, `Tier5 = 80`
  - `KillStreakOnFireSpell = 0` / `PrestigeSpell = 0` → utilise racial mapping
  - `DefenseDurationMs = 30000` — délai entre lock et redispatch (build #81)
- `env/dist/etc/worldserver.conf` : `Logger.playerbots = 4,Console Server`

### Build / test cycle

```bash
# Modifier le code dans modules/mod-playerbots/src/Bot/RandomPlayerbotMgr.cpp
docker compose up -d --build ac-worldserver
# Observer logs
docker compose logs -f ac-worldserver 2>&1 | grep -E "TeleportTo|MovePoint|stuck"
# Vérifier distribution
docker compose exec ac-database mysql -uroot -ppassword acore_characters \
  -e "SELECT zone, COUNT(*) FROM characters WHERE online=1 GROUP BY zone ORDER BY 2 DESC LIMIT 10;"
```

### Configuration tuning suggérée

| Param | Valeur actuelle | Range conseillée |
|---|---|---|
| `ConquestCapitalPct` | 30 | 10-50 (haut = plus en ville, bas = plus aux zones) |
| `ConquestSquadSize` | 5 | 3-8 |
| `MinRandomBotTeleportInterval` | 60 | 30-300 |
| `MaxRandomBotTeleportInterval` | 300 | 60-1800 |
| `ConquestWalkSameContinent` | 1 | 0 = pur teleport |

---

Cette doc est un point d'ancrage du chantier IA bots. Toute évolution / observation devrait être ajoutée ici pour garder le fil.
