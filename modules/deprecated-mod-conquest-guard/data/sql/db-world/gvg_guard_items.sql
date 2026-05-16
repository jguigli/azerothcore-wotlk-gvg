-- Conquest Guard - Spawn Items
-- This SQL creates items that spawn ogre guard NPCs when used
-- All items are single-use (maxcount = 1) and use icon from entry 23716

-- ============================================
-- Brute ogre spawn item (entry 80030)
-- ============================================
DELETE FROM `item_template` WHERE `entry` = 80030;

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
    80030,                                          -- entry
    15,                                             -- class (Miscellaneous)
    0,                                              -- subclass
    'Brute ogre',                                   -- name
    23716,                                          -- displayid
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
    5,                                              -- maxcount (5 par joueur)
    5,                                              -- stackable (5 par joueur)
    70246,                                          -- spellid_1 (Spawn Ogre Guard)
    0,                                              -- spelltrigger_1 (0 = On Use)
    0,                                              -- spellcategory_1
    0,                                              -- spellcategorycooldown_1
    0,                                              -- bonding (0 = No binding)
    'Faire apparaitre un Brute ogre.',             -- description
    'ConquestGuardItem'                                  -- ScriptName
);

-- ============================================
-- Ogre-mage spawn item (entry 80031)
-- ============================================
DELETE FROM `item_template` WHERE `entry` = 80031;

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
    80031,                                          -- entry
    15,                                             -- class (Miscellaneous)
    0,                                              -- subclass
    'Ogre-mage',                                    -- name
    23716,                                          -- displayid
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
    5,                                              -- maxcount (5 par joueur)
    5,                                             -- stackable (5 par joueur)
    70246,                                          -- spellid_1 (Spawn Ogre Guard)
    0,                                              -- spelltrigger_1 (0 = On Use)
    0,                                              -- spellcategory_1
    0,                                              -- spellcategorycooldown_1
    0,                                              -- bonding (0 = No binding)
    'Faire apparaitre un Ogre-mage.',              -- description
    'ConquestGuardItem'                                  -- ScriptName
);

-- ============================================
-- Ecraseur ogre spawn item (entry 80032)
-- ============================================
DELETE FROM `item_template` WHERE `entry` = 80032;

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
    80032,                                          -- entry
    15,                                             -- class (Miscellaneous)
    0,                                              -- subclass
    'Ecraseur ogre',                                -- name
    23716,                                          -- displayid
    3,                                              -- Quality (Rare - bleu)
    0,                                              -- Flags
    1,                                              -- BuyCount
    0,                                              -- BuyPrice
    0,                                              -- SellPrice
    0,                                              -- InventoryType (Non-equipable)
    -1,                                             -- AllowableClass (all classes)
    -1,                                             -- AllowableRace (all races)
    80,                                             -- ItemLevel
    80,                                             -- RequiredLevel
    5,                                              -- maxcount (5 par joueur)
    5,                                             -- stackable (5 par joueur)
    70246,                                          -- spellid_1 (Spawn Ogre Guard)
    0,                                              -- spelltrigger_1 (0 = On Use)
    0,                                              -- spellcategory_1
    0,                                              -- spellcategorycooldown_1
    0,                                              -- bonding (0 = No binding)
    'Faire apparaitre un Ecraseur ogre.',          -- description
    'ConquestGuardItem'                                  -- ScriptName
);

-- ============================================
-- Chaman ogre spawn item (entry 80033)
-- ============================================
DELETE FROM `item_template` WHERE `entry` = 80033;

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
    80033,                                          -- entry
    15,                                             -- class (Miscellaneous)
    0,                                              -- subclass
    'Chaman ogre',                                  -- name
    23716,                                          -- displayid
    3,                                              -- Quality (Rare - bleu)
    0,                                              -- Flags
    1,                                              -- BuyCount
    0,                                              -- BuyPrice
    0,                                              -- SellPrice
    0,                                              -- InventoryType (Non-equipable)
    -1,                                             -- AllowableClass (all classes)
    -1,                                             -- AllowableRace (all races)
    80,                                             -- ItemLevel
    80,                                             -- RequiredLevel
    5,                                              -- maxcount (5 par joueur)
    5,                                             -- stackable (5 par joueur)
    70246,                                          -- spellid_1 (Spawn Ogre Guard)
    0,                                              -- spelltrigger_1 (0 = On Use)
    0,                                              -- spellcategory_1
    0,                                              -- spellcategorycooldown_1
    0,                                              -- bonding (0 = No binding)
    'Faire apparaitre un Chaman ogre.',            -- description
    'ConquestGuardItem'                                  -- ScriptName
);

-- ============================================
-- Massacreur ogre spawn item (entry 80034)
-- ============================================
DELETE FROM `item_template` WHERE `entry` = 80034;

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
    80034,                                          -- entry
    15,                                             -- class (Miscellaneous)
    0,                                              -- subclass
    'Massacreur ogre',                              -- name
    23716,                                          -- displayid
    4,                                              -- Quality (Epic - violet)
    0,                                              -- Flags
    1,                                              -- BuyCount
    0,                                              -- BuyPrice
    0,                                              -- SellPrice
    0,                                              -- InventoryType (Non-equipable)
    -1,                                             -- AllowableClass (all classes)
    -1,                                             -- AllowableRace (all races)
    80,                                             -- ItemLevel
    80,                                             -- RequiredLevel
    5,                                              -- maxcount (5 par joueur)
    5,                                             -- stackable (5 par joueur)
    70246,                                          -- spellid_1 (Spawn Ogre Guard)
    0,                                              -- spelltrigger_1 (0 = On Use)
    0,                                              -- spellcategory_1
    0,                                              -- spellcategorycooldown_1
    0,                                              -- bonding (0 = No binding)
    'Faire apparaitre un Massacreur ogre.',        -- description
    'ConquestGuardItem'                                  -- ScriptName
);

-- ============================================
-- Démoniste ogre spawn item (entry 80035)
-- ============================================
DELETE FROM `item_template` WHERE `entry` = 80035;

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
    80035,                                          -- entry
    15,                                             -- class (Miscellaneous)
    0,                                              -- subclass
    'Démoniste ogre',                               -- name
    23716,                                          -- displayid
    4,                                              -- Quality (Epic - violet)
    0,                                              -- Flags
    1,                                              -- BuyCount
    0,                                              -- BuyPrice
    0,                                              -- SellPrice
    0,                                              -- InventoryType (Non-equipable)
    -1,                                             -- AllowableClass (all classes)
    -1,                                             -- AllowableRace (all races)
    80,                                             -- ItemLevel
    80,                                             -- RequiredLevel
    5,                                              -- maxcount (5 par joueur)
    5,                                              -- stackable (5 par joueur)
    70246,                                          -- spellid_1 (Spawn Ogre Guard)
    0,                                              -- spelltrigger_1 (0 = On Use)
    0,                                              -- spellcategory_1
    0,                                              -- spellcategorycooldown_1
    0,                                              -- bonding (0 = No binding)
    'Faire apparaitre un Démoniste ogre.',         -- description
    'ConquestGuardItem'                                  -- ScriptName
);
