-- Conquest Build Module - Item Configuration pour Grand Fort 3
-- Creates build item: Kit de grand fort 3 (80012)

-- Delete existing item
DELETE FROM `item_template` WHERE `entry` = 80012;

-- Kit de grand fort 3 (80012)
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
    80012,                                          -- entry
    15,                                             -- class (Miscellaneous)
    0,                                              -- subclass
    'Kit de grand fort 3',                         -- name
    7494,                                           -- displayid (Arclight Spanner icon - same as item 6219)
    5,                                              -- Quality (Legendary - orange)
    0,                                              -- Flags
    1,                                              -- BuyCount
    100000,                                         -- BuyPrice (10 gold)
    25000,                                          -- SellPrice (2 gold 50 silver)
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
    'Construit un grand fort complet avec remparts et batiment.',  -- description
    'item_conquest_build_fort_rempart_2'                    -- ScriptName (utilise le script de spawn rempart 2)
);

-- Informations
SELECT 'Kit de grand fort 3 créé avec succès!' AS Result;
SELECT 'Item ID: 80012 - Kit de grand fort 3 (Rareté 5)' AS Info;
SELECT 'Utilisez .additem 80012' AS Command;

