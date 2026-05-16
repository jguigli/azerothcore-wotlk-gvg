-- ============================================================================
-- Conquest Mounts v15 — Fix GHOST_VISIBILITY (cause: vehicles invisibles en
-- non-GM)
-- ============================================================================
-- Le flag CREATURE_FLAG_EXTRA_GHOST_VISIBILITY (0x400) etait set dans
-- flags_extra=344407930. Effect : creature visible UNIQUEMENT par les
-- joueurs morts (fantomes). GMs voient tout par override, joueurs vivants
-- en phase 1 ne voient rien.
-- 344407930 & ~0x400 = 344406906.
-- ============================================================================

UPDATE `creature_template` SET `flags_extra` = 344406906
WHERE `entry` IN (
    -- Vehicles parents
    400200, 400201, 400209, 400310, 400311, 400312,
    400314, 400315, 400326,
    -- Accessoires turrets / chairs / lance-flammes
    400206, 400207, 400208, 400210, 400211, 400313, 400316,
    400317, 400318, 400319, 400320, 400321,
    400322, 400323, 400324, 400325, 400327, 400328
)
AND `flags_extra` = 344407930;

SELECT CONCAT('v15 OK : ', ROW_COUNT(), ' rows updated') AS Result;
