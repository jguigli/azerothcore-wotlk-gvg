-- ============================================================================
-- Conquest Mounts v13 — Leviathan tweaks (scale + speed + seats sieges + turret)
-- ============================================================================

-- 1. Siege chair (400313) DisplayScale 0.3
UPDATE `creature_template_model` SET `DisplayScale` = 0.3 WHERE `CreatureID` = 400313;

-- 2. Tourelle Leviathan (400316) DisplayScale 0.3
UPDATE `creature_template_model` SET `DisplayScale` = 0.3 WHERE `CreatureID` = 400316;

-- 3. Tourelle Leviathan spells : 67452 + 67461
DELETE FROM `creature_template_spell` WHERE `CreatureID` = 400316;
INSERT INTO `creature_template_spell` (`CreatureID`, `Index`, `Spell`, `VerifiedBuild`) VALUES
(400316, 0, 67452, 10314),
(400316, 1, 67461, 10314);

-- 4. Leviathan : seats 1+2 = Siege chairs (400313) au lieu de Lance-flammes
DELETE FROM `vehicle_template_accessory` WHERE `entry` IN (400314, 400315);

INSERT INTO `vehicle_template_accessory` (`entry`, `accessory_entry`, `seat_id`, `minion`, `description`, `summontype`, `summontimer`) VALUES
(400314, 400313, 1, 0, 'Leviathan A - siege seat 1', 6, 30000),
(400314, 400313, 2, 0, 'Leviathan A - siege seat 2', 6, 30000),
(400314, 400316, 7, 0, 'Leviathan A - turret main',  6, 30000);

INSERT INTO `vehicle_template_accessory` VALUES
(400315, 400313, 1, 0, 'Leviathan H - siege seat 1', 6, 30000),
(400315, 400313, 2, 0, 'Leviathan H - siege seat 2', 6, 30000),
(400315, 400316, 7, 0, 'Leviathan H - turret main',  6, 30000);

SELECT 'v13 OK' AS Result;
