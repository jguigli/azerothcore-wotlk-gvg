-- Conquest Marker Spawner System
-- This SQL creates the marker item, marker NPC, and spawner NPC

-- ============================================
-- Kit de marqueur de spawn (item entry 80040)
-- ============================================
DELETE FROM `item_template` WHERE `entry` = 80040;

INSERT INTO `item_template` (
    `entry`, 
    `class`, 
    `subclass`, 
    `name`, 
    `displayid`, 
    `Quality`, 
    `Flags`, 
    `BuyCount`, 
    `BuyPrice`, 
    `SellPrice`, 
    `InventoryType`, 
    `AllowableClass`, 
    `AllowableRace`, 
    `ItemLevel`, 
    `RequiredLevel`, 
    `maxcount`, 
    `stackable`, 
    `spellid_1`, 
    `spelltrigger_1`, 
    `spellcategory_1`, 
    `spellcategorycooldown_1`, 
    `bonding`, 
    `description`, 
    `ScriptName`
) VALUES (
    80040,                                          -- entry
    15,                                             -- class (Miscellaneous)
    0,                                              -- subclass
    'Kit de marqueur de spawn',                    -- name
    23716,                                          -- displayid (même que les autres kits)
    2,                                              -- Quality (Uncommon - vert)
    0,                                              -- Flags
    1,                                              -- BuyCount
    0,                                              -- BuyPrice
    0,                                              -- SellPrice
    0,                                              -- InventoryType (Non-equipable)
    -1,                                             -- AllowableClass (all classes)
    -1,                                             -- AllowableRace (all races)
    80,                                             -- ItemLevel
    80,                                             -- RequiredLevel
    1,                                              -- maxcount (1 par joueur)
    1,                                              -- stackable (1 par joueur)
    70246,                                          -- spellid_1 (dummy spell, le script gère le spawn)
    0,                                              -- spelltrigger_1 (0 = On Use)
    0,                                              -- spellcategory_1
    0,                                              -- spellcategorycooldown_1
    0,                                              -- bonding (0 = No binding)
    'Faire apparaitre un Marqueur de spawn.',      -- description
    'ConquestMarkerItem'                                 -- ScriptName
);

-- ============================================
-- Marqueur de spawn (NPC entry 400103)
-- ============================================
DELETE FROM `creature_template_spell` WHERE `CreatureID` = 400103;
DELETE FROM `creature_template_addon` WHERE `entry` = 400103;
DELETE FROM `creature_template_model` WHERE `CreatureID` = 400103;
DELETE FROM `creature_template` WHERE `entry` = 400103;

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
    `mechanic_immune_mask`, `spell_school_immune_mask`, `flags_extra`, `ScriptName`, `VerifiedBuild`
) VALUES (
    400103, 0, 0, 0, 0, 0, 'Marqueur de spawn', NULL, NULL, 0, 
    80, 80, 0, 35, 0, 1, 1.14286, 1, 1, 20, 1, 0, 0, 
    1.0, 2000, 2000, 1, 1, 1, 33554688, 2048, 0, 0, 0, 
    0, 0, 0, 10, 0, 0, 0, 0, 0, 0, 0, 0, 
    '', 0, 1, 1, 1, 1, 1, 0, 0, 1, 
    0, 0, 128, '', 12340
);

INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES
(400103, 0, 20577, 1, 1, 12340);

INSERT INTO `creature_template_addon` (`entry`, `path_id`, `mount`, `bytes1`, `bytes2`, `emote`, `visibilityDistanceType`, `auras`) VALUES
(400103, 0, 0, 0, 1, 0, 0, NULL);

-- ============================================
-- Spawner (NPC entry 400102)
-- ============================================
DELETE FROM `creature_template_spell` WHERE `CreatureID` = 400102;
DELETE FROM `creature_template_addon` WHERE `entry` = 400102;
DELETE FROM `creature_template_model` WHERE `CreatureID` = 400102;
DELETE FROM `creature_template` WHERE `entry` = 400102;

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
    `mechanic_immune_mask`, `spell_school_immune_mask`, `flags_extra`, `ScriptName`, `VerifiedBuild`
) VALUES (
    400102, 0, 0, 0, 0, 0, 'Spawner', NULL, NULL, 0, 
    80, 80, 0, 35, 0, 1, 1.14286, 1, 1, 20, 1, 0, 0, 
    1.0, 2000, 2000, 1, 1, 1, 33554688, 2048, 0, 0, 0, 
    0, 0, 0, 10, 0, 0, 0, 0, 0, 0, 0, 0, 
    '', 0, 1, 1, 1, 1, 1, 0, 0, 1, 
    0, 0, 128, 'ConquestSpawner', 12340
);

INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES
(400102, 0, 7109, 1, 1, 12340);

INSERT INTO `creature_template_addon` (`entry`, `path_id`, `mount`, `bytes1`, `bytes2`, `emote`, `visibilityDistanceType`, `auras`) VALUES
(400102, 0, 0, 0, 1, 0, 0, NULL);

