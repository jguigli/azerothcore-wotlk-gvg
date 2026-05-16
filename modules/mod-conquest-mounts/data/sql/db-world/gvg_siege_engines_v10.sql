-- ============================================================================
-- Conquest Mounts v10 — Make all turrets mountable (VehicleId 436)
-- ============================================================================
-- Vanilla 34777 Siege Turret a VehicleId 436 (1 seat gunner) ET est installable
-- comme accessory sur 34776 Siege Engine. Pattern qui marche pour les 2 :
--   1. Player clique tourelle -> spellclick 67830 -> entre dans vehicle 436
--      seat 0 (rôle gunner).
--   2. Vehicle parent (B27/P-W8/M2) appelle InstallAllAccessories ->
--      HandleSpellClick fait que la tourelle entre dans le seat parent.
-- Nos tourelles avaient VehicleId=0 -> NON mountable. Fix.

UPDATE `creature_template` SET `VehicleId` = 436 WHERE `entry` IN (
    400211, -- Canon massif (B27 seat 7)
    400317, -- Tourelle pisteur M2
    400318, -- Tourelle baroudeur P-W8 Alliance
    400321, -- Tourelle baroudeur P-W8 Horde
    400319, -- Tourelle destructeur B27 Horde
    400320, -- Tourelle destructeur B27 Alliance
    400316  -- Tourelle Leviathan
);

SELECT 'v10 OK' AS Result;
