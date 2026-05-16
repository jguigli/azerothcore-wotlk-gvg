-- ============================================================================
-- Conquest Mounts v14 — Protecteur E800 (vehicle + 2 tourelles)
-- ============================================================================
--   400326 Protecteur E800       (vehicle, scale 1, speed 1, both factions)
--   400327 Canon de protecteur E800   (seat 7, display 28526, spells 66541+66186)
--   400328 Tourelle de protecteur E800 (seats 1+2, display 29489, spell 67452)
-- ============================================================================

DELETE FROM `creature_template_spell`     WHERE `CreatureID` IN (400326, 400327, 400328);
DELETE FROM `creature_template_addon`     WHERE `entry`      IN (400326, 400327, 400328);
DELETE FROM `npc_spellclick_spells`       WHERE `npc_entry`  IN (400326, 400327, 400328);
DELETE FROM `creature_template_model`     WHERE `CreatureID` IN (400326, 400327, 400328);
DELETE FROM `creature_template`           WHERE `entry`      IN (400326, 400327, 400328);
DELETE FROM `vehicle_template_accessory`  WHERE `entry`      IN (400326);

-- =========================== 400326 Protecteur E800 (vehicle parent) ==========
-- VehicleId 514 (Horde Siege Engine layout) - 4 seats 0/1/2/7, seat 0 = driver.
-- ScriptName 'ConquestSiegeEngine' pour passer par notre AI custom (HP/scale).
INSERT INTO `creature_template`
(`entry`, `name`, `subname`, `IconName`, `minlevel`, `maxlevel`, `faction`, `npcflag`,
 `speed_walk`, `speed_run`, `detection_range`, `unit_class`, `rank`, `type`, `type_flags`,
 `family`, `VehicleId`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`,
 `DamageModifier`, `HoverHeight`, `unit_flags`, `unit_flags2`, `movementId`,
 `flags_extra`, `RegenHealth`, `HealthModifier`, `ManaModifier`, `ArmorModifier`,
 `ExperienceModifier`, `AIName`, `MovementType`, `ScriptName`, `VerifiedBuild`)
VALUES
(400326, 'Protecteur E800', '', 'vehichleCursor', 70, 70, 35, 16777216,
 1.0, 1.0, 20, 1, 0, 9, 393256,
 0, 514, 2000, 2000, 1, 1,
 1, 1, 16384, 2048, 113,
 344407930, 0, 1, 1, 1,
 1, '', 0, 'ConquestSiegeEngine', 10314);

INSERT INTO `creature_template_model` VALUES (400326, 0, 28650, 1, 1, 10314);

INSERT INTO `creature_template_addon` (`entry`, `path_id`, `mount`, `bytes1`, `bytes2`, `emote`, `visibilityDistanceType`, `auras`) VALUES
(400326, 0, 0, 0, 1, 0, 4, NULL);

-- Spellclick pour permettre au joueur de monter (driver seat 0)
INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(400326, 46598, 1, 0),
(400326, 66245, 1, 0);

-- =========================== 400327 Canon de protecteur E800 (seat 7) =========
INSERT INTO `creature_template`
(`entry`, `name`, `subname`, `IconName`, `minlevel`, `maxlevel`, `faction`, `npcflag`,
 `speed_walk`, `speed_run`, `detection_range`, `unit_class`, `rank`, `type`, `type_flags`,
 `family`, `VehicleId`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`,
 `DamageModifier`, `HoverHeight`, `unit_flags`, `unit_flags2`, `movementId`,
 `flags_extra`, `RegenHealth`, `HealthModifier`, `ManaModifier`, `ArmorModifier`,
 `ExperienceModifier`, `AIName`, `MovementType`, `ScriptName`, `VerifiedBuild`)
VALUES
(400327, 'Canon de protecteur E800', '', 'Gunner', 70, 70, 35, 16777216,
 1.2, 1.14286, 20, 1, 0, 9, 262184,
 0, 436, 2000, 2000, 1, 1,
 1, 15.9116, 2, 2048, 121,
 344407930, 0, 1, 1, 1,
 1, '', 0, '', 10314);

INSERT INTO `creature_template_model` VALUES (400327, 0, 28526, 1, 1, 10314);
INSERT INTO `creature_template_addon` VALUES (400327, 0, 0, 0, 1, 0, 4, NULL);
INSERT INTO `npc_spellclick_spells` VALUES (400327, 67830, 1, 0);
INSERT INTO `creature_template_spell` (`CreatureID`, `Index`, `Spell`, `VerifiedBuild`) VALUES
(400327, 0, 66541, 10314),
(400327, 1, 66186, 10314);

-- =========================== 400328 Tourelle de protecteur E800 (seats 1+2) ===
INSERT INTO `creature_template`
(`entry`, `name`, `subname`, `IconName`, `minlevel`, `maxlevel`, `faction`, `npcflag`,
 `speed_walk`, `speed_run`, `detection_range`, `unit_class`, `rank`, `type`, `type_flags`,
 `family`, `VehicleId`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`,
 `DamageModifier`, `HoverHeight`, `unit_flags`, `unit_flags2`, `movementId`,
 `flags_extra`, `RegenHealth`, `HealthModifier`, `ManaModifier`, `ArmorModifier`,
 `ExperienceModifier`, `AIName`, `MovementType`, `ScriptName`, `VerifiedBuild`)
VALUES
(400328, 'Tourelle de protecteur E800', '', 'Gunner', 70, 70, 35, 16777216,
 1.2, 1.14286, 20, 1, 0, 9, 262184,
 0, 436, 2000, 2000, 1, 1,
 1, 15.9116, 2, 2048, 121,
 344407930, 0, 1, 1, 1,
 1, '', 0, '', 10314);

INSERT INTO `creature_template_model` VALUES (400328, 0, 29489, 1, 1, 10314);
INSERT INTO `creature_template_addon` VALUES (400328, 0, 0, 0, 1, 0, 4, NULL);
INSERT INTO `npc_spellclick_spells` VALUES (400328, 67830, 1, 0);
INSERT INTO `creature_template_spell` VALUES (400328, 0, 67452, 10314);

-- =========================== Vehicle accessories ==============================
INSERT INTO `vehicle_template_accessory` (`entry`, `accessory_entry`, `seat_id`, `minion`, `description`, `summontype`, `summontimer`) VALUES
(400326, 400328, 1, 0, 'Protecteur E800 - tourelle gauche', 6, 30000),
(400326, 400328, 2, 0, 'Protecteur E800 - tourelle droite', 6, 30000),
(400326, 400327, 7, 0, 'Protecteur E800 - canon principal', 6, 30000);

SELECT 'v14 OK' AS Result;
