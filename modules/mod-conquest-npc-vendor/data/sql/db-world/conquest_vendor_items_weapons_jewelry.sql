-- ============================================================================
-- Conquest Vendor Items — Armes + Bijoux
--
-- DEPRECATED : les armes et bijoux PvE sont maintenant intégrés dans les
-- fichiers tier-specific (conquest_vendor_items_pve_t1.sql / pve_t2_t3.sql)
-- et PvP dans conquest_vendor_items_pvp.sql, via les vendors 400260 (Armes)
-- et 400263 (Offset). Ce fichier sert maintenant uniquement au cleanup des
-- anciens vendors armes/bijoux (400230, 400240-400243) au cas où.
-- ============================================================================

DELETE FROM `conquest_vendor_items` WHERE `npc_entry` IN
  (400230, 400240, 400241, 400242, 400243);
