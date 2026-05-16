-- Conquest Frontline — Fix DK custom sets : AllowableClass était 1101/690 (ne contenait pas DK).
-- DK = class 6 → bit 32. On force AllowableClass = 32 (DK uniquement).

UPDATE `item_template` SET `AllowableClass` = 32 WHERE `entry` BETWEEN 80100 AND 80138;

SELECT entry, name, AllowableClass FROM item_template WHERE entry BETWEEN 80100 AND 80138 ORDER BY entry LIMIT 6;
