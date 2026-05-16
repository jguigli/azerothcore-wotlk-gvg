-- Conquest Mounts - Siege Engines
-- This SQL creates siege engine NPCs based on existing entries
-- All entries are usable by both Alliance and Horde (faction 35) unless specified otherwise

-- ============================================
-- Baroudeur P-W8 (entry 400200)
-- Based on entry 35069 (Horde Siege Engine) but usable by both factions
-- Renamed from "Engin de siège rouge" to "Baroudeur P-W8"
-- Speed changed to 2
-- ============================================
DELETE FROM `creature_template_spell` WHERE `CreatureID` = 400200;
DELETE FROM `creature_template_addon` WHERE `entry` = 400200;
DELETE FROM `npc_spellclick_spells` WHERE `npc_entry` = 400200;
DELETE FROM `creature_template_model` WHERE `CreatureID` = 400200;
DELETE FROM `creature_template` WHERE `entry` = 400200;

INSERT INTO `creature_template` (`entry`, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`, `KillCredit1`, `KillCredit2`, `name`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`, `speed_walk`, `speed_run`, `speed_swim`, `speed_flight`, `detection_range`, `scale`, `rank`, `dmgschool`, `DamageModifier`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`, `unit_class`, `unit_flags`, `unit_flags2`, `dynamicflags`, `family`, `trainer_type`, `trainer_spell`, `trainer_class`, `trainer_race`, `type`, `type_flags`, `lootid`, `pickpocketloot`, `skinloot`, `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `HoverHeight`, `HealthModifier`, `ManaModifier`, `ArmorModifier`, `ExperienceModifier`, `RacialLeader`, `movementId`, `RegenHealth`, `mechanic_immune_mask`, `spell_school_immune_mask`, `flags_extra`, `ScriptName`, `VerifiedBuild`) VALUES
(400200, 0, 0, 0, 0, 0, 'Baroudeur P-W8', '', 'vehichleCursor', 0, 70, 70, 0, 35, 16777216, 2.0, 2.0, 1, 1, 20, 1, 0, 0, 1, 2000, 2000, 1, 1, 4, 16384, 2048, 0, 0, 0, 0, 0, 0, 9, 393256, 0, 0, 0, 0, 514, 0, 0, '', 0, 1, 61.7284, 1, 1, 1, 0, 113, 0, 344407930, 0, 0, 'ConquestSiegeEngine', 10314);

INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES
(400200, 0, 26403, 1, 1, 10314);

INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(400200, 46598, 1, 0),
(400200, 66245, 1, 0);

INSERT INTO `creature_template_addon` (`entry`, `path_id`, `mount`, `bytes1`, `bytes2`, `emote`, `visibilityDistanceType`, `auras`) VALUES
(400200, 0, 0, 0, 1, 0, 4, NULL);

INSERT INTO `creature_template_spell` (`CreatureID`, `Index`, `Spell`, `VerifiedBuild`) VALUES
(400200, 0, 67796, 10314),
(400200, 1, 67797, 10314);

-- ============================================
-- Destructeur B27 (entry 400201)
-- Based on entry 34776 (Alliance Siege Engine) but usable by both factions
-- Renamed from "Engin de siège bleu" to "Destructeur B27"
-- Two destruction turrets replace the two combat turrets
-- ============================================
DELETE FROM `creature_template_spell` WHERE `CreatureID` = 400201;
DELETE FROM `creature_template_addon` WHERE `entry` = 400201;
DELETE FROM `npc_spellclick_spells` WHERE `npc_entry` = 400201;
DELETE FROM `creature_template_model` WHERE `CreatureID` = 400201;
DELETE FROM `creature_template` WHERE `entry` = 400201;

INSERT INTO `creature_template` (`entry`, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`, `KillCredit1`, `KillCredit2`, `name`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`, `speed_walk`, `speed_run`, `speed_swim`, `speed_flight`, `detection_range`, `scale`, `rank`, `dmgschool`, `DamageModifier`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`, `unit_class`, `unit_flags`, `unit_flags2`, `dynamicflags`, `family`, `trainer_type`, `trainer_spell`, `trainer_class`, `trainer_race`, `type`, `type_flags`, `lootid`, `pickpocketloot`, `skinloot`, `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `HoverHeight`, `HealthModifier`, `ManaModifier`, `ArmorModifier`, `ExperienceModifier`, `RacialLeader`, `movementId`, `RegenHealth`, `mechanic_immune_mask`, `spell_school_immune_mask`, `flags_extra`, `ScriptName`, `VerifiedBuild`) VALUES
(400201, 0, 0, 0, 0, 0, 'Destructeur B27', '', 'vehichleCursor', 0, 70, 70, 0, 35, 16777216, 1.2, 1, 1, 1, 20, 1.5, 0, 0, 1, 2000, 2000, 1, 1, 4, 16384, 2048, 0, 0, 0, 0, 0, 0, 9, 393256, 0, 0, 0, 0, 435, 0, 0, '', 0, 1, 61.7284, 1, 1, 1, 0, 113, 0, 344407930, 0, 0, 'ConquestSiegeEngine', 10314);

INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES
(400201, 0, 25292, 1, 1, 10314);

INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(400201, 46598, 1, 0),
(400201, 66245, 1, 0);

INSERT INTO `creature_template_addon` (`entry`, `path_id`, `mount`, `bytes1`, `bytes2`, `emote`, `visibilityDistanceType`, `auras`) VALUES
(400201, 0, 0, 0, 1, 0, 4, NULL);

INSERT INTO `creature_template_spell` (`CreatureID`, `Index`, `Spell`, `VerifiedBuild`) VALUES
(400201, 0, 67796, 10314),
(400201, 1, 67797, 10314);

-- ============================================
-- Catapulte (entry 400202)
-- Based on entry 34793 (Catapult)
-- ============================================
DELETE FROM `creature_template_spell` WHERE `CreatureID` = 400202;
DELETE FROM `creature_template_addon` WHERE `entry` = 400202;
DELETE FROM `npc_spellclick_spells` WHERE `npc_entry` = 400202;
DELETE FROM `creature_template_model` WHERE `CreatureID` = 400202;
DELETE FROM `creature_template` WHERE `entry` = 400202;

INSERT INTO `creature_template` (`entry`, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`, `KillCredit1`, `KillCredit2`, `name`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`, `speed_walk`, `speed_run`, `speed_swim`, `speed_flight`, `detection_range`, `scale`, `rank`, `dmgschool`, `DamageModifier`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`, `unit_class`, `unit_flags`, `unit_flags2`, `dynamicflags`, `family`, `trainer_type`, `trainer_spell`, `trainer_class`, `trainer_race`, `type`, `type_flags`, `lootid`, `pickpocketloot`, `skinloot`, `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `HoverHeight`, `HealthModifier`, `ManaModifier`, `ArmorModifier`, `ExperienceModifier`, `RacialLeader`, `movementId`, `RegenHealth`, `mechanic_immune_mask`, `spell_school_immune_mask`, `flags_extra`, `ScriptName`, `VerifiedBuild`) VALUES
(400202, 0, 0, 0, 0, 0, 'Catapulte', '', 'vehichleCursor', 0, 70, 70, 0, 35, 16777216, 2.8, 2.42857, 1, 1, 20, 1, 0, 0, 1, 2000, 2000, 1, 1, 4, 16384, 2048, 0, 0, 0, 0, 0, 0, 9, 393256, 0, 0, 0, 0, 438, 0, 0, '', 0, 1, 3.7037, 1, 1, 1, 0, 207, 0, 344407930, 0, 0, 'ConquestSiegeEngine', 10314);

INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES
(400202, 0, 23884, 1, 1, 10314);

INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(400202, 66245, 1, 0);

INSERT INTO `creature_template_addon` (`entry`, `path_id`, `mount`, `bytes1`, `bytes2`, `emote`, `visibilityDistanceType`, `auras`) VALUES
(400202, 0, 0, 0, 1, 0, 4, NULL);

INSERT INTO `creature_template_spell` (`CreatureID`, `Index`, `Spell`, `VerifiedBuild`) VALUES
(400202, 0, 66218, 10314),
(400202, 1, 66296, 10314);

-- ============================================
-- Lanceur de glaive violet (entry 400203)
-- Based on entry 34802 (Alliance Glaive Thrower) but usable by both factions
-- ============================================
DELETE FROM `creature_template_spell` WHERE `CreatureID` = 400203;
DELETE FROM `creature_template_addon` WHERE `entry` = 400203;
DELETE FROM `npc_spellclick_spells` WHERE `npc_entry` = 400203;
DELETE FROM `creature_template_model` WHERE `CreatureID` = 400203;
DELETE FROM `creature_template` WHERE `entry` = 400203;

INSERT INTO `creature_template` (`entry`, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`, `KillCredit1`, `KillCredit2`, `name`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`, `speed_walk`, `speed_run`, `speed_swim`, `speed_flight`, `detection_range`, `scale`, `rank`, `dmgschool`, `DamageModifier`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`, `unit_class`, `unit_flags`, `unit_flags2`, `dynamicflags`, `family`, `trainer_type`, `trainer_spell`, `trainer_class`, `trainer_race`, `type`, `type_flags`, `lootid`, `pickpocketloot`, `skinloot`, `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `HoverHeight`, `HealthModifier`, `ManaModifier`, `ArmorModifier`, `ExperienceModifier`, `RacialLeader`, `movementId`, `RegenHealth`, `mechanic_immune_mask`, `spell_school_immune_mask`, `flags_extra`, `ScriptName`, `VerifiedBuild`) VALUES
(400203, 0, 0, 0, 0, 0, 'Lanceur de glaive violet', '', 'vehichleCursor', 0, 70, 70, 0, 35, 16777216, 3.2, 1.14286, 1, 1, 20, 1, 0, 0, 1, 2000, 2000, 1, 1, 1, 16384, 2048, 0, 0, 0, 0, 0, 0, 9, 131080, 0, 0, 0, 0, 447, 0, 0, '', 0, 1, 6.17284, 1, 1, 1, 0, 129, 0, 344407930, 0, 0, 'ConquestSiegeEngine', 10314);

INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES
(400203, 0, 29642, 1, 1, 10314);

INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(400203, 68503, 1, 0);

INSERT INTO `creature_template_addon` (`entry`, `path_id`, `mount`, `bytes1`, `bytes2`, `emote`, `visibilityDistanceType`, `auras`) VALUES
(400203, 0, 0, 0, 1, 0, 5, NULL);

INSERT INTO `creature_template_spell` (`CreatureID`, `Index`, `Spell`, `VerifiedBuild`) VALUES
(400203, 0, 66456, 10314),
(400203, 1, 67195, 10314);

-- ============================================
-- Lanceur de glaive orange (entry 400204)
-- Based on entry 35273 (Horde Glaive Thrower) but usable by both factions
-- ============================================
DELETE FROM `creature_template_spell` WHERE `CreatureID` = 400204;
DELETE FROM `creature_template_addon` WHERE `entry` = 400204;
DELETE FROM `npc_spellclick_spells` WHERE `npc_entry` = 400204;
DELETE FROM `creature_template_model` WHERE `CreatureID` = 400204;
DELETE FROM `creature_template` WHERE `entry` = 400204;

INSERT INTO `creature_template` (`entry`, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`, `KillCredit1`, `KillCredit2`, `name`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`, `speed_walk`, `speed_run`, `speed_swim`, `speed_flight`, `detection_range`, `scale`, `rank`, `dmgschool`, `DamageModifier`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`, `unit_class`, `unit_flags`, `unit_flags2`, `dynamicflags`, `family`, `trainer_type`, `trainer_spell`, `trainer_class`, `trainer_race`, `type`, `type_flags`, `lootid`, `pickpocketloot`, `skinloot`, `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `HoverHeight`, `HealthModifier`, `ManaModifier`, `ArmorModifier`, `ExperienceModifier`, `RacialLeader`, `movementId`, `RegenHealth`, `mechanic_immune_mask`, `spell_school_immune_mask`, `flags_extra`, `ScriptName`, `VerifiedBuild`) VALUES
(400204, 0, 0, 0, 0, 0, 'Lanceur de glaive orange', '', 'vehichleCursor', 0, 70, 70, 0, 35, 16777216, 3.2, 1.14286, 1, 1, 20, 1, 0, 0, 1, 2000, 2000, 1, 1, 1, 16384, 2048, 0, 0, 0, 0, 0, 0, 9, 131080, 0, 0, 0, 0, 447, 0, 0, '', 0, 1, 6.17284, 1, 1, 1, 0, 129, 0, 344407930, 0, 0, 'ConquestSiegeEngine', 10314);

INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES
(400204, 0, 29734, 1, 1, 10314);

INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(400204, 68503, 1, 0);

INSERT INTO `creature_template_addon` (`entry`, `path_id`, `mount`, `bytes1`, `bytes2`, `emote`, `visibilityDistanceType`, `auras`) VALUES
(400204, 0, 0, 0, 1, 0, 5, NULL);

INSERT INTO `creature_template_spell` (`CreatureID`, `Index`, `Spell`, `VerifiedBuild`) VALUES
(400204, 0, 67034, 10314),
(400204, 1, 67195, 10314);

-- ============================================
-- Demolisseur (entry 400205)
-- Based on entry 34775 (Demolisher)
-- Speed changed to 2
-- ============================================
DELETE FROM `creature_template_spell` WHERE `CreatureID` = 400205;
DELETE FROM `creature_template_addon` WHERE `entry` = 400205;
DELETE FROM `npc_spellclick_spells` WHERE `npc_entry` = 400205;
DELETE FROM `creature_template_model` WHERE `CreatureID` = 400205;
DELETE FROM `creature_template` WHERE `entry` = 400205;

INSERT INTO `creature_template` (`entry`, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`, `KillCredit1`, `KillCredit2`, `name`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`, `speed_walk`, `speed_run`, `speed_swim`, `speed_flight`, `detection_range`, `scale`, `rank`, `dmgschool`, `DamageModifier`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`, `unit_class`, `unit_flags`, `unit_flags2`, `dynamicflags`, `family`, `trainer_type`, `trainer_spell`, `trainer_class`, `trainer_race`, `type`, `type_flags`, `lootid`, `pickpocketloot`, `skinloot`, `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `HoverHeight`, `HealthModifier`, `ManaModifier`, `ArmorModifier`, `ExperienceModifier`, `RacialLeader`, `movementId`, `RegenHealth`, `mechanic_immune_mask`, `spell_school_immune_mask`, `flags_extra`, `ScriptName`, `VerifiedBuild`) VALUES
(400205, 0, 0, 0, 0, 0, 'Demolisseur', '', 'vehichleCursor', 0, 70, 70, 0, 35, 16777216, 2.0, 2.0, 1, 1, 20, 1, 0, 0, 1, 2000, 2000, 1, 1, 1, 16384, 2048, 0, 0, 0, 0, 0, 0, 9, 131080, 0, 0, 0, 0, 509, 0, 0, '', 0, 1, 17.284, 1, 1, 1, 0, 112, 0, 344407930, 0, 0, 'ConquestSiegeEngine', 10314);

INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES
(400205, 0, 27658, 1, 1, 10314);

INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(400205, 66245, 1, 0);

INSERT INTO `creature_template_addon` (`entry`, `path_id`, `mount`, `bytes1`, `bytes2`, `emote`, `visibilityDistanceType`, `auras`) VALUES
(400205, 0, 0, 0, 1, 0, 5, NULL);

INSERT INTO `creature_template_spell` (`CreatureID`, `Index`, `Spell`, `VerifiedBuild`) VALUES
(400205, 0, 67440, 10314),
(400205, 1, 67441, 10314);

-- ============================================
-- Tourelle de combat (entry 400206)
-- Version custom basée sur 34935 (Horde Gunship Cannon)
-- ============================================
DELETE FROM `creature_template_spell` WHERE `CreatureID` = 400206;
DELETE FROM `creature_template_addon` WHERE `entry` = 400206;
DELETE FROM `npc_spellclick_spells` WHERE `npc_entry` = 400206;
DELETE FROM `creature_template_model` WHERE `CreatureID` = 400206;
DELETE FROM `creature_template` WHERE `entry` = 400206;

INSERT INTO `creature_template` (`entry`, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`, `KillCredit1`, `KillCredit2`, `name`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`, `speed_walk`, `speed_run`, `speed_swim`, `speed_flight`, `detection_range`, `scale`, `rank`, `dmgschool`, `DamageModifier`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`, `unit_class`, `unit_flags`, `unit_flags2`, `dynamicflags`, `family`, `trainer_type`, `trainer_spell`, `trainer_class`, `trainer_race`, `type`, `type_flags`, `lootid`, `pickpocketloot`, `skinloot`, `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `HoverHeight`, `HealthModifier`, `ManaModifier`, `ArmorModifier`, `ExperienceModifier`, `RacialLeader`, `movementId`, `RegenHealth`, `mechanic_immune_mask`, `spell_school_immune_mask`, `flags_extra`, `ScriptName`, `VerifiedBuild`) VALUES
(400206, 0, 0, 0, 0, 0, 'Tourelle de combat', '', 'Gunner', 0, 70, 70, 0, 35, 16777216, 1.2, 1.14286, 1, 1, 20, 1, 0, 0, 1, 2000, 2000, 1, 1, 1, 2, 2048, 0, 0, 0, 0, 0, 0, 9, 262184, 0, 0, 0, 0, 436, 0, 0, '', 0, 1, 15.9116, 1, 1, 1, 0, 121, 0, 344407930, 0, 0, '', 10314);

INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES
(400206, 0, 29489, 1, 1, 10314);

INSERT INTO `creature_template_addon` (`entry`, `path_id`, `mount`, `bytes1`, `bytes2`, `emote`, `visibilityDistanceType`, `auras`) VALUES
(400206, 0, 0, 0, 1, 0, 4, NULL);

INSERT INTO `creature_template_spell` (`CreatureID`, `Index`, `Spell`, `VerifiedBuild`) VALUES
(400206, 0, 67452, 10314),
(400206, 1, 66541, 10314);

INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(400206, 67830, 1, 0);

-- ============================================
-- ============================================
-- ============================================
-- Tourelle de combat 2 (entry 400210)
-- Identique à 400206 (deuxième canon de combat)
-- Moved from 400209 to 400210 to make room for Pisteur M2
-- ============================================
DELETE FROM `npc_spellclick_spells` WHERE `npc_entry` = 400210;
DELETE FROM `creature_template_spell` WHERE `CreatureID` = 400210;
DELETE FROM `creature_template_addon` WHERE `entry` = 400210;
DELETE FROM `creature_template_model` WHERE `CreatureID` = 400210;
DELETE FROM `creature_template` WHERE `entry` = 400210;

INSERT INTO `creature_template` (`entry`, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`, `KillCredit1`, `KillCredit2`, `name`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`, `speed_walk`, `speed_run`, `speed_swim`, `speed_flight`, `detection_range`, `scale`, `rank`, `dmgschool`, `DamageModifier`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`, `unit_class`, `unit_flags`, `unit_flags2`, `dynamicflags`, `family`, `trainer_type`, `trainer_spell`, `trainer_class`, `trainer_race`, `type`, `type_flags`, `lootid`, `pickpocketloot`, `skinloot`, `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `HoverHeight`, `HealthModifier`, `ManaModifier`, `ArmorModifier`, `ExperienceModifier`, `RacialLeader`, `movementId`, `RegenHealth`, `mechanic_immune_mask`, `spell_school_immune_mask`, `flags_extra`, `ScriptName`, `VerifiedBuild`) VALUES
(400210, 0, 0, 0, 0, 0, 'Tourelle de combat', '', 'Gunner', 0, 70, 70, 0, 35, 16777216, 1.2, 1.14286, 1, 1, 20, 1, 0, 0, 1, 2000, 2000, 1, 1, 1, 2, 2048, 0, 0, 0, 0, 0, 0, 9, 262184, 0, 0, 0, 0, 436, 0, 0, '', 0, 1, 15.9116, 1, 1, 1, 0, 121, 0, 344407930, 0, 0, '', 10314);

INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES
(400210, 0, 29489, 1, 1, 10314);

INSERT INTO `creature_template_addon` (`entry`, `path_id`, `mount`, `bytes1`, `bytes2`, `emote`, `visibilityDistanceType`, `auras`) VALUES
(400210, 0, 0, 0, 1, 0, 4, NULL);

INSERT INTO `creature_template_spell` (`CreatureID`, `Index`, `Spell`, `VerifiedBuild`) VALUES
(400210, 0, 67452, 10314),
(400210, 1, 66541, 10314);

INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(400210, 67830, 1, 0);

-- ============================================
-- ============================================
-- ============================================
-- Pisteur M2 (entry 400209)
-- Based on entry 400201 (Destructeur B27) but:
-- - displayID = 26430
-- - speed = 3
-- - Remove two front turrets (seat_id 1 and 2)
-- - Keep only one combat turret in main position (seat_id 7, entry 400206)
-- ============================================
DELETE FROM `creature_template_spell` WHERE `CreatureID` = 400209;
DELETE FROM `creature_template_addon` WHERE `entry` = 400209;
DELETE FROM `npc_spellclick_spells` WHERE `npc_entry` = 400209;
DELETE FROM `creature_template_model` WHERE `CreatureID` = 400209;
DELETE FROM `creature_template` WHERE `entry` = 400209;

INSERT INTO `creature_template` (`entry`, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`, `KillCredit1`, `KillCredit2`, `name`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`, `speed_walk`, `speed_run`, `speed_swim`, `speed_flight`, `detection_range`, `scale`, `rank`, `dmgschool`, `DamageModifier`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`, `unit_class`, `unit_flags`, `unit_flags2`, `dynamicflags`, `family`, `trainer_type`, `trainer_spell`, `trainer_class`, `trainer_race`, `type`, `type_flags`, `lootid`, `pickpocketloot`, `skinloot`, `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `HoverHeight`, `HealthModifier`, `ManaModifier`, `ArmorModifier`, `ExperienceModifier`, `RacialLeader`, `movementId`, `RegenHealth`, `mechanic_immune_mask`, `spell_school_immune_mask`, `flags_extra`, `ScriptName`, `VerifiedBuild`) VALUES
(400209, 0, 0, 0, 0, 0, 'Pisteur M2', '', 'vehichleCursor', 0, 70, 70, 0, 35, 16777216, 3.0, 3.0, 1, 1, 20, 0.6, 0, 0, 1, 2000, 2000, 1, 1, 4, 16384, 2048, 0, 0, 0, 0, 0, 0, 9, 393256, 0, 0, 0, 0, 435, 0, 0, '', 0, 1, 61.7284, 1, 1, 1, 0, 113, 0, 344407930, 0, 0, 'ConquestSiegeEngine', 10314);

INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES
(400209, 0, 25292, 1, 1, 10314);

INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(400209, 46598, 1, 0),
(400209, 66245, 1, 0);

INSERT INTO `creature_template_addon` (`entry`, `path_id`, `mount`, `bytes1`, `bytes2`, `emote`, `visibilityDistanceType`, `auras`) VALUES
(400209, 0, 0, 0, 1, 0, 4, NULL);

INSERT INTO `creature_template_spell` (`CreatureID`, `Index`, `Spell`, `VerifiedBuild`) VALUES
(400209, 0, 67796, 10314),
(400209, 1, 67797, 10314);

-- ============================================
-- ============================================
-- Tourelle de destruction (entry 400207)
-- Clone direct de la grosse tourelle de l'engin 34776 (entry 34777)
-- Utilisée à la place 7 du Baroudeur P-W8 (400200)
-- ============================================
DELETE FROM `creature_template_spell`   WHERE `CreatureID` = 400207;
DELETE FROM `creature_template_addon`   WHERE `entry`       = 400207;
DELETE FROM `npc_spellclick_spells`     WHERE `npc_entry`   = 400207;
DELETE FROM `creature_template_model`   WHERE `CreatureID`  = 400207;
DELETE FROM `creature_template`         WHERE `entry`       = 400207;

INSERT INTO `creature_template` (
    `entry`, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`,
    `KillCredit1`, `KillCredit2`, `name`, `subname`, `IconName`, `gossip_menu_id`,
    `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`, `speed_walk`, `speed_run`,
    `speed_swim`, `speed_flight`, `detection_range`, `scale`, `rank`, `dmgschool`,
    `DamageModifier`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`,
    `unit_class`, `unit_flags`, `unit_flags2`, `dynamicflags`, `family`, `trainer_type`,
    `trainer_spell`, `trainer_class`, `trainer_race`, `type`, `type_flags`, `lootid`,
    `pickpocketloot`, `skinloot`, `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`,
    `AIName`, `MovementType`, `HoverHeight`, `HealthModifier`, `ManaModifier`,
    `ArmorModifier`, `ExperienceModifier`, `RacialLeader`, `movementId`, `RegenHealth`,
    `mechanic_immune_mask`, `spell_school_immune_mask`, `flags_extra`, `ScriptName`,
    `VerifiedBuild`
)
SELECT
    400207, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`,
    `KillCredit1`, `KillCredit2`, 'Tourelle de destruction', `subname`, `IconName`, `gossip_menu_id`,
    `minlevel`, `maxlevel`, `exp`, 35, `npcflag`, `speed_walk`, `speed_run`,
    `speed_swim`, `speed_flight`, `detection_range`, `scale`, `rank`, `dmgschool`,
    `DamageModifier`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`,
    `unit_class`, `unit_flags`, `unit_flags2`, `dynamicflags`, `family`, `trainer_type`,
    `trainer_spell`, `trainer_class`, `trainer_race`, `type`, `type_flags`, `lootid`,
    `pickpocketloot`, `skinloot`, `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`,
    `AIName`, `MovementType`, `HoverHeight`, `HealthModifier`, `ManaModifier`,
    `ArmorModifier`, `ExperienceModifier`, `RacialLeader`, `movementId`, `RegenHealth`,
    `mechanic_immune_mask`, `spell_school_immune_mask`, `flags_extra`, `ScriptName`,
    `VerifiedBuild`
FROM `creature_template` WHERE `entry` = 34777;

INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`)
SELECT 400207, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`
FROM `creature_template_model` WHERE `CreatureID` = 34777;

INSERT INTO `creature_template_addon` (`entry`, `path_id`, `mount`, `bytes1`, `bytes2`, `emote`, `visibilityDistanceType`, `auras`)
SELECT 400207, `path_id`, `mount`, `bytes1`, `bytes2`, `emote`, `visibilityDistanceType`, `auras`
FROM `creature_template_addon` WHERE `entry` = 34777;

INSERT INTO `creature_template_spell` (`CreatureID`, `Index`, `Spell`, `VerifiedBuild`)
SELECT 400207, `Index`, `Spell`, `VerifiedBuild`
FROM `creature_template_spell` WHERE `CreatureID` = 34777;

INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`)
SELECT 400207, `spell_id`, `cast_flags`, `user_type`
FROM `npc_spellclick_spells` WHERE `npc_entry` = 34777;

-- ============================================
-- ============================================
-- Tourelle de destruction (entry 400208)
-- Clone de 34777 (même comportement que la tourelle de siège bleue native)
-- ============================================
DELETE FROM `creature_template_spell`   WHERE `CreatureID` = 400208;
DELETE FROM `creature_template_addon`   WHERE `entry`       = 400208;
DELETE FROM `npc_spellclick_spells`     WHERE `npc_entry`   = 400208;
DELETE FROM `creature_template_model`   WHERE `CreatureID`  = 400208;
DELETE FROM `creature_template`         WHERE `entry`       = 400208;

INSERT INTO `creature_template` (
    `entry`, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`,
    `KillCredit1`, `KillCredit2`, `name`, `subname`, `IconName`, `gossip_menu_id`,
    `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`, `speed_walk`, `speed_run`,
    `speed_swim`, `speed_flight`, `detection_range`, `scale`, `rank`, `dmgschool`,
    `DamageModifier`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`,
    `unit_class`, `unit_flags`, `unit_flags2`, `dynamicflags`, `family`, `trainer_type`,
    `trainer_spell`, `trainer_class`, `trainer_race`, `type`, `type_flags`, `lootid`,
    `pickpocketloot`, `skinloot`, `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`,
    `AIName`, `MovementType`, `HoverHeight`, `HealthModifier`, `ManaModifier`,
    `ArmorModifier`, `ExperienceModifier`, `RacialLeader`, `movementId`, `RegenHealth`,
    `mechanic_immune_mask`, `spell_school_immune_mask`, `flags_extra`, `ScriptName`,
    `VerifiedBuild`
)
SELECT
    400208, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`,
    `KillCredit1`, `KillCredit2`, 'Tourelle de destruction', `subname`, `IconName`, `gossip_menu_id`,
    `minlevel`, `maxlevel`, `exp`, 35, `npcflag`, `speed_walk`, `speed_run`,
    `speed_swim`, `speed_flight`, `detection_range`, `scale`, `rank`, `dmgschool`,
    `DamageModifier`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`,
    `unit_class`, `unit_flags`, `unit_flags2`, `dynamicflags`, `family`, `trainer_type`,
    `trainer_spell`, `trainer_class`, `trainer_race`, `type`, `type_flags`, `lootid`,
    `pickpocketloot`, `skinloot`, `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`,
    `AIName`, `MovementType`, `HoverHeight`, `HealthModifier`, `ManaModifier`,
    `ArmorModifier`, `ExperienceModifier`, `RacialLeader`, `movementId`, `RegenHealth`,
    `mechanic_immune_mask`, `spell_school_immune_mask`, `flags_extra`, `ScriptName`,
    `VerifiedBuild`
FROM `creature_template` WHERE `entry` = 34777;

INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`)
SELECT 400208, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`
FROM `creature_template_model` WHERE `CreatureID` = 34777;

INSERT INTO `creature_template_addon` (`entry`, `path_id`, `mount`, `bytes1`, `bytes2`, `emote`, `visibilityDistanceType`, `auras`)
SELECT 400208, `path_id`, `mount`, `bytes1`, `bytes2`, `emote`, `visibilityDistanceType`, `auras`
FROM `creature_template_addon` WHERE `entry` = 34777;

INSERT INTO `creature_template_spell` (`CreatureID`, `Index`, `Spell`, `VerifiedBuild`)
SELECT 400208, `Index`, `Spell`, `VerifiedBuild`
FROM `creature_template_spell` WHERE `CreatureID` = 34777;

INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`)
SELECT 400208, `spell_id`, `cast_flags`, `user_type`
FROM `npc_spellclick_spells` WHERE `npc_entry` = 34777;

-- ============================================
-- ============================================
-- Canon massif (entry 400211)
-- Clone de 400208 mais avec displayID 29488 et scale 2 (appliqué via C++)
-- Utilisée à la place 7 du Destructeur B27 (400201)
-- ============================================
DELETE FROM `creature_template_spell`   WHERE `CreatureID` = 400211;
DELETE FROM `creature_template_addon`   WHERE `entry`       = 400211;
DELETE FROM `npc_spellclick_spells`     WHERE `npc_entry`   = 400211;
DELETE FROM `creature_template_model`   WHERE `CreatureID`  = 400211;
DELETE FROM `creature_template`         WHERE `entry`       = 400211;

INSERT INTO `creature_template` (
    `entry`, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`,
    `KillCredit1`, `KillCredit2`, `name`, `subname`, `IconName`, `gossip_menu_id`,
    `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`, `speed_walk`, `speed_run`,
    `speed_swim`, `speed_flight`, `detection_range`, `scale`, `rank`, `dmgschool`,
    `DamageModifier`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`,
    `unit_class`, `unit_flags`, `unit_flags2`, `dynamicflags`, `family`, `trainer_type`,
    `trainer_spell`, `trainer_class`, `trainer_race`, `type`, `type_flags`, `lootid`,
    `pickpocketloot`, `skinloot`, `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`,
    `AIName`, `MovementType`, `HoverHeight`, `HealthModifier`, `ManaModifier`,
    `ArmorModifier`, `ExperienceModifier`, `RacialLeader`, `movementId`, `RegenHealth`,
    `mechanic_immune_mask`, `spell_school_immune_mask`, `flags_extra`, `ScriptName`,
    `VerifiedBuild`
)
SELECT
    400211, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`,
    `KillCredit1`, `KillCredit2`, 'Canon massif', `subname`, `IconName`, `gossip_menu_id`,
    `minlevel`, `maxlevel`, `exp`, 35, `npcflag`, `speed_walk`, `speed_run`,
    `speed_swim`, `speed_flight`, `detection_range`, `scale`, `rank`, `dmgschool`,
    `DamageModifier`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`,
    `unit_class`, `unit_flags`, `unit_flags2`, `dynamicflags`, `family`, `trainer_type`,
    `trainer_spell`, `trainer_class`, `trainer_race`, `type`, `type_flags`, `lootid`,
    `pickpocketloot`, `skinloot`, `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`,
    `AIName`, `MovementType`, `HoverHeight`, `HealthModifier`, `ManaModifier`,
    `ArmorModifier`, `ExperienceModifier`, `RacialLeader`, `movementId`, `RegenHealth`,
    `mechanic_immune_mask`, `spell_school_immune_mask`, `flags_extra`, `ScriptName`,
    `VerifiedBuild`
FROM `creature_template` WHERE `entry` = 400208;

INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES
(400211, 0, 29488, 1, 1, 10314);

INSERT INTO `creature_template_addon` (`entry`, `path_id`, `mount`, `bytes1`, `bytes2`, `emote`, `visibilityDistanceType`, `auras`)
SELECT 400211, `path_id`, `mount`, `bytes1`, `bytes2`, `emote`, `visibilityDistanceType`, `auras`
FROM `creature_template_addon` WHERE `entry` = 400208;

INSERT INTO `creature_template_spell` (`CreatureID`, `Index`, `Spell`, `VerifiedBuild`)
SELECT 400211, `Index`, `Spell`, `VerifiedBuild`
FROM `creature_template_spell` WHERE `CreatureID` = 400208;

INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`)
SELECT 400211, `spell_id`, `cast_flags`, `user_type`
FROM `npc_spellclick_spells` WHERE `npc_entry` = 400208;

-- ============================================
-- Vehicle Template Accessories
-- Defines which turrets are attached to which siege engines
-- ============================================

-- Destructeur B27 (400201) - turrets
-- Note: Vehicle 435 has seats 0, 1, 2, and 7 (not 3) - same as siege engine 34776
-- Using summontimer 30000 like siege engine 34776 to prevent despawn
-- Two destruction turrets (400208) replace the two combat turrets
-- Main turret (seat 7) uses 400211 with displayID 29488 (scale 2 applied via C++ script)
-- minion = 0 to prevent auto-unsummon of accessories
DELETE FROM `vehicle_template_accessory` WHERE `entry` = 400201;
INSERT INTO `vehicle_template_accessory` (`entry`, `accessory_entry`, `seat_id`, `minion`, `description`, `summontype`, `summontimer`) VALUES
(400201, 400208, 1, 0, 'Destructeur B27 - tourelle de destruction 1', 6, 30000),
(400201, 400208, 2, 0, 'Destructeur B27 - tourelle de destruction 2', 6, 30000),
(400201, 400211, 7, 0, 'Destructeur B27 - tourelle de destruction principale (displayID 29488, scale 2 via C++)', 6, 30000);

-- Baroudeur P-W8 (400200) - turrets
-- Note: Vehicle 514 has seats 0, 1, 2, and 7 (not 3) - same as siege engine 34776
-- Using summontimer 30000 like siege engine 34776 to prevent despawn
-- minion = 0 to prevent auto-unsummon of accessories
DELETE FROM `vehicle_template_accessory` WHERE `entry` = 400200;
INSERT INTO `vehicle_template_accessory` (`entry`, `accessory_entry`, `seat_id`, `minion`, `description`, `summontype`, `summontimer`) VALUES
(400200, 400206, 2, 0, 'Baroudeur P-W8 - combat turret 1 (front left)', 6, 30000),
(400200, 400210, 1, 0, 'Baroudeur P-W8 - combat turret 2 (rear)', 6, 30000),
(400200, 400207, 7, 0, 'Baroudeur P-W8 - destruction turret (main position)', 6, 30000);

-- Pisteur M2 (400209) - turrets
-- Only one combat turret in main position (seat_id 7)
-- No front turrets (seat_id 1 and 2 removed)
-- minion = 0 to prevent auto-unsummon of accessories
DELETE FROM `vehicle_template_accessory` WHERE `entry` = 400209;
INSERT INTO `vehicle_template_accessory` (`entry`, `accessory_entry`, `seat_id`, `minion`, `description`, `summontype`, `summontimer`) VALUES
(400209, 400206, 7, 0, 'Pisteur M2 - tourelle de combat principale', 6, 30000);

