-- Conquest Mounts - Mount Creature Template
-- This SQL creates a mountable NPC creature based on entry 33782 (Argent Warhorse)
-- with a speed modifier of 2.5

-- Insert the mount creature template
-- Based on entry 33782 (Argent Warhorse) configuration
-- Key properties: VehicleId = 349, npcflag = 16777216 (mountable), type = 10, displayId = 28918
-- Speed modifier: 2.5 (speed_run = 2.5)
DELETE FROM `creature_template` WHERE `entry` = 400000;
DELETE FROM `creature_template_model` WHERE `CreatureID` = 400000;
DELETE FROM `npc_spellclick_spells` WHERE `npc_entry` = 400000;

INSERT INTO `creature_template` (`entry`, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`, `KillCredit1`, `KillCredit2`, `name`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`, `speed_walk`, `speed_run`, `speed_swim`, `speed_flight`, `detection_range`, `scale`, `rank`, `dmgschool`, `DamageModifier`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`, `unit_class`, `unit_flags`, `unit_flags2`, `dynamicflags`, `family`, `trainer_type`, `trainer_spell`, `trainer_class`, `trainer_race`, `type`, `type_flags`, `lootid`, `pickpocketloot`, `skinloot`, `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `HoverHeight`, `HealthModifier`, `ManaModifier`, `ArmorModifier`, `ExperienceModifier`, `RacialLeader`, `movementId`, `RegenHealth`, `mechanic_immune_mask`, `spell_school_immune_mask`, `flags_extra`, `ScriptName`, `VerifiedBuild`) VALUES
(400000, 0, 0, 0, 0, 0, 'Cheval de guerre', '', 'vehichleCursor', 0, 80, 80, 2, 35, 16777216, 1, 2.5, 1, 1, 18, 1, 0, 0, 1, 2000, 2000, 1, 1, 1, 0, 2048, 0, 0, 0, 0, 0, 0, 10, 0, 0, 0, 0, 0, 349, 0, 0, '', 0, 1, 3.96825, 1, 1, 1, 0, 157, 0, 0, 0, 0, 'ConquestMountsCreature', 12340);

-- Insert the creature display model
INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES
(400000, 0, 14582, 1, 1, 12340);

-- Insert the spell click entry (required for mounting)
-- Same spell as entry 33782 (Argent Warhorse)
INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(400000, 63151, 1, 0);

-- Explanation of important fields:
-- entry: 400000 - The creature entry ID used in the script
-- name: 'Monture' - French name meaning "Mount"
-- VehicleId: 349 - Same as entry 33782, allows the creature to be mounted
-- npcflag: 16777216 - UNIT_NPC_FLAG_SPELLCLICK (0x01000000) - allows players to click and mount this creature (will be set automatically by VehicleKit)
-- type: 10 - Creature type that can be mounted
-- displayId: 14582
-- speed_run: 2.5 - Speed modifier (2.5x base speed)
-- ScriptName: 'ConquestMountsCreature' - Links the creature to its script


-- Loup de guerre
DELETE FROM `creature_template` WHERE `entry` = 400001;
DELETE FROM `creature_template_model` WHERE `CreatureID` = 400001;
DELETE FROM `npc_spellclick_spells` WHERE `npc_entry` = 400001;

INSERT INTO `creature_template` (`entry`, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`, `KillCredit1`, `KillCredit2`, `name`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`, `speed_walk`, `speed_run`, `speed_swim`, `speed_flight`, `detection_range`, `scale`, `rank`, `dmgschool`, `DamageModifier`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`, `unit_class`, `unit_flags`, `unit_flags2`, `dynamicflags`, `family`, `trainer_type`, `trainer_spell`, `trainer_class`, `trainer_race`, `type`, `type_flags`, `lootid`, `pickpocketloot`, `skinloot`, `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `HoverHeight`, `HealthModifier`, `ManaModifier`, `ArmorModifier`, `ExperienceModifier`, `RacialLeader`, `movementId`, `RegenHealth`, `mechanic_immune_mask`, `spell_school_immune_mask`, `flags_extra`, `ScriptName`, `VerifiedBuild`) VALUES
(400001, 0, 0, 0, 0, 0, 'Loup de guerre', '', 'vehichleCursor', 0, 80, 80, 2, 35, 16777216, 1, 2.5, 1, 1, 18, 1, 0, 0, 1, 2000, 2000, 1, 1, 1, 0, 2048, 0, 0, 0, 0, 0, 0, 10, 0, 0, 0, 0, 0, 349, 0, 0, '', 0, 1, 3.96825, 1, 1, 1, 0, 157, 0, 0, 0, 0, 'ConquestMountsCreature', 12340);

INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES
(400001, 0, 14334, 1, 1, 12340);

INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(400001, 63151, 1, 0);

-- Dechiqueteur gobelin
DELETE FROM `creature_template` WHERE `entry` = 400002;
DELETE FROM `creature_template_model` WHERE `CreatureID` = 400002;
DELETE FROM `npc_spellclick_spells` WHERE `npc_entry` = 400002;

INSERT INTO `creature_template` (`entry`, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`, `KillCredit1`, `KillCredit2`, `name`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`, `speed_walk`, `speed_run`, `speed_swim`, `speed_flight`, `detection_range`, `scale`, `rank`, `dmgschool`, `DamageModifier`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`, `unit_class`, `unit_flags`, `unit_flags2`, `dynamicflags`, `family`, `trainer_type`, `trainer_spell`, `trainer_class`, `trainer_race`, `type`, `type_flags`, `lootid`, `pickpocketloot`, `skinloot`, `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `HoverHeight`, `HealthModifier`, `ManaModifier`, `ArmorModifier`, `ExperienceModifier`, `RacialLeader`, `movementId`, `RegenHealth`, `mechanic_immune_mask`, `spell_school_immune_mask`, `flags_extra`, `ScriptName`, `VerifiedBuild`) VALUES
(400002, 0, 0, 0, 0, 0, 'Déchiqueteur gobelin', '', 'vehichleCursor', 0, 80, 80, 2, 35, 16777216, 1, 2.5, 1, 1, 18, 1, 0, 0, 1, 2000, 2000, 1, 1, 1, 0, 2048, 0, 0, 0, 0, 0, 0, 10, 0, 0, 0, 0, 0, 349, 0, 0, '', 0, 1, 3.96825, 1, 1, 1, 0, 157, 0, 0, 0, 0, 'ConquestMountsCreature', 12340);

INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES
(400002, 0, 26612, 1, 1, 12340);

INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(400002, 63151, 1, 0);

-- Mecano-tank
DELETE FROM `creature_template` WHERE `entry` = 400003;
DELETE FROM `creature_template_model` WHERE `CreatureID` = 400003;
DELETE FROM `npc_spellclick_spells` WHERE `npc_entry` = 400003;

INSERT INTO `creature_template` (`entry`, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`, `KillCredit1`, `KillCredit2`, `name`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`, `speed_walk`, `speed_run`, `speed_swim`, `speed_flight`, `detection_range`, `scale`, `rank`, `dmgschool`, `DamageModifier`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`, `unit_class`, `unit_flags`, `unit_flags2`, `dynamicflags`, `family`, `trainer_type`, `trainer_spell`, `trainer_class`, `trainer_race`, `type`, `type_flags`, `lootid`, `pickpocketloot`, `skinloot`, `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `HoverHeight`, `HealthModifier`, `ManaModifier`, `ArmorModifier`, `ExperienceModifier`, `RacialLeader`, `movementId`, `RegenHealth`, `mechanic_immune_mask`, `spell_school_immune_mask`, `flags_extra`, `ScriptName`, `VerifiedBuild`) VALUES
(400003, 0, 0, 0, 0, 0, 'Mécano-tank', '', 'vehichleCursor', 0, 80, 80, 2, 35, 16777216, 1, 2.5, 1, 1, 18, 1, 0, 0, 1, 2000, 2000, 1, 1, 1, 0, 2048, 0, 0, 0, 0, 0, 0, 10, 0, 0, 0, 0, 0, 349, 0, 0, '', 0, 1, 3.96825, 1, 1, 1, 0, 157, 0, 0, 0, 0, 'ConquestMountsCreature', 12340);

INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES
(400003, 0, 31664, 1, 1, 12340);

INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(400003, 63151, 1, 0);