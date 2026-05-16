-- ============================================================================
-- Conquest Mounts v9 — Spells Lance-flammes + Leviathan driver seat libre
-- ============================================================================

-- 1. Ajouter les spells flame (66183 + 66186) sur les 4 Lance-flammes.
--    Sans ca, les joueurs s'asseyent dans le canon mais pas de boutons en
--    actionbar (vehicle spellbar prend les spells du creature_template_spell).
DELETE FROM `creature_template_spell` WHERE `CreatureID` IN (400322, 400323, 400324, 400325);
INSERT INTO `creature_template_spell` (`CreatureID`, `Index`, `Spell`, `VerifiedBuild`) VALUES
(400322, 0, 66183, 10314),
(400322, 1, 66186, 10314),
(400323, 0, 66183, 10314),
(400323, 1, 66186, 10314),
(400324, 0, 66183, 10314),
(400324, 1, 66186, 10314),
(400325, 0, 66183, 10314),
(400325, 1, 66186, 10314);

-- 2. Leviathan : liberer le seat 0 pour permettre au joueur d'y entrer comme
--    driver. Vehicle 340 (Flame Leviathan) a 5 seats (0/1/2/3/7) ; on les
--    avait TOUS pris par accessoires, donc AddPassenger ne trouvait pas de
--    seat libre = mount impossible.
--    On garde seats 1/2/3 = Lance-flammes + seat 7 = Tourelle. Seat 0 libre
--    = entree player. Resultat : 3 chairs + 1 turret + 1 driver (au lieu de 4).
DELETE FROM `vehicle_template_accessory` WHERE `entry` IN (400314, 400315);

INSERT INTO `vehicle_template_accessory` (`entry`, `accessory_entry`, `seat_id`, `minion`, `description`, `summontype`, `summontimer`) VALUES
(400314, 400324, 1, 0, 'Leviathan A - lance flamme 1',   6, 30000),
(400314, 400324, 2, 0, 'Leviathan A - lance flamme 2',   6, 30000),
(400314, 400324, 3, 0, 'Leviathan A - lance flamme 3',   6, 30000),
(400314, 400316, 7, 0, 'Leviathan A - turret main',      6, 30000);

INSERT INTO `vehicle_template_accessory` VALUES
(400315, 400325, 1, 0, 'Leviathan H - lance flamme 1',   6, 30000),
(400315, 400325, 2, 0, 'Leviathan H - lance flamme 2',   6, 30000),
(400315, 400325, 3, 0, 'Leviathan H - lance flamme 3',   6, 30000),
(400315, 400316, 7, 0, 'Leviathan H - turret main',      6, 30000);

SELECT 'v9 OK' AS Result;
