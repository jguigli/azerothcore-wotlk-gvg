-- 1. CRÉER UNE TABLE DE BACKUP COMPLÈTE
DROP TABLE IF EXISTS item_template_backup;

CREATE TABLE item_template_backup AS
SELECT *
FROM item_template;

-- 2. ENLEVER TOUT TYPE DE BINDING
-- bonding = 0 → objet non lié
UPDATE item_template
SET bonding = 0;

-- 3. Enlever le flag soulbound dans 'flags'
-- Le flag 1 = ITEM_FLAG_SOULBOUND
UPDATE item_template
SET flags = flags & ~1;
