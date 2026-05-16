/*
 * Conquest Frontline — Script loader
 */

// Forward declarations
void AddSC_outdoorpvp_conquest();
void AddSC_ConquestFrontlineAuto();
void AddSC_ConquestPointsHooks();
void AddSC_ConquestForcePvP();
void AddSC_ConquestEquipBots();
void AddSC_ConquestCommands();
void AddSC_ConquestDefenseRedispatch();

void Addmod_conquest_frontlineScripts()
{
    AddSC_outdoorpvp_conquest();
    AddSC_ConquestFrontlineAuto();
    AddSC_ConquestPointsHooks();
    AddSC_ConquestForcePvP();
    AddSC_ConquestEquipBots();
    AddSC_ConquestCommands();
    AddSC_ConquestDefenseRedispatch();
}
