CREATE TABLE IF NOT EXISTS `conquest_player_specialization` (
    `player_guid` bigint(20) unsigned NOT NULL,
    `class_name` varchar(32) NOT NULL,
    `specialization` varchar(32) NOT NULL,
    `last_updated` int unsigned NOT NULL DEFAULT 0,
    PRIMARY KEY (`player_guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

