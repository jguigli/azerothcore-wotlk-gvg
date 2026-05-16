-- Conquest Recruitment Kit System
-- This SQL creates the recruitment kit item

-- ============================================
-- Kit de recrutement (item entry 80041)
-- ============================================
DELETE FROM `item_template` WHERE `entry` = 80041;

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
    80041,                                          -- entry
    15,                                             -- class (Miscellaneous)
    0,                                              -- subclass
    'Kit de recrutement',                          -- name
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
    0,                                              -- maxcount (0 = unlimited)
    1,                                              -- stackable
    70000,                                              -- spellid_1
    0,                                              -- spelltrigger_1
    0,                                              -- spellcategory_1
    0,                                              -- spellcategorycooldown_1
    0,                                              -- bonding (No binding)
    'Permet de recruter jusqu''à 3 PNJs de votre guilde pour vous suivre.', -- description
    'ConquestRecruitmentKit'                            -- ScriptName
);

