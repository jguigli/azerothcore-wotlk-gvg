-- ============================================================================
-- Conquest Mounts v11 — Spells de tir sur chaque tourelle mountable
-- ============================================================================
-- Vanilla spells dispo :
--   67461 + 67462 = Siege Turret (34777) -- "Ram" / "Steam Rush"
--   67452 + 66541 = Keep Cannon (34944) -- "Cannon Blast" / "Cannon Blast"
--   66529          = Horde Gunship Cannon
--   66183 + 66186 = Flame Turret (Lance-flammes, deja sur 400322-400325)
-- ============================================================================

DELETE FROM `creature_template_spell` WHERE `CreatureID` IN
    (400211, 400316, 400317, 400318, 400319, 400320, 400321);

-- Canon massif B27 (400211) -> Keep Cannon spells (heavy bombing)
INSERT INTO `creature_template_spell` (`CreatureID`, `Index`, `Spell`, `VerifiedBuild`) VALUES
(400211, 0, 67452, 10314),
(400211, 1, 66541, 10314);

-- Tourelle pisteur M2 (400317) -> Keep Cannon spells (mobile cannon)
INSERT INTO `creature_template_spell` VALUES
(400317, 0, 67452, 10314),
(400317, 1, 66541, 10314);

-- Tourelles baroudeur P-W8 (400318/400321) -> Siege Turret spells
INSERT INTO `creature_template_spell` VALUES
(400318, 0, 67461, 10314),
(400318, 1, 67462, 10314),
(400321, 0, 67461, 10314),
(400321, 1, 67462, 10314);

-- Tourelles destructeur B27 lateral (400319/400320) -> Siege Turret spells
INSERT INTO `creature_template_spell` VALUES
(400319, 0, 67461, 10314),
(400319, 1, 67462, 10314),
(400320, 0, 67461, 10314),
(400320, 1, 67462, 10314);

-- Tourelle Leviathan (400316) -> Keep Cannon spells (heavy)
INSERT INTO `creature_template_spell` VALUES
(400316, 0, 67452, 10314),
(400316, 1, 66541, 10314);

SELECT 'v11 OK' AS Result;
