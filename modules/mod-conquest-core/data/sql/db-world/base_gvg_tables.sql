CREATE TABLE IF NOT EXISTS `conquest_player_stats` (
    `player_guid` bigint(20) unsigned NOT NULL,
    `kills` int NOT NULL DEFAULT 0,
    `deaths` int NOT NULL DEFAULT 0,
    `last_updated` int unsigned NOT NULL DEFAULT 0,
    PRIMARY KEY (`player_guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `conquest_guild_stats` (
    `guild_id` int unsigned NOT NULL,
    `zone_controlled` int unsigned NOT NULL DEFAULT 0,
    `gold_earned` bigint unsigned NOT NULL DEFAULT 0,
    PRIMARY KEY (`guild_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `conquest_global_config` (
    `conf_key` varchar(64) NOT NULL,
    `conf_value` varchar(255) NOT NULL,
    PRIMARY KEY (`conf_key`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `conquest_guild_relations` (
`guild_a` INT UNSIGNED NOT NULL,
`guild_b` INT UNSIGNED NOT NULL,
`relation` TINYINT NOT NULL DEFAULT 0,
`expire_at` INT UNSIGNED DEFAULT 0,
PRIMARY KEY (`guild_a`, `guild_b`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

