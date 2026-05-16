-- ============================================================================
-- Conquest Vendor Items — Legendes (400264) / Consommables (400265) / Reactifs (400266)
-- ============================================================================

DELETE FROM `conquest_vendor_items` WHERE `npc_entry` IN (400264, 400265, 400266);

-- ----------------------------------------------------------------------------
-- 400264 LEGENDES (PC) - items legendaires quality=5 + epics standout (PC)
-- Section : "Legendaires"     - 1000 PC chacun
-- Section : "Epics Iconiques" - 500 PC chacun
-- ----------------------------------------------------------------------------

-- Legendaires Vanilla (quality = 5, max ilvl)
INSERT IGNORE INTO `conquest_vendor_items` (`npc_entry`, `item_id`, `price`, `currency`, `slot_label`, `display_order`)
SELECT 400264, `entry`, 1000, 'PC', 'Legendaires', 10 + `subclass`
FROM `item_template`
WHERE `quality` = 5
  AND `class` = 2
  AND `RequiredLevel` >= 60
  AND `ItemLevel` >= 75;

-- Epics standout (vanilla iconiques : Vis'kag, Bonereaver, Maladath, etc.)
-- Filtres : quality=4 ilvl 71-85 + noms iconiques. Prix 500 PC.
INSERT IGNORE INTO `conquest_vendor_items` (`npc_entry`, `item_id`, `price`, `currency`, `slot_label`, `display_order`)
SELECT 400264, `entry`, 500, 'PC', 'Epics Iconiques', 10 + `subclass`
FROM `item_template`
WHERE `quality` = 4
  AND `class` = 2
  AND `RequiredLevel` = 60
  AND `ItemLevel` BETWEEN 71 AND 85
  AND (
       `name` LIKE 'Vis''kag%' OR `name` LIKE 'Bonereaver%' OR
       `name` LIKE 'Maladath%' OR `name` LIKE 'Ashkandi%' OR
       `name` LIKE 'Crul''shorukh%' OR `name` LIKE 'Death''s Sting%' OR
       `name` LIKE 'Brutality Blade%' OR `name` LIKE 'Mish''undare%' OR
       `name` LIKE 'Doom''s Edge%' OR `name` LIKE 'Hand of Justice%' OR
       `name` LIKE 'Spinal Reaper%' OR `name` LIKE 'Drake Talon%' OR
       `name` LIKE 'Lok''amir%' OR `name` LIKE 'Chromatically Tempered%'
      );

-- ----------------------------------------------------------------------------
-- 400265 CONSOMMABLES (gold) - potions/elixirs/flasques/nourriture
-- Sections : "Potions" / "Elixirs" / "Flasques" / "Nourriture"
-- Prix faible en gold (vendor classique)
-- ----------------------------------------------------------------------------

-- Potions (BuyPrice gold via vendor system natif — on met le prix vendor a 0
-- pour que ce soit le BuyPrice du item_template qui s'applique).
-- Section "Potions" : potions de soin/mana/regen.
INSERT IGNORE INTO `conquest_vendor_items` (`npc_entry`, `item_id`, `price`, `currency`, `slot_label`, `display_order`) VALUES
(400265, 13446, 0, 'GOLD', 'Potions',   10), -- Major Healing Potion
(400265, 13444, 0, 'GOLD', 'Potions',   20), -- Major Mana Potion
(400265, 18253, 0, 'GOLD', 'Potions',   30), -- Major Rejuvenation Potion
(400265, 13442, 0, 'GOLD', 'Potions',   40), -- Mighty Rage Potion
(400265, 13455, 0, 'GOLD', 'Potions',   50), -- Greater Stoneshield Potion
(400265,  9030, 0, 'GOLD', 'Potions',   60), -- Restorative Potion
(400265,  3387, 0, 'GOLD', 'Potions',   70), -- Limited Invulnerability Potion
(400265,  6149, 0, 'GOLD', 'Potions',   80), -- Greater Mana Potion
(400265,  3928, 0, 'GOLD', 'Potions',   90); -- Superior Healing Potion

-- Section "Elixirs"
INSERT IGNORE INTO `conquest_vendor_items` (`npc_entry`, `item_id`, `price`, `currency`, `slot_label`, `display_order`) VALUES
(400265, 13452, 0, 'GOLD', 'Elixirs',   10), -- Elixir of the Mongoose
(400265, 13453, 0, 'GOLD', 'Elixirs',   20), -- Elixir of Brute Force
(400265, 13454, 0, 'GOLD', 'Elixirs',   30), -- Greater Arcane Elixir
(400265, 13447, 0, 'GOLD', 'Elixirs',   40), -- Elixir of the Sages
(400265,  9224, 0, 'GOLD', 'Elixirs',   50), -- Elixir of Demonslaying
(400265,  9088, 0, 'GOLD', 'Elixirs',   60), -- Gift of Arthas
(400265, 17708, 0, 'GOLD', 'Elixirs',   70), -- Elixir of Frost Power
(400265, 21546, 0, 'GOLD', 'Elixirs',   80); -- Elixir of Greater Firepower

-- Section "Flasques" (BuyPrice elevee — laissons vendor system gerer)
INSERT IGNORE INTO `conquest_vendor_items` (`npc_entry`, `item_id`, `price`, `currency`, `slot_label`, `display_order`) VALUES
(400265, 13510, 0, 'GOLD', 'Flasques',  10), -- Flask of the Titans
(400265, 13511, 0, 'GOLD', 'Flasques',  20), -- Flask of Distilled Wisdom
(400265, 13512, 0, 'GOLD', 'Flasques',  30), -- Flask of Supreme Power
(400265, 13513, 0, 'GOLD', 'Flasques',  40); -- Flask of Chromatic Resistance

-- Section "Nourriture"
INSERT IGNORE INTO `conquest_vendor_items` (`npc_entry`, `item_id`, `price`, `currency`, `slot_label`, `display_order`) VALUES
(400265, 18045, 0, 'GOLD', 'Nourriture', 10), -- Spotted Yellowtail
(400265, 21072, 0, 'GOLD', 'Nourriture', 20), -- Smoked Desert Dumplings
(400265, 21023, 0, 'GOLD', 'Nourriture', 30), -- Dirge's Kickin' Chimaerok Chops
(400265, 13935, 0, 'GOLD', 'Nourriture', 40), -- Tender Wolf Steak
(400265, 13931, 0, 'GOLD', 'Nourriture', 50), -- Nightfin Soup
(400265, 21031, 0, 'GOLD', 'Nourriture', 60), -- Runn Tum Tuber Surprise
(400265,  8766, 0, 'GOLD', 'Nourriture', 70); -- Morning Glory Dew

-- ----------------------------------------------------------------------------
-- 400266 REACTIFS (gold) - bandages/pierres/huiles/runes
-- Sections : "Bandages" / "Pierres" / "Huiles" / "Runes Mage"
-- ----------------------------------------------------------------------------

-- Bandages
INSERT IGNORE INTO `conquest_vendor_items` (`npc_entry`, `item_id`, `price`, `currency`, `slot_label`, `display_order`) VALUES
(400266, 14530, 0, 'GOLD', 'Bandages',  10), -- Heavy Runecloth Bandage
(400266, 14529, 0, 'GOLD', 'Bandages',  20), -- Runecloth Bandage
(400266,  8545, 0, 'GOLD', 'Bandages',  30), -- Heavy Mageweave Bandage
(400266,  8544, 0, 'GOLD', 'Bandages',  40); -- Mageweave Bandage

-- Pierres aiguiseuses / weight stones
INSERT IGNORE INTO `conquest_vendor_items` (`npc_entry`, `item_id`, `price`, `currency`, `slot_label`, `display_order`) VALUES
(400266, 18262, 0, 'GOLD', 'Pierres',   10), -- Elemental Sharpening Stone
(400266, 12404, 0, 'GOLD', 'Pierres',   20), -- Dense Sharpening Stone
(400266, 12643, 0, 'GOLD', 'Pierres',   30), -- Dense Weightstone
(400266,  7964, 0, 'GOLD', 'Pierres',   40); -- Solid Sharpening Stone

-- Huiles (mana/wizard oil)
INSERT IGNORE INTO `conquest_vendor_items` (`npc_entry`, `item_id`, `price`, `currency`, `slot_label`, `display_order`) VALUES
(400266, 20748, 0, 'GOLD', 'Huiles',    10), -- Brilliant Mana Oil
(400266, 20749, 0, 'GOLD', 'Huiles',    20), -- Brilliant Wizard Oil
(400266, 20747, 0, 'GOLD', 'Huiles',    30), -- Lesser Mana Oil
(400266, 20746, 0, 'GOLD', 'Huiles',    40); -- Lesser Wizard Oil

-- Runes Mage (reagents)
INSERT IGNORE INTO `conquest_vendor_items` (`npc_entry`, `item_id`, `price`, `currency`, `slot_label`, `display_order`) VALUES
(400266, 17031, 0, 'GOLD', 'Runes Mage', 10), -- Rune of Teleportation
(400266, 17032, 0, 'GOLD', 'Runes Mage', 20), -- Rune of Portals
(400266, 17056, 0, 'GOLD', 'Runes Mage', 30), -- Light Feather (Levitate)
(400266, 17057, 0, 'GOLD', 'Runes Mage', 40), -- Shiny Fish Scales (Water Walking)
(400266, 17058, 0, 'GOLD', 'Runes Mage', 50); -- Fish Oil (Underwater Breathing)
