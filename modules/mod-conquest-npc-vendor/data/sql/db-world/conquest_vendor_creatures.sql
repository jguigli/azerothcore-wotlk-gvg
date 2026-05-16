-- ============================================================================
-- Conquest NPC Vendor — 4 PNJs consolides (Armes/Armures/Offpart/Offset)
-- Display: goblin female 17819, subname "Azeroth Conquest", npcflag VENDOR+GOSSIP
-- ============================================================================
-- Categories :
--   400260 Armes   : epees, masses, haches, arcs, baguettes, boucliers, off-hand
--   400261 Armures : torse, tete, epaules, jambes, mains (sets principaux)
--   400262 Offpart : capes, brassards, ceintures, bottes
--   400263 Offset  : bijoux (anneaux, cous, trinkets)
-- Gossip : sections T1 / T2 / T2.5 / T3 / PVP Rare / PVP Epique
-- ============================================================================

-- Cleanup des anciens vendors (entries 400210-400266) AINSI QUE 400201/400211
-- qui sont reclamees par mod-conquest-mounts (Destructeur B27 + Canon massif).
DELETE FROM `creature_template_model` WHERE `CreatureID` BETWEEN 400210 AND 400266;
DELETE FROM `creature_template`       WHERE `entry`       BETWEEN 400210 AND 400266;
DELETE FROM `creature`                WHERE `id1`         BETWEEN 400210 AND 400266;
-- Cleanup des anciens spawns reserves
DELETE FROM `creature`                WHERE `guid`        BETWEEN 4000000 AND 4000200;

-- ----------------------------------------------------------------------------
-- creature_template — 4 entries
--   faction 35  : neutre amical
--   npcflag 129 : UNIT_NPC_FLAG_GOSSIP (1) | UNIT_NPC_FLAG_VENDOR (128)
--                 -> CanCreatureGossipHello fire en premier, puis on appelle
--                    SendListInventory(sub_entry) au click section.
-- ----------------------------------------------------------------------------
INSERT INTO `creature_template`
(`entry`, `name`, `subname`, `minlevel`, `maxlevel`,
 `faction`, `npcflag`, `unit_class`, `rank`, `type`, `AIName`, `ScriptName`)
VALUES
(400260, 'Armes',         'Azeroth Conquest', 60, 60, 35, 129, 1, 0, 7, '', ''),
(400261, 'Armures',       'Azeroth Conquest', 60, 60, 35, 129, 1, 0, 7, '', ''),
(400262, 'Offpart',       'Azeroth Conquest', 60, 60, 35, 129, 1, 0, 7, '', ''),
(400263, 'Offset',        'Azeroth Conquest', 60, 60, 35, 129, 1, 0, 7, '', ''),
(400264, 'Legendes',      'Azeroth Conquest', 60, 60, 35, 129, 1, 0, 7, '', ''),
(400265, 'Consommables',  'Azeroth Conquest', 60, 60, 35, 129, 1, 0, 7, '', ''),
(400266, 'Reactifs',      'Azeroth Conquest', 60, 60, 35, 129, 1, 0, 7, '', '');

-- creature_template_model — goblin female 17819 pour tous
INSERT INTO `creature_template_model`
(`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`) VALUES
(400260, 0, 17819, 1.0, 1),
(400261, 0, 17819, 1.0, 1),
(400262, 0, 17819, 1.0, 1),
(400263, 0, 17819, 1.0, 1),
(400264, 0, 17819, 1.0, 1),
(400265, 0, 17819, 1.0, 1),
(400266, 0, 17819, 1.0, 1);

-- ----------------------------------------------------------------------------
-- Spawns : 4 NPCs alignes dans chacune des 8 capitales (32 spawns).
-- GUIDs reserves : 4000000 - 4000031 (8 capitales * 4 NPCs).
-- Le user peut repositionner via .npc move <guid>.
-- ----------------------------------------------------------------------------

-- STORMWIND (Old Town)
INSERT INTO `creature` (`guid`,`id1`,`map`,`zoneId`,`areaId`,`spawnMask`,`phaseMask`,
 `equipment_id`,`position_x`,`position_y`,`position_z`,`orientation`,
 `spawntimesecs`,`MovementType`) VALUES
(4000000, 400260, 0, 1519, 1519, 1, 1, 0, -8842.0,  600.0, 94.0, 0, 86400, 0),
(4000001, 400261, 0, 1519, 1519, 1, 1, 0, -8838.0,  600.0, 94.0, 0, 86400, 0),
(4000002, 400262, 0, 1519, 1519, 1, 1, 0, -8834.0,  600.0, 94.0, 0, 86400, 0),
(4000003, 400263, 0, 1519, 1519, 1, 1, 0, -8830.0,  600.0, 94.0, 0, 86400, 0);

-- IRONFORGE
INSERT INTO `creature` (`guid`,`id1`,`map`,`zoneId`,`areaId`,`spawnMask`,`phaseMask`,
 `equipment_id`,`position_x`,`position_y`,`position_z`,`orientation`,
 `spawntimesecs`,`MovementType`) VALUES
(4000004, 400260, 0, 1537, 1537, 1, 1, 0, -4900.0, -940.0, 502.0, 0, 86400, 0),
(4000005, 400261, 0, 1537, 1537, 1, 1, 0, -4896.0, -940.0, 502.0, 0, 86400, 0),
(4000006, 400262, 0, 1537, 1537, 1, 1, 0, -4892.0, -940.0, 502.0, 0, 86400, 0),
(4000007, 400263, 0, 1537, 1537, 1, 1, 0, -4888.0, -940.0, 502.0, 0, 86400, 0);

-- DARNASSUS
INSERT INTO `creature` (`guid`,`id1`,`map`,`zoneId`,`areaId`,`spawnMask`,`phaseMask`,
 `equipment_id`,`position_x`,`position_y`,`position_z`,`orientation`,
 `spawntimesecs`,`MovementType`) VALUES
(4000008, 400260, 1, 1657, 1657, 1, 1, 0, 9680.0, 2497.0, 1335.0, 0, 86400, 0),
(4000009, 400261, 1, 1657, 1657, 1, 1, 0, 9684.0, 2497.0, 1335.0, 0, 86400, 0),
(4000010, 400262, 1, 1657, 1657, 1, 1, 0, 9688.0, 2497.0, 1335.0, 0, 86400, 0),
(4000011, 400263, 1, 1657, 1657, 1, 1, 0, 9692.0, 2497.0, 1335.0, 0, 86400, 0);

-- EXODAR
INSERT INTO `creature` (`guid`,`id1`,`map`,`zoneId`,`areaId`,`spawnMask`,`phaseMask`,
 `equipment_id`,`position_x`,`position_y`,`position_z`,`orientation`,
 `spawntimesecs`,`MovementType`) VALUES
(4000012, 400260, 530, 3557, 3557, 1, 1, 0, -3961.0, -11600.0, -138.0, 0, 86400, 0),
(4000013, 400261, 530, 3557, 3557, 1, 1, 0, -3957.0, -11600.0, -138.0, 0, 86400, 0),
(4000014, 400262, 530, 3557, 3557, 1, 1, 0, -3953.0, -11600.0, -138.0, 0, 86400, 0),
(4000015, 400263, 530, 3557, 3557, 1, 1, 0, -3949.0, -11600.0, -138.0, 0, 86400, 0);

-- ORGRIMMAR
INSERT INTO `creature` (`guid`,`id1`,`map`,`zoneId`,`areaId`,`spawnMask`,`phaseMask`,
 `equipment_id`,`position_x`,`position_y`,`position_z`,`orientation`,
 `spawntimesecs`,`MovementType`) VALUES
(4000016, 400260, 1, 1637, 1637, 1, 1, 0, 1676.0, -4290.0, 62.0, 0, 86400, 0),
(4000017, 400261, 1, 1637, 1637, 1, 1, 0, 1680.0, -4290.0, 62.0, 0, 86400, 0),
(4000018, 400262, 1, 1637, 1637, 1, 1, 0, 1684.0, -4290.0, 62.0, 0, 86400, 0),
(4000019, 400263, 1, 1637, 1637, 1, 1, 0, 1688.0, -4290.0, 62.0, 0, 86400, 0);

-- UNDERCITY
INSERT INTO `creature` (`guid`,`id1`,`map`,`zoneId`,`areaId`,`spawnMask`,`phaseMask`,
 `equipment_id`,`position_x`,`position_y`,`position_z`,`orientation`,
 `spawntimesecs`,`MovementType`) VALUES
(4000020, 400260, 0, 1497, 1497, 1, 1, 0, 1640.0, 240.0, -43.0, 0, 86400, 0),
(4000021, 400261, 0, 1497, 1497, 1, 1, 0, 1644.0, 240.0, -43.0, 0, 86400, 0),
(4000022, 400262, 0, 1497, 1497, 1, 1, 0, 1648.0, 240.0, -43.0, 0, 86400, 0),
(4000023, 400263, 0, 1497, 1497, 1, 1, 0, 1652.0, 240.0, -43.0, 0, 86400, 0);

-- THUNDERBLUFF
INSERT INTO `creature` (`guid`,`id1`,`map`,`zoneId`,`areaId`,`spawnMask`,`phaseMask`,
 `equipment_id`,`position_x`,`position_y`,`position_z`,`orientation`,
 `spawntimesecs`,`MovementType`) VALUES
(4000024, 400260, 1, 1638, 1638, 1, 1, 0, -1280.0, 145.0, 131.0, 0, 86400, 0),
(4000025, 400261, 1, 1638, 1638, 1, 1, 0, -1276.0, 145.0, 131.0, 0, 86400, 0),
(4000026, 400262, 1, 1638, 1638, 1, 1, 0, -1272.0, 145.0, 131.0, 0, 86400, 0),
(4000027, 400263, 1, 1638, 1638, 1, 1, 0, -1268.0, 145.0, 131.0, 0, 86400, 0);

-- SILVERMOON
INSERT INTO `creature` (`guid`,`id1`,`map`,`zoneId`,`areaId`,`spawnMask`,`phaseMask`,
 `equipment_id`,`position_x`,`position_y`,`position_z`,`orientation`,
 `spawntimesecs`,`MovementType`) VALUES
(4000028, 400260, 530, 3487, 3487, 1, 1, 0, 9497.0, -7279.0, 14.0, 0, 86400, 0),
(4000029, 400261, 530, 3487, 3487, 1, 1, 0, 9501.0, -7279.0, 14.0, 0, 86400, 0),
(4000030, 400262, 530, 3487, 3487, 1, 1, 0, 9505.0, -7279.0, 14.0, 0, 86400, 0),
(4000031, 400263, 530, 3487, 3487, 1, 1, 0, 9509.0, -7279.0, 14.0, 0, 86400, 0);

-- ----------------------------------------------------------------------------
-- Spawns supplementaires : 3 PNJs (400264 Legendes / 400265 Consommables /
-- 400266 Reactifs) dans les 8 capitales. GUIDs 4000032 - 4000055.
-- ----------------------------------------------------------------------------
INSERT INTO `creature` (`guid`,`id1`,`map`,`zoneId`,`areaId`,`spawnMask`,`phaseMask`,
 `equipment_id`,`position_x`,`position_y`,`position_z`,`orientation`,
 `spawntimesecs`,`MovementType`) VALUES
-- STORMWIND
(4000032, 400264, 0,   1519, 1519, 1, 1, 0, -8826.0,   600.0,   94.0, 0, 86400, 0),
(4000033, 400265, 0,   1519, 1519, 1, 1, 0, -8822.0,   600.0,   94.0, 0, 86400, 0),
(4000034, 400266, 0,   1519, 1519, 1, 1, 0, -8818.0,   600.0,   94.0, 0, 86400, 0),
-- IRONFORGE
(4000035, 400264, 0,   1537, 1537, 1, 1, 0, -4884.0,  -940.0,  502.0, 0, 86400, 0),
(4000036, 400265, 0,   1537, 1537, 1, 1, 0, -4880.0,  -940.0,  502.0, 0, 86400, 0),
(4000037, 400266, 0,   1537, 1537, 1, 1, 0, -4876.0,  -940.0,  502.0, 0, 86400, 0),
-- DARNASSUS
(4000038, 400264, 1,   1657, 1657, 1, 1, 0,  9696.0,  2497.0, 1335.0, 0, 86400, 0),
(4000039, 400265, 1,   1657, 1657, 1, 1, 0,  9700.0,  2497.0, 1335.0, 0, 86400, 0),
(4000040, 400266, 1,   1657, 1657, 1, 1, 0,  9704.0,  2497.0, 1335.0, 0, 86400, 0),
-- EXODAR
(4000041, 400264, 530, 3557, 3557, 1, 1, 0, -3945.0,-11600.0, -138.0, 0, 86400, 0),
(4000042, 400265, 530, 3557, 3557, 1, 1, 0, -3941.0,-11600.0, -138.0, 0, 86400, 0),
(4000043, 400266, 530, 3557, 3557, 1, 1, 0, -3937.0,-11600.0, -138.0, 0, 86400, 0),
-- ORGRIMMAR
(4000044, 400264, 1,   1637, 1637, 1, 1, 0,  1692.0, -4290.0,   62.0, 0, 86400, 0),
(4000045, 400265, 1,   1637, 1637, 1, 1, 0,  1696.0, -4290.0,   62.0, 0, 86400, 0),
(4000046, 400266, 1,   1637, 1637, 1, 1, 0,  1700.0, -4290.0,   62.0, 0, 86400, 0),
-- UNDERCITY
(4000047, 400264, 0,   1497, 1497, 1, 1, 0,  1656.0,   240.0,  -43.0, 0, 86400, 0),
(4000048, 400265, 0,   1497, 1497, 1, 1, 0,  1660.0,   240.0,  -43.0, 0, 86400, 0),
(4000049, 400266, 0,   1497, 1497, 1, 1, 0,  1664.0,   240.0,  -43.0, 0, 86400, 0),
-- THUNDERBLUFF
(4000050, 400264, 1,   1638, 1638, 1, 1, 0, -1264.0,   145.0,  131.0, 0, 86400, 0),
(4000051, 400265, 1,   1638, 1638, 1, 1, 0, -1260.0,   145.0,  131.0, 0, 86400, 0),
(4000052, 400266, 1,   1638, 1638, 1, 1, 0, -1256.0,   145.0,  131.0, 0, 86400, 0),
-- SILVERMOON
(4000053, 400264, 530, 3487, 3487, 1, 1, 0,  9513.0, -7279.0,   14.0, 0, 86400, 0),
(4000054, 400265, 530, 3487, 3487, 1, 1, 0,  9517.0, -7279.0,   14.0, 0, 86400, 0),
(4000055, 400266, 530, 3487, 3487, 1, 1, 0,  9521.0, -7279.0,   14.0, 0, 86400, 0);
