/*
 * Copyright (C) 2016+ AzerothCore
 */

// #include "ScriptMgr.h"
// #include "Guild.h"
// #include "GuildMgr.h"
// #include "Player.h"
// #include "WorldSession.h"
// #include "DatabaseEnv.h"
// #include "Log.h"
// #include "GameObject.h"
// #include "GameTime.h"
// #include "GuildScript.h"
// #include "AllGameObjectScript.h"
// #include "PlayerScript.h"
// #include <unordered_map>
// #include <unordered_set>
// #include <mutex>

// namespace
// {
//     constexpr uint32 BASE_CHEST_ENTRY = 80100;
//     constexpr uint32 SYSTEM_GUILD_ID_BASE = 1000000; // Base ID pour les guildes système
    
//     // Map pour associer l'identifiant unique du coffre aux guild IDs
//     // Chaque GameObject a sa propre guilde système
//     std::unordered_map<uint64, uint32> s_chestIdToGuildId;
    
//     // Map pour associer les guild IDs aux identifiants des coffres
//     std::unordered_map<uint32, uint64> s_guildIdToChestId;
    
//     // Map pour suivre les guildes temporaires associées aux joueurs
//     std::unordered_map<ObjectGuid, std::unordered_set<uint32>> s_playerTemporaryGuilds;
    
//     // Compteur séquentiel pour les IDs de guilde système
//     // Initialisé au démarrage depuis la DB
//     uint32 s_nextSystemGuildId = SYSTEM_GUILD_ID_BASE;
//     std::mutex s_guildIdMutex; // Protection thread-safe pour le compteur
//     bool s_counterInitialized = false; // Flag pour lazy initialization
    
//     uint64 GetChestUniqueId(GameObject const* go)
//     {
//         if (!go)
//             return 0;
//         uint64 spawnId = go->GetSpawnId();
//         if (spawnId != 0)
//             return spawnId;
//         return go->GetGUID().GetCounter();
//     }
    
//     // Fonction pour initialiser le compteur de guilde système (lazy initialization)
//     void InitializeSystemGuildCounter()
//     {
//         if (s_counterInitialized)
//             return;
            
//         std::lock_guard<std::mutex> lock(s_guildIdMutex);
        
//         // Double-check après avoir acquis le lock
//         if (s_counterInitialized)
//             return;
        
//         // Vérifier si la table existe avant de faire la requête
//         // Si la table n'existe pas, on initialise juste le compteur à la base
//         QueryResult tableCheck = CharacterDatabase.Query(
//             "SELECT COUNT(*) FROM information_schema.tables WHERE table_schema = DATABASE() AND table_name = 'conquest_shared_chest_guild_mapping'");
        
//         if (tableCheck && tableCheck->Fetch()[0].Get<uint64>() > 0)
//         {
//             // Charger le mapping existant depuis la DB
//             QueryResult mappingResult = CharacterDatabase.Query(
//                 "SELECT chest_spawn_id, guild_id FROM conquest_shared_chest_guild_mapping");
//             if (mappingResult)
//         {
//             do
//             {
//                 Field* fields = mappingResult->Fetch();
//                 uint64 chestSpawnId = fields[0].Get<uint64>();
//                 uint32 guildId = fields[1].Get<uint32>();
                
//                 s_chestIdToGuildId[chestSpawnId] = guildId;
//                 s_guildIdToChestId[guildId] = chestSpawnId;
                
//                 // Mettre à jour le compteur pour être supérieur au max ID trouvé
//                 if (guildId >= s_nextSystemGuildId)
//                 {
//                     s_nextSystemGuildId = guildId + 1;
//                 }
//             } while (mappingResult->NextRow());
            
//             LOG_INFO("server.loading", "ConquestSharedChestGuildBankScript: Loaded {} chest-guild mappings from DB, next guild ID: {}", 
//                 s_chestIdToGuildId.size(), s_nextSystemGuildId);
//         }
        
//         }
//         else
//         {
//             LOG_INFO("server.loading", "ConquestSharedChestGuildBankScript: Table conquest_shared_chest_guild_mapping does not exist yet, initializing counter to base");
//         }
        
//         // S'assurer que le compteur est au moins à la base
//         if (s_nextSystemGuildId < SYSTEM_GUILD_ID_BASE)
//         {
//             s_nextSystemGuildId = SYSTEM_GUILD_ID_BASE;
//         }
        
//         s_counterInitialized = true;
//         LOG_INFO("server.loading", "ConquestSharedChestGuildBankScript: System guild counter initialized to {}", s_nextSystemGuildId);
//     }
    
//     // Fonction pour obtenir ou créer un ID de guilde pour un GameObject (basé sur identifiant unique)
//     // Chaque GameObject coffre a son propre identifiant unique, donc chaque coffre aura sa propre guilde système
//     // et son propre stockage indépendant dans guild_bank_item
//     uint32 GetOrCreateGuildIdForChest(uint64 chestId)
//     {
//         // Initialiser le compteur de manière lazy (la première fois qu'on accède à un coffre)
//         InitializeSystemGuildCounter();
        
//         // Vérifier si la guilde existe déjà en mémoire
//         auto it = s_chestIdToGuildId.find(chestId);
//         if (it != s_chestIdToGuildId.end())
//         {
//             uint32 guildId = it->second;
//             // Vérifier si la guilde est chargée
//             if (sGuildMgr->GetGuildById(guildId))
//             {
//                 LOG_INFO("server.loading", "ConquestSharedChestGuildBankScript: Using existing guild {} for chest id {}", guildId, chestId);
//                 return guildId;
//             }
//         }
        
//         // Vérifier dans la table de mapping si le coffre existe déjà
//         QueryResult mappingResult = CharacterDatabase.Query(
//             "SELECT guild_id FROM conquest_shared_chest_guild_mapping WHERE chest_spawn_id = {}", chestId);
//         if (mappingResult)
//         {
//             uint32 guildId = mappingResult->Fetch()[0].Get<uint32>();
//             s_chestIdToGuildId[chestId] = guildId;
//             s_guildIdToChestId[guildId] = chestId;
            
//             // Vérifier si la guilde existe dans la DB
//             QueryResult guildResult = CharacterDatabase.Query("SELECT guildid FROM guild WHERE guildid = {}", guildId);
//             if (guildResult)
//             {
//                 // La guilde existe dans la DB, l'associer au coffre
            
//             // Charger la guilde si elle n'est pas déjà chargée
//             if (!sGuildMgr->GetGuildById(guildId))
//             {
//                 // Charger la guilde depuis la DB manuellement
//                 QueryResult guildResult = CharacterDatabase.Query(
//                     "SELECT g.guildid, g.name, g.leaderguid, g.EmblemStyle, g.EmblemColor, g.BorderStyle, g.BorderColor, "
//                     "g.BackgroundColor, g.info, g.motd, g.createdate, g.BankMoney, COUNT(gbt.guildid) "
//                     "FROM guild g LEFT JOIN guild_bank_tab gbt ON g.guildid = gbt.guildid "
//                     "WHERE g.guildid = {} GROUP BY g.guildid",
//                     guildId);
                
//                 if (guildResult)
//                 {
//                     Field* fields = guildResult->Fetch();
//                     Guild* guild = new Guild();
//                     if (guild->LoadFromDB(fields))
//                     {
//                         sGuildMgr->AddGuild(guild);
                        
//                         // Charger les rangs
//                         QueryResult rankResult = CharacterDatabase.Query(
//                             "SELECT guildid, rid, rname, rights, BankMoneyPerDay FROM guild_rank WHERE guildid = {} ORDER BY rid ASC",
//                             guildId);
//                         if (rankResult)
//                         {
//                             do
//                             {
//                                 guild->LoadRankFromDB(rankResult->Fetch());
//                             } while (rankResult->NextRow());
//                         }
                        
//                         // Charger les membres
//                         QueryResult memberResult = CharacterDatabase.Query(
//                             "SELECT guildid, gm.guid, `rank`, pnote, offnote, w.tab0, w.tab1, w.tab2, w.tab3, w.tab4, w.tab5, "
//                             "w.money, c.name, c.level, c.class, c.gender, c.zone, c.account, c.logout_time "
//                             "FROM guild_member gm "
//                             "LEFT JOIN guild_member_withdraw w ON gm.guid = w.guid "
//                             "LEFT JOIN characters c ON c.guid = gm.guid WHERE gm.guildid = {}",
//                             guildId);
//                         if (memberResult)
//                         {
//                             do
//                             {
//                                 guild->LoadMemberFromDB(memberResult->Fetch());
//                             } while (memberResult->NextRow());
//                         }
                        
//                         // Charger les items de banque (sera fait automatiquement par GuildMgr au démarrage, mais on peut le faire ici aussi)
//                         // Les items seront chargés automatiquement quand on accède à la banque
//                     }
//                     else
//                     {
//                         delete guild;
//                     }
//                 }
//             }
            
//                 LOG_INFO("server.loading", "ConquestSharedChestGuildBankScript: Loaded existing guild {} for chest id {} from mapping table", guildId, chestId);
//                 return guildId;
//             }
//             else
//             {
//                 LOG_ERROR("server.loading", "ConquestSharedChestGuildBankScript: Mapping exists but guild {} not found in DB for chest id {}", guildId, chestId);
//                 // Le mapping existe mais la guilde n'existe pas, on va créer une nouvelle guilde
//             }
//         }
        
//         // Nouveau coffre : générer un ID de guilde séquentiel
//         std::lock_guard<std::mutex> lock(s_guildIdMutex);
//         uint32 guildId = s_nextSystemGuildId++;
        
//         // Vérifier que l'ID n'est pas déjà utilisé (sécurité supplémentaire)
//         QueryResult checkResult = CharacterDatabase.Query("SELECT guildid FROM guild WHERE guildid = {}", guildId);
//         if (checkResult)
//         {
//             // ID déjà utilisé, trouver le prochain disponible
//             QueryResult maxResult = CharacterDatabase.Query(
//                 "SELECT MAX(guildid) FROM guild WHERE guildid >= {}", SYSTEM_GUILD_ID_BASE);
//             if (maxResult)
//             {
//                 uint32 maxId = maxResult->Fetch()[0].Get<uint32>();
//                 guildId = maxId + 1;
//                 s_nextSystemGuildId = guildId + 1;
//             }
//             else
//             {
//                 guildId = SYSTEM_GUILD_ID_BASE;
//                 s_nextSystemGuildId = SYSTEM_GUILD_ID_BASE + 1;
//             }
//         }
        
//         // Créer le mapping dans la DB avant de créer la guilde
//         CharacterDatabase.DirectExecute(
//             "INSERT INTO conquest_shared_chest_guild_mapping (chest_spawn_id, guild_id) VALUES ({}, {})",
//             chestId, guildId);
        
//         // Créer la guilde dans la DB
//         // Chaque coffre aura sa propre guilde système avec son propre stockage
//         CharacterDatabase.DirectExecute(
//             "INSERT INTO guild (guildid, name, leaderguid, info, motd, createdate, EmblemStyle, EmblemColor, BorderStyle, BorderColor, BackgroundColor, BankMoney) "
//             "VALUES ({}, 'Shared Chest {}', 0, 'System guild for shared chest id {}', 'Shared Chest System - Each chest has independent storage', {}, 0, 0, 0, 0, 0, 0)",
//             guildId, chestId, chestId, GameTime::GetGameTime().count());

//         // Créer les rangs de guilde (minimum 5)
//         for (uint8 i = 0; i < 5; ++i)
//         {
//             uint32 rights = (i == 0) ? 0x001DF1FF : 0; // Tous les droits pour le rang 0
//             CharacterDatabase.DirectExecute(
//                 "INSERT INTO guild_rank (guildid, rid, rname, rights, BankMoneyPerDay) "
//                 "VALUES ({}, {}, 'Rank {}', {}, 0)",
//                 guildId, i, i + 1, rights);
//         }

//         // Créer les onglets de banque (6 onglets)
//         for (uint8 i = 0; i < 6; ++i)
//         {
//             CharacterDatabase.DirectExecute(
//                 "INSERT INTO guild_bank_tab (guildid, TabId, TabName, TabIcon, TabText) "
//                 "VALUES ({}, {}, 'Tab {}', '', '')",
//                 guildId, i, i + 1);
//         }

//         // Donner tous les droits de banque au rang 0
//         for (uint8 i = 0; i < 6; ++i)
//         {
//             CharacterDatabase.DirectExecute(
//                 "INSERT INTO guild_bank_right (guildid, TabId, rid, gbright, SlotPerDay) "
//                 "VALUES ({}, {}, 0, {}, {})",
//                 guildId, i, 0xFF, 0xFFFFFFFF);
//         }
        
//         // Associer la guilde au coffre en mémoire
//         // Le mapping est déjà créé dans la DB
//         s_guildIdToChestId[guildId] = chestId;
//         s_chestIdToGuildId[chestId] = guildId;
        
//         LOG_INFO("server.loading", "ConquestSharedChestGuildBankScript: Created new system guild {} for chest id {} (sequential ID)", guildId, chestId);
        
//         // Charger la guilde depuis la DB
//         // La guilde vient d'être créée, on doit la charger en mémoire
//         QueryResult guildResult = CharacterDatabase.Query(
//             "SELECT g.guildid, g.name, g.leaderguid, g.EmblemStyle, g.EmblemColor, g.BorderStyle, g.BorderColor, "
//             "g.BackgroundColor, g.info, g.motd, g.createdate, g.BankMoney, COUNT(gbt.guildid) "
//             "FROM guild g LEFT JOIN guild_bank_tab gbt ON g.guildid = gbt.guildid "
//             "WHERE g.guildid = {} GROUP BY g.guildid",
//             guildId);
        
//         if (guildResult)
//         {
//             Field* fields = guildResult->Fetch();
//             Guild* guild = new Guild();
//             if (guild->LoadFromDB(fields))
//             {
//                 sGuildMgr->AddGuild(guild);
                
//                 // Charger les rangs
//                 QueryResult rankResult = CharacterDatabase.Query(
//                     "SELECT guildid, rid, rname, rights, BankMoneyPerDay FROM guild_rank WHERE guildid = {} ORDER BY rid ASC",
//                     guildId);
//                 if (rankResult)
//                 {
//                     do
//                     {
//                         guild->LoadRankFromDB(rankResult->Fetch());
//                     } while (rankResult->NextRow());
//                 }

//                 // Charger les droits de banque
//                 QueryResult rightResult = CharacterDatabase.Query(
//                     "SELECT guildid, TabId, rid, gbright, SlotPerDay FROM guild_bank_right WHERE guildid = {}",
//                     guildId);
//                 if (rightResult)
//                 {
//                     do
//                     {
//                         guild->LoadBankRightFromDB(rightResult->Fetch());
//                     } while (rightResult->NextRow());
//                 }

//                 // Charger les onglets de banque
//                 QueryResult tabResult = CharacterDatabase.Query(
//                     "SELECT guildid, TabId, TabName, TabIcon, TabText FROM guild_bank_tab WHERE guildid = {}",
//                     guildId);
//                 if (tabResult)
//                 {
//                     do
//                     {
//                         guild->LoadBankTabFromDB(tabResult->Fetch());
//                     } while (tabResult->NextRow());
//                 }
                
//                 // Les membres et items de banque seront chargés à la demande
//             }
//             else
//             {
//                 delete guild;
//             }
//         }
        
//         LOG_INFO("server.loading", "ConquestSharedChestGuildBankScript: Created NEW guild {} for chest id {} - This chest now has its own independent storage", guildId, chestId);
        
//         return guildId;
//     }
// }

// // GuildScript pour intercepter les opérations de banque de guilde
// class ConquestSharedChestGuildBankGuildScript : public GuildScript
// {
// public:
//     ConquestSharedChestGuildBankGuildScript() : GuildScript("ConquestSharedChestGuildBankGuildScript") 
//     {
//         LOG_INFO("server.loading", "ConquestSharedChestGuildBankGuildScript: script registered");
//         // L'initialisation se fera de manière lazy lors du premier accès à un coffre
//     }


//     Guild* GetGuildForGameObject(WorldSession* session, GameObject const* go) override
//     {
//         if (!go || go->GetEntry() != BASE_CHEST_ENTRY)
//             return nullptr;
        
//         uint64 chestId = GetChestUniqueId(go);
//         if (chestId == 0)
//             return nullptr;
        
//         auto it = s_chestIdToGuildId.find(chestId);
//         uint32 guildId = 0;
//         if (it != s_chestIdToGuildId.end())
//         {
//             guildId = it->second;
//         }
//         else
//         {
//             // Aucun enregistrement, créer/charger la guilde système maintenant
//             guildId = GetOrCreateGuildIdForChest(chestId);
//         }
        
//         if (!guildId)
//             return nullptr;
        
//         LOG_INFO("server.loading", "ConquestSharedChestGuildBankGuildScript: GetGuildForGameObject chestId {} -> guild {}", chestId, guildId);

//         Guild* guild = sGuildMgr->GetGuildById(guildId);
//         if (!guild)
//         {
//             LOG_WARN("server.loading", "ConquestSharedChestGuildBankGuildScript: Guild {} for chestId {} not loaded yet, loading from DB", guildId, chestId);

//             QueryResult guildResult = CharacterDatabase.Query(
//                 "SELECT g.guildid, g.name, g.leaderguid, g.EmblemStyle, g.EmblemColor, g.BorderStyle, g.BorderColor, "
//                 "g.BackgroundColor, g.info, g.motd, g.createdate, g.BankMoney, COUNT(gbt.guildid) "
//                 "FROM guild g LEFT JOIN guild_bank_tab gbt ON g.guildid = gbt.guildid "
//                 "WHERE g.guildid = {} GROUP BY g.guildid",
//                 guildId);

//             if (guildResult)
//             {
//                 // Vérifier à nouveau si la guilde n'a pas été chargée entre-temps (race condition)
//                 guild = sGuildMgr->GetGuildById(guildId);
//                 if (guild)
//                 {
//                     LOG_INFO("server.loading", "ConquestSharedChestGuildBankGuildScript: Guild {} was loaded by another thread, using existing instance", guildId);
//                 }
//                 else
//                 {
//                     Guild* newGuild = new Guild();
//                     if (newGuild->LoadFromDB(guildResult->Fetch()))
//                     {
//                         sGuildMgr->AddGuild(newGuild);
//                         guild = newGuild;

//                         QueryResult rankResult = CharacterDatabase.Query(
//                             "SELECT guildid, rid, rname, rights, BankMoneyPerDay FROM guild_rank WHERE guildid = {} ORDER BY rid ASC",
//                             guildId);
//                         if (rankResult)
//                         {
//                             do
//                             {
//                                 guild->LoadRankFromDB(rankResult->Fetch());
//                             } while (rankResult->NextRow());
//                         }

//                         QueryResult memberResult = CharacterDatabase.Query(
//                             "SELECT guildid, gm.guid, `rank`, pnote, offnote, w.tab0, w.tab1, w.tab2, w.tab3, w.tab4, w.tab5, "
//                             "w.money, c.name, c.level, c.class, c.gender, c.zone, c.account, c.logout_time "
//                             "FROM guild_member gm "
//                             "LEFT JOIN guild_member_withdraw w ON gm.guid = w.guid "
//                             "LEFT JOIN characters c ON c.guid = gm.guid "
//                             "WHERE guildid = {}",
//                             guildId);
//                         if (memberResult)
//                         {
//                             do
//                             {
//                                 guild->LoadMemberFromDB(memberResult->Fetch());
//                             } while (memberResult->NextRow());
//                         }

//                         QueryResult rightResult = CharacterDatabase.Query(
//                             "SELECT guildid, TabId, rid, gbright, SlotPerDay FROM guild_bank_right WHERE guildid = {}",
//                             guildId);
//                         if (rightResult)
//                         {
//                             do
//                             {
//                                 guild->LoadBankRightFromDB(rightResult->Fetch());
//                             } while (rightResult->NextRow());
//                         }

//                         QueryResult tabResult = CharacterDatabase.Query(
//                             "SELECT guildid, TabId, TabName, TabIcon, TabText FROM guild_bank_tab WHERE guildid = {}",
//                             guildId);
//                         if (tabResult)
//                         {
//                             do
//                             {
//                                 guild->LoadBankTabFromDB(tabResult->Fetch());
//                             } while (tabResult->NextRow());
//                         }

//                         QueryResult itemResult = CharacterDatabase.Query(
//                             "SELECT creatorGuid, giftCreatorGuid, count, duration, charges, flags, enchantments, randomPropertyId, durability, playedTime, text, "
//                             "g.guildid, gbt.TabId, gbt.SlotId, gbt.item_guid, ii.itemEntry "
//                             "FROM guild_bank_item gbt INNER JOIN guild g ON g.guildid = gbt.guildid "
//                             "INNER JOIN item_instance ii ON gbt.item_guid = ii.guid "
//                             "WHERE g.guildid = {}",
//                             guildId);
//                         if (itemResult)
//                         {
//                             do
//                             {
//                                 guild->LoadBankItemFromDB(itemResult->Fetch());
//                             } while (itemResult->NextRow());
//                         }
//                     }
//                     else
//                     {
//                         delete newGuild;
//                         guild = nullptr;
//                     }
//                 }
//             }
//             else
//             {
//                 LOG_ERROR("server.loading", "ConquestSharedChestGuildBankGuildScript: Unable to load guild {} from database", guildId);
//                 guild = nullptr;
//             }
//         }
        
//         if (guild && session && session->GetPlayer())
//         {
//             Player* player = session->GetPlayer();
//             LOG_INFO("server.loading", "ConquestSharedChestGuildBankGuildScript: GetGuildForGameObject called by player {} for guild {} (member count before: {})", 
//                 player->GetName(), guild->GetId(), guild->GetMemberSize());
            
//             // Toujours s'assurer que le joueur est membre temporaire avec les bonnes permissions
//             // Cela permet à plusieurs joueurs d'avoir simultanément accès au même coffre
//             if (Guild::Member* member = guild->EnsureTemporaryMember(player))
//             {
//                 s_playerTemporaryGuilds[player->GetGUID()].insert(guild->GetId());
//                 LOG_INFO("server.loading", "ConquestSharedChestGuildBankGuildScript: Added/Updated temporary member {} to guild {} (rank: {}, member count after: {})", 
//                     player->GetName(), guild->GetId(), member->GetRankId(), guild->GetMemberSize());
//             }
//             else
//             {
//                 LOG_ERROR("server.loading", "ConquestSharedChestGuildBankGuildScript: Failed to add temporary member {} to guild {}", 
//                     player->GetName(), guild->GetId());
//             }
//         }

//         return guild;
//     }

// };

// // GameObjectScript pour intercepter l'activation et créer/assigner la guilde
// class ConquestSharedChestGuildBankGameObjectScript : public GameObjectScript
// {
// public:
//     ConquestSharedChestGuildBankGameObjectScript() : GameObjectScript("ConquestSharedChestGuildBank") 
//     {
//         LOG_INFO("server.loading", "ConquestSharedChestGuildBankGameObjectScript: script registered");
//     }

//     bool OnGossipHello(Player* player, GameObject* go) override
//     {
//         // Le GameObject de type GUILD_BANK ne passe pas par OnGossipHello
//         // L'ouverture est gérée directement par le core via GetGuildForGameObject
//         return false;
//     }
// };

// // AllGameObjectScript pour intercepter tous les clics sur les GameObjects
// class ConquestSharedChestGuildBankAllScript : public AllGameObjectScript
// {
// public:
//     ConquestSharedChestGuildBankAllScript() : AllGameObjectScript("ConquestSharedChestGuildBankAllScript") { }

//     bool CanGameObjectGossipHello(Player* player, GameObject* go) override
//     {
//         if (!player || !go)
//             return false; // Laisser le comportement par défaut
        
//         if (go->GetEntry() != BASE_CHEST_ENTRY)
//             return false; // Laisser le comportement par défaut

//         // Retourner false pour permettre à GameObjectScript::OnGossipHello d'être appelé
//         return true;
//     }
// };

// // PlayerScript pour nettoyer les mappings à la déconnexion
// class ConquestSharedChestGuildBankPlayerScript : public PlayerScript
// {
// public:
//     ConquestSharedChestGuildBankPlayerScript() : PlayerScript("ConquestSharedChestGuildBankPlayerScript") { }

//     void OnPlayerLogout(Player* player) override
//     {
//         if (!player)
//             return;

//         // Nettoyer les membres temporaires des guildes système
//         auto it = s_playerTemporaryGuilds.find(player->GetGUID());
//         if (it != s_playerTemporaryGuilds.end())
//         {
//             for (uint32 guildId : it->second)
//                 if (Guild* guild = sGuildMgr->GetGuildById(guildId))
//                     guild->RemoveTemporaryMember(player->GetGUID());
//             s_playerTemporaryGuilds.erase(it);
//         }
//     }
// };

void AddConquestSharedChestGuildBankScripts()
{
    // LOG_INFO("server.loading", "ConquestSharedChestGuildBankScript: AddConquestSharedChestGuildBankScripts called - registering scripts");
    // new ConquestSharedChestGuildBankGuildScript();
    // new ConquestSharedChestGuildBankGameObjectScript();
    // new ConquestSharedChestGuildBankAllScript();
    // new ConquestSharedChestGuildBankPlayerScript();
    // LOG_INFO("server.loading", "ConquestSharedChestGuildBankScript: All scripts registered successfully");
}

