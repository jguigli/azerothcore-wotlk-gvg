-- ============================================================================
-- Conquest Mounts — Siege Engines (3 vehicles + turrets) + GMK Vendor
-- Migration v2 : schema AzerothCore actuel (sans columns scale/trainer_*/immune)
-- ============================================================================
-- Entries :
--   400101 GMK (vendor de vehicules, gossip)
--   400200 Baroudeur P-W8  (vehicle, scale C++=1.0)
--   400201 Destructeur B27 (vehicle, scale C++=1.5 applique au runtime)
--   400209 Pisteur M2      (vehicle, scale C++=0.6 applique au runtime)
--   400206 Tourelle de combat       (turret, P-W8 seat 2)
--   400207 Tourelle de destruction  (turret, P-W8 seat 7)
--   400208 Tourelle de destruction  (turret, B27 seats 1/2)
--   400210 Tourelle de combat 2     (turret, P-W8 seat 1)
--   400211 Canon massif             (turret, B27 seat 7, scale C++=2.0)
-- ============================================================================

-- Cleanup complet (override des anciens vendors qui squattent ces entries)
DELETE FROM `creature_template_spell`  WHERE `CreatureID` IN (400101, 400200, 400201, 400206, 400207, 400208, 400209, 400210, 400211);
DELETE FROM `creature_template_addon`  WHERE `entry`       IN (400101, 400200, 400201, 400206, 400207, 400208, 400209, 400210, 400211);
DELETE FROM `npc_spellclick_spells`    WHERE `npc_entry`   IN (400200, 400201, 400206, 400209, 400210);
DELETE FROM `creature_template_model`  WHERE `CreatureID`  IN (400101, 400200, 400201, 400206, 400207, 400208, 400209, 400210, 400211);
DELETE FROM `creature_template`        WHERE `entry`       IN (400101, 400200, 400201, 400206, 400207, 400208, 400209, 400210, 400211);
DELETE FROM `vehicle_template_accessory` WHERE `entry` IN (400200, 400201, 400209);

-- ============================================================================
-- 400101 GMK (vendeur de vehicules, gossip uniquement, script C++ handle vente)
-- ============================================================================
INSERT INTO `creature_template`
(`entry`, `name`, `subname`, `IconName`, `minlevel`, `maxlevel`, `faction`, `npcflag`,
 `speed_walk`, `speed_run`, `detection_range`, `unit_class`, `rank`, `type`, `BaseAttackTime`,
 `RangeAttackTime`, `BaseVariance`, `RangeVariance`, `DamageModifier`, `HoverHeight`,
 `HealthModifier`, `ManaModifier`, `ArmorModifier`, `ExperienceModifier`, `RegenHealth`,
 `AIName`, `MovementType`, `flags_extra`, `ScriptName`)
VALUES
(400101, 'GMK', 'Vendeur de vehicules', '', 80, 80, 35, 1,
 1.0, 1.14286, 20, 1, 0, 7, 2000,
 2000, 1, 1, 1, 1,
 1, 1, 1, 1, 1,
 '', 0, 0, 'ConquestVehicleVendorNPC');

INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES
(400101, 0, 25848, 1, 1, 12340);

-- ============================================================================
-- 400200 Baroudeur P-W8 (clone de 35069 horde siege engine, faction 35 neutre)
-- VehicleId 514, flags_extra 344407930 (custom no-aggro etc.)
-- ============================================================================
INSERT INTO `creature_template`
(`entry`, `name`, `subname`, `IconName`, `minlevel`, `maxlevel`, `faction`, `npcflag`,
 `speed_walk`, `speed_run`, `detection_range`, `unit_class`, `rank`, `type`, `type_flags`,
 `family`, `VehicleId`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`,
 `DamageModifier`, `HoverHeight`, `unit_flags`, `unit_flags2`, `movementId`,
 `flags_extra`, `RegenHealth`, `HealthModifier`, `ManaModifier`, `ArmorModifier`,
 `ExperienceModifier`, `AIName`, `MovementType`, `ScriptName`, `VerifiedBuild`)
VALUES
(400200, 'Baroudeur P-W8', '', 'vehichleCursor', 70, 70, 35, 16777216,
 2.0, 2.0, 20, 1, 0, 9, 393256,
 0, 514, 2000, 2000, 1, 1,
 1, 61.7284, 16384, 2048, 113,
 344407930, 0, 1, 1, 1,
 1, '', 0, 'ConquestSiegeEngine', 10314);

INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES
(400200, 0, 26403, 1, 1, 10314);

INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(400200, 46598, 1, 0),
(400200, 66245, 1, 0);

INSERT INTO `creature_template_addon` (`entry`, `path_id`, `mount`, `bytes1`, `bytes2`, `emote`, `visibilityDistanceType`, `auras`) VALUES
(400200, 0, 0, 0, 1, 0, 4, NULL);

INSERT INTO `creature_template_spell` (`CreatureID`, `Index`, `Spell`, `VerifiedBuild`) VALUES
(400200, 0, 67796, 10314),
(400200, 1, 67797, 10314);

-- ============================================================================
-- 400201 Destructeur B27 (clone de 34776 alliance siege engine, faction 35)
-- VehicleId 435
-- ============================================================================
INSERT INTO `creature_template`
(`entry`, `name`, `subname`, `IconName`, `minlevel`, `maxlevel`, `faction`, `npcflag`,
 `speed_walk`, `speed_run`, `detection_range`, `unit_class`, `rank`, `type`, `type_flags`,
 `family`, `VehicleId`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`,
 `DamageModifier`, `HoverHeight`, `unit_flags`, `unit_flags2`, `movementId`,
 `flags_extra`, `RegenHealth`, `HealthModifier`, `ManaModifier`, `ArmorModifier`,
 `ExperienceModifier`, `AIName`, `MovementType`, `ScriptName`, `VerifiedBuild`)
VALUES
(400201, 'Destructeur B27', '', 'vehichleCursor', 70, 70, 35, 16777216,
 1.2, 1.0, 20, 1, 0, 9, 393256,
 0, 435, 2000, 2000, 1, 1,
 1, 61.7284, 16384, 2048, 113,
 344407930, 0, 1, 1, 1,
 1, '', 0, 'ConquestSiegeEngine', 10314);

INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES
(400201, 0, 25292, 1.5, 1, 10314);

INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(400201, 46598, 1, 0),
(400201, 66245, 1, 0);

INSERT INTO `creature_template_addon` (`entry`, `path_id`, `mount`, `bytes1`, `bytes2`, `emote`, `visibilityDistanceType`, `auras`) VALUES
(400201, 0, 0, 0, 1, 0, 4, NULL);

INSERT INTO `creature_template_spell` (`CreatureID`, `Index`, `Spell`, `VerifiedBuild`) VALUES
(400201, 0, 67796, 10314),
(400201, 1, 67797, 10314);

-- ============================================================================
-- 400209 Pisteur M2 (variant rapide du B27, displayID 26430 sera ovveride C++)
-- VehicleId 435, scale 0.6 via C++
-- ============================================================================
INSERT INTO `creature_template`
(`entry`, `name`, `subname`, `IconName`, `minlevel`, `maxlevel`, `faction`, `npcflag`,
 `speed_walk`, `speed_run`, `detection_range`, `unit_class`, `rank`, `type`, `type_flags`,
 `family`, `VehicleId`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`,
 `DamageModifier`, `HoverHeight`, `unit_flags`, `unit_flags2`, `movementId`,
 `flags_extra`, `RegenHealth`, `HealthModifier`, `ManaModifier`, `ArmorModifier`,
 `ExperienceModifier`, `AIName`, `MovementType`, `ScriptName`, `VerifiedBuild`)
VALUES
(400209, 'Pisteur M2', '', 'vehichleCursor', 70, 70, 35, 16777216,
 3.0, 3.0, 20, 1, 0, 9, 393256,
 0, 435, 2000, 2000, 1, 1,
 1, 61.7284, 16384, 2048, 113,
 344407930, 0, 1, 1, 1,
 1, '', 0, 'ConquestSiegeEngine', 10314);

INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES
(400209, 0, 25292, 0.6, 1, 10314);

INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(400209, 46598, 1, 0),
(400209, 66245, 1, 0);

INSERT INTO `creature_template_addon` (`entry`, `path_id`, `mount`, `bytes1`, `bytes2`, `emote`, `visibilityDistanceType`, `auras`) VALUES
(400209, 0, 0, 0, 1, 0, 4, NULL);

INSERT INTO `creature_template_spell` (`CreatureID`, `Index`, `Spell`, `VerifiedBuild`) VALUES
(400209, 0, 67796, 10314),
(400209, 1, 67797, 10314);

-- ============================================================================
-- 400206 Tourelle de combat (clone de 34935 Horde Gunship Cannon)
-- ============================================================================
INSERT INTO `creature_template`
(`entry`, `name`, `subname`, `IconName`, `minlevel`, `maxlevel`, `faction`, `npcflag`,
 `speed_walk`, `speed_run`, `detection_range`, `unit_class`, `rank`, `type`, `type_flags`,
 `family`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`,
 `DamageModifier`, `HoverHeight`, `unit_flags`, `unit_flags2`, `movementId`,
 `flags_extra`, `RegenHealth`, `HealthModifier`, `ManaModifier`, `ArmorModifier`,
 `ExperienceModifier`, `AIName`, `MovementType`, `ScriptName`, `VerifiedBuild`)
VALUES
(400206, 'Tourelle de combat', '', 'Gunner', 70, 70, 35, 16777216,
 1.2, 1.14286, 20, 1, 0, 9, 262184,
 0, 2000, 2000, 1, 1,
 1, 15.9116, 2, 2048, 121,
 344407930, 0, 1, 1, 1,
 1, '', 0, '', 10314);

INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES
(400206, 0, 29489, 1, 1, 10314);

INSERT INTO `creature_template_addon` (`entry`, `path_id`, `mount`, `bytes1`, `bytes2`, `emote`, `visibilityDistanceType`, `auras`) VALUES
(400206, 0, 0, 0, 1, 0, 4, NULL);

INSERT INTO `creature_template_spell` (`CreatureID`, `Index`, `Spell`, `VerifiedBuild`) VALUES
(400206, 0, 67452, 10314),
(400206, 1, 66541, 10314);

INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(400206, 67830, 1, 0);

-- ============================================================================
-- 400210 Tourelle de combat 2 (identique a 400206, P-W8 seat 1)
-- ============================================================================
INSERT INTO `creature_template`
(`entry`, `name`, `subname`, `IconName`, `minlevel`, `maxlevel`, `faction`, `npcflag`,
 `speed_walk`, `speed_run`, `detection_range`, `unit_class`, `rank`, `type`, `type_flags`,
 `family`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`,
 `DamageModifier`, `HoverHeight`, `unit_flags`, `unit_flags2`, `movementId`,
 `flags_extra`, `RegenHealth`, `HealthModifier`, `ManaModifier`, `ArmorModifier`,
 `ExperienceModifier`, `AIName`, `MovementType`, `ScriptName`, `VerifiedBuild`)
VALUES
(400210, 'Tourelle de combat', '', 'Gunner', 70, 70, 35, 16777216,
 1.2, 1.14286, 20, 1, 0, 9, 262184,
 0, 2000, 2000, 1, 1,
 1, 15.9116, 2, 2048, 121,
 344407930, 0, 1, 1, 1,
 1, '', 0, '', 10314);

INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES
(400210, 0, 29489, 1, 1, 10314);

INSERT INTO `creature_template_addon` (`entry`, `path_id`, `mount`, `bytes1`, `bytes2`, `emote`, `visibilityDistanceType`, `auras`) VALUES
(400210, 0, 0, 0, 1, 0, 4, NULL);

INSERT INTO `creature_template_spell` (`CreatureID`, `Index`, `Spell`, `VerifiedBuild`) VALUES
(400210, 0, 67452, 10314),
(400210, 1, 66541, 10314);

INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(400210, 67830, 1, 0);

-- ============================================================================
-- 400207/400208 Tourelles de destruction (clones de 34777, vanilla siege turret)
-- 400211 Canon massif : clone de 400208 + displayID 29488 + scale C++ 2.0
-- Pour eviter de cloner tout 34777 (dependances vanilla), on insere les essentiels.
-- ============================================================================
INSERT INTO `creature_template`
(`entry`, `name`, `subname`, `IconName`, `minlevel`, `maxlevel`, `faction`, `npcflag`,
 `speed_walk`, `speed_run`, `detection_range`, `unit_class`, `rank`, `type`, `type_flags`,
 `family`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`,
 `DamageModifier`, `HoverHeight`, `unit_flags`, `unit_flags2`, `movementId`,
 `flags_extra`, `RegenHealth`, `HealthModifier`, `ManaModifier`, `ArmorModifier`,
 `ExperienceModifier`, `AIName`, `MovementType`, `ScriptName`, `VerifiedBuild`)
VALUES
(400207, 'Tourelle de destruction', '', 'Gunner', 70, 70, 35, 16777216,
 1.2, 1.14286, 20, 1, 0, 9, 262184,
 0, 2000, 2000, 1, 1,
 1, 15.9116, 2, 2048, 121,
 344407930, 0, 1, 1, 1,
 1, '', 0, '', 10314),
(400208, 'Tourelle de destruction', '', 'Gunner', 70, 70, 35, 16777216,
 1.2, 1.14286, 20, 1, 0, 9, 262184,
 0, 2000, 2000, 1, 1,
 1, 15.9116, 2, 2048, 121,
 344407930, 0, 1, 1, 1,
 1, '', 0, '', 10314),
(400211, 'Canon massif', '', 'Gunner', 70, 70, 35, 16777216,
 1.2, 1.14286, 20, 1, 0, 9, 262184,
 0, 2000, 2000, 1, 1,
 1, 15.9116, 2, 2048, 121,
 344407930, 0, 1, 1, 1,
 1, '', 0, '', 10314);

INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES
(400207, 0, 29489, 1, 1, 10314),
(400208, 0, 29489, 1, 1, 10314),
(400211, 0, 29488, 1, 1, 10314);

-- ============================================================================
-- Vehicle Template Accessories (qui monte sur quel siege engine)
-- ============================================================================
-- Destructeur B27 (400201) : 2 tourelles destruction laterales (seat 1, 2)
-- + 1 tourelle principale Canon massif (seat 7)
INSERT INTO `vehicle_template_accessory` (`entry`, `accessory_entry`, `seat_id`, `minion`, `description`, `summontype`, `summontimer`) VALUES
(400201, 400208, 1, 0, 'B27 - destruction left', 6, 30000),
(400201, 400208, 2, 0, 'B27 - destruction right', 6, 30000),
(400201, 400211, 7, 0, 'B27 - Canon massif main', 6, 30000);

-- Baroudeur P-W8 (400200) : 2 tourelles combat (seats 1, 2) + 1 destruction (seat 7)
INSERT INTO `vehicle_template_accessory` (`entry`, `accessory_entry`, `seat_id`, `minion`, `description`, `summontype`, `summontimer`) VALUES
(400200, 400206, 2, 0, 'P-W8 - combat front-left', 6, 30000),
(400200, 400210, 1, 0, 'P-W8 - combat rear', 6, 30000),
(400200, 400207, 7, 0, 'P-W8 - destruction main', 6, 30000);

-- Pisteur M2 (400209) : 1 tourelle combat principale (seat 7)
INSERT INTO `vehicle_template_accessory` (`entry`, `accessory_entry`, `seat_id`, `minion`, `description`, `summontype`, `summontimer`) VALUES
(400209, 400206, 7, 0, 'M2 - combat main', 6, 30000);

SELECT CONCAT('Migration OK: ', COUNT(*), ' entries inserees') AS Result
FROM `creature_template` WHERE `entry` IN (400101, 400200, 400201, 400206, 400207, 400208, 400209, 400210, 400211);
