-- ============================================================================
-- Conquest Vendor Items — PvE Tier 2 (BWL + Onyxia), 2.5 (AQ40), 3 (Naxx40)
-- Eclate sur les 4 vendors (400260-400263).
-- slot_label : "T2" / "T2.5" / "T3" / currency : PC
-- ============================================================================

DELETE FROM `conquest_vendor_items` WHERE `npc_entry` IN (400260, 400261, 400262, 400263)
  AND `slot_label` IN ('T2', 'T2.5', 'T3');

-- ----------------------------------------------------------------------------
-- TIER 2 (BWL + Onyxia, ilvl 73-78)
-- Patterns : %of Wrath, Judgement%, Dragonstalker%, %of Ten Storms,
--            Bloodfang%, Stormrage%, Netherwind%, %of Transcendence, Nemesis%
-- Prix : 60 PC armure / 80 PC arme / 50 PC offpart / 60 PC bijou
-- ----------------------------------------------------------------------------

-- 400261 Armures T2
INSERT IGNORE INTO `conquest_vendor_items` (`npc_entry`, `item_id`, `price`, `currency`, `slot_label`, `display_order`)
SELECT 400261, `entry`, 0, 'PC', 'T2',
  CASE `InventoryType` WHEN 1 THEN 10 WHEN 3 THEN 20 WHEN 5 THEN 30 WHEN 7 THEN 40 WHEN 10 THEN 50 END
FROM `item_template`
WHERE `quality` = 4 AND `ItemLevel` BETWEEN 73 AND 78 AND `RequiredLevel` = 60
  AND `class` = 4 AND `InventoryType` IN (1, 3, 5, 7, 10)
  AND (
       `name` LIKE '%of Wrath' OR `name` LIKE 'Judgement%' OR
       `name` LIKE 'Dragonstalker%' OR `name` LIKE '%of Ten Storms' OR
       `name` LIKE 'Bloodfang%' OR `name` LIKE 'Stormrage%' OR
       `name` LIKE 'Netherwind%' OR `name` LIKE '%of Transcendence' OR
       `name` LIKE 'Nemesis%'
      );

-- 400262 Offpart T2
INSERT IGNORE INTO `conquest_vendor_items` (`npc_entry`, `item_id`, `price`, `currency`, `slot_label`, `display_order`)
SELECT 400262, `entry`, 0, 'PC', 'T2',
  CASE `InventoryType` WHEN 16 THEN 10 WHEN 9 THEN 20 WHEN 6 THEN 30 WHEN 8 THEN 40 END
FROM `item_template`
WHERE `quality` = 4 AND `ItemLevel` BETWEEN 73 AND 78 AND `RequiredLevel` = 60
  AND `class` = 4 AND `InventoryType` IN (6, 8, 9, 16)
  AND (
       `name` LIKE '%of Wrath' OR `name` LIKE 'Judgement%' OR
       `name` LIKE 'Dragonstalker%' OR `name` LIKE '%of Ten Storms' OR
       `name` LIKE 'Bloodfang%' OR `name` LIKE 'Stormrage%' OR
       `name` LIKE 'Netherwind%' OR `name` LIKE '%of Transcendence' OR
       `name` LIKE 'Nemesis%'
      );

-- 400260 Armes T2 (BWL + Ony, weapons)
INSERT IGNORE INTO `conquest_vendor_items` (`npc_entry`, `item_id`, `price`, `currency`, `slot_label`, `display_order`)
SELECT 400260, `entry`, 0, 'PC', 'T2', 10 + `subclass`
FROM `item_template`
WHERE `quality` = 4 AND `ItemLevel` BETWEEN 73 AND 78 AND `RequiredLevel` = 60
  AND `class` = 2 AND `InventoryType` IN (13, 15, 17, 21, 22, 23, 25, 26, 28);

-- 400263 Offset T2 (bijoux ilvl 73-78)
INSERT IGNORE INTO `conquest_vendor_items` (`npc_entry`, `item_id`, `price`, `currency`, `slot_label`, `display_order`)
SELECT 400263, `entry`, 0, 'PC', 'T2',
  CASE `InventoryType` WHEN 2 THEN 10 WHEN 11 THEN 20 WHEN 12 THEN 30 END
FROM `item_template`
WHERE `quality` = 4 AND `ItemLevel` BETWEEN 73 AND 78 AND `RequiredLevel` = 60
  AND `class` = 4 AND `InventoryType` IN (2, 11, 12);

-- ----------------------------------------------------------------------------
-- TIER 2.5 (AQ40, ilvl 78-84)
-- Patterns : Conqueror%, Avenger%, Striker%, Stormcaller%, Deathdealer%,
--            Genesis%, Enigma%, %of the Oracle, Doomcaller%
-- Prix : 90 / 120 / 75 / 90 PC
-- ----------------------------------------------------------------------------

-- 400261 Armures T2.5
INSERT IGNORE INTO `conquest_vendor_items` (`npc_entry`, `item_id`, `price`, `currency`, `slot_label`, `display_order`)
SELECT 400261, `entry`, 0, 'PC', 'T2.5',
  CASE `InventoryType` WHEN 1 THEN 10 WHEN 3 THEN 20 WHEN 5 THEN 30 WHEN 7 THEN 40 WHEN 10 THEN 50 END
FROM `item_template`
WHERE `quality` = 4 AND `ItemLevel` BETWEEN 78 AND 84 AND `RequiredLevel` = 60
  AND `class` = 4 AND `InventoryType` IN (1, 3, 5, 7, 10)
  AND (
       `name` LIKE 'Conqueror%' OR `name` LIKE 'Avenger%' OR
       `name` LIKE 'Striker%' OR `name` LIKE 'Stormcaller%' OR
       `name` LIKE 'Deathdealer%' OR `name` LIKE 'Genesis%' OR
       `name` LIKE 'Enigma%' OR `name` LIKE '%of the Oracle' OR
       `name` LIKE 'Doomcaller%'
      );

-- 400262 Offpart T2.5
INSERT IGNORE INTO `conquest_vendor_items` (`npc_entry`, `item_id`, `price`, `currency`, `slot_label`, `display_order`)
SELECT 400262, `entry`, 0, 'PC', 'T2.5',
  CASE `InventoryType` WHEN 16 THEN 10 WHEN 9 THEN 20 WHEN 6 THEN 30 WHEN 8 THEN 40 END
FROM `item_template`
WHERE `quality` = 4 AND `ItemLevel` BETWEEN 78 AND 84 AND `RequiredLevel` = 60
  AND `class` = 4 AND `InventoryType` IN (6, 8, 9, 16)
  AND (
       `name` LIKE 'Conqueror%' OR `name` LIKE 'Avenger%' OR
       `name` LIKE 'Striker%' OR `name` LIKE 'Stormcaller%' OR
       `name` LIKE 'Deathdealer%' OR `name` LIKE 'Genesis%' OR
       `name` LIKE 'Enigma%' OR `name` LIKE '%of the Oracle' OR
       `name` LIKE 'Doomcaller%'
      );

-- 400260 Armes T2.5 (AQ40 weapons)
INSERT IGNORE INTO `conquest_vendor_items` (`npc_entry`, `item_id`, `price`, `currency`, `slot_label`, `display_order`)
SELECT 400260, `entry`, 0, 'PC', 'T2.5', 10 + `subclass`
FROM `item_template`
WHERE `quality` = 4 AND `ItemLevel` BETWEEN 78 AND 84 AND `RequiredLevel` = 60
  AND `class` = 2 AND `InventoryType` IN (13, 15, 17, 21, 22, 23, 25, 26, 28);

-- 400263 Offset T2.5
INSERT IGNORE INTO `conquest_vendor_items` (`npc_entry`, `item_id`, `price`, `currency`, `slot_label`, `display_order`)
SELECT 400263, `entry`, 0, 'PC', 'T2.5',
  CASE `InventoryType` WHEN 2 THEN 10 WHEN 11 THEN 20 WHEN 12 THEN 30 END
FROM `item_template`
WHERE `quality` = 4 AND `ItemLevel` BETWEEN 78 AND 84 AND `RequiredLevel` = 60
  AND `class` = 4 AND `InventoryType` IN (2, 11, 12);

-- ----------------------------------------------------------------------------
-- TIER 3 (Naxx40, ilvl 88-95)
-- Patterns : Dreadnaught%, Cryptstalker%, Earthshatter%, Bonescythe%,
--            Dreamwalker%, Frostfire%, %of Faith, Plagueheart%, Redemption%
-- Prix : 150 / 180 / 120 / 150 PC
-- ----------------------------------------------------------------------------

-- 400261 Armures T3
INSERT IGNORE INTO `conquest_vendor_items` (`npc_entry`, `item_id`, `price`, `currency`, `slot_label`, `display_order`)
SELECT 400261, `entry`, 150, 'PC', 'T3',
  CASE `InventoryType` WHEN 1 THEN 10 WHEN 3 THEN 20 WHEN 5 THEN 30 WHEN 7 THEN 40 WHEN 10 THEN 50 END
FROM `item_template`
WHERE `quality` = 4 AND `ItemLevel` BETWEEN 88 AND 95 AND `RequiredLevel` = 60
  AND `class` = 4 AND `InventoryType` IN (1, 3, 5, 7, 10)
  AND (
       `name` LIKE 'Dreadnaught%' OR `name` LIKE 'Cryptstalker%' OR
       `name` LIKE 'Earthshatter%' OR `name` LIKE 'Bonescythe%' OR
       `name` LIKE 'Dreamwalker%' OR `name` LIKE 'Frostfire%' OR
       `name` LIKE '%of Faith' OR `name` LIKE 'Plagueheart%' OR
       `name` LIKE 'Redemption%'
      );

-- 400262 Offpart T3
INSERT IGNORE INTO `conquest_vendor_items` (`npc_entry`, `item_id`, `price`, `currency`, `slot_label`, `display_order`)
SELECT 400262, `entry`, 120, 'PC', 'T3',
  CASE `InventoryType` WHEN 16 THEN 10 WHEN 9 THEN 20 WHEN 6 THEN 30 WHEN 8 THEN 40 END
FROM `item_template`
WHERE `quality` = 4 AND `ItemLevel` BETWEEN 88 AND 95 AND `RequiredLevel` = 60
  AND `class` = 4 AND `InventoryType` IN (6, 8, 9, 16)
  AND (
       `name` LIKE 'Dreadnaught%' OR `name` LIKE 'Cryptstalker%' OR
       `name` LIKE 'Earthshatter%' OR `name` LIKE 'Bonescythe%' OR
       `name` LIKE 'Dreamwalker%' OR `name` LIKE 'Frostfire%' OR
       `name` LIKE '%of Faith' OR `name` LIKE 'Plagueheart%' OR
       `name` LIKE 'Redemption%'
      );

-- 400260 Armes T3 (Naxx40 weapons)
INSERT IGNORE INTO `conquest_vendor_items` (`npc_entry`, `item_id`, `price`, `currency`, `slot_label`, `display_order`)
SELECT 400260, `entry`, 180, 'PC', 'T3', 10 + `subclass`
FROM `item_template`
WHERE `quality` = 4 AND `ItemLevel` BETWEEN 88 AND 95 AND `RequiredLevel` = 60
  AND `class` = 2 AND `InventoryType` IN (13, 15, 17, 21, 22, 23, 25, 26, 28);

-- 400263 Offset T3
INSERT IGNORE INTO `conquest_vendor_items` (`npc_entry`, `item_id`, `price`, `currency`, `slot_label`, `display_order`)
SELECT 400263, `entry`, 150, 'PC', 'T3',
  CASE `InventoryType` WHEN 2 THEN 10 WHEN 11 THEN 20 WHEN 12 THEN 30 END
FROM `item_template`
WHERE `quality` = 4 AND `ItemLevel` BETWEEN 88 AND 95 AND `RequiredLevel` = 60
  AND `class` = 4 AND `InventoryType` IN (2, 11, 12);
