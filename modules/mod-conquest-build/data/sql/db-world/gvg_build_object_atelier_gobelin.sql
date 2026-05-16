-- Conquest Build Module - Atelier Gobelin GameObject Configuration
-- Crée le GameObject 400027 (Atelier Gobelin) basé sur 192030

-- =====================================================
-- Création de l'ATELIER GOBELIN (GameObject 400027)
-- Basé sur l'entry 192030
-- =====================================================

-- Supprimer si existe
DELETE FROM `gameobject_template` WHERE `entry` = 400027;

-- Créer l'atelier gobelin basé sur le game object 192030
INSERT INTO `gameobject_template` 
SELECT 
    400027,                                 -- Entry de l'atelier gobelin Conquest
    type,                                   -- Type du game object source
    displayId,                              -- DisplayId du game object 192030
    'Atelier gobelin',                      -- Nom
    IconName,
    castBarCaption,
    unk1,
    size,
    Data0, Data1, Data2, Data3, Data4, Data5, Data6, Data7, Data8, Data9, Data10,
    Data11, Data12, Data13, Data14, Data15, Data16, Data17, Data18, Data19, Data20,
    Data21, Data22, Data23,
    AIName,
    'go_conquest_build_structure',              -- Script custom pour l'atelier gobelin
    VerifiedBuild
FROM `gameobject_template` 
WHERE `entry` = 192030;

-- Configuration de l'atelier gobelin comme destructible
UPDATE `gameobject_template` SET 
    `type` = 33,                            -- GAMEOBJECT_TYPE_DESTRUCTIBLE_BUILDING
    `ScriptName` = 'go_conquest_build_structure',-- Script pour gérer la destruction
    `Data0` = 300000,                       -- IntactNumHits (300,000 PV)
    `Data1` = 0,                            -- CreditProxyCreature (0 = unused)
    `Data2` = 0,                            -- unused
    `Data3` = 0,                            -- DamagedDisplayId (0 = use same model when damaged)
    `Data4` = 0,                            -- DestroyedDisplayId (0 = use same model when destroyed)
    `Data5` = 0,                            -- DamagedNumHits (0 = go directly to destroyed state)
    `Data6` = 1,                            -- unused
    `Data7` = 0,                            -- unused
    `Data8` = 0,                            -- unused
    `Data9` = 0,                            -- unused
    `Data10` = 0,                           -- unused
    `Data11` = 0,                           -- unused
    `Data12` = 0,                           -- unused
    `Data13` = 0,                           -- unused
    `Data14` = 0,                           -- unused
    `Data15` = 0,                           -- unused
    `Data16` = 0,                           -- unused
    `Data17` = 0,                           -- unused
    `Data18` = 0,                           -- unused
    `Data19` = 0,                           -- unused
    `Data20` = 0,                           -- unused
    `Data21` = 0,                           -- unused
    `Data22` = 0                            -- DestructibleData (DBC entry, 0 = default)
WHERE `entry` = 400027;

-- Vérification
SELECT CONCAT('GameObject Atelier gobelin 400027 configuré avec 300000 PV.') AS Result;

-- =====================================================
-- INFORMATIONS
-- =====================================================
-- Type 33 = GAMEOBJECT_TYPE_DESTRUCTIBLE_BUILDING
-- Peut être attaqué par des engins de siège
-- Quand la santé atteint 0, le GameObject est détruit
--
-- Data0 = IntactNumHits (Points de vie totaux)
-- Data3 = DamagedDisplayId (Model ID quand endommagé, 0 = même model)
-- Data4 = DestroyedDisplayId (Model ID quand détruit, 0 = même model)
-- Data5 = DamagedNumHits (PV pour passer à "endommagé", 0 = pas d'état endommagé)
-- Data22 = DestructibleData (Référence DBC, 0 = défaut)
-- =====================================================

