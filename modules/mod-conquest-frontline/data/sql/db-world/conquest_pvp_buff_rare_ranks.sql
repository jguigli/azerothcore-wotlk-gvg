-- Conquest Frontline — Buff des sets rare PvP (starter set des bots)
-- +50 stamina (sur le slot 1 si déjà stamina, sinon premier slot vide)
-- +15 résilience (premier slot vide)
--
-- Cibles :
--   Alliance : Knight-Captain (R7), Knight-Lieutenant (R5), Lieutenant Commander (R8)
--   Horde    : Blood Guard (R5), Legionnaire (R6), Champion (R7)

USE acore_world;

-- ============================================================================
-- Step 1 : ajouter +50 stamina (slot 1 si stam, sinon slots 2/3 vides)
-- ============================================================================
UPDATE `item_template` SET `stat_value1` = `stat_value1` + 50
WHERE (`name` LIKE 'Knight-Captain''s %'
    OR `name` LIKE 'Knight-Lieutenant''s %'
    OR `name` LIKE 'Lieutenant Commander''s %'
    OR `name` LIKE 'Blood Guard''s %'
    OR `name` LIKE 'Legionnaire''s %'
    OR `name` LIKE 'Champion''s %')
  AND `stat_type1` = 7;  -- déjà stamina, on l'augmente

-- Step 1b : items où stam n'est pas slot1 mais slot2 (rare)
UPDATE `item_template` SET `stat_value2` = `stat_value2` + 50
WHERE (`name` LIKE 'Knight-Captain''s %'
    OR `name` LIKE 'Knight-Lieutenant''s %'
    OR `name` LIKE 'Lieutenant Commander''s %'
    OR `name` LIKE 'Blood Guard''s %'
    OR `name` LIKE 'Legionnaire''s %'
    OR `name` LIKE 'Champion''s %')
  AND `stat_type1` != 7 AND `stat_type2` = 7;

-- ============================================================================
-- Step 2 : ajouter +15 résilience au premier slot vide (slot 3, 4, 5 ou 6)
-- ============================================================================

-- Slot 3 vide → met resi en 3
UPDATE `item_template` SET `stat_type3` = 35, `stat_value3` = 15
WHERE (`name` LIKE 'Knight-Captain''s %'
    OR `name` LIKE 'Knight-Lieutenant''s %'
    OR `name` LIKE 'Lieutenant Commander''s %'
    OR `name` LIKE 'Blood Guard''s %'
    OR `name` LIKE 'Legionnaire''s %'
    OR `name` LIKE 'Champion''s %')
  AND `stat_type3` = 0
  AND `stat_type1` != 35 AND `stat_type2` != 35;  -- pas déjà de resi

-- Slot 4 vide (si slot 3 occupé non-resi)
UPDATE `item_template` SET `stat_type4` = 35, `stat_value4` = 15
WHERE (`name` LIKE 'Knight-Captain''s %'
    OR `name` LIKE 'Knight-Lieutenant''s %'
    OR `name` LIKE 'Lieutenant Commander''s %'
    OR `name` LIKE 'Blood Guard''s %'
    OR `name` LIKE 'Legionnaire''s %'
    OR `name` LIKE 'Champion''s %')
  AND `stat_type4` = 0
  AND `stat_type1` != 35 AND `stat_type2` != 35 AND `stat_type3` != 35;

-- ============================================================================
-- Vérification
-- ============================================================================
SELECT
  CASE
    WHEN name LIKE 'Knight-Captain''s %' THEN 'Knight-Captain'
    WHEN name LIKE 'Knight-Lieutenant''s %' THEN 'Knight-Lieutenant'
    WHEN name LIKE 'Lieutenant Commander''s %' THEN 'Lieutenant Commander'
    WHEN name LIKE 'Blood Guard''s %' THEN 'Blood Guard'
    WHEN name LIKE 'Legionnaire''s %' THEN 'Legionnaire'
    WHEN name LIKE 'Champion''s %' THEN 'Champion'
  END AS rank_name,
  COUNT(*) AS items,
  SUM(CASE WHEN stat_type1 = 35 OR stat_type2 = 35 OR stat_type3 = 35 OR stat_type4 = 35 OR stat_type5 = 35 OR stat_type6 = 35 THEN 1 ELSE 0 END) AS has_resilience
FROM item_template
WHERE name LIKE 'Knight-Captain''s %' OR name LIKE 'Knight-Lieutenant''s %' OR name LIKE 'Lieutenant Commander''s %'
   OR name LIKE 'Blood Guard''s %' OR name LIKE 'Legionnaire''s %' OR name LIKE 'Champion''s %'
GROUP BY rank_name;
