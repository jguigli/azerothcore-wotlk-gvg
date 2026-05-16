-- Enregistre OutdoorPvPConquest dans outdoorpvp_template
-- TypeId 8 = OUTDOOR_PVP_CONQUEST (cf src/server/game/OutdoorPvP/OutdoorPvP.h)
-- ScriptName "outdoorpvp_conquest" = nom du OutdoorPvPScript (cf OutdoorPvPConquest.cpp)
DELETE FROM `outdoorpvp_template` WHERE `TypeId` IN (8, 100);
INSERT INTO `outdoorpvp_template` (`TypeId`, `ScriptName`, `comment`)
VALUES (8, 'outdoorpvp_conquest', 'Conquest Frontline — capture points système custom Conquest');
