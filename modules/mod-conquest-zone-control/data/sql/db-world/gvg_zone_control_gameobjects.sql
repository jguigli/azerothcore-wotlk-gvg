-- Conquest Zone Control Module - Banner GameObjects
-- Creates 4 banner gameobjects for zone control system

-- Delete existing gameobjects
DELETE FROM `gameobject_template` WHERE `entry` IN (400010, 400011, 400012, 400013);

-- 400010: Disputed Horde Banner (spawned manually)
INSERT INTO `gameobject_template` (
    `entry`,
    `type`,
    `displayId`,
    `name`,
    `IconName`,
    `castBarCaption`,
    `unk1`,
    `size`,
    `Data0`,
    `Data1`,
    `Data2`,
    `Data3`,
    `Data4`,
    `Data5`,
    `Data6`,
    `Data7`,
    `Data8`,
    `Data9`,
    `Data10`,
    `Data11`,
    `Data12`,
    `Data13`,
    `Data14`,
    `Data15`,
    `Data16`,
    `Data17`,
    `Data18`,
    `Data19`,
    `Data20`,
    `Data21`,
    `Data22`,
    `Data23`,
    `ScriptName`
) VALUES (
    400010,                          -- entry
    1,                               -- type (GAMEOBJECT_TYPE_BUTTON)
    5774,                            -- displayId (Horde Disputed Banner)
    'Banniere Horde Disputee',      -- name
    '',                              -- IconName
    '',                              -- castBarCaption
    '',                              -- unk1
    1.0,                             -- size
    0,                               -- Data0
    1479,                            -- Data1
    0,                               -- Data2
    180102,                          -- Data3
    1,                               -- Data4
    0,                               -- Data5
    0,                               -- Data6
    0,                               -- Data7
    1,                               -- Data8
    0,                               -- Data9
    0,                               -- Data10
    0,                               -- Data11
    0,                               -- Data12
    0,                               -- Data13
    0,                               -- Data14
    0,                               -- Data15
    0,                               -- Data16
    0,                               -- Data17
    0,                               -- Data18
    0,                               -- Data19
    0,                               -- Data20
    0,                               -- Data21
    0,                               -- Data22
    0,                               -- Data23
    'ConquestZoneControlBanner'          -- ScriptName
);

-- 400011: Disputed Alliance Banner (spawned manually)
INSERT INTO `gameobject_template` (
    `entry`,
    `type`,
    `displayId`,
    `name`,
    `IconName`,
    `castBarCaption`,
    `unk1`,
    `size`,
    `Data0`,
    `Data1`,
    `Data2`,
    `Data3`,
    `Data4`,
    `Data5`,
    `Data6`,
    `Data7`,
    `Data8`,
    `Data9`,
    `Data10`,
    `Data11`,
    `Data12`,
    `Data13`,
    `Data14`,
    `Data15`,
    `Data16`,
    `Data17`,
    `Data18`,
    `Data19`,
    `Data20`,
    `Data21`,
    `Data22`,
    `Data23`,
    `ScriptName`
) VALUES (
    400011,                          -- entry
    1,                               -- type (GAMEOBJECT_TYPE_BUTTON)
    5772,                            -- displayId (Alliance Disputed Banner)
    'Banniere Alliance Disputee',   -- name
    '',                              -- IconName
    '',                              -- castBarCaption
    '',                              -- unk1
    1.0,                             -- size
    0,                               -- Data0
    1479,                            -- Data1
    0,                               -- Data2
    180102,                          -- Data3
    1,                               -- Data4
    0,                               -- Data5
    0,                               -- Data6
    0,                               -- Data7
    1,                               -- Data8
    0,                               -- Data9
    0,                               -- Data10
    0,                               -- Data11
    0,                               -- Data12
    0,                               -- Data13
    0,                               -- Data14
    0,                               -- Data15
    0,                               -- Data16
    0,                               -- Data17
    0,                               -- Data18
    0,                               -- Data19
    0,                               -- Data20
    0,                               -- Data21
    0,                               -- Data22
    0,                               -- Data23
    'ConquestZoneControlBanner'          -- ScriptName
);

-- 400012: Horde Banner (controlled by Horde guild)
INSERT INTO `gameobject_template` (
    `entry`,
    `type`,
    `displayId`,
    `name`,
    `IconName`,
    `castBarCaption`,
    `unk1`,
    `size`,
    `Data0`,
    `Data1`,
    `Data2`,
    `Data3`,
    `Data4`,
    `Data5`,
    `Data6`,
    `Data7`,
    `Data8`,
    `Data9`,
    `Data10`,
    `Data11`,
    `Data12`,
    `Data13`,
    `Data14`,
    `Data15`,
    `Data16`,
    `Data17`,
    `Data18`,
    `Data19`,
    `Data20`,
    `Data21`,
    `Data22`,
    `Data23`,
    `ScriptName`
) VALUES (
    400012,                          -- entry
    1,                               -- type (GAMEOBJECT_TYPE_BUTTON)
    5773,                            -- displayId (Horde Banner)
    'Banniere Horde',               -- name
    '',                              -- IconName
    '',                              -- castBarCaption
    '',                              -- unk1
    1.0,                             -- size
    0,                               -- Data0
    1479,                            -- Data1
    3000,                            -- Data2
    180101,                          -- Data3
    1,                               -- Data4
    0,                               -- Data5
    0,                               -- Data6
    0,                               -- Data7
    1,                               -- Data8
    0,                               -- Data9
    0,                               -- Data10
    0,                               -- Data11
    0,                               -- Data12
    0,                               -- Data13
    0,                               -- Data14
    0,                               -- Data15
    0,                               -- Data16
    0,                               -- Data17
    0,                               -- Data18
    0,                               -- Data19
    0,                               -- Data20
    0,                               -- Data21
    0,                               -- Data22
    0,                               -- Data23
    'ConquestZoneControlBanner'          -- ScriptName
);

-- 400013: Alliance Banner (controlled by Alliance guild)
INSERT INTO `gameobject_template` (
    `entry`,
    `type`,
    `displayId`,
    `name`,
    `IconName`,
    `castBarCaption`,
    `unk1`,
    `size`,
    `Data0`,
    `Data1`,
    `Data2`,
    `Data3`,
    `Data4`,
    `Data5`,
    `Data6`,
    `Data7`,
    `Data8`,
    `Data9`,
    `Data10`,
    `Data11`,
    `Data12`,
    `Data13`,
    `Data14`,
    `Data15`,
    `Data16`,
    `Data17`,
    `Data18`,
    `Data19`,
    `Data20`,
    `Data21`,
    `Data22`,
    `Data23`,
    `ScriptName`
) VALUES (
    400013,                          -- entry
    1,                               -- type (GAMEOBJECT_TYPE_BUTTON)
    5771,                            -- displayId (Alliance Banner)
    'Banniere Alliance',            -- name
    '',                              -- IconName
    '',                              -- castBarCaption
    '',                              -- unk1
    1.0,                             -- size
    0,                               -- Data0
    1479,                            -- Data1
    3000,                            -- Data2
    180100,                          -- Data3
    1,                               -- Data4
    0,                               -- Data5
    0,                               -- Data6
    0,                               -- Data7
    1,                               -- Data8
    0,                               -- Data9
    0,                               -- Data10
    0,                               -- Data11
    0,                               -- Data12
    0,                               -- Data13
    0,                               -- Data14
    0,                               -- Data15
    0,                               -- Data16
    0,                               -- Data17
    0,                               -- Data18
    0,                               -- Data19
    0,                               -- Data20
    0,                               -- Data21
    0,                               -- Data22
    0,                               -- Data23
    'ConquestZoneControlBanner'          -- ScriptName
);

-- Verification
SELECT 'Banners de controle de zone crees avec succes!' AS Result;
