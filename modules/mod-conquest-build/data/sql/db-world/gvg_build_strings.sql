-- Conquest Build Module - Strings
-- Add custom strings for the module

DELETE FROM `acore_string` WHERE `entry` BETWEEN 35411 AND 35416;
INSERT INTO `acore_string` (`entry`, `content_default`) VALUES
(35411, 'Le système de construction Conquest est désactivé.'),
(35412, 'Structure construite avec succès !'),
(35413, 'Erreur lors de la construction de la structure.'),
(35414, 'Vous ne pouvez pas construire ici.'),
(35415, 'Trop proche d\'une autre structure. Distance minimum : %f mètres.'),
(35416, 'Vous avez atteint le nombre maximum de structures autorisées.');

