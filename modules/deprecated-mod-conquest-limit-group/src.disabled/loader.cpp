/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

// From SC
void AddMyPlayerScripts();

// Add all
// The function name is generated from AC_ADD_SCRIPT_LOADER("mod_conquest_limit_group", ...) in CMakeLists.txt
// It converts "mod_conquest_limit_group" to "Addmod_conquest_limit_groupScripts"
void Addmod_conquest_limit_groupScripts()
{
    AddMyPlayerScripts();
}

