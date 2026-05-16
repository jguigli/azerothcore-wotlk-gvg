/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

#include "Log.h"

// From module scripts
void AddConquestZoneControlScripts();
void AddConquestSharedChestGuildBankScripts();

// Add all scripts
// cf. the naming convention https://github.com/azerothcore/azerothcore-wotlk/blob/master/doc/changelog/master.md#how-to-upgrade-4
// additionally replace all '-' in the module folder name with '_' here
void Addmod_conquest_zone_controlScripts()
{
    LOG_INFO("server.loading", "ConquestSharedChestGuildBank: Addmod_conquest_zone_controlScripts called - loading mod-zone-control scripts");
    AddConquestZoneControlScripts();
    AddConquestSharedChestGuildBankScripts(); // Système de coffre de guilde partagé
    LOG_INFO("server.loading", "ConquestSharedChestGuildBank: Addmod_conquest_zone_controlScripts completed");
}

