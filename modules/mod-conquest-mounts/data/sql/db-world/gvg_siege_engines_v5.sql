-- ============================================================================
-- Conquest Mounts v5 — Fix accessories (minion=1) + tourelles custom par vehicule
-- ============================================================================
-- DIAGNOSTIC : vanilla Leviathan (33113) utilise minion=1 dans
-- vehicle_template_accessory. minion=1 ajoute UNIT_MASK_ACCESSORY au spawn,
-- ce qui maintient correctement l'accessory attache au parent. Sans ca, les
-- accessoires se detachent et disparaissent (= notre bug).
--
-- v5 :
--   - Set minion=1 sur tous nos vehicles custom.
--   - Cree 4 nouvelles tourelles par vehicle :
--     400317 Tourelle pisteur M2 (display 27101)
--     400318 Tourelle baroudeur P-W8 (display 29489 = identique a 400207)
--     400319 Tourelle destructeur B27 Horde (display 28106)
--     400320 Tourelle destructeur B27 Alliance (display 25301)
--   - Remplace les anciennes tourelles dans vehicle_template_accessory.
-- ============================================================================

-- ============== 1. Force minion=1 sur tous nos accessoires ==================
-- v5 originally tried minion=1 but ca cassait les accessoires deja fonctionnels
-- (les destruction turrets disparaissaient). Revert a minion=0.
UPDATE `vehicle_template_accessory` SET `minion` = 0
WHERE `entry` IN (400200, 400201, 400209, 400310, 400311, 400312, 400314, 400315);

-- ============== 2. Nouvelles tourelles custom ===============================
DELETE FROM `creature_template_spell` WHERE `CreatureID` IN (400317, 400318, 400319, 400320);
DELETE FROM `creature_template_addon` WHERE `entry`      IN (400317, 400318, 400319, 400320);
DELETE FROM `npc_spellclick_spells`   WHERE `npc_entry`  IN (400317, 400318, 400319, 400320);
DELETE FROM `creature_template_model` WHERE `CreatureID` IN (400317, 400318, 400319, 400320);
DELETE FROM `creature_template`       WHERE `entry`      IN (400317, 400318, 400319, 400320);

-- 400317 Tourelle pisteur M2 (clone 400207, displayID 27101)
INSERT INTO `creature_template` SELECT
  400317, difficulty_entry_1, difficulty_entry_2, difficulty_entry_3, KillCredit1, KillCredit2,
  'Tourelle pisteur M2' AS name, '' AS subname, IconName, gossip_menu_id, minlevel, maxlevel, exp, faction,
  npcflag, speed_walk, speed_run, speed_swim, speed_flight, detection_range, `rank`, dmgschool,
  DamageModifier, BaseAttackTime, RangeAttackTime, BaseVariance, RangeVariance, unit_class, unit_flags,
  unit_flags2, dynamicflags, family, `type`, type_flags, lootid, pickpocketloot, skinloot, PetSpellDataId,
  VehicleId, mingold, maxgold, AIName, MovementType, HoverHeight,
  HealthModifier, ManaModifier, ArmorModifier, ExperienceModifier, RacialLeader, movementId,
  RegenHealth, CreatureImmunitiesId, flags_extra, ScriptName, VerifiedBuild
FROM `creature_template` WHERE `entry` = 400207;
INSERT INTO `creature_template_model` VALUES (400317, 0, 27101, 1, 1, 10314);
INSERT INTO `creature_template_addon` (`entry`, `path_id`, `mount`, `bytes1`, `bytes2`, `emote`, `visibilityDistanceType`, `auras`) VALUES (400317, 0, 0, 0, 1, 0, 4, NULL);
INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES (400317, 67830, 1, 0);

-- 400318 Tourelle baroudeur P-W8 (clone 400207, meme displayID 29489)
INSERT INTO `creature_template` SELECT
  400318, difficulty_entry_1, difficulty_entry_2, difficulty_entry_3, KillCredit1, KillCredit2,
  'Tourelle baroudeur P-W8', '', IconName, gossip_menu_id, minlevel, maxlevel, exp, faction,
  npcflag, speed_walk, speed_run, speed_swim, speed_flight, detection_range, `rank`, dmgschool,
  DamageModifier, BaseAttackTime, RangeAttackTime, BaseVariance, RangeVariance, unit_class, unit_flags,
  unit_flags2, dynamicflags, family, `type`, type_flags, lootid, pickpocketloot, skinloot, PetSpellDataId,
  VehicleId, mingold, maxgold, AIName, MovementType, HoverHeight,
  HealthModifier, ManaModifier, ArmorModifier, ExperienceModifier, RacialLeader, movementId,
  RegenHealth, CreatureImmunitiesId, flags_extra, ScriptName, VerifiedBuild
FROM `creature_template` WHERE `entry` = 400207;
INSERT INTO `creature_template_model` VALUES (400318, 0, 29489, 1, 1, 10314);
INSERT INTO `creature_template_addon` VALUES (400318, 0, 0, 0, 1, 0, 4, NULL);
INSERT INTO `npc_spellclick_spells` VALUES (400318, 67830, 1, 0);

-- 400319 Tourelle destructeur B27 Horde (clone 400208, displayID 28106)
INSERT INTO `creature_template` SELECT
  400319, difficulty_entry_1, difficulty_entry_2, difficulty_entry_3, KillCredit1, KillCredit2,
  'Tourelle destructeur B27', 'Horde', IconName, gossip_menu_id, minlevel, maxlevel, exp, faction,
  npcflag, speed_walk, speed_run, speed_swim, speed_flight, detection_range, `rank`, dmgschool,
  DamageModifier, BaseAttackTime, RangeAttackTime, BaseVariance, RangeVariance, unit_class, unit_flags,
  unit_flags2, dynamicflags, family, `type`, type_flags, lootid, pickpocketloot, skinloot, PetSpellDataId,
  VehicleId, mingold, maxgold, AIName, MovementType, HoverHeight,
  HealthModifier, ManaModifier, ArmorModifier, ExperienceModifier, RacialLeader, movementId,
  RegenHealth, CreatureImmunitiesId, flags_extra, ScriptName, VerifiedBuild
FROM `creature_template` WHERE `entry` = 400208;
INSERT INTO `creature_template_model` VALUES (400319, 0, 28106, 1, 1, 10314);
INSERT INTO `creature_template_addon` VALUES (400319, 0, 0, 0, 1, 0, 4, NULL);
INSERT INTO `npc_spellclick_spells` VALUES (400319, 67830, 1, 0);

-- 400320 Tourelle destructeur B27 Alliance (clone 400208, displayID 25301)
INSERT INTO `creature_template` SELECT
  400320, difficulty_entry_1, difficulty_entry_2, difficulty_entry_3, KillCredit1, KillCredit2,
  'Tourelle destructeur B27', 'Alliance', IconName, gossip_menu_id, minlevel, maxlevel, exp, faction,
  npcflag, speed_walk, speed_run, speed_swim, speed_flight, detection_range, `rank`, dmgschool,
  DamageModifier, BaseAttackTime, RangeAttackTime, BaseVariance, RangeVariance, unit_class, unit_flags,
  unit_flags2, dynamicflags, family, `type`, type_flags, lootid, pickpocketloot, skinloot, PetSpellDataId,
  VehicleId, mingold, maxgold, AIName, MovementType, HoverHeight,
  HealthModifier, ManaModifier, ArmorModifier, ExperienceModifier, RacialLeader, movementId,
  RegenHealth, CreatureImmunitiesId, flags_extra, ScriptName, VerifiedBuild
FROM `creature_template` WHERE `entry` = 400208;
INSERT INTO `creature_template_model` VALUES (400320, 0, 25301, 1, 1, 10314);
INSERT INTO `creature_template_addon` VALUES (400320, 0, 0, 0, 1, 0, 4, NULL);
INSERT INTO `npc_spellclick_spells` VALUES (400320, 67830, 1, 0);

-- ============== 3. Update vehicle_template_accessory ========================
-- Remplace les tourelles seat 7 sur M2 et P-W8, et seats 1+2 sur B27.
DELETE FROM `vehicle_template_accessory` WHERE `entry` IN (400200, 400201, 400209, 400310, 400311, 400312);

-- P-W8 rouge (400200) + P-W8 bleu (400311) : seats 1+2 = Siege chair, seat 7 = Tourelle baroudeur P-W8 (400318)
INSERT INTO `vehicle_template_accessory` (`entry`, `accessory_entry`, `seat_id`, `minion`, `description`, `summontype`, `summontimer`) VALUES
(400200, 400313, 1, 0, 'P-W8 Horde - siege seat 1',           6, 30000),
(400200, 400313, 2, 0, 'P-W8 Horde - siege seat 2',           6, 30000),
(400200, 400318, 7, 0, 'P-W8 Horde - turret main',            6, 30000),
(400311, 400313, 1, 0, 'P-W8 Alliance - siege seat 1',        6, 30000),
(400311, 400313, 2, 0, 'P-W8 Alliance - siege seat 2',        6, 30000),
(400311, 400318, 7, 0, 'P-W8 Alliance - turret main',         6, 30000);

-- B27 bleu (400201) : seats 1+2 = Tourelle B27 Alliance (400320), seat 7 = Canon massif (400211)
INSERT INTO `vehicle_template_accessory` VALUES
(400201, 400320, 1, 0, 'B27 Alliance - turret left',          6, 30000),
(400201, 400320, 2, 0, 'B27 Alliance - turret right',         6, 30000),
(400201, 400211, 7, 0, 'B27 Alliance - Canon massif main',    6, 30000);

-- B27 rouge (400312) : seats 1+2 = Tourelle B27 Horde (400319), seat 7 = Canon massif (400211)
INSERT INTO `vehicle_template_accessory` VALUES
(400312, 400319, 1, 0, 'B27 Horde - turret left',             6, 30000),
(400312, 400319, 2, 0, 'B27 Horde - turret right',            6, 30000),
(400312, 400211, 7, 0, 'B27 Horde - Canon massif main',       6, 30000);

-- M2 bleu (400209) + M2 rouge (400310) : seat 7 = Tourelle pisteur M2 (400317)
INSERT INTO `vehicle_template_accessory` VALUES
(400209, 400317, 7, 0, 'M2 Alliance - turret main',           6, 30000),
(400310, 400317, 7, 0, 'M2 Horde - turret main',              6, 30000);

SELECT CONCAT('v5 OK: ', COUNT(*), ' tourelles custom')
FROM `creature_template` WHERE `entry` IN (400317, 400318, 400319, 400320);
