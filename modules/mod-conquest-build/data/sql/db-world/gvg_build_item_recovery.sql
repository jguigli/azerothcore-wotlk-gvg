-- Conquest Build Module - Recovery Tool Item
-- Item 80002 - Permet de récupérer les structures mal placées

DELETE FROM `item_template` WHERE `entry` = 80002;

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
    80002,                                          -- entry
    15,                                             -- class (Miscellaneous)
    0,                                              -- subclass
    'Outil de recuperation Conquest',                   -- name
    6833,                                           -- displayid (Hammer icon)
    3,                                              -- Quality (Rare - bleu)
    0,                                              -- Flags
    1,                                              -- BuyCount
    5000,                                           -- BuyPrice (50 silver)
    1250,                                           -- SellPrice (12.5 silver)
    0,                                              -- InventoryType (Non-equipable)
    -1,                                             -- AllowableClass (all classes)
    -1,                                             -- AllowableRace (all races)
    80,                                             -- ItemLevel
    80,                                             -- RequiredLevel
    1,                                              -- maxcount (1 seul par joueur)
    1,                                              -- stackable (ne stack pas)
    70000,                                           -- spellid_1 (Recover Conquest Build - instant cast, no target)
    0,                                              -- spelltrigger_1 (0 = On Use)
    0,                                              -- spellcategory_1
    0,                                              -- spellcategorycooldown_1
    1,                                              -- bonding (1 = Bind on Pickup)
    'Permet de recuperer les structures a proximite.', -- description
    'item_conquest_build_recovery'                       -- ScriptName
);

-- Informations
SELECT 'Outil de Récupération Conquest créé avec succès!' AS Result;
SELECT 'Item ID: 80002 - Outil de Récupération Conquest' AS Info;
SELECT 'Utilisez .additem 80002' AS Command;

