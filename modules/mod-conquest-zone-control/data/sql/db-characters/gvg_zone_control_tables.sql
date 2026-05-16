-- Conquest Zone Control Module - Database Tables
-- Stores zone control information

CREATE TABLE IF NOT EXISTS `conquest_zone_control` (
    `zone_guid` bigint(20) unsigned NOT NULL,
    `guild_id` int unsigned NOT NULL DEFAULT 0,
    `faction` tinyint unsigned NOT NULL DEFAULT 0, -- 0 = neutral/disputed, 1 = alliance, 2 = horde
    `original_entry` int unsigned NOT NULL DEFAULT 0, -- Original disputed banner entry (400010 or 400011)
    `captured_at` int unsigned NOT NULL DEFAULT 0,
    `last_reward_time` int unsigned NOT NULL DEFAULT 0,
    PRIMARY KEY (`zone_guid`),
    KEY `idx_guild_id` (`guild_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Table to store temporarily despawned creatures
CREATE TABLE IF NOT EXISTS `conquest_zone_control_despawned_creatures` (
    `zone_guid` bigint(20) unsigned NOT NULL,
    `creature_guid` bigint(20) unsigned NOT NULL,
    `spawn_id` bigint(20) unsigned NOT NULL,
    `map_id` int unsigned NOT NULL,
    `x` float NOT NULL,
    `y` float NOT NULL,
    `z` float NOT NULL,
    `o` float NOT NULL,
    PRIMARY KEY (`zone_guid`, `creature_guid`),
    KEY `idx_zone_guid` (`zone_guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
