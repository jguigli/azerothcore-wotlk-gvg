/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

// From module scripts
void AddSC_ConquestVendorScript();
void AddSC_ConquestVehicleVendor();

// Add all scripts
void Addmod_conquest_npc_vendorScripts()
{
    AddSC_ConquestVendorScript();
    AddSC_ConquestVehicleVendor();
}

