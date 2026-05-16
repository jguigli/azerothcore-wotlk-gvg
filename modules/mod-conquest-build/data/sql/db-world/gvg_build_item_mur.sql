-- Conquest Build Module - Item Configuration pour Mur
-- Creates build item: Mur Kit (80004)

-- Delete existing item
DELETE FROM `item_template` WHERE `entry` = 80004;

-- Mur Kit (80004)
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
    80004,                                          -- entry
    15,                                             -- class (Miscellaneous)
    0,                                              -- subclass
    'Kit de mur',                                   -- name
    7494,                                           -- displayid (Arclight Spanner icon - same as item 6219)
    2,                                              -- Quality (Uncommon - vert)
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
    'Construit un mur défensif. Usage unique.',     -- description
    'item_conquest_build_wall'                           -- ScriptName
);

-- Informations
SELECT 'Kit de Mur Conquest créé avec succès!' AS Result;
SELECT 'Item ID: 80004 - Kit de Mur Conquest (GameObject 400022, Rareté 2)' AS Info;
SELECT 'Utilisez .additem 80004' AS Command;

