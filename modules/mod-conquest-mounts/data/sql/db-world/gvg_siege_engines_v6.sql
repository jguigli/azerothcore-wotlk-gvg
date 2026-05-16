-- ============================================================================
-- Conquest Mounts v6 — Scales + 400317 Keep Cannon clone + Leviathan addon
-- ============================================================================

-- 1. Siege chair 400313 DisplayScale 0.5 (utilise sur P-W8 + Leviathan)
UPDATE `creature_template_model` SET `DisplayScale` = 0.5 WHERE `CreatureID` = 400313;

-- 2. Tourelles destructeur B27 (400319 Horde + 400320 Alliance) DisplayScale 0.75
UPDATE `creature_template_model` SET `DisplayScale` = 0.75 WHERE `CreatureID` IN (400319, 400320);

-- 3. 400317 Tourelle pisteur M2 : adopter les attributs de Keep Cannon (34944)
--    -> VehicleId 510, spellclick 68458, unit_flags 16388, faction 35.
UPDATE `creature_template`
SET `VehicleId` = 510,
    `unit_flags` = 16388,
    `unit_flags2` = 2048,
    `flags_extra` = 0,
    `faction` = 35,
    `type` = 9,
    `type_flags` = (SELECT `type_flags` FROM (SELECT `type_flags` FROM `creature_template` WHERE `entry` = 34944) AS t),
    `BaseAttackTime` = 2000,
    `RangeAttackTime` = 2000
WHERE `entry` = 400317;

DELETE FROM `npc_spellclick_spells` WHERE `npc_entry` = 400317;
INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(400317, 68458, 1, 0);

-- 4. creature_template_addon pour Leviathan (manquait, copy de 33113)
DELETE FROM `creature_template_addon` WHERE `entry` IN (400314, 400315);
INSERT INTO `creature_template_addon` (`entry`, `path_id`, `mount`, `bytes1`, `bytes2`, `emote`, `visibilityDistanceType`, `auras`) VALUES
(400314, 0, 0, 0, 1, 0, 3, NULL),
(400315, 0, 0, 0, 1, 0, 3, NULL);

-- 5. CRITICAL : npc_spellclick_spells obligatoire pour que ObjectMgr charge
--    vehicle_template_accessory (cf ObjectMgr.cpp:4029). Sans ca, les
--    accessoires du Leviathan sont REJETES SILENCIEUSEMENT au boot.
INSERT IGNORE INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(400314, 46598, 1, 0),
(400315, 46598, 1, 0);

SELECT 'v6 OK' AS Result;
