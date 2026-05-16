-- Conquest Guard - Ogre Creatures
-- This SQL creates ogre guard NPCs based on existing entries with modified HP, level, and rank

-- ============================================
-- Brute ogre (entry 400300)
-- Based on entry 5229 (Gordunni Ogre)
-- HP: 50000, Level: 80
-- ============================================
DELETE FROM `creature_template_spell` WHERE `CreatureID` = 400300;
DELETE FROM `creature_template_addon` WHERE `entry` = 400300;
DELETE FROM `creature_template_model` WHERE `CreatureID` = 400300;
DELETE FROM `creature_template` WHERE `entry` = 400300;

INSERT INTO `creature_template` (`entry`, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`, `KillCredit1`, `KillCredit2`, `name`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`, `speed_walk`, `speed_run`, `speed_swim`, `speed_flight`, `detection_range`, `scale`, `rank`, `dmgschool`, `DamageModifier`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`, `unit_class`, `unit_flags`, `unit_flags2`, `dynamicflags`, `family`, `trainer_type`, `trainer_spell`, `trainer_class`, `trainer_race`, `type`, `type_flags`, `lootid`, `pickpocketloot`, `skinloot`, `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `HoverHeight`, `HealthModifier`, `ManaModifier`, `ArmorModifier`, `ExperienceModifier`, `RacialLeader`, `movementId`, `RegenHealth`, `mechanic_immune_mask`, `spell_school_immune_mask`, `flags_extra`, `ScriptName`, `VerifiedBuild`) VALUES
(400300, 0, 0, 0, 0, 0, 'Brute ogre', NULL, NULL, 0, 80, 80, 0, 45, 0, 1, 1.14286, 1, 1, 18, 1, 0, 0, 4.0, 2000, 2000, 1, 1, 1, 0, 2048, 0, 0, 0, 0, 0, 0, 7, 0, 5229, 5229, 0, 0, 0, 72, 322, 'SmartAI', 1, 1, 1.2, 1, 1, 1, 0, 0, 1, 0, 0, 0, 'ConquestGuard', 12340);

INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES
(400300, 0, 597, 1, 1, 12340);

INSERT INTO `creature_template_addon` (`entry`, `path_id`, `mount`, `bytes1`, `bytes2`, `emote`, `visibilityDistanceType`, `auras`) VALUES
(400300, 0, 0, 0, 1, 0, 0, NULL);

-- Note: Entry 5229 (Gordunni Ogre) has no spells, so 400300 has no spells either

-- ============================================
-- Ogre-mage (entry 400301)
-- Based on entry 5237 (Gordunni Ogre Mage)
-- HP: 50000, Level: 80
-- ============================================
DELETE FROM `creature_template_spell` WHERE `CreatureID` = 400301;
DELETE FROM `creature_template_addon` WHERE `entry` = 400301;
DELETE FROM `creature_template_model` WHERE `CreatureID` = 400301;
DELETE FROM `creature_template` WHERE `entry` = 400301;

INSERT INTO `creature_template` (`entry`, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`, `KillCredit1`, `KillCredit2`, `name`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`, `speed_walk`, `speed_run`, `speed_swim`, `speed_flight`, `detection_range`, `scale`, `rank`, `dmgschool`, `DamageModifier`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`, `unit_class`, `unit_flags`, `unit_flags2`, `dynamicflags`, `family`, `trainer_type`, `trainer_spell`, `trainer_class`, `trainer_race`, `type`, `type_flags`, `lootid`, `pickpocketloot`, `skinloot`, `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `HoverHeight`, `HealthModifier`, `ManaModifier`, `ArmorModifier`, `ExperienceModifier`, `RacialLeader`, `movementId`, `RegenHealth`, `mechanic_immune_mask`, `spell_school_immune_mask`, `flags_extra`, `ScriptName`, `VerifiedBuild`) VALUES
(400301, 0, 0, 0, 0, 0, 'Ogre-mage', NULL, NULL, 0, 80, 80, 0, 45, 0, 1, 1.14286, 1, 1, 18, 1, 0, 0, 4.0, 2000, 2000, 1, 1, 2, 0, 2048, 0, 0, 0, 0, 0, 0, 7, 0, 5237, 5237, 0, 0, 0, 60, 374, 'SmartAI', 1, 1, 1.2, 2.0, 1, 1, 0, 0, 1, 0, 0, 0, 'ConquestGuard', 12340);

INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES
(400301, 0, 11558, 1, 1, 12340);

INSERT INTO `creature_template_addon` (`entry`, `path_id`, `mount`, `bytes1`, `bytes2`, `emote`, `visibilityDistanceType`, `auras`) VALUES
(400301, 0, 0, 0, 1, 0, 0, NULL);

-- Copy spells from entry 5237 (Gordunni Ogre Mage)
INSERT INTO `creature_template_spell` (`CreatureID`, `Index`, `Spell`, `VerifiedBuild`) VALUES
(400301, 0, 8401, 12340),
(400301, 1, 7322, 12340);

-- ============================================
-- Ecraseur ogre (entry 400302)
-- Based on entry 21046 (Boulder'mok Brute)
-- HP: 100000, Level: 80
-- ============================================
DELETE FROM `creature_template_spell` WHERE `CreatureID` = 400302;
DELETE FROM `creature_template_addon` WHERE `entry` = 400302;
DELETE FROM `creature_template_model` WHERE `CreatureID` = 400302;
DELETE FROM `creature_template` WHERE `entry` = 400302;

INSERT INTO `creature_template` (`entry`, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`, `KillCredit1`, `KillCredit2`, `name`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`, `speed_walk`, `speed_run`, `speed_swim`, `speed_flight`, `detection_range`, `scale`, `rank`, `dmgschool`, `DamageModifier`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`, `unit_class`, `unit_flags`, `unit_flags2`, `dynamicflags`, `family`, `trainer_type`, `trainer_spell`, `trainer_class`, `trainer_race`, `type`, `type_flags`, `lootid`, `pickpocketloot`, `skinloot`, `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `HoverHeight`, `HealthModifier`, `ManaModifier`, `ArmorModifier`, `ExperienceModifier`, `RacialLeader`, `movementId`, `RegenHealth`, `mechanic_immune_mask`, `spell_school_immune_mask`, `flags_extra`, `ScriptName`, `VerifiedBuild`) VALUES
(400302, 0, 0, 0, 0, 0, 'Ecraseur ogre', '', NULL, 0, 80, 80, 1, 1780, 0, 1, 1.14286, 1, 1, 20, 1, 0, 0, 7.0, 2000, 2000, 1, 1, 1, 32768, 2048, 0, 0, 0, 0, 0, 0, 7, 0, 21046, 21046, 0, 0, 0, 194, 259, 'SmartAI', 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0, 'ConquestGuard', 12340);

INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES
(400302, 0, 20017, 1, 1, 12340);

INSERT INTO `creature_template_addon` (`entry`, `path_id`, `mount`, `bytes1`, `bytes2`, `emote`, `visibilityDistanceType`, `auras`) VALUES
(400302, 0, 0, 0, 1, 0, 0, NULL);

-- Copy spells from entry 21046 (Boulder'mok Brute)
INSERT INTO `creature_template_spell` (`CreatureID`, `Index`, `Spell`, `VerifiedBuild`) VALUES
(400302, 0, 37577, 12340),
(400302, 1, 8599, 12340);

-- ============================================
-- Chaman ogre (entry 400303)
-- Based on entry 21047 (Boulder'mok Shaman)
-- HP: 100000, Level: 80
-- ============================================
DELETE FROM `creature_template_spell` WHERE `CreatureID` = 400303;
DELETE FROM `creature_template_addon` WHERE `entry` = 400303;
DELETE FROM `creature_template_model` WHERE `CreatureID` = 400303;
DELETE FROM `creature_template` WHERE `entry` = 400303;

INSERT INTO `creature_template` (`entry`, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`, `KillCredit1`, `KillCredit2`, `name`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`, `speed_walk`, `speed_run`, `speed_swim`, `speed_flight`, `detection_range`, `scale`, `rank`, `dmgschool`, `DamageModifier`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`, `unit_class`, `unit_flags`, `unit_flags2`, `dynamicflags`, `family`, `trainer_type`, `trainer_spell`, `trainer_class`, `trainer_race`, `type`, `type_flags`, `lootid`, `pickpocketloot`, `skinloot`, `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `HoverHeight`, `HealthModifier`, `ManaModifier`, `ArmorModifier`, `ExperienceModifier`, `RacialLeader`, `movementId`, `RegenHealth`, `mechanic_immune_mask`, `spell_school_immune_mask`, `flags_extra`, `ScriptName`, `VerifiedBuild`) VALUES
(400303, 0, 0, 0, 0, 0, 'Chaman ogre', '', NULL, 0, 80, 80, 1, 1780, 0, 1, 1.14286, 1, 1, 20, 1, 0, 0, 7.0, 2000, 2000, 1, 1, 2, 32768, 2048, 0, 0, 0, 0, 0, 0, 7, 0, 21047, 21047, 0, 0, 0, 167, 222, 'SmartAI', 1, 1, 1, 2.0, 1, 1, 0, 0, 1, 0, 0, 0, 'ConquestGuard', 12340);

INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES
(400303, 0, 20019, 1, 1, 12340);

INSERT INTO `creature_template_addon` (`entry`, `path_id`, `mount`, `bytes1`, `bytes2`, `emote`, `visibilityDistanceType`, `auras`) VALUES
(400303, 0, 0, 0, 1, 0, 0, NULL);

-- Copy spells from entry 21047 (Boulder'mok Shaman)
INSERT INTO `creature_template_spell` (`CreatureID`, `Index`, `Spell`, `VerifiedBuild`) VALUES
(400303, 0, 28902, 12340),
(400303, 1, 11986, 12340),
(400303, 2, 12550, 12340);

-- ============================================
-- Massacreur ogre (entry 400304)
-- Based on entry 21296 (Bladespire Champion)
-- HP: 200000, Level: 80, Rank: Elite (1)
-- ============================================
DELETE FROM `creature_template_spell` WHERE `CreatureID` = 400304;
DELETE FROM `creature_template_addon` WHERE `entry` = 400304;
DELETE FROM `creature_template_model` WHERE `CreatureID` = 400304;
DELETE FROM `creature_template` WHERE `entry` = 400304;

INSERT INTO `creature_template` (`entry`, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`, `KillCredit1`, `KillCredit2`, `name`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`, `speed_walk`, `speed_run`, `speed_swim`, `speed_flight`, `detection_range`, `scale`, `rank`, `dmgschool`, `DamageModifier`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`, `unit_class`, `unit_flags`, `unit_flags2`, `dynamicflags`, `family`, `trainer_type`, `trainer_spell`, `trainer_class`, `trainer_race`, `type`, `type_flags`, `lootid`, `pickpocketloot`, `skinloot`, `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `HoverHeight`, `HealthModifier`, `ManaModifier`, `ArmorModifier`, `ExperienceModifier`, `RacialLeader`, `movementId`, `RegenHealth`, `mechanic_immune_mask`, `spell_school_immune_mask`, `flags_extra`, `ScriptName`, `VerifiedBuild`) VALUES
(400304, 0, 0, 0, 19995, 0, 'Massacreur ogre', NULL, NULL, 0, 80, 80, 1, 1780, 0, 1.6, 1.14286, 1, 1, 20, 1, 1, 0, 12.0, 2000, 2000, 1, 1, 1, 32768, 2048, 0, 0, 0, 0, 0, 0, 7, 0, 21296, 21296, 0, 0, 0, 198, 263, 'SmartAI', 1, 1, 1.1, 1, 1, 1, 0, 51, 1, 0, 0, 0, 'ConquestGuard', 12340);

INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES
(400304, 0, 19756, 1, 1, 12340);

INSERT INTO `creature_template_addon` (`entry`, `path_id`, `mount`, `bytes1`, `bytes2`, `emote`, `visibilityDistanceType`, `auras`) VALUES
(400304, 0, 0, 0, 1, 0, 0, NULL);

-- Copy spells from entry 21296 (Bladespire Champion)
INSERT INTO `creature_template_spell` (`CreatureID`, `Index`, `Spell`, `VerifiedBuild`) VALUES
(400304, 0, 37777, 12340),
(400304, 1, 8078, 12340);

-- ============================================
-- Démoniste ogre (entry 400305)
-- Based on entry 11448 (Gordok Warlock)
-- HP: 200000, Level: 80, Rank: Elite (1)
-- ============================================
DELETE FROM `creature_template_spell` WHERE `CreatureID` = 400305;
DELETE FROM `creature_template_addon` WHERE `entry` = 400305;
DELETE FROM `creature_template_model` WHERE `CreatureID` = 400305;
DELETE FROM `creature_template` WHERE `entry` = 400305;

INSERT INTO `creature_template` (`entry`, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`, `KillCredit1`, `KillCredit2`, `name`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`, `speed_walk`, `speed_run`, `speed_swim`, `speed_flight`, `detection_range`, `scale`, `rank`, `dmgschool`, `DamageModifier`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`, `unit_class`, `unit_flags`, `unit_flags2`, `dynamicflags`, `family`, `trainer_type`, `trainer_spell`, `trainer_class`, `trainer_race`, `type`, `type_flags`, `lootid`, `pickpocketloot`, `skinloot`, `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `HoverHeight`, `HealthModifier`, `ManaModifier`, `ArmorModifier`, `ExperienceModifier`, `RacialLeader`, `movementId`, `RegenHealth`, `mechanic_immune_mask`, `spell_school_immune_mask`, `flags_extra`, `ScriptName`, `VerifiedBuild`) VALUES
(400305, 0, 0, 0, 0, 0, 'Démoniste ogre', NULL, NULL, 0, 80, 80, 0, 45, 0, 1, 1.14286, 1, 1, 20, 1, 1, 0, 12.0, 2000, 2000, 1, 1, 8, 0, 2048, 0, 0, 0, 0, 0, 0, 7, 0, 11448, 11441, 0, 0, 0, 565, 742, 'SmartAI', 1, 1, 5, 3, 1, 1, 0, 0, 1, 0, 0, 0, 'ConquestGuard', 12340);

INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES
(400305, 0, 14423, 1, 1, 12340);

INSERT INTO `creature_template_addon` (`entry`, `path_id`, `mount`, `bytes1`, `bytes2`, `emote`, `visibilityDistanceType`, `auras`) VALUES
(400305, 0, 0, 0, 1, 0, 0, NULL);

-- Copy spells from entry 11448 (Gordok Warlock) - spells are in smart_scripts, not creature_template_spell
-- These are the combat spells used by the original creature
INSERT INTO `creature_template_spell` (`CreatureID`, `Index`, `Spell`, `VerifiedBuild`) VALUES
(400305, 0, 12739, 12340),  -- Shadow Bolt
(400305, 1, 12742, 12340),  -- Immolate
(400305, 2, 8994, 12340),   -- Banish
(400305, 3, 13338, 12340);  -- Curse of Tongues

