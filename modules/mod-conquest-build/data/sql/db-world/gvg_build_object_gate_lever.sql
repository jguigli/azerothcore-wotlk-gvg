-- Conquest Build Module - Gate Lever GameObject
-- Créer un levier (400001) pour contrôler la herse Conquest

-- Supprimer si existe
DELETE FROM `gameobject_template` WHERE `entry` = 400001;

-- Créer le levier basé sur 17156 (Door Lever)
INSERT INTO `gameobject_template` 
SELECT 
    400001,                                 -- Entry du levier Conquest
    type,                                   -- Type 1 (Button)
    displayId,                              -- DisplayId 295 (levier)
    'Levier de herse',                  -- Nom
    IconName,
    castBarCaption,
    unk1,
    size,
    Data0, Data1, Data2, Data3, Data4, Data5, Data6, Data7, Data8, Data9, Data10,
    Data11, Data12, Data13, Data14, Data15, Data16, Data17, Data18, Data19, Data20,
    Data21, Data22, Data23,
    AIName,
    'go_conquest_build_gate_lever',              -- Script custom pour le levier
    VerifiedBuild
FROM `gameobject_template` 
WHERE `entry` = 17156;

-- Vérification
SELECT CONCAT('Levier Conquest 400001 créé.') AS Result;
SELECT entry, name, type, displayId, ScriptName FROM gameobject_template WHERE entry = 400001;

