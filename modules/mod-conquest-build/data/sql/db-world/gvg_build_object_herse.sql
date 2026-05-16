-- Conquest Build Module - Herse GameObject Configuration
-- Crée le GameObject 400026 (Herse Conquest) basé sur 176966

-- =====================================================
-- Création de la HERSE Conquest (GameObject 400026)
-- Basé sur l'entry 176966
-- =====================================================

-- Supprimer si existe
DELETE FROM `gameobject_template` WHERE `entry` = 400026;

-- Créer la herse basé sur le game object 176966
INSERT INTO `gameobject_template` 
SELECT 
    400026,                                 -- Entry de la herse Conquest
    type,                                   -- Type du game object source
    displayId,                              -- DisplayId du game object 176966
    'Herse Conquest',                            -- Nom
    IconName,
    castBarCaption,
    unk1,
    size,
    Data0, Data1, Data2, Data3, Data4, Data5, Data6, Data7, Data8, Data9, Data10,
    Data11, Data12, Data13, Data14, Data15, Data16, Data17, Data18, Data19, Data20,
    Data21, Data22, Data23,
    AIName,
    'go_conquest_build_gate',                   -- Script custom pour la herse
    VerifiedBuild
FROM `gameobject_template` 
WHERE `entry` = 176966;

-- Configuration de la herse comme Door avec contrôle de guilde
UPDATE `gameobject_template` SET 
    `type` = 0,                             -- GAMEOBJECT_TYPE_DOOR
    `ScriptName` = 'go_conquest_build_gate',     -- Script pour contrôle guilde
    `Data0` = 0,                            -- startOpen (0 = fermée par défaut)
    `Data1` = 0,                            -- open
    `Data2` = 0,                            -- autoClose (0 = ne ferme pas auto)
    `Data3` = 0,                            -- noDamageImmune
    `Data4` = 0,                            -- openTextID
    `Data5` = 0,                            -- closeTextID
    `Data6` = 0                             -- ignoredByPathing
WHERE `entry` = 400026;

-- Vérification
SELECT CONCAT('GameObject Herse Conquest 400026 créé avec succès!') AS Result;
SELECT entry, name, type, displayId, Data0, ScriptName FROM gameobject_template WHERE entry = 400026;

