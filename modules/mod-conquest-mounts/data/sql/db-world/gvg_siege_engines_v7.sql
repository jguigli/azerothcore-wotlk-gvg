-- ============================================================================
-- Conquest Mounts v7 — Fix M2 turret attach + Leviathan mount + turret scale 0.8
-- ============================================================================

-- 1. Revert 400317 (Tourelle pisteur M2) au pattern accessoire standard.
--    VehicleId=510 + spell 68458 (Keep Cannon) cassait l'install accessory.
--    On garde uniquement le displayID 27101 (modele canon donjon) ; les autres
--    attributs sont alignes sur 400206 (combat turret) qui fonctionne.
UPDATE `creature_template`
SET `VehicleId` = 0,
    `unit_flags` = 2,
    `unit_flags2` = 2048,
    `flags_extra` = 344407930,
    `faction` = 35
WHERE `entry` = 400317;

DELETE FROM `npc_spellclick_spells` WHERE `npc_entry` = 400317;
INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(400317, 67830, 1, 0);

-- 2. Tourelle Leviathan (400316) scale 0.8
UPDATE `creature_template_model` SET `DisplayScale` = 0.8 WHERE `CreatureID` = 400316;

-- 3. Leviathan (400314 + 400315) : ajouter spell 66245 (Ride Vehicle Hardcoded)
--    pour permettre au joueur de monter, comme les Wintergrasp Siege Engine.
--    46598 seul ne suffit pas pour le vehicle 340 (Flame Leviathan).
INSERT IGNORE INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(400314, 66245, 1, 0),
(400315, 66245, 1, 0);

SELECT 'v7 OK' AS Result;
