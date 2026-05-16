-- ============================================================================
-- Conquest Mounts v12 — Spells ajustes + Leviathan vehicleId fix
-- ============================================================================

-- 1. Spells par tourelle (request utilisateur)
DELETE FROM `creature_template_spell` WHERE `CreatureID` IN
    (400211, 400316, 400317, 400318, 400319, 400320, 400321);

-- Canon massif (400211) : 67461 + 66541
INSERT INTO `creature_template_spell` (`CreatureID`, `Index`, `Spell`, `VerifiedBuild`) VALUES
(400211, 0, 67461, 10314),
(400211, 1, 66541, 10314);

-- Tourelle pisteur M2 (400317) : 67452 seul
INSERT INTO `creature_template_spell` VALUES
(400317, 0, 67452, 10314);

-- Tourelles baroudeur P-W8 (400318/400321) : 67461 seul
INSERT INTO `creature_template_spell` VALUES
(400318, 0, 67461, 10314),
(400321, 0, 67461, 10314);

-- Tourelles destructeur B27 (400319/400320) : 66541 seul
INSERT INTO `creature_template_spell` VALUES
(400319, 0, 66541, 10314),
(400320, 0, 66541, 10314);

-- Tourelle Leviathan (400316) : garde 67452 + 66541
INSERT INTO `creature_template_spell` VALUES
(400316, 0, 67452, 10314),
(400316, 1, 66541, 10314);

-- 2. Leviathan : changer VehicleId 340 (Flame Leviathan boss, seat 0 pas
--    driver-capable) -> 514 (Horde Siege Engine, seat 0 driver). On garde
--    le modele visuel 28875 mais le vehicule a 4 seats (0/1/2/7) au lieu de 5.
UPDATE `creature_template` SET `VehicleId` = 514 WHERE `entry` IN (400314, 400315);

-- 3. Update accessories Leviathan : seats 1/2 = Lance-flammes, seat 7 = Tourelle.
--    Seat 0 reste libre pour le joueur driver. On perd 1 Lance-flamme.
DELETE FROM `vehicle_template_accessory` WHERE `entry` IN (400314, 400315);

INSERT INTO `vehicle_template_accessory` (`entry`, `accessory_entry`, `seat_id`, `minion`, `description`, `summontype`, `summontimer`) VALUES
(400314, 400324, 1, 0, 'Leviathan A - lance flamme 1', 6, 30000),
(400314, 400324, 2, 0, 'Leviathan A - lance flamme 2', 6, 30000),
(400314, 400316, 7, 0, 'Leviathan A - turret main',    6, 30000);

INSERT INTO `vehicle_template_accessory` VALUES
(400315, 400325, 1, 0, 'Leviathan H - lance flamme 1', 6, 30000),
(400315, 400325, 2, 0, 'Leviathan H - lance flamme 2', 6, 30000),
(400315, 400316, 7, 0, 'Leviathan H - turret main',    6, 30000);

SELECT 'v12 OK' AS Result;
