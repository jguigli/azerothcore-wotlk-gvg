-- Conquest Build Module - Item Configuration pour Grand Mur
-- Creates build item: Grand Mur Kit (80000)

-- Delete existing item
DELETE FROM `item_template` WHERE `entry` = 80000;

-- Wall Kit (80000)
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
    80000,                                          -- entry
    15,                                             -- class (Miscellaneous)
    0,                                              -- subclass
    'Kit de grand mur',                             -- name
    7494,                                           -- displayid (Arclight Spanner icon - same as item 6219)
    4,                                              -- Quality (Epic - pourpre)
    0,                                              -- Flags
    1,                                              -- BuyCount
    10000,                                          -- BuyPrice (1 gold)
    1250,                                           -- SellPrice (12.5 silver)
    0,                                              -- InventoryType (Non-equipable)
    -1,                                             -- AllowableClass (all classes)
    -1,                                             -- AllowableRace (all races)
    80,                                             -- ItemLevel
    80,                                             -- RequiredLevel
    0,                                              -- maxcount (0 = no limit)
    20,                                             -- stackable (max 20 per stack)
    63046,                                          -- spellid_1 (Summon Conquest Build - 30 yard range)
    0,                                              -- spelltrigger_1 (0 = On Use)
    0,                                              -- spellcategory_1
    0,                                              -- spellcategorycooldown_1
    0,                                              -- bonding (0 = No binding)
    'Construit un grand mur défensif. Usage unique.',     -- description
    'item_conquest_build_wall'                           -- ScriptName
);

-- Informations
SELECT 'Kit de grand mur créé avec succès!' AS Result;
SELECT 'Item ID: 80000 - Kit de grand mur' AS Info;
SELECT 'Utilisez .additem 80000' AS Command;

