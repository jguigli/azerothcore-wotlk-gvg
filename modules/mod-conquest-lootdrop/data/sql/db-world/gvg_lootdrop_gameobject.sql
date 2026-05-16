-- Conquest Loot Drop - Gameobject Template for Loot Bag
-- This SQL creates the gameobject template for the player corpse loot bag

-- Insert the loot bag gameobject template
-- Based on entry 186736 (Money Bag) configuration for lootability
-- Key differences: Data0 = 0 (no lock), Data1 = 0 (dynamic loot), Data3 = 1 (consumable), Data19 = 1 (despawn after loot), size = 0.4
DELETE FROM `gameobject_template` WHERE `entry` = 400002;
INSERT INTO `gameobject_template` (`entry`, `type`, `displayId`, `name`, `IconName`, `castBarCaption`, `unk1`, `size`, `Data0`, `Data1`, `Data2`, `Data3`, `Data4`, `Data5`, `Data6`, `Data7`, `Data8`, `Data9`, `Data10`, `Data11`, `Data12`, `Data13`, `Data14`, `Data15`, `Data16`, `Data17`, `Data18`, `Data19`, `Data20`, `Data21`, `Data22`, `Data23`, `AIName`, `ScriptName`, `VerifiedBuild`) VALUES
(400002, 3, 323, 'Sac de butin du joueur', '', '', '', 0.4, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, '', 'ConquestLootDropGameObject', 12340);

-- Explanation of important fields:
-- entry: 400002 - The gameobject entry ID used in the script
-- type: 3 - GAMEOBJECT_TYPE_CHEST (can be looted like a chest)
-- displayId: 323 - Display model for a bag/sack (same as entry 186736)
-- name: 'Sac de butin du joueur' - French name meaning "Player Loot Bag"
-- size: 0.4 - Same size as Money Bag (186736)
-- Data0: 0 - No lock required (unlike 186736 which has lockId 57)
-- Data1: 0 - Dynamic loot (unlike 186736 which has lootId 22891)
-- Data3: 1 - Consumable (allows the chest to be opened and looted, same as 186736)
-- Data19: 1 - Allows the object to be despawned after looting
