-- Conquest Frontline — Items currency visibles dans l'inventaire
-- 400301 : Marque de Conquête (zone capture reward)
-- 400302 : Marque de Bataille (PVP kill reward) — future-proof si on veut la migrer aussi

DELETE FROM `item_template` WHERE `entry` IN (400301, 400302);

INSERT INTO `item_template`
    (`entry`, `class`, `subclass`, `name`, `displayid`, `Quality`,
     `Flags`, `BuyCount`, `BuyPrice`, `SellPrice`,
     `InventoryType`, `AllowableClass`, `AllowableRace`,
     `ItemLevel`, `RequiredLevel`, `RequiredSkill`, `RequiredSkillRank`,
     `maxcount`, `stackable`, `ContainerSlots`, `bonding`, `description`)
VALUES
    (400301, 15, 0, 'Marque de Conquête', 18495, 3,
     0, 1, 0, 0,
     0, -1, -1,
     60, 1, 0, 0,
     0, 200, 0, 1,
     'Symbole de domination territoriale. Échangeable au Quartier-maître Conquête.'),
    (400302, 15, 0, 'Marque de Bataille', 18494, 3,
     0, 1, 0, 0,
     0, -1, -1,
     60, 1, 0, 0,
     0, 200, 0, 1,
     'Témoignage de victoires sur le champ de bataille. Échangeable au Sergent d''Armes.');
