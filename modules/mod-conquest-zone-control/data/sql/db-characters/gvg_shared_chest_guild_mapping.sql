-- Conquest Shared Chest Guild Mapping Table
-- This table maps shared chest spawn IDs to system guild IDs
-- This ensures persistent mapping and sequential guild IDs for shared chests
-- Each GameObject chest has its own independent storage via a system guild

CREATE TABLE IF NOT EXISTS `conquest_shared_chest_guild_mapping` (
    `chest_spawn_id` bigint(20) unsigned NOT NULL COMMENT 'GameObject spawn ID (or GUID if spawnId is 0)',
    `guild_id` int unsigned NOT NULL COMMENT 'System guild ID (>= 1000000) associated with this chest',
    PRIMARY KEY (`chest_spawn_id`),
    UNIQUE KEY `idx_guild_id` (`guild_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='Maps shared chest GameObject spawn IDs to system guild IDs for independent storage';

