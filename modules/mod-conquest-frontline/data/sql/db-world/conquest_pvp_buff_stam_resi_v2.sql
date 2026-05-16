-- Conquest Frontline — Ajustement balance PvP v2
-- Réduction de la résilience de 60 → 15 sur tous les items précédemment buffés.
-- Identifie via stat_type=35 ET stat_value=60 (signature unique du buff v1).
-- Stamina (+50) reste inchangée.

UPDATE `item_template` SET `stat_value1` = 15 WHERE `stat_type1` = 35 AND `stat_value1` = 60;
UPDATE `item_template` SET `stat_value2` = 15 WHERE `stat_type2` = 35 AND `stat_value2` = 60;
UPDATE `item_template` SET `stat_value3` = 15 WHERE `stat_type3` = 35 AND `stat_value3` = 60;
UPDATE `item_template` SET `stat_value4` = 15 WHERE `stat_type4` = 35 AND `stat_value4` = 60;
UPDATE `item_template` SET `stat_value5` = 15 WHERE `stat_type5` = 35 AND `stat_value5` = 60;
UPDATE `item_template` SET `stat_value6` = 15 WHERE `stat_type6` = 35 AND `stat_value6` = 60;
UPDATE `item_template` SET `stat_value7` = 15 WHERE `stat_type7` = 35 AND `stat_value7` = 60;
UPDATE `item_template` SET `stat_value8` = 15 WHERE `stat_type8` = 35 AND `stat_value8` = 60;

SELECT COUNT(*) AS items_updated FROM item_template
WHERE (stat_type1 = 35 AND stat_value1 = 15) OR (stat_type2 = 35 AND stat_value2 = 15)
   OR (stat_type3 = 35 AND stat_value3 = 15) OR (stat_type4 = 35 AND stat_value4 = 15)
   OR (stat_type5 = 35 AND stat_value5 = 15) OR (stat_type6 = 35 AND stat_value6 = 15)
   OR (stat_type7 = 35 AND stat_value7 = 15) OR (stat_type8 = 35 AND stat_value8 = 15);
