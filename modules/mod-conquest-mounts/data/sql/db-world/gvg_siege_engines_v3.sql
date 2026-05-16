-- ============================================================================
-- Conquest Mounts v3 — Variants faction + Leviathan 330 + Siege chair
-- ============================================================================
-- Updates :
--   400200 P-W8 Red    -> subname "Horde",    HealthMod 1.43, scale 0.75
--   400201 B27 Blue    -> subname "Alliance", HealthMod 1.90, scale 1.0
--   400203 Glaive Violet -> subname "Alliance"
--   400204 Glaive Orange -> rename "jaune",  subname "Horde"
--   400209 M2 Blue     -> subname "Alliance", HealthMod 0.95, scale 0.5
-- New entries :
--   400310 M2 Red (Horde)     clone 400209  displayId 26403
--   400311 P-W8 Blue (Alliance) clone 400200 displayId 25292
--   400312 B27 Red (Horde)    clone 400201  displayId 26403
--   400313 Siege chair        clone 34812   displayId 29205
--   400314 Leviathan Alliance clone 33113   displayId 28875
--   400315 Leviathan Horde    clone 33113   displayId 28875
-- HealthMod calcule approx (base ~21k pour lvl 70 unit_class=1) ; le C++
-- ajustera la HP exacte au spawn via SetMaxHealth/SetHealth.
-- ============================================================================

-- =========================== UPDATES EXISTANTS ==============================
UPDATE `creature_template` SET `subname` = 'Horde',    `HealthModifier` = 1.43 WHERE `entry` = 400200;
UPDATE `creature_template` SET `subname` = 'Alliance', `HealthModifier` = 1.90 WHERE `entry` = 400201;
UPDATE `creature_template` SET `subname` = 'Alliance' WHERE `entry` = 400203;
UPDATE `creature_template` SET `subname` = 'Horde', `name` = 'Lanceur de glaive jaune' WHERE `entry` = 400204;
UPDATE `creature_template` SET `subname` = 'Alliance', `HealthModifier` = 0.95 WHERE `entry` = 400209;

UPDATE `creature_template_model` SET `DisplayScale` = 0.75 WHERE `CreatureID` = 400200;
UPDATE `creature_template_model` SET `DisplayScale` = 1.0  WHERE `CreatureID` = 400201;
UPDATE `creature_template_model` SET `DisplayScale` = 0.5  WHERE `CreatureID` = 400209;
UPDATE `creature_template_model` SET `DisplayScale` = 1.0  WHERE `CreatureID` = 400211;

-- ====================== CLEANUP DES NEW ENTRIES (idempotent) ===============
DELETE FROM `creature_template_spell`     WHERE `CreatureID` IN (400310, 400311, 400312, 400313, 400314, 400315);
DELETE FROM `creature_template_addon`     WHERE `entry`      IN (400310, 400311, 400312, 400313, 400314, 400315);
DELETE FROM `npc_spellclick_spells`       WHERE `npc_entry`  IN (400310, 400311, 400312, 400313, 400314, 400315);
DELETE FROM `creature_template_model`     WHERE `CreatureID` IN (400310, 400311, 400312, 400313, 400314, 400315);
DELETE FROM `creature_template`           WHERE `entry`      IN (400310, 400311, 400312, 400313, 400314, 400315);
DELETE FROM `vehicle_template_accessory`  WHERE `entry`      IN (400200, 400201, 400209, 400310, 400311, 400312, 400314, 400315);

-- =========================== 400310 PISTEUR M2 ROUGE (Horde) ================
INSERT INTO `creature_template` SELECT
  400310 AS entry, difficulty_entry_1, difficulty_entry_2, difficulty_entry_3, KillCredit1, KillCredit2,
  'Pisteur M2' AS name, 'Horde' AS subname, IconName, gossip_menu_id, minlevel, maxlevel, exp, faction,
  npcflag, speed_walk, speed_run, speed_swim, speed_flight, detection_range, `rank`, dmgschool,
  DamageModifier, BaseAttackTime, RangeAttackTime, BaseVariance, RangeVariance, unit_class, unit_flags,
  unit_flags2, dynamicflags, family, `type`, type_flags, lootid, pickpocketloot, skinloot, PetSpellDataId,
  VehicleId, mingold, maxgold, AIName, MovementType, HoverHeight,
  0.95 AS HealthModifier, ManaModifier, ArmorModifier, ExperienceModifier, RacialLeader, movementId,
  RegenHealth, CreatureImmunitiesId, flags_extra, 'ConquestSiegeEngine' AS ScriptName, VerifiedBuild
FROM `creature_template` WHERE `entry` = 400209;

INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`)
VALUES (400310, 0, 26403, 0.5, 1, 10314);

INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(400310, 46598, 1, 0),
(400310, 66245, 1, 0);

INSERT INTO `creature_template_addon` (`entry`, `path_id`, `mount`, `bytes1`, `bytes2`, `emote`, `visibilityDistanceType`, `auras`)
SELECT 400310, path_id, mount, bytes1, bytes2, emote, visibilityDistanceType, auras
FROM `creature_template_addon` WHERE `entry` = 400209;

INSERT INTO `creature_template_spell` (`CreatureID`, `Index`, `Spell`, `VerifiedBuild`)
SELECT 400310, `Index`, Spell, VerifiedBuild FROM `creature_template_spell` WHERE `CreatureID` = 400209;

-- =========================== 400311 BAROUDEUR P-W8 BLEU (Alliance) ==========
INSERT INTO `creature_template` SELECT
  400311, difficulty_entry_1, difficulty_entry_2, difficulty_entry_3, KillCredit1, KillCredit2,
  'Baroudeur P-W8', 'Alliance', IconName, gossip_menu_id, minlevel, maxlevel, exp, faction,
  npcflag, speed_walk, speed_run, speed_swim, speed_flight, detection_range, `rank`, dmgschool,
  DamageModifier, BaseAttackTime, RangeAttackTime, BaseVariance, RangeVariance, unit_class, unit_flags,
  unit_flags2, dynamicflags, family, `type`, type_flags, lootid, pickpocketloot, skinloot, PetSpellDataId,
  VehicleId, mingold, maxgold, AIName, MovementType, HoverHeight,
  1.43, ManaModifier, ArmorModifier, ExperienceModifier, RacialLeader, movementId,
  RegenHealth, CreatureImmunitiesId, flags_extra, 'ConquestSiegeEngine', VerifiedBuild
FROM `creature_template` WHERE `entry` = 400200;

INSERT INTO `creature_template_model` VALUES (400311, 0, 25292, 0.75, 1, 10314);

INSERT INTO `npc_spellclick_spells` VALUES
(400311, 46598, 1, 0),
(400311, 66245, 1, 0);

INSERT INTO `creature_template_addon`
SELECT 400311, path_id, mount, bytes1, bytes2, emote, visibilityDistanceType, auras
FROM `creature_template_addon` WHERE `entry` = 400200;

INSERT INTO `creature_template_spell`
SELECT 400311, `Index`, Spell, VerifiedBuild FROM `creature_template_spell` WHERE `CreatureID` = 400200;

-- =========================== 400312 DESTRUCTEUR B27 ROUGE (Horde) ===========
INSERT INTO `creature_template` SELECT
  400312, difficulty_entry_1, difficulty_entry_2, difficulty_entry_3, KillCredit1, KillCredit2,
  'Destructeur B27', 'Horde', IconName, gossip_menu_id, minlevel, maxlevel, exp, faction,
  npcflag, speed_walk, speed_run, speed_swim, speed_flight, detection_range, `rank`, dmgschool,
  DamageModifier, BaseAttackTime, RangeAttackTime, BaseVariance, RangeVariance, unit_class, unit_flags,
  unit_flags2, dynamicflags, family, `type`, type_flags, lootid, pickpocketloot, skinloot, PetSpellDataId,
  VehicleId, mingold, maxgold, AIName, MovementType, HoverHeight,
  1.90, ManaModifier, ArmorModifier, ExperienceModifier, RacialLeader, movementId,
  RegenHealth, CreatureImmunitiesId, flags_extra, 'ConquestSiegeEngine', VerifiedBuild
FROM `creature_template` WHERE `entry` = 400201;

INSERT INTO `creature_template_model` VALUES (400312, 0, 26403, 1.0, 1, 10314);

INSERT INTO `npc_spellclick_spells` VALUES
(400312, 46598, 1, 0),
(400312, 66245, 1, 0);

INSERT INTO `creature_template_addon`
SELECT 400312, path_id, mount, bytes1, bytes2, emote, visibilityDistanceType, auras
FROM `creature_template_addon` WHERE `entry` = 400201;

INSERT INTO `creature_template_spell`
SELECT 400312, `Index`, Spell, VerifiedBuild FROM `creature_template_spell` WHERE `CreatureID` = 400201;

-- =========================== 400313 SIEGE CHAIR (clone 34812) ===============
-- ScriptName VIDE (on ne veut pas npc_pilgrims_bounty_chair) ; faction 35.
INSERT INTO `creature_template` SELECT
  400313, difficulty_entry_1, difficulty_entry_2, difficulty_entry_3, KillCredit1, KillCredit2,
  'Siege', '', IconName, gossip_menu_id, minlevel, maxlevel, exp, 35 AS faction,
  npcflag, speed_walk, speed_run, speed_swim, speed_flight, detection_range, `rank`, dmgschool,
  DamageModifier, BaseAttackTime, RangeAttackTime, BaseVariance, RangeVariance, unit_class, unit_flags,
  unit_flags2, dynamicflags, family, `type`, type_flags, lootid, pickpocketloot, skinloot, PetSpellDataId,
  VehicleId, mingold, maxgold, AIName, MovementType, HoverHeight,
  HealthModifier, ManaModifier, ArmorModifier, ExperienceModifier, RacialLeader, movementId,
  RegenHealth, CreatureImmunitiesId, flags_extra, '' AS ScriptName, VerifiedBuild
FROM `creature_template` WHERE `entry` = 34812;

INSERT INTO `creature_template_model` VALUES (400313, 0, 29205, 1.0, 1, 10314);

-- =========================== 400314/400315 LEVIATHAN 330 ====================
-- Clone 33113 (Flame Leviathan). ScriptName 'ConquestSiegeEngine' pour passer
-- par notre AI custom. faction 35, HealthMod 2.4 (approx 50k, C++ override).
INSERT INTO `creature_template` SELECT
  400314, difficulty_entry_1, difficulty_entry_2, difficulty_entry_3, KillCredit1, KillCredit2,
  'Leviathan 330', 'Alliance', IconName, 0, 70, 70, exp, 35,
  npcflag, speed_walk, speed_run, speed_swim, speed_flight, detection_range, 0, dmgschool,
  DamageModifier, BaseAttackTime, RangeAttackTime, BaseVariance, RangeVariance, unit_class, unit_flags,
  unit_flags2, dynamicflags, family, `type`, type_flags, lootid, pickpocketloot, skinloot, PetSpellDataId,
  VehicleId, mingold, maxgold, AIName, MovementType, HoverHeight,
  2.4, ManaModifier, ArmorModifier, ExperienceModifier, RacialLeader, movementId,
  RegenHealth, CreatureImmunitiesId, 344407930, 'ConquestSiegeEngine', VerifiedBuild
FROM `creature_template` WHERE `entry` = 33113;

INSERT INTO `creature_template` SELECT
  400315, difficulty_entry_1, difficulty_entry_2, difficulty_entry_3, KillCredit1, KillCredit2,
  'Leviathan 330', 'Horde', IconName, 0, 70, 70, exp, 35,
  npcflag, speed_walk, speed_run, speed_swim, speed_flight, detection_range, 0, dmgschool,
  DamageModifier, BaseAttackTime, RangeAttackTime, BaseVariance, RangeVariance, unit_class, unit_flags,
  unit_flags2, dynamicflags, family, `type`, type_flags, lootid, pickpocketloot, skinloot, PetSpellDataId,
  VehicleId, mingold, maxgold, AIName, MovementType, HoverHeight,
  2.4, ManaModifier, ArmorModifier, ExperienceModifier, RacialLeader, movementId,
  RegenHealth, CreatureImmunitiesId, 344407930, 'ConquestSiegeEngine', VerifiedBuild
FROM `creature_template` WHERE `entry` = 33113;

INSERT INTO `creature_template_model` VALUES
(400314, 0, 28875, 0.5, 1, 10314),
(400315, 0, 28875, 0.5, 1, 10314);

-- ====================== VEHICLE TEMPLATE ACCESSORIES ========================
-- P-W8 rouge (400200) + P-W8 bleu (400311) : seats 1+2 = Siege chair (400313)
--                                              seat 7 = destruction main (400207)
INSERT INTO `vehicle_template_accessory` (`entry`, `accessory_entry`, `seat_id`, `minion`, `description`, `summontype`, `summontimer`) VALUES
(400200, 400313, 1, 0, 'P-W8 Horde - siege seat 1',    6, 30000),
(400200, 400313, 2, 0, 'P-W8 Horde - siege seat 2',    6, 30000),
(400200, 400207, 7, 0, 'P-W8 Horde - destruction main',6, 30000),
(400311, 400313, 1, 0, 'P-W8 Alliance - siege seat 1', 6, 30000),
(400311, 400313, 2, 0, 'P-W8 Alliance - siege seat 2', 6, 30000),
(400311, 400207, 7, 0, 'P-W8 Alliance - destruction main', 6, 30000);

-- B27 bleu (400201) + B27 rouge (400312) : seats 1+2 = destruction (400208), seat 7 = Canon massif (400211)
INSERT INTO `vehicle_template_accessory` VALUES
(400201, 400208, 1, 0, 'B27 Alliance - destruction left',  6, 30000),
(400201, 400208, 2, 0, 'B27 Alliance - destruction right', 6, 30000),
(400201, 400211, 7, 0, 'B27 Alliance - Canon massif main', 6, 30000),
(400312, 400208, 1, 0, 'B27 Horde - destruction left',     6, 30000),
(400312, 400208, 2, 0, 'B27 Horde - destruction right',    6, 30000),
(400312, 400211, 7, 0, 'B27 Horde - Canon massif main',    6, 30000);

-- M2 bleu (400209) + M2 rouge (400310) : seat 7 = combat (400206, scale 0.75 via C++)
INSERT INTO `vehicle_template_accessory` VALUES
(400209, 400206, 7, 0, 'M2 Alliance - combat main', 6, 30000),
(400310, 400206, 7, 0, 'M2 Horde - combat main',    6, 30000);

-- Leviathan 330 (400314/400315) : seats 0,1,2,3 = Siege chair ; seat 7 = Flame Leviathan Turret (33139)
INSERT INTO `vehicle_template_accessory` VALUES
(400314, 400313, 0, 0, 'Leviathan A - siege seat 0', 6, 30000),
(400314, 400313, 1, 0, 'Leviathan A - siege seat 1', 6, 30000),
(400314, 400313, 2, 0, 'Leviathan A - siege seat 2', 6, 30000),
(400314, 400313, 3, 0, 'Leviathan A - siege seat 3', 6, 30000),
(400314, 33139,  7, 0, 'Leviathan A - vanilla turret', 6, 30000),
(400315, 400313, 0, 0, 'Leviathan H - siege seat 0', 6, 30000),
(400315, 400313, 1, 0, 'Leviathan H - siege seat 1', 6, 30000),
(400315, 400313, 2, 0, 'Leviathan H - siege seat 2', 6, 30000),
(400315, 400313, 3, 0, 'Leviathan H - siege seat 3', 6, 30000),
(400315, 33139,  7, 0, 'Leviathan H - vanilla turret', 6, 30000);

SELECT CONCAT('v3 migration OK: ', COUNT(*), ' entries')
FROM `creature_template` WHERE `entry` IN (400310, 400311, 400312, 400313, 400314, 400315);
