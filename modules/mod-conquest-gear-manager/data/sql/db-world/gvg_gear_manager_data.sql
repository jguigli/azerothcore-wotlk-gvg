-- Conquest Gear Manager Module - Gear Data Storage
-- Table to store JSON gear data for specializations

CREATE TABLE IF NOT EXISTS `conquest_gear_data` (
    `id` int unsigned NOT NULL AUTO_INCREMENT,
    `season` varchar(8) NOT NULL DEFAULT 's5',
    `class_name` varchar(32) NOT NULL,
    `specialization` varchar(32) NOT NULL,
    `json_data` longtext NOT NULL,
    `last_updated` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    PRIMARY KEY (`id`),
    UNIQUE KEY `unique_spec` (`season`, `class_name`, `specialization`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
