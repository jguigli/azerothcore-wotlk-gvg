-- Conquest Build Module - Gate Kit Item
-- Item 80003 - Kit de grande herse

DELETE FROM `item_template` WHERE `entry` = 80003;

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
    80003,                                          -- entry
    15,                                             -- class (Miscellaneous)
    0,                                              -- subclass
    'Kit de grande herse',                          -- name
    7494,                                           -- displayid (Arclight Spanner icon - same as item 6219)
    4,                                              -- Quality (Rare - bleu)
    0,                                              -- Flags
    1,                                              -- BuyCount
    20000,                                          -- BuyPrice (2 gold)
    5000,                                           -- SellPrice (50 silver)
    0,                                              -- InventoryType (Non-equipable)
    -1,                                             -- AllowableClass (all classes)
    -1,                                             -- AllowableRace (all races)
    70,                                             -- ItemLevel
    70,                                             -- RequiredLevel
    0,                                              -- maxcount (0 = no limit)
    20,                                             -- stackable (max 20 per stack)
    63046,                                           -- spellid_1 (Summon Conquest Build - 30 yard range) 54407
    0,                                              -- spelltrigger_1 (0 = On Use)
    0,                                              -- spellcategory_1
    0,                                              -- spellcategorycooldown_1
    0,                                              -- bonding (0 = No binding)
    'Construit une fortification: 2 tours destructibles + 1 herse centrale (40 yards). Si une tour est détruite, tout le système disparaît. Seule votre guilde peut ouvrir/fermer la herse.',  -- description
    'item_conquest_build_gate'                           -- ScriptName
);

-- Informations
SELECT 'Kit de grande herse créé avec succès!' AS Result;
SELECT 'Item ID: 80003 - Kit de grande herse' AS Info;
SELECT 'Utilisez .additem 80003' AS Command;

