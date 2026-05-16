/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

#include "ConquestCommands.h"
#include "ConquestCore.h"
#include "Chat.h"
#include "Player.h"
#include "Log.h"

using namespace Acore::ChatCommands;

ConquestCommands::ConquestCommands() : CommandScript("ConquestCommands")
{
}

// FIXED: Use static ChatCommandTable with initializer lists
ChatCommandTable ConquestCommands::GetCommands() const
{
    static ChatCommandTable relationSubCommands =
    {
        { "set",  HandleConquestRelationSetCommand,  SEC_ADMINISTRATOR, Console::No },
        { "list", HandleConquestRelationListCommand, SEC_GAMEMASTER,    Console::No }
    };

    static ChatCommandTable conquestSubCommands =
    {
        { "reload",   HandleConquestReloadCommand, SEC_GAMEMASTER, Console::No },
        { "relation", relationSubCommands }
    };

    static ChatCommandTable commandTable =
    {
        { "conquest", conquestSubCommands }
    };

    return commandTable;
}

bool ConquestCommands::HandleConquestReloadCommand(ChatHandler* handler)
{
    ConquestCore::Instance().LoadRelationsFromDB();
    handler->SendSysMessage("[mod-gvg-core] Reloaded relations from DB.");
    return true;
}

bool ConquestCommands::HandleConquestRelationSetCommand(ChatHandler* handler, uint32 guildA, uint32 guildB, int8 relation)
{
    ConquestCore::Instance().SetGuildRelation(guildA, guildB, relation);
    handler->PSendSysMessage("Guild relation set between {} and {} to {}.", guildA, guildB, (int)relation);
    return true;
}

bool ConquestCommands::HandleConquestRelationListCommand(ChatHandler* handler)
{
    handler->SendSysMessage("Relations listing: (use SQL for full view)");
    handler->SendSysMessage("Use DB query: SELECT * FROM conquest_guild_relations");
    return true;
}

// Add all command scripts
void AddConquestCommandScripts()
{
    new ConquestCommands();
}

