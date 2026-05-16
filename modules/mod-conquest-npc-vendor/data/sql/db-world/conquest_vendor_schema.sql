-- ============================================================================
-- Conquest NPC Vendor — Schéma DB pour vendors lvl 60 (PvP rare/epic, PvE T1-T3,
-- bijoux, armes, consommables, divers).
-- ============================================================================

-- Catalogue des NPCs vendor avec leur catégorie sémantique
DROP TABLE IF EXISTS `conquest_vendor_npc`;
CREATE TABLE `conquest_vendor_npc` (
  `npc_entry` INT UNSIGNED NOT NULL PRIMARY KEY,
  `name`      VARCHAR(64)  NOT NULL,
  `category`  VARCHAR(32)  NOT NULL,
  `description` VARCHAR(255) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Items vendus par chaque NPC. Currency : PB = Points de Bataille, PC = Points
-- de Conquête, GOLD = pièces vanilla.
DROP TABLE IF EXISTS `conquest_vendor_items`;
CREATE TABLE `conquest_vendor_items` (
  `npc_entry`     INT UNSIGNED NOT NULL,
  `item_id`       INT UNSIGNED NOT NULL,
  `price`         INT UNSIGNED NOT NULL DEFAULT 1,
  `currency`      ENUM('PB','PC','GOLD') NOT NULL DEFAULT 'PB',
  `slot_label`    VARCHAR(64) DEFAULT NULL,
  `display_order` INT UNSIGNED DEFAULT 100,
  PRIMARY KEY (`npc_entry`, `item_id`),
  INDEX `idx_npc_order` (`npc_entry`, `display_order`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- ============================================================================
-- Catalogue des 4 NPCs consolides (entries 400260-400263)
-- Chaque NPC porte toutes les tiers/raretes via gossip sections (slot_label) :
--   T1 / T2 / T2.5 / T3 / PVP Rare / PVP Epique
-- ============================================================================
INSERT IGNORE INTO `conquest_vendor_npc` (`npc_entry`, `name`, `category`, `description`) VALUES
(400260, 'Armes',         'weapons',     'Toutes armes PvE T1-T3 + PvP rare/epique'),
(400261, 'Armures',       'armor',       'Sets armure tete/torse/epaules/jambes/mains, toutes tiers + PvP'),
(400262, 'Offpart',       'offpart',     'Capes, brassards, ceintures, bottes toutes tiers + PvP'),
(400263, 'Offset',        'offset',      'Bijoux (anneaux, cous, trinkets) PvE + PvP'),
(400264, 'Legendes',      'legendary',   'Items legendaires vanilla + epics iconiques'),
(400265, 'Consommables',  'consumables', 'Potions, elixirs, flasques, nourriture'),
(400266, 'Reactifs',      'reagents',    'Bandages, pierres, huiles, runes mage');
