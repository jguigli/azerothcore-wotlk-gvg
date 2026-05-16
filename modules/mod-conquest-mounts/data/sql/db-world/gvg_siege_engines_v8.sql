-- ============================================================================
-- Conquest Mounts v8 — Display fixes + Lance-flammes + Leviathan mount fix
-- ============================================================================

-- 1. Pisteur M2 turret (400317) : revert display 29489
UPDATE `creature_template_model` SET `CreatureDisplayID` = 29489 WHERE `CreatureID` = 400317;

-- 2. P-W8 turret 400318 = Alliance display 25301 (rename)
UPDATE `creature_template` SET `name` = 'Tourelle baroudeur P-W8', `subname` = 'Alliance' WHERE `entry` = 400318;
UPDATE `creature_template_model` SET `CreatureDisplayID` = 25301 WHERE `CreatureID` = 400318;

-- 3. CREATE 400321 P-W8 turret Horde (clone 400318, displayId 28106)
DELETE FROM `creature_template_spell` WHERE `CreatureID` = 400321;
DELETE FROM `creature_template_addon` WHERE `entry` = 400321;
DELETE FROM `npc_spellclick_spells`   WHERE `npc_entry` = 400321;
DELETE FROM `creature_template_model` WHERE `CreatureID` = 400321;
DELETE FROM `creature_template`       WHERE `entry` = 400321;

INSERT INTO `creature_template` SELECT
  400321, difficulty_entry_1, difficulty_entry_2, difficulty_entry_3, KillCredit1, KillCredit2,
  'Tourelle baroudeur P-W8', 'Horde', IconName, gossip_menu_id, minlevel, maxlevel, exp, faction,
  npcflag, speed_walk, speed_run, speed_swim, speed_flight, detection_range, `rank`, dmgschool,
  DamageModifier, BaseAttackTime, RangeAttackTime, BaseVariance, RangeVariance, unit_class, unit_flags,
  unit_flags2, dynamicflags, family, `type`, type_flags, lootid, pickpocketloot, skinloot, PetSpellDataId,
  VehicleId, mingold, maxgold, AIName, MovementType, HoverHeight,
  HealthModifier, ManaModifier, ArmorModifier, ExperienceModifier, RacialLeader, movementId,
  RegenHealth, CreatureImmunitiesId, flags_extra, ScriptName, VerifiedBuild
FROM `creature_template` WHERE `entry` = 400318;
INSERT INTO `creature_template_model` VALUES (400321, 0, 28106, 1, 1, 10314);
INSERT INTO `creature_template_addon` (`entry`, `path_id`, `mount`, `bytes1`, `bytes2`, `emote`, `visibilityDistanceType`, `auras`) VALUES (400321, 0, 0, 0, 1, 0, 4, NULL);
INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES (400321, 67830, 1, 0);

-- 4. Lance-flamme baroudeur P-W8 (clone 34778 / 36356, faction 35)
DELETE FROM `creature_template_spell` WHERE `CreatureID` IN (400322, 400323, 400324, 400325);
DELETE FROM `creature_template_addon` WHERE `entry`      IN (400322, 400323, 400324, 400325);
DELETE FROM `npc_spellclick_spells`   WHERE `npc_entry`  IN (400322, 400323, 400324, 400325);
DELETE FROM `creature_template_model` WHERE `CreatureID` IN (400322, 400323, 400324, 400325);
DELETE FROM `creature_template`       WHERE `entry`      IN (400322, 400323, 400324, 400325);

-- 400322 Lance flamme baroudeur P-W8 Alliance (clone 34778)
INSERT INTO `creature_template` SELECT
  400322, difficulty_entry_1, difficulty_entry_2, difficulty_entry_3, KillCredit1, KillCredit2,
  'Lance flamme baroudeur P-W8', 'Alliance', IconName, gossip_menu_id, minlevel, maxlevel, exp, 35,
  npcflag, speed_walk, speed_run, speed_swim, speed_flight, detection_range, `rank`, dmgschool,
  DamageModifier, BaseAttackTime, RangeAttackTime, BaseVariance, RangeVariance, unit_class, unit_flags,
  unit_flags2, dynamicflags, family, `type`, type_flags, lootid, pickpocketloot, skinloot, PetSpellDataId,
  VehicleId, mingold, maxgold, AIName, MovementType, HoverHeight,
  HealthModifier, ManaModifier, ArmorModifier, ExperienceModifier, RacialLeader, movementId,
  RegenHealth, CreatureImmunitiesId, 344407930, '', VerifiedBuild
FROM `creature_template` WHERE `entry` = 34778;
INSERT INTO `creature_template_model` VALUES (400322, 0, 29424, 1, 1, 10314);
INSERT INTO `creature_template_addon` VALUES (400322, 0, 0, 0, 1, 0, 4, NULL);
INSERT INTO `npc_spellclick_spells` VALUES (400322, 67830, 1, 0);

-- 400323 Lance flamme baroudeur P-W8 Horde (clone 36356)
INSERT INTO `creature_template` SELECT
  400323, difficulty_entry_1, difficulty_entry_2, difficulty_entry_3, KillCredit1, KillCredit2,
  'Lance flamme baroudeur P-W8', 'Horde', IconName, gossip_menu_id, minlevel, maxlevel, exp, 35,
  npcflag, speed_walk, speed_run, speed_swim, speed_flight, detection_range, `rank`, dmgschool,
  DamageModifier, BaseAttackTime, RangeAttackTime, BaseVariance, RangeVariance, unit_class, unit_flags,
  unit_flags2, dynamicflags, family, `type`, type_flags, lootid, pickpocketloot, skinloot, PetSpellDataId,
  VehicleId, mingold, maxgold, AIName, MovementType, HoverHeight,
  HealthModifier, ManaModifier, ArmorModifier, ExperienceModifier, RacialLeader, movementId,
  RegenHealth, CreatureImmunitiesId, 344407930, '', VerifiedBuild
FROM `creature_template` WHERE `entry` = 36356;
INSERT INTO `creature_template_model` VALUES (400323, 0, 30080, 1, 1, 10314);
INSERT INTO `creature_template_addon` VALUES (400323, 0, 0, 0, 1, 0, 4, NULL);
INSERT INTO `npc_spellclick_spells` VALUES (400323, 67830, 1, 0);

-- 400324 Lance-flamme leviathan 330 Alliance (clone 34778, meme que 400322)
INSERT INTO `creature_template` SELECT
  400324, difficulty_entry_1, difficulty_entry_2, difficulty_entry_3, KillCredit1, KillCredit2,
  'Lance-flamme leviathan 330', 'Alliance', IconName, gossip_menu_id, minlevel, maxlevel, exp, 35,
  npcflag, speed_walk, speed_run, speed_swim, speed_flight, detection_range, `rank`, dmgschool,
  DamageModifier, BaseAttackTime, RangeAttackTime, BaseVariance, RangeVariance, unit_class, unit_flags,
  unit_flags2, dynamicflags, family, `type`, type_flags, lootid, pickpocketloot, skinloot, PetSpellDataId,
  VehicleId, mingold, maxgold, AIName, MovementType, HoverHeight,
  HealthModifier, ManaModifier, ArmorModifier, ExperienceModifier, RacialLeader, movementId,
  RegenHealth, CreatureImmunitiesId, 344407930, '', VerifiedBuild
FROM `creature_template` WHERE `entry` = 34778;
INSERT INTO `creature_template_model` VALUES (400324, 0, 29424, 1, 1, 10314);
INSERT INTO `creature_template_addon` VALUES (400324, 0, 0, 0, 1, 0, 4, NULL);
INSERT INTO `npc_spellclick_spells` VALUES (400324, 67830, 1, 0);

-- 400325 Lance-flamme leviathan 330 Horde (clone 36356)
INSERT INTO `creature_template` SELECT
  400325, difficulty_entry_1, difficulty_entry_2, difficulty_entry_3, KillCredit1, KillCredit2,
  'Lance-flamme leviathan 330', 'Horde', IconName, gossip_menu_id, minlevel, maxlevel, exp, 35,
  npcflag, speed_walk, speed_run, speed_swim, speed_flight, detection_range, `rank`, dmgschool,
  DamageModifier, BaseAttackTime, RangeAttackTime, BaseVariance, RangeVariance, unit_class, unit_flags,
  unit_flags2, dynamicflags, family, `type`, type_flags, lootid, pickpocketloot, skinloot, PetSpellDataId,
  VehicleId, mingold, maxgold, AIName, MovementType, HoverHeight,
  HealthModifier, ManaModifier, ArmorModifier, ExperienceModifier, RacialLeader, movementId,
  RegenHealth, CreatureImmunitiesId, 344407930, '', VerifiedBuild
FROM `creature_template` WHERE `entry` = 36356;
INSERT INTO `creature_template_model` VALUES (400325, 0, 30080, 1, 1, 10314);
INSERT INTO `creature_template_addon` VALUES (400325, 0, 0, 0, 1, 0, 4, NULL);
INSERT INTO `npc_spellclick_spells` VALUES (400325, 67830, 1, 0);

-- 5. CRITICAL : Leviathan (400314/400315) avait npcflag=0 herite du boss
--    vanilla 33113. SANS UNIT_NPC_FLAG_SPELLCLICK (16777216), cliquer ne
--    declenche rien. unit_flags 64 (UNK_6) heritage du boss aussi -> changer
--    a 16384 comme les autres siege engines.
UPDATE `creature_template`
SET `npcflag` = 16777216,
    `unit_flags` = 16384
WHERE `entry` IN (400314, 400315);

-- 6. UPDATE vehicle_template_accessory
DELETE FROM `vehicle_template_accessory` WHERE `entry` IN (400200, 400311, 400314, 400315);

-- P-W8 rouge (400200, Horde) : seats 1+2 = Lance flamme Horde, seat 7 = Tourelle P-W8 Horde
INSERT INTO `vehicle_template_accessory` (`entry`, `accessory_entry`, `seat_id`, `minion`, `description`, `summontype`, `summontimer`) VALUES
(400200, 400323, 1, 0, 'P-W8 Horde - lance flamme left',  6, 30000),
(400200, 400323, 2, 0, 'P-W8 Horde - lance flamme right', 6, 30000),
(400200, 400321, 7, 0, 'P-W8 Horde - turret main',         6, 30000);

-- P-W8 bleu (400311, Alliance) : seats 1+2 = Lance flamme Alliance, seat 7 = Tourelle P-W8 Alliance
INSERT INTO `vehicle_template_accessory` VALUES
(400311, 400322, 1, 0, 'P-W8 Alliance - lance flamme left',  6, 30000),
(400311, 400322, 2, 0, 'P-W8 Alliance - lance flamme right', 6, 30000),
(400311, 400318, 7, 0, 'P-W8 Alliance - turret main',         6, 30000);

-- Leviathan Alliance (400314) : seats 0/1/2/3 = Lance-flamme leviathan Alliance, seat 7 = Tourelle Leviathan
INSERT INTO `vehicle_template_accessory` VALUES
(400314, 400324, 0, 0, 'Leviathan A - lance flamme 0',   6, 30000),
(400314, 400324, 1, 0, 'Leviathan A - lance flamme 1',   6, 30000),
(400314, 400324, 2, 0, 'Leviathan A - lance flamme 2',   6, 30000),
(400314, 400324, 3, 0, 'Leviathan A - lance flamme 3',   6, 30000),
(400314, 400316, 7, 0, 'Leviathan A - turret main',      6, 30000);

-- Leviathan Horde (400315) : seats 0/1/2/3 = Lance-flamme leviathan Horde, seat 7 = Tourelle Leviathan
INSERT INTO `vehicle_template_accessory` VALUES
(400315, 400325, 0, 0, 'Leviathan H - lance flamme 0',   6, 30000),
(400315, 400325, 1, 0, 'Leviathan H - lance flamme 1',   6, 30000),
(400315, 400325, 2, 0, 'Leviathan H - lance flamme 2',   6, 30000),
(400315, 400325, 3, 0, 'Leviathan H - lance flamme 3',   6, 30000),
(400315, 400316, 7, 0, 'Leviathan H - turret main',      6, 30000);

SELECT CONCAT('v8 OK : ', COUNT(*), ' new entries')
FROM `creature_template` WHERE `entry` IN (400321, 400322, 400323, 400324, 400325);
