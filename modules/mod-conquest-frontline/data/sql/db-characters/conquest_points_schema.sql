-- Conquest Frontline — Wallet joueur (Points de Conquête + Points de Bataille)
-- Persistance simple par GUID joueur.

CREATE TABLE IF NOT EXISTS `character_conquest_points` (
    `guid` INT UNSIGNED NOT NULL,
    `conquest_points` INT UNSIGNED NOT NULL DEFAULT 0,
    `battle_points` INT UNSIGNED NOT NULL DEFAULT 0,
    `updated_at` INT UNSIGNED NOT NULL DEFAULT 0,
    PRIMARY KEY (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
