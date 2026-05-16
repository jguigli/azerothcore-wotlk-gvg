-- ============================================================================
-- Conquest Frontline — Spawn des 5 bannières de capture MVP
-- ============================================================================
--
-- Spawn manuel des 5 bannières de capture dans les zones contestées MVP.
-- Le GameObject 400010 ("Bannière de Conquête") déclenche automatiquement :
--   - L'enregistrement comme capture point via mod-conquest-frontline (hook OnGameObjectAddWorld)
--   - La UI gauge native côté client (via worldstate du capture point type 29)
--
-- Pour spawn manuel via commande GM (alternative à ce SQL) :
--   .gob add 400010
-- ============================================================================

-- Cleanup éventuel des spawns précédents (par ID guid réservé custom)
DELETE FROM `gameobject` WHERE `guid` BETWEEN 5400010 AND 5400014;

-- ============================================================================
-- Bannière 1 — Crossroads (Carrefour des Barrens, Kalimdor, Horde-friendly)
-- ============================================================================
INSERT INTO `gameobject`
(`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`,
 `position_x`, `position_y`, `position_z`, `orientation`,
 `rotation0`, `rotation1`, `rotation2`, `rotation3`,
 `spawntimesecs`, `animprogress`, `state`, `ScriptName`, `VerifiedBuild`)
VALUES
(5400010, 400010, 1, 17, 380, 1, 1,
 -435, -2660, 92, 0,
 0, 0, 0, 1,
 -3600, 100, 1, '', 0);

-- ============================================================================
-- Bannière 2 — Astranaar (Ashenvale, Kalimdor, Alliance-friendly)
-- ============================================================================
INSERT INTO `gameobject`
(`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`,
 `position_x`, `position_y`, `position_z`, `orientation`,
 `rotation0`, `rotation1`, `rotation2`, `rotation3`,
 `spawntimesecs`, `animprogress`, `state`, `ScriptName`, `VerifiedBuild`)
VALUES
(5400011, 400010, 1, 331, 333, 1, 1,
 2783, -311, 105, 0,
 0, 0, 0, 1,
 -3600, 100, 1, '', 0);

-- ============================================================================
-- Bannière 3 — Southshore (Hillsbrad Foothills, Eastern Kingdoms, Alliance)
-- ============================================================================
INSERT INTO `gameobject`
(`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`,
 `position_x`, `position_y`, `position_z`, `orientation`,
 `rotation0`, `rotation1`, `rotation2`, `rotation3`,
 `spawntimesecs`, `animprogress`, `state`, `ScriptName`, `VerifiedBuild`)
VALUES
(5400012, 400010, 0, 267, 272, 1, 1,
 -668, -513, 19, 0,
 0, 0, 0, 1,
 -3600, 100, 1, '', 0);

-- ============================================================================
-- Bannière 4 — Tarren Mill (Hillsbrad Foothills, EK, Horde-friendly)
-- ============================================================================
INSERT INTO `gameobject`
(`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`,
 `position_x`, `position_y`, `position_z`, `orientation`,
 `rotation0`, `rotation1`, `rotation2`, `rotation3`,
 `spawntimesecs`, `animprogress`, `state`, `ScriptName`, `VerifiedBuild`)
VALUES
(5400013, 400010, 0, 267, 273, 1, 1,
 155, -680, 55, 0,
 0, 0, 0, 1,
 -3600, 100, 1, '', 0);

-- ============================================================================
-- Bannière 5 — Gadgetzan (Tanaris, Kalimdor, Neutre)
-- ============================================================================
INSERT INTO `gameobject`
(`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`,
 `position_x`, `position_y`, `position_z`, `orientation`,
 `rotation0`, `rotation1`, `rotation2`, `rotation3`,
 `spawntimesecs`, `animprogress`, `state`, `ScriptName`, `VerifiedBuild`)
VALUES
(5400014, 400010, 1, 440, 803, 1, 1,
 -7160, -3819, 9, 0,
 0, 0, 0, 1,
 -3600, 100, 1, '', 0);

-- ============================================================================
-- Note : spawntimesecs = -3600 (négatif) = respawn permanent, ne disparaît pas
-- ============================================================================
