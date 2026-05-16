-- Conquest Gear Manager Module - NPC Configuration
-- NPC entry 400100 - Gear Manager NPC

DELETE FROM `creature_template_model` WHERE `CreatureID` = 400100;
DELETE FROM `creature_template` WHERE `entry` = 400100;

INSERT INTO `creature_template` (
    `entry`,
    `difficulty_entry_1`,
    `difficulty_entry_2`,
    `difficulty_entry_3`,
    `KillCredit1`,
    `KillCredit2`,
    `name`,
    `subname`,
    `IconName`,
    `gossip_menu_id`,
    `minlevel`,
    `maxlevel`,
    `exp`,
    `faction`,
    `npcflag`,
    `speed_walk`,
    `speed_run`,
    `speed_swim`,
    `speed_flight`,
    `detection_range`,
    `rank`,
    `dmgschool`,
    `DamageModifier`,
    `BaseAttackTime`,
    `RangeAttackTime`,
    `BaseVariance`,
    `RangeVariance`,
    `unit_class`,
    `unit_flags`,
    `unit_flags2`,
    `dynamicflags`,
    `family`,
    `type`,
    `type_flags`,
    `lootid`,
    `pickpocketloot`,
    `skinloot`,
    `PetSpellDataId`,
    `VehicleId`,
    `mingold`,
    `maxgold`,
    `AIName`,
    `MovementType`,
    `HoverHeight`,
    `HealthModifier`,
    `ManaModifier`,
    `ArmorModifier`,
    `ExperienceModifier`,
    `RacialLeader`,
    `movementId`,
    `RegenHealth`,
    `flags_extra`,
    `ScriptName`
) VALUES (
    400100,  -- entry
    0,       -- difficulty_entry_1
    0,       -- difficulty_entry_2
    0,       -- difficulty_entry_3
    0,       -- KillCredit1
    0,       -- KillCredit2
    'Freeze Corleone',  -- name
    'Starter pack',         -- subname
    '',      -- IconName
    0,       -- gossip_menu_id
    80,      -- minlevel
    80,      -- maxlevel
    0,       -- exp
    35,      -- faction (friendly)
    1,       -- npcflag (GOSSIP)
    1,       -- speed_walk
    1.14286, -- speed_run
    1,       -- speed_swim
    1,       -- speed_flight
    20,      -- detection_range
    0,       -- rank
    0,       -- dmgschool
    1,       -- DamageModifier
    2000,    -- BaseAttackTime
    2000,    -- RangeAttackTime
    1,       -- BaseVariance
    1,       -- RangeVariance
    1,       -- unit_class
    0,       -- unit_flags
    0,       -- unit_flags2
    0,       -- dynamicflags
    0,       -- family
    7,       -- type (Humanoid)
    0,       -- type_flags
    0,       -- lootid
    0,       -- pickpocketloot
    0,       -- skinloot
    0,       -- PetSpellDataId
    0,       -- VehicleId
    0,       -- mingold
    0,       -- maxgold
    '',      -- AIName
    0,       -- MovementType
    1,       -- HoverHeight
    1,       -- HealthModifier
    1,       -- ManaModifier
    1,       -- ArmorModifier
    1,       -- ExperienceModifier
    0,       -- RacialLeader
    0,       -- movementId
    1,       -- RegenHealth
    0,       -- flags_extra
    'ConquestGearManagerNPC'  -- ScriptName
);

-- Insert the creature display model (displayid 22545)
INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES
(400100, 0, 22545, 1, 1, 12340);

-- Vérification
SELECT CONCAT('NPC 400100 (Gestionnaire d''Équipement) créé avec succès.') AS Result;

