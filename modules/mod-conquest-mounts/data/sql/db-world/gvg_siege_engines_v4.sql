-- ============================================================================
-- Conquest Mounts v4 — Fix Siege chair + Leviathan turret + subnames refresh
-- ============================================================================

-- 1. Force re-application des subnames (le worldserver les avait dejà en DB,
--    mais ce SQL re-confirme pour eviter toute confusion).
UPDATE `creature_template` SET `subname` = 'Horde'    WHERE `entry` = 400200;
UPDATE `creature_template` SET `subname` = 'Alliance' WHERE `entry` = 400201;
UPDATE `creature_template` SET `subname` = 'Alliance' WHERE `entry` = 400203;
UPDATE `creature_template` SET `subname` = 'Horde'    WHERE `entry` = 400204;
UPDATE `creature_template` SET `subname` = 'Alliance' WHERE `entry` = 400209;

-- 2. Siege chair 400313 : flags_extra pour stabilite accessoire + spellclick
--    pour qu'on puisse cliquer dessus et s'y asseoir (comme Turkey Chair).
UPDATE `creature_template` SET `flags_extra` = 344407930 WHERE `entry` = 400313;

DELETE FROM `npc_spellclick_spells` WHERE `npc_entry` = 400313;
INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(400313, 46598, 1, 0); -- Ride Vehicle generic

-- 3. Nouveau Leviathan turret 400316 (clone 33139 mais mountable + neutre)
--    Le 33139 vanilla a faction 1965 (Ulduar hostile) -> spawn dans notre
--    contexte amical echoue. On clone avec faction 35 + spellclick.
DELETE FROM `creature_template_spell`  WHERE `CreatureID` = 400316;
DELETE FROM `creature_template_addon`  WHERE `entry`      = 400316;
DELETE FROM `npc_spellclick_spells`    WHERE `npc_entry`  = 400316;
DELETE FROM `creature_template_model`  WHERE `CreatureID` = 400316;
DELETE FROM `creature_template`        WHERE `entry`      = 400316;

INSERT INTO `creature_template` SELECT
  400316, difficulty_entry_1, difficulty_entry_2, difficulty_entry_3, KillCredit1, KillCredit2,
  'Tourelle Leviathan' AS name, '' AS subname, IconName, 0, minlevel, maxlevel, exp, 35 AS faction,
  16777216 AS npcflag, speed_walk, speed_run, speed_swim, speed_flight, detection_range, `rank`, dmgschool,
  DamageModifier, BaseAttackTime, RangeAttackTime, BaseVariance, RangeVariance, unit_class,
  2 AS unit_flags, 2048 AS unit_flags2, dynamicflags, family, `type`, type_flags, lootid, pickpocketloot, skinloot, PetSpellDataId,
  VehicleId, mingold, maxgold, '' AS AIName, MovementType, HoverHeight,
  HealthModifier, ManaModifier, ArmorModifier, ExperienceModifier, RacialLeader, movementId,
  RegenHealth, CreatureImmunitiesId, 344407930 AS flags_extra, '' AS ScriptName, VerifiedBuild
FROM `creature_template` WHERE `entry` = 33139;

INSERT INTO `creature_template_model`
SELECT 400316, Idx, CreatureDisplayID, DisplayScale, Probability, VerifiedBuild
FROM `creature_template_model` WHERE `CreatureID` = 33139;

INSERT INTO `creature_template_addon` (`entry`, `path_id`, `mount`, `bytes1`, `bytes2`, `emote`, `visibilityDistanceType`, `auras`)
VALUES (400316, 0, 0, 0, 1, 0, 4, NULL);

INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(400316, 46598, 1, 0);

-- 4. Update vehicle_template_accessory pour Leviathan : utiliser 400316 au lieu de 33139
DELETE FROM `vehicle_template_accessory` WHERE `entry` IN (400314, 400315);
INSERT INTO `vehicle_template_accessory` (`entry`, `accessory_entry`, `seat_id`, `minion`, `description`, `summontype`, `summontimer`) VALUES
(400314, 400313, 0, 0, 'Leviathan A - siege seat 0',  6, 30000),
(400314, 400313, 1, 0, 'Leviathan A - siege seat 1',  6, 30000),
(400314, 400313, 2, 0, 'Leviathan A - siege seat 2',  6, 30000),
(400314, 400313, 3, 0, 'Leviathan A - siege seat 3',  6, 30000),
(400314, 400316, 7, 0, 'Leviathan A - turret main',   6, 30000),
(400315, 400313, 0, 0, 'Leviathan H - siege seat 0',  6, 30000),
(400315, 400313, 1, 0, 'Leviathan H - siege seat 1',  6, 30000),
(400315, 400313, 2, 0, 'Leviathan H - siege seat 2',  6, 30000),
(400315, 400313, 3, 0, 'Leviathan H - siege seat 3',  6, 30000),
(400315, 400316, 7, 0, 'Leviathan H - turret main',   6, 30000);

SELECT CONCAT('v4 OK: ', COUNT(*), ' entries cles')
FROM `creature_template` WHERE `entry` IN (400313, 400316);
