-- ============================================================================
-- Conquest Vendor Items — PvP Rare + PvP Epique
-- 4 vendors :
--   400260 Armes   : armes PvP (InventoryType 13/14/15/17/21/22/23/25/26/28)
--   400261 Armures : tete/epaules/torse/jambes/mains (1/3/5/7/10)
--   400262 Offpart : ceinture/pieds/poignets/cape (6/8/9/16)
--   400263 Offset  : bijoux (cou/anneau/trinket = 2/11/12 -- pas de bijoux PvP rare/epique R14 dans Vanilla, mais on couvre par filtre)
-- slot_label : "PVP Rare" ou "PVP Epique" (= sections de gossip)
-- currency   : PB pour tous
-- ============================================================================

DELETE FROM `conquest_vendor_items` WHERE `npc_entry` IN (400260, 400261, 400262, 400263)
  AND `slot_label` IN ('PVP Rare', 'PVP Epique');

-- ----------------------------------------------------------------------------
-- PVP RARE (Lieutenant Commander / Blood Guard, R7-R10 epic blue equiv)
-- Prix : 30 PB / piece d'armure, 80 PB / arme
-- ----------------------------------------------------------------------------

-- 400260 Armes - PVP Rare (armes rare PvP rangs R5-R10 alliance + horde)
-- Patterns ajoutes : Marshal's, Legionnaire's, Knight-Captain's, Stone Guard's, Champion's
-- quality=3 (bleu rare) pour eviter de capturer les Champion's epic AQ40.
INSERT IGNORE INTO `conquest_vendor_items` (`npc_entry`, `item_id`, `price`, `currency`, `slot_label`, `display_order`)
SELECT 400260, `entry`, 0, 'PB', 'PVP Rare', 10 + `subclass`
FROM `item_template`
WHERE (`name` LIKE 'Lieutenant Commander%' OR `name` LIKE 'Blood Guard%'
       OR `name` LIKE 'Marshal''s %' OR `name` LIKE 'Legionnaire''s %'
       OR `name` LIKE 'Knight-Captain''s %' OR `name` LIKE 'Stone Guard''s %'
       OR `name` LIKE 'Champion''s %')
  AND `quality` = 3
  AND `class` IN (2, 4) -- weapon or armor (shields)
  AND `InventoryType` IN (13, 14, 15, 17, 21, 22, 23, 25, 26, 28);

-- 400261 Armures - PVP Rare (5 pieces principales)
INSERT IGNORE INTO `conquest_vendor_items` (`npc_entry`, `item_id`, `price`, `currency`, `slot_label`, `display_order`)
SELECT 400261, `entry`, 0, 'PB', 'PVP Rare',
  CASE `InventoryType` WHEN 1 THEN 10 WHEN 3 THEN 20 WHEN 5 THEN 30 WHEN 7 THEN 40 WHEN 10 THEN 50 END
FROM `item_template`
WHERE (`name` LIKE 'Lieutenant Commander%' OR `name` LIKE 'Blood Guard%')
  AND `class` = 4
  AND `InventoryType` IN (1, 3, 5, 7, 10)
  AND `name` NOT LIKE '%Mount%';

-- 400262 Offpart - PVP Rare (cape/brassards/ceinture/bottes)
INSERT IGNORE INTO `conquest_vendor_items` (`npc_entry`, `item_id`, `price`, `currency`, `slot_label`, `display_order`)
SELECT 400262, `entry`, 0, 'PB', 'PVP Rare',
  CASE `InventoryType` WHEN 16 THEN 10 WHEN 9 THEN 20 WHEN 6 THEN 30 WHEN 8 THEN 40 END
FROM `item_template`
WHERE (`name` LIKE 'Lieutenant Commander%' OR `name` LIKE 'Blood Guard%')
  AND `class` = 4
  AND `InventoryType` IN (6, 8, 9, 16)
  AND `name` NOT LIKE '%Mount%';

-- 400263 Offset - PVP Rare (bijoux PvP : medailles d'insigne ranks)
INSERT IGNORE INTO `conquest_vendor_items` (`npc_entry`, `item_id`, `price`, `currency`, `slot_label`, `display_order`)
SELECT 400263, `entry`, 0, 'PB', 'PVP Rare',
  CASE `InventoryType` WHEN 2 THEN 10 WHEN 11 THEN 20 WHEN 12 THEN 30 END
FROM `item_template`
WHERE (`name` LIKE 'Lieutenant Commander%' OR `name` LIKE 'Blood Guard%'
       OR `name` LIKE 'Insignia of%')
  AND `InventoryType` IN (2, 11, 12);

-- ----------------------------------------------------------------------------
-- PVP EPIQUE (Field Marshal / Warlord / Grand Marshal / High Warlord, R12-R14)
-- Prix : 100 PB / armure, 200 PB / arme, 60 PB / offpart, 70 PB / bijou
-- ----------------------------------------------------------------------------

-- 400260 Armes - PVP Epique (Grand Marshal / High Warlord)
INSERT IGNORE INTO `conquest_vendor_items` (`npc_entry`, `item_id`, `price`, `currency`, `slot_label`, `display_order`)
SELECT 400260, `entry`,
  CASE
    WHEN `InventoryType` = 17 THEN 250 -- 2H
    WHEN `InventoryType` IN (22, 14, 23) THEN 80 -- off-hand / shield
    ELSE 200
  END,
  'PB', 'PVP Epique', 10 + `subclass`
FROM `item_template`
WHERE (`name` LIKE 'Grand Marshal%' OR `name` LIKE 'High Warlord%')
  AND `class` IN (2, 4)
  AND `InventoryType` IN (13, 14, 15, 17, 21, 22, 23, 25, 26, 28);

-- 400261 Armures - PVP Epique (Field Marshal + Warlord, tete/torse/epaules/jambes/mains)
INSERT IGNORE INTO `conquest_vendor_items` (`npc_entry`, `item_id`, `price`, `currency`, `slot_label`, `display_order`)
SELECT 400261, `entry`, 100, 'PB', 'PVP Epique',
  CASE `InventoryType` WHEN 1 THEN 10 WHEN 3 THEN 20 WHEN 5 THEN 30 WHEN 7 THEN 40 WHEN 10 THEN 50 END
FROM `item_template`
WHERE (`name` LIKE 'Field Marshal%' OR `name` LIKE 'Warlord''s%' OR `name` LIKE 'Warlord %')
  AND `class` = 4
  AND `InventoryType` IN (1, 3, 5, 7, 10)
  AND `name` NOT LIKE '%Mount%';

-- 400262 Offpart - PVP Epique
INSERT IGNORE INTO `conquest_vendor_items` (`npc_entry`, `item_id`, `price`, `currency`, `slot_label`, `display_order`)
SELECT 400262, `entry`, 60, 'PB', 'PVP Epique',
  CASE `InventoryType` WHEN 16 THEN 10 WHEN 9 THEN 20 WHEN 6 THEN 30 WHEN 8 THEN 40 END
FROM `item_template`
WHERE (`name` LIKE 'Field Marshal%' OR `name` LIKE 'Warlord''s%' OR `name` LIKE 'Warlord %')
  AND `class` = 4
  AND `InventoryType` IN (6, 8, 9, 16)
  AND `name` NOT LIKE '%Mount%';

-- 400263 Offset - PVP Epique (bijoux high rank)
INSERT IGNORE INTO `conquest_vendor_items` (`npc_entry`, `item_id`, `price`, `currency`, `slot_label`, `display_order`)
SELECT 400263, `entry`, 70, 'PB', 'PVP Epique',
  CASE `InventoryType` WHEN 2 THEN 10 WHEN 11 THEN 20 WHEN 12 THEN 30 END
FROM `item_template`
WHERE (`name` LIKE 'Field Marshal%' OR `name` LIKE 'Warlord''s%' OR `name` LIKE 'Warlord %'
       OR `name` LIKE 'Grand Marshal%' OR `name` LIKE 'High Warlord%')
  AND `InventoryType` IN (2, 11, 12);
