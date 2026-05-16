-- Conquest Build Module - Structures Table
-- This table stores all spawned structures (walls, towers, gates)

CREATE TABLE IF NOT EXISTS `conquest_build_structures` (
    `guid` BIGINT UNSIGNED NOT NULL COMMENT 'GameObject spawn ID',
    `player_guid` BIGINT UNSIGNED NOT NULL COMMENT 'Player who built the structure',
    `guild_id` INT UNSIGNED NOT NULL DEFAULT 0 COMMENT 'Guild ID of the player',
    `entry` INT UNSIGNED NOT NULL COMMENT 'GameObject entry ID',
    `map` SMALLINT UNSIGNED NOT NULL COMMENT 'Map ID',
    `position_x` FLOAT NOT NULL COMMENT 'X position',
    `position_y` FLOAT NOT NULL COMMENT 'Y position',
    `position_z` FLOAT NOT NULL COMMENT 'Z position',
    `orientation` FLOAT NOT NULL COMMENT 'Orientation',
    `build_type` TINYINT UNSIGNED NOT NULL COMMENT '0=Wall, 1=Tower, 2=Gate',
    `build_time` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT 'When the structure was built',
    `group_id` BIGINT UNSIGNED NULL DEFAULT NULL COMMENT 'Group ID for linked structures (gate system: 2 towers + 1 gate)',
    PRIMARY KEY (`guid`),
    KEY `idx_group` (`group_id`),
    KEY `idx_player` (`player_guid`),
    KEY `idx_guild` (`guild_id`),
    KEY `idx_map` (`map`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='Conquest Build - Player structures';

