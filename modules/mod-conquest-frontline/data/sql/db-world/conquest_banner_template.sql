-- Conquest Frontline — GameObject template pour la bannière de capture custom
-- Type 29 = GAMEOBJECT_TYPE_CAPTURE_POINT
-- Data0 = radius (yards) | Data12 = neutralPercent | Data16 = minTime | Data17 = maxTime

DELETE FROM `gameobject_template` WHERE `entry` = 400010;

INSERT INTO `gameobject_template`
(`entry`, `type`, `displayId`, `name`, `IconName`, `size`,
 `Data0`, `Data1`, `Data2`, `Data3`, `Data4`, `Data5`, `Data6`, `Data7`,
 `Data8`, `Data9`, `Data10`, `Data11`, `Data12`, `Data13`, `Data14`, `Data15`,
 `Data16`, `Data17`, `Data18`, `Data19`, `Data20`, `Data21`,
 `ScriptName`)
VALUES
(400010, 29, 6834, 'Bannière de Conquête', '', 2.0,
 45,    -- Data0  radius (45 yd)
 0,     -- Data1  spell
 2426,  -- Data2  worldState1 (emprunté EP Crown Guard Tower neutral state)
 2427,  -- Data3  worldState2 (active progress)
 0,     -- Data4  winEventID1
 0,     -- Data5  winEventID2
 0,     -- Data6  contestedEventID1
 0,     -- Data7  contestedEventID2
 0,     -- Data8  progressEventID1
 0,     -- Data9  progressEventID2
 0,     -- Data10 neutralEventID1
 0,     -- Data11 neutralEventID2
 30,    -- Data12 neutralPercent (30%)
 0,     -- Data13 worldstate3
 0,     -- Data14 minSuperiority (0 = pas de seuil min)
 0,     -- Data15 maxSuperiority
 30,    -- Data16 minTime (30s pour fill avec vitesse max)
 60,    -- Data17 maxTime (jauge de 0 à 60)
 1,     -- Data18 large (visible de loin)
 0,     -- Data19 highlight
 0,     -- Data20 startingValue
 0,     -- Data21 unidirectional (0 = bidirectionnel A/H)
 '');
