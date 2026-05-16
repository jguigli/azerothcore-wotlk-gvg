-- Conquest Frontline — Banner ownership persistence table.
--
-- Originally defined by mod-conquest-zone-control. Re-declared here as
-- CREATE TABLE IF NOT EXISTS so the conquest-frontline DB writes from
-- ConquestCapturePoint::ChangeTeam() succeed even when mod-conquest-zone-control
-- is disabled.
--
-- Schema is unchanged from the original to remain compatible if both modules
-- ever run side-by-side.

CREATE TABLE IF NOT EXISTS `conquest_zone_control` (
    `zone_guid` bigint(20) unsigned NOT NULL,
    `guild_id` int unsigned NOT NULL DEFAULT 0,
    `faction` tinyint unsigned NOT NULL DEFAULT 0,
    `original_entry` int unsigned NOT NULL DEFAULT 0,
    `captured_at` int unsigned NOT NULL DEFAULT 0,
    `last_reward_time` int unsigned NOT NULL DEFAULT 0,
    PRIMARY KEY (`zone_guid`),
    KEY `idx_guild_id` (`guild_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
