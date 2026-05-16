-- Conquest Frontline — Marque de Champion (Phase 5)
-- Item currency-like obtenue sur kill de joueurs en kill streak.
-- Dépensable au Forgeron des Légendes (entry 400202) pour items legendary.

DELETE FROM `item_template` WHERE `entry` = 400300;

INSERT INTO `item_template`
    (`entry`, `class`, `subclass`, `name`, `displayid`, `Quality`,
     `Flags`, `BuyCount`, `BuyPrice`, `SellPrice`,
     `InventoryType`, `AllowableClass`, `AllowableRace`,
     `ItemLevel`, `RequiredLevel`, `RequiredSkill`, `RequiredSkillRank`,
     `maxcount`, `stackable`, `ContainerSlots`, `bonding`, `description`)
VALUES
    (400300, 15, 0, 'Marque de Champion', 18491, 3,
     0, 1, 0, 0,
     0, -1, -1,
     80, 1, 0, 0,
     0, 200, 0, 1,
     'Une marque de bravoure forgée dans le sang. Échangeable au Forgeron des Légendes.');
