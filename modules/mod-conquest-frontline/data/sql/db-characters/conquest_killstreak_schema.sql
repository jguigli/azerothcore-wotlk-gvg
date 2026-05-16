-- Conquest Frontline — Kill Streak (Phase 5)
-- Tracker par joueur. Le streak courant reset à 0 sur mort, max_streak persiste pour le scoreboard.

CREATE TABLE IF NOT EXISTS `character_conquest_killstreak` (
    `guid` INT UNSIGNED NOT NULL,
    `current_streak` SMALLINT UNSIGNED NOT NULL DEFAULT 0,
    `max_streak` SMALLINT UNSIGNED NOT NULL DEFAULT 0,
    `updated_at` INT UNSIGNED NOT NULL DEFAULT 0,
    PRIMARY KEY (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
