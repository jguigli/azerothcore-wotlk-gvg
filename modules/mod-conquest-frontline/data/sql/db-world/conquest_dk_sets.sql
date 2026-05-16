-- ============================================================================
-- Conquest DK PvP Sets — sets custom rare et épique pour Death Knight
-- ============================================================================
--
-- 4 sets × 8 pièces armor + 1 arme 2H = 36 items custom
--
-- Visuels :
--   Set RARE  → "Savage Saronite Battlegear Recolor" (transmog set 1350)
--   Set ÉPIQUE → "Scourgelord's Frigid Battleplate" (transmog set 969 + T10 25-Heroic)
--
-- Plages d'entries :
--   80100-80108 : Rare Alliance "Knight-Lieutenant's Dreadplate ..."
--   80110-80118 : Rare Horde    "Blood Guard's Dreadplate ..."
--   80120-80128 : Epic Alliance "Lieutenant Commander's Dreadplate ..."
--   80130-80138 : Epic Horde    "Champion's Dreadplate ..."
--
-- AllowableClass : 32 (DK uniquement, pas de cross avec Warrior)
-- AllowableRace  : 1101 Alliance (1+4+8+64+1024) / 690 Horde (2+16+32+128+512)
-- Quality        : 3 (rare) ou 4 (epic)
-- RequiredLevel  : 60
-- ============================================================================

-- ====== CLEANUP avant insertion ======
DELETE FROM `item_template` WHERE `entry` BETWEEN 80100 AND 80138;



-- ============================================================================
-- SET RARE ALLIANCE — Knight-Lieutenant's Dreadplate (visuel Savage Saronite)
-- ItemLevel 71 (R10-R12 equivalent), Quality 3
-- ============================================================================

INSERT INTO `item_template`
(`entry`, `class`, `subclass`, `name`, `displayid`, `Quality`, `Flags`, `BuyPrice`, `SellPrice`,
 `InventoryType`, `AllowableClass`, `AllowableRace`, `ItemLevel`, `RequiredLevel`,
 `armor`, `bonding`, `Material`, `Sheath`, `itemset`, `MaxDurability`,
 `stat_type1`, `stat_value1`, `stat_type2`, `stat_value2`, `stat_type3`, `stat_value3`,
 `stat_type4`, `stat_value4`, `stat_type5`, `stat_value5`,
 `ScriptName`, `VerifiedBuild`)
VALUES
-- Head (Slot 0, InvType 1) — displayId 52302 Savage Saronite Skullshield
(80100, 4, 4, 'Knight-Lieutenant''s Dreadplate Skullshield', 52302, 3, 0, 0, 0,
 1, 32, 1101, 71, 60,
 750, 1, 8, 0, 0, 100,
 4, 32, 7, 60, 35, 40, 0, 0, 0, 0,
 '', 0),
-- Shoulders (Slot 2, InvType 3) — displayId 51604
(80101, 4, 4, 'Knight-Lieutenant''s Dreadplate Pauldrons', 51604, 3, 0, 0, 0,
 3, 32, 1101, 71, 60,
 650, 1, 8, 0, 0, 80,
 4, 26, 7, 50, 35, 35, 0, 0, 0, 0,
 '', 0),
-- Chest (Slot 4, InvType 5) — displayId 50991 Savage Saronite Hauberk
(80102, 4, 4, 'Knight-Lieutenant''s Dreadplate Hauberk', 50991, 3, 0, 0, 0,
 5, 32, 1101, 71, 60,
 850, 1, 8, 0, 0, 120,
 4, 35, 7, 70, 35, 50, 0, 0, 0, 0,
 '', 0),
-- Belt (Slot 5, InvType 6) — displayId 53005 Savage Saronite Waistguard
(80103, 4, 4, 'Knight-Lieutenant''s Dreadplate Waistguard', 53005, 3, 0, 0, 0,
 6, 32, 1101, 71, 60,
 550, 1, 8, 0, 0, 60,
 4, 22, 7, 45, 35, 30, 0, 0, 0, 0,
 '', 0),
-- Legs (Slot 6, InvType 7) — displayId 53003 Savage Saronite Legplates
(80104, 4, 4, 'Knight-Lieutenant''s Dreadplate Legplates', 53003, 3, 0, 0, 0,
 7, 32, 1101, 71, 60,
 800, 1, 8, 0, 0, 100,
 4, 30, 7, 65, 35, 45, 0, 0, 0, 0,
 '', 0),
-- Feet (Slot 7, InvType 8) — displayId 53004 Savage Saronite Walkers
(80105, 4, 4, 'Knight-Lieutenant''s Dreadplate Walkers', 53004, 3, 0, 0, 0,
 8, 32, 1101, 71, 60,
 600, 1, 8, 0, 0, 80,
 4, 24, 7, 50, 35, 35, 0, 0, 0, 0,
 '', 0),
-- Wrists (Slot 8, InvType 9) — displayId 51710 Savage Saronite Bracers
(80106, 4, 4, 'Knight-Lieutenant''s Dreadplate Bracers', 51710, 3, 0, 0, 0,
 9, 32, 1101, 71, 60,
 450, 1, 8, 0, 0, 50,
 4, 18, 7, 40, 35, 25, 0, 0, 0, 0,
 '', 0),
-- Hands (Slot 9, InvType 10) — displayId 53110 Savage Saronite Gauntlets
(80107, 4, 4, 'Knight-Lieutenant''s Dreadplate Gauntlets', 53110, 3, 0, 0, 0,
 10, 32, 1101, 71, 60,
 600, 1, 8, 0, 0, 80,
 4, 24, 7, 50, 35, 35, 0, 0, 0, 0,
 '', 0),
-- 2H Sword (Slot 15, InvType 17, class 2 weapon, subclass 8 = 2H Sword)
(80108, 2, 8, 'Knight-Lieutenant''s Greatsword', 32837, 3, 0, 0, 0,
 17, 32, 1101, 71, 60,
 0, 1, 0, 3, 0, 100,
 4, 30, 7, 60, 35, 30, 0, 0, 0, 0,
 '', 0);


-- ============================================================================
-- SET RARE HORDE — Blood Guard's Dreadplate (mêmes visuels, nom différent)
-- ============================================================================

INSERT INTO `item_template`
(`entry`, `class`, `subclass`, `name`, `displayid`, `Quality`, `Flags`, `BuyPrice`, `SellPrice`,
 `InventoryType`, `AllowableClass`, `AllowableRace`, `ItemLevel`, `RequiredLevel`,
 `armor`, `bonding`, `Material`, `Sheath`, `itemset`, `MaxDurability`,
 `stat_type1`, `stat_value1`, `stat_type2`, `stat_value2`, `stat_type3`, `stat_value3`,
 `stat_type4`, `stat_value4`, `stat_type5`, `stat_value5`,
 `ScriptName`, `VerifiedBuild`)
VALUES
(80110, 4, 4, 'Blood Guard''s Dreadplate Skullshield', 52302, 3, 0, 0, 0,
 1, 32, 690, 71, 60, 750, 1, 8, 0, 0, 100,
 4, 32, 7, 60, 35, 40, 0, 0, 0, 0, '', 0),
(80111, 4, 4, 'Blood Guard''s Dreadplate Pauldrons', 51604, 3, 0, 0, 0,
 3, 32, 690, 71, 60, 650, 1, 8, 0, 0, 80,
 4, 26, 7, 50, 35, 35, 0, 0, 0, 0, '', 0),
(80112, 4, 4, 'Blood Guard''s Dreadplate Hauberk', 50991, 3, 0, 0, 0,
 5, 32, 690, 71, 60, 850, 1, 8, 0, 0, 120,
 4, 35, 7, 70, 35, 50, 0, 0, 0, 0, '', 0),
(80113, 4, 4, 'Blood Guard''s Dreadplate Waistguard', 53005, 3, 0, 0, 0,
 6, 32, 690, 71, 60, 550, 1, 8, 0, 0, 60,
 4, 22, 7, 45, 35, 30, 0, 0, 0, 0, '', 0),
(80114, 4, 4, 'Blood Guard''s Dreadplate Legplates', 53003, 3, 0, 0, 0,
 7, 32, 690, 71, 60, 800, 1, 8, 0, 0, 100,
 4, 30, 7, 65, 35, 45, 0, 0, 0, 0, '', 0),
(80115, 4, 4, 'Blood Guard''s Dreadplate Walkers', 53004, 3, 0, 0, 0,
 8, 32, 690, 71, 60, 600, 1, 8, 0, 0, 80,
 4, 24, 7, 50, 35, 35, 0, 0, 0, 0, '', 0),
(80116, 4, 4, 'Blood Guard''s Dreadplate Bracers', 51710, 3, 0, 0, 0,
 9, 32, 690, 71, 60, 450, 1, 8, 0, 0, 50,
 4, 18, 7, 40, 35, 25, 0, 0, 0, 0, '', 0),
(80117, 4, 4, 'Blood Guard''s Dreadplate Gauntlets', 53110, 3, 0, 0, 0,
 10, 32, 690, 71, 60, 600, 1, 8, 0, 0, 80,
 4, 24, 7, 50, 35, 35, 0, 0, 0, 0, '', 0),
(80118, 2, 8, 'Blood Guard''s Greatsword', 32837, 3, 0, 0, 0,
 17, 32, 690, 71, 60, 0, 1, 0, 3, 0, 100,
 4, 30, 7, 60, 35, 30, 0, 0, 0, 0, '', 0);


-- ============================================================================
-- SET EPIC ALLIANCE — Lieutenant Commander's Dreadplate (visuel Sanctified Scourgelord T10-H)
-- ItemLevel 92 (R13-R14 equivalent), Quality 4
-- ============================================================================

INSERT INTO `item_template`
(`entry`, `class`, `subclass`, `name`, `displayid`, `Quality`, `Flags`, `BuyPrice`, `SellPrice`,
 `InventoryType`, `AllowableClass`, `AllowableRace`, `ItemLevel`, `RequiredLevel`,
 `armor`, `bonding`, `Material`, `Sheath`, `itemset`, `MaxDurability`,
 `stat_type1`, `stat_value1`, `stat_type2`, `stat_value2`, `stat_type3`, `stat_value3`,
 `stat_type4`, `stat_value4`, `stat_type5`, `stat_value5`,
 `ScriptName`, `VerifiedBuild`)
VALUES
-- Head — displayId 64587 Sanctified Scourgelord Helmet (Heroic 25)
(80120, 4, 4, 'Lieutenant Commander''s Dreadplate Helmet', 64587, 4, 0, 0, 0,
 1, 32, 1101, 92, 60, 1100, 1, 8, 0, 0, 100,
 4, 55, 7, 110, 35, 70, 32, 30, 0, 0, '', 0),
-- Shoulders — displayId 64707 Sanctified Scourgelord Pauldrons
(80121, 4, 4, 'Lieutenant Commander''s Dreadplate Pauldrons', 64707, 4, 0, 0, 0,
 3, 32, 1101, 92, 60, 950, 1, 8, 0, 0, 80,
 4, 45, 7, 95, 35, 60, 32, 25, 0, 0, '', 0),
-- Chest — displayId 64584 Sanctified Scourgelord Battleplate
(80122, 4, 4, 'Lieutenant Commander''s Dreadplate Battleplate', 64584, 4, 0, 0, 0,
 5, 32, 1101, 92, 60, 1250, 1, 8, 0, 0, 120,
 4, 60, 7, 120, 35, 80, 32, 35, 0, 0, '', 0),
-- Belt — displayId 64798 Coldwraith Links
(80123, 4, 4, 'Lieutenant Commander''s Dreadplate Girdle', 64798, 4, 0, 0, 0,
 6, 32, 1101, 92, 60, 800, 1, 8, 0, 0, 60,
 4, 38, 7, 80, 35, 50, 32, 20, 0, 0, '', 0),
-- Legs — displayId 64588 Sanctified Scourgelord Legplates
(80124, 4, 4, 'Lieutenant Commander''s Dreadplate Legguards', 64588, 4, 0, 0, 0,
 7, 32, 1101, 92, 60, 1150, 1, 8, 0, 0, 100,
 4, 55, 7, 110, 35, 70, 32, 30, 0, 0, '', 0),
-- Feet — displayId 64797 Blood-Soaked Saronite Stompers
(80125, 4, 4, 'Lieutenant Commander''s Dreadplate Sabatons', 64797, 4, 0, 0, 0,
 8, 32, 1101, 92, 60, 900, 1, 8, 0, 0, 80,
 4, 42, 7, 85, 35, 55, 32, 25, 0, 0, '', 0),
-- Wrists — displayId 64799 Bracers of Dark Reckoning
(80126, 4, 4, 'Lieutenant Commander''s Dreadplate Bracers', 64799, 4, 0, 0, 0,
 9, 32, 1101, 92, 60, 700, 1, 8, 0, 0, 50,
 4, 32, 7, 70, 35, 45, 0, 0, 0, 0, '', 0),
-- Hands — displayId 64585 Sanctified Scourgelord Gauntlets
(80127, 4, 4, 'Lieutenant Commander''s Dreadplate Gauntlets', 64585, 4, 0, 0, 0,
 10, 32, 1101, 92, 60, 900, 1, 8, 0, 0, 80,
 4, 42, 7, 85, 35, 55, 32, 25, 0, 0, '', 0),
-- 2H Sword Epic
(80128, 2, 8, 'Lieutenant Commander''s Reaper', 50223, 4, 0, 0, 0,
 17, 32, 1101, 92, 60, 0, 1, 0, 3, 0, 100,
 4, 55, 7, 100, 35, 50, 32, 40, 0, 0, '', 0);


-- ============================================================================
-- SET EPIC HORDE — Champion's Dreadplate (mêmes visuels)
-- ============================================================================

INSERT INTO `item_template`
(`entry`, `class`, `subclass`, `name`, `displayid`, `Quality`, `Flags`, `BuyPrice`, `SellPrice`,
 `InventoryType`, `AllowableClass`, `AllowableRace`, `ItemLevel`, `RequiredLevel`,
 `armor`, `bonding`, `Material`, `Sheath`, `itemset`, `MaxDurability`,
 `stat_type1`, `stat_value1`, `stat_type2`, `stat_value2`, `stat_type3`, `stat_value3`,
 `stat_type4`, `stat_value4`, `stat_type5`, `stat_value5`,
 `ScriptName`, `VerifiedBuild`)
VALUES
(80130, 4, 4, 'Champion''s Dreadplate Helmet', 64587, 4, 0, 0, 0,
 1, 32, 690, 92, 60, 1100, 1, 8, 0, 0, 100,
 4, 55, 7, 110, 35, 70, 32, 30, 0, 0, '', 0),
(80131, 4, 4, 'Champion''s Dreadplate Pauldrons', 64707, 4, 0, 0, 0,
 3, 32, 690, 92, 60, 950, 1, 8, 0, 0, 80,
 4, 45, 7, 95, 35, 60, 32, 25, 0, 0, '', 0),
(80132, 4, 4, 'Champion''s Dreadplate Battleplate', 64584, 4, 0, 0, 0,
 5, 32, 690, 92, 60, 1250, 1, 8, 0, 0, 120,
 4, 60, 7, 120, 35, 80, 32, 35, 0, 0, '', 0),
(80133, 4, 4, 'Champion''s Dreadplate Girdle', 64798, 4, 0, 0, 0,
 6, 32, 690, 92, 60, 800, 1, 8, 0, 0, 60,
 4, 38, 7, 80, 35, 50, 32, 20, 0, 0, '', 0),
(80134, 4, 4, 'Champion''s Dreadplate Legguards', 64588, 4, 0, 0, 0,
 7, 32, 690, 92, 60, 1150, 1, 8, 0, 0, 100,
 4, 55, 7, 110, 35, 70, 32, 30, 0, 0, '', 0),
(80135, 4, 4, 'Champion''s Dreadplate Sabatons', 64797, 4, 0, 0, 0,
 8, 32, 690, 92, 60, 900, 1, 8, 0, 0, 80,
 4, 42, 7, 85, 35, 55, 32, 25, 0, 0, '', 0),
(80136, 4, 4, 'Champion''s Dreadplate Bracers', 64799, 4, 0, 0, 0,
 9, 32, 690, 92, 60, 700, 1, 8, 0, 0, 50,
 4, 32, 7, 70, 35, 45, 0, 0, 0, 0, '', 0),
(80137, 4, 4, 'Champion''s Dreadplate Gauntlets', 64585, 4, 0, 0, 0,
 10, 32, 690, 92, 60, 900, 1, 8, 0, 0, 80,
 4, 42, 7, 85, 35, 55, 32, 25, 0, 0, '', 0),
(80138, 2, 8, 'Champion''s Reaper', 50223, 4, 0, 0, 0,
 17, 32, 690, 92, 60, 0, 1, 0, 3, 0, 100,
 4, 55, 7, 100, 35, 50, 32, 40, 0, 0, '', 0);

-- ============================================================================
-- Récap stats par tier
--   Rare  : Strength ~25, Stamina ~55, Resilience ~35 (par pièce moyenne)
--   Epic  : Strength ~45, Stamina ~95, Resilience ~60, Crit ~25 (par pièce moyenne)
--
-- stat_type code reference :
--   4 = Strength
--   7 = Stamina
--   32 = Critical Strike Rating
--   35 = Resilience Rating
--   36 = Haste Rating
--   44 = Armor Penetration Rating
-- ============================================================================
