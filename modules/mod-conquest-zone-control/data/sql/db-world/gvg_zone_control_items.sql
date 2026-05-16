-- Conquest Zone Control Module - Resource Items
-- Creates two resource items: Or Conquest (80020) and Bois Conquest (80021)

-- Delete existing items
DELETE FROM `item_template` WHERE `entry` IN (80020, 80021);

-- Or Conquest (80020)
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
    `bonding`, 
    `description`, 
    `Material`
) VALUES (
    80020,                                          -- entry
    15,                                             -- class (Miscellaneous)
    0,                                              -- subclass
    'Or Conquest',                                       -- name
    58679,                                          -- displayid (same as item 45978 - Solid Gold Coin)
    1,                                              -- Quality (Common - gris)
    0,                                              -- Flags
    1,                                              -- BuyCount
    0,                                              -- BuyPrice
    0,                                              -- SellPrice
    0,                                              -- InventoryType (Non-equipable)
    -1,                                             -- AllowableClass (all classes)
    -1,                                             -- AllowableRace (all races)
    1,                                              -- ItemLevel
    0,                                              -- RequiredLevel
    0,                                              -- maxcount (0 = no limit)
    10000,                                          -- stackable (max 10000 per stack)
    0,                                              -- bonding (0 = No binding)
    'Ressource Conquest',                                -- description
    4                                               -- Material (Metal)
);

-- Bois Conquest (80021)
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
    `bonding`, 
    `description`, 
    `Material`
) VALUES (
    80021,                                          -- entry
    7,                                              -- class (Trade Goods)
    11,                                             -- subclass (Other)
    'Bois Conquest',                                     -- name
    21102,                                          -- displayid (same as item 4470 - Simple Wood)
    1,                                              -- Quality (Common - gris)
    0,                                              -- Flags
    1,                                              -- BuyCount
    0,                                              -- BuyPrice
    0,                                              -- SellPrice
    0,                                              -- InventoryType (Non-equipable)
    -1,                                             -- AllowableClass (all classes)
    -1,                                             -- AllowableRace (all races)
    1,                                              -- ItemLevel
    0,                                              -- RequiredLevel
    0,                                              -- maxcount (0 = no limit)
    10000,                                          -- stackable (max 10000 per stack)
    0,                                              -- bonding (0 = No binding)
    'Ressource Conquest',                                -- description
    -1                                              -- Material (None)
);

-- Informations
SELECT 'Items de ressources Conquest créés avec succès!' AS Result;
SELECT 'Item ID: 80020 - Or Conquest' AS Info;
SELECT 'Item ID: 80021 - Bois Conquest' AS Info;
SELECT 'Utilisez .additem 80020 ou .additem 80021' AS Command;

