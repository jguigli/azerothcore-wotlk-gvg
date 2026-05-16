-- Conquest Build Module - Item Configuration pour Grand Fort
-- Creates build item: Grand Fort Kit (80009)

-- Delete existing item
DELETE FROM `item_template` WHERE `entry` = 80009;

-- Grand Fort Kit (80009)
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
    80009,                                          -- entry
    15,                                             -- class (Miscellaneous)
    0,                                              -- subclass
    'Kit de grand fort',                           -- name
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
    'Construit un grand fort complet: 1 grande herse devant, 1 grand mur derrière, 1 grand mur à gauche, 1 grand mur à droite, et 2 grandes tours avec 1 grand mur derrière. Utilisez l''outil de récupération pour récupérer tous les kits.',  -- description
    'item_conquest_build_grand_fort'                    -- ScriptName
);

-- Informations
SELECT 'Kit de grand fort créé avec succès!' AS Result;
SELECT 'Item ID: 80009 - Kit de grand fort (Rareté 5)' AS Info;
SELECT 'Utilisez .additem 80009' AS Command;

