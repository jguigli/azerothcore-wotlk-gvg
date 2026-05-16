-- ============================================================================
-- Conquest Frontline — Waypoint GameObject template
-- ============================================================================
-- Entry 400100 = "Conquest Waypoint" — GO invisible servant de point de
-- navigation pour le routing des bots banner-to-banner.
--
-- Workflow placement (GM en jeu) :
--   .gob add 400100        → place un waypoint à ta position courante
--   .gob move <guid>       → déplace le waypoint
--   .gob delete <guid>     → supprime le waypoint
--   .gob near 30           → liste les GOs/waypoints autour de toi (30y)
--
-- Le ConquestWaypointMgr scan tous les GO entry=400100 au boot, calcule les
-- edges auto par proximité (<500y same-map, navmesh validé), expose
-- getRoute(start, end) via Dijkstra.
--
-- Display 6671 (Lightwell — globule lumineux bleu/blanc, joli et visible).
-- Visibilité gérée côté C++ : OnGameObjectAddWorld force phaseMask=2, et
-- ConquestGMPhase passe les GMs en phaseMask=3 pour qu'ils voient.
-- ============================================================================

DELETE FROM `gameobject_template` WHERE `entry` = 400100;

INSERT INTO `gameobject_template`
(`entry`, `type`, `displayId`, `name`, `IconName`, `castBarCaption`, `unk1`, `size`,
 `Data0`, `Data1`, `Data2`, `Data3`, `Data4`, `Data5`, `Data6`, `Data7`,
 `Data8`, `Data9`, `Data10`, `Data11`, `Data12`, `Data13`, `Data14`,
 `Data15`, `Data16`, `Data17`, `Data18`, `Data19`, `Data20`, `Data21`, `Data22`, `Data23`,
 `AIName`, `ScriptName`, `VerifiedBuild`)
VALUES
(400100, 5, 6671, 'Conquest Waypoint', '', '', '', 1.0,
 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0,
 '', '', 0);
