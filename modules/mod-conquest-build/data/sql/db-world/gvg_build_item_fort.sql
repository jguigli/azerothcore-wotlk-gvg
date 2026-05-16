-- Conquest Build Module - Item Configuration pour Fort
-- Creates build item: Fort Kit (80008)

-- Delete existing item
DELETE FROM `item_template` WHERE `entry` = 80008;

-- Fort Kit (80008)
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
    80008,                                          -- entry
    15,                                             -- class (Miscellaneous)
    0,                                              -- subclass
    'Kit de fort',                                 -- name
    7494,                                           -- displayid (Arclight Spanner icon - same as item 6219)
    5,                                              -- Quality (Legendary - orange)
    0,                                              -- Flags
    1,                                              -- BuyCount
    50000,                                          -- BuyPrice (5 gold)
    12500,                                          -- SellPrice (1 gold 25 silver)
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
    'Construit un fort complet: 1 herse devant, 1 mur derrière, 1 mur à gauche, 1 mur à droite, et 2 tours avec 1 mur derrière. Utilisez l''outil de récupération pour récupérer tous les kits.',  -- description
    'item_conquest_build_fort'                          -- ScriptName
);

-- Informations
SELECT 'Kit de fort créé avec succès!' AS Result;
SELECT 'Item ID: 80008 - Kit de fort (Rareté 5)' AS Info;
SELECT 'Utilisez .additem 80008' AS Command;

