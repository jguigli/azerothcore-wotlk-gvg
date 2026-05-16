-- ============================================================================
-- Conquest Vendor Items — PvE Tier 1 (Molten Core)
-- Eclate sur les 4 vendors (400260-400263) via filtres InventoryType.
-- slot_label : "T1" / currency : PC
-- ============================================================================

DELETE FROM `conquest_vendor_items` WHERE `npc_entry` IN (400260, 400261, 400262, 400263)
  AND `slot_label` = 'T1';

-- Patterns par classe pour T1 :
--   Warrior  : %of Might
--   Paladin  : Lawbringer%
--   Hunter   : Giantstalker%
--   Shaman   : Earthfury%
--   Rogue    : Nightslayer%
--   Druid    : Cenarion%
--   Mage     : Arcanist%
--   Priest   : %of Prophecy
--   Warlock  : Felheart%

-- 400261 Armures T1 (tete/epaules/torse/jambes/mains) - 30 PC
INSERT IGNORE INTO `conquest_vendor_items` (`npc_entry`, `item_id`, `price`, `currency`, `slot_label`, `display_order`)
SELECT 400261, `entry`, 0, 'PC', 'T1',
  CASE `InventoryType` WHEN 1 THEN 10 WHEN 3 THEN 20 WHEN 5 THEN 30 WHEN 7 THEN 40 WHEN 10 THEN 50 END
FROM `item_template`
WHERE `quality` = 4
  AND `ItemLevel` BETWEEN 65 AND 72
  AND `RequiredLevel` = 60
  AND `class` = 4
  AND `InventoryType` IN (1, 3, 5, 7, 10)
  AND (
       `name` LIKE '%of Might' OR `name` LIKE 'Lawbringer%' OR
       `name` LIKE 'Giantstalker%' OR `name` LIKE 'Earthfury%' OR
       `name` LIKE 'Nightslayer%' OR `name` LIKE 'Cenarion%' OR
       `name` LIKE 'Arcanist%' OR `name` LIKE '%of Prophecy' OR
       `name` LIKE 'Felheart%'
      );

-- 400262 Offpart T1 (ceinture/bottes/poignets/cape) - 25 PC
INSERT IGNORE INTO `conquest_vendor_items` (`npc_entry`, `item_id`, `price`, `currency`, `slot_label`, `display_order`)
SELECT 400262, `entry`, 0, 'PC', 'T1',
  CASE `InventoryType` WHEN 16 THEN 10 WHEN 9 THEN 20 WHEN 6 THEN 30 WHEN 8 THEN 40 END
FROM `item_template`
WHERE `quality` = 4
  AND `ItemLevel` BETWEEN 65 AND 72
  AND `RequiredLevel` = 60
  AND `class` = 4
  AND `InventoryType` IN (6, 8, 9, 16)
  AND (
       `name` LIKE '%of Might' OR `name` LIKE 'Lawbringer%' OR
       `name` LIKE 'Giantstalker%' OR `name` LIKE 'Earthfury%' OR
       `name` LIKE 'Nightslayer%' OR `name` LIKE 'Cenarion%' OR
       `name` LIKE 'Arcanist%' OR `name` LIKE '%of Prophecy' OR
       `name` LIKE 'Felheart%'
      );

-- 400260 Armes T1 (MC + ZG, classe weapon) - 40 PC
INSERT IGNORE INTO `conquest_vendor_items` (`npc_entry`, `item_id`, `price`, `currency`, `slot_label`, `display_order`)
SELECT 400260, `entry`, 0, 'PC', 'T1', 10 + `subclass`
FROM `item_template`
WHERE `quality` = 4
  AND `ItemLevel` BETWEEN 65 AND 72
  AND `RequiredLevel` = 60
  AND `class` = 2 -- weapon class
  AND `InventoryType` IN (13, 15, 17, 21, 22, 23, 25, 26, 28);

-- 400263 Offset T1 (bijoux PvE ilvl 65-72) - 40 PC
INSERT IGNORE INTO `conquest_vendor_items` (`npc_entry`, `item_id`, `price`, `currency`, `slot_label`, `display_order`)
SELECT 400263, `entry`, 0, 'PC', 'T1',
  CASE `InventoryType` WHEN 2 THEN 10 WHEN 11 THEN 20 WHEN 12 THEN 30 END
FROM `item_template`
WHERE `quality` = 4
  AND `ItemLevel` BETWEEN 65 AND 72
  AND `RequiredLevel` = 60
  AND `class` = 4
  AND `InventoryType` IN (2, 11, 12);
