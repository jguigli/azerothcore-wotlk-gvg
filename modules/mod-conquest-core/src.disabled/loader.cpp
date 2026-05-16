/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

// From module scripts
void AddConquestCoreScripts();
void AddConquestHostilityScripts();
void AddConquestCommandScripts();
void AddConquestRestrictionsScripts();
void AddConquestPlayerStartScripts();
void AddConquestPlayerDeathScripts();

// Add all scripts
// cf. the naming convention https://github.com/azerothcore/azerothcore-wotlk/blob/master/doc/changelog/master.md#how-to-upgrade-4
// additionally replace all '-' in the module folder name with '_' here
void Addmod_conquest_coreScripts()
{
    AddConquestCoreScripts();
    AddConquestHostilityScripts();
    AddConquestCommandScripts();
    AddConquestRestrictionsScripts();
    AddConquestPlayerStartScripts();
    AddConquestPlayerDeathScripts();
}

