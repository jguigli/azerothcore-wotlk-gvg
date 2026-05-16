/*
 * Conquest Respawn Capital — chaque race respawn dans sa capitale après mort
 *
 * Hook OnPlayerReleasedGhost  : téléport en capitale au moment où le joueur libère son fantôme
 * Hook OnPlayerCreate         : nouveau perso direct en capitale (optionnel)
 * Hook OnPlayerLogin          : update homebind à la capitale (optionnel — hearthstone)
 *
 * Mapping race → capitale (cf gameplay-design.md section Personnages) :
 *   Alliance : Human/Dwarf/NightElf/Gnome/Draenei → Stormwind/Ironforge/Darnassus/Ironforge/Exodar
 *   Horde    : Orc/Undead/Tauren/Troll/BloodElf  → Orgrimmar/Undercity/ThunderBluff/Orgrimmar/Silvermoon
 */

#include "ScriptMgr.h"
#include "Player.h"
#include "Config.h"
#include "Log.h"
#include "World.h"

namespace
{
    struct CapitalLocation
    {
        uint32 mapId;
        float  x;
        float  y;
        float  z;
        float  o;
        uint32 zoneId; // pour homebind
        uint32 areaId; // pour homebind (optional, mais cohérent)
    };

    // Coords standard des capitales (lieu central de chaque ville)
    constexpr CapitalLocation CAPITAL_STORMWIND   = {   0, -8842.09f,   626.16f,   94.05f, 3.62f, 1519,  1519 };
    constexpr CapitalLocation CAPITAL_IRONFORGE   = {   0, -4918.88f,  -940.40f,  501.55f, 5.40f, 1537,  1537 };
    constexpr CapitalLocation CAPITAL_DARNASSUS   = {   1,  9670.49f,  2497.07f, 1335.41f, 0.00f, 1657,  1657 };
    constexpr CapitalLocation CAPITAL_EXODAR      = { 530, -3961.64f,-11643.50f, -138.45f, 0.00f, 3557,  3557 };
    constexpr CapitalLocation CAPITAL_ORGRIMMAR   = {   1,  1676.21f, -4315.29f,   61.52f, 0.00f, 1637,  1637 };
    constexpr CapitalLocation CAPITAL_UNDERCITY   = {   0,  1633.75f,   240.17f,  -43.10f, 3.16f, 1497,  1497 };
    constexpr CapitalLocation CAPITAL_THUNDERBLUFF= {   1, -1290.16f,   145.62f,  130.65f, 0.00f, 1638,  1638 };
    constexpr CapitalLocation CAPITAL_SILVERMOON  = { 530,  9487.41f, -7279.41f,   14.30f, 0.00f, 3487,  3487 };

    CapitalLocation GetCapitalForRace(uint8 race)
    {
        switch (race)
        {
            case RACE_HUMAN:        return CAPITAL_STORMWIND;
            case RACE_DWARF:        return CAPITAL_IRONFORGE;
            case RACE_NIGHTELF:     return CAPITAL_DARNASSUS;
            case RACE_GNOME:        return CAPITAL_IRONFORGE;
            case RACE_DRAENEI:      return CAPITAL_EXODAR;

            case RACE_ORC:          return CAPITAL_ORGRIMMAR;
            case RACE_UNDEAD_PLAYER:return CAPITAL_UNDERCITY;
            case RACE_TAUREN:       return CAPITAL_THUNDERBLUFF;
            case RACE_TROLL:        return CAPITAL_ORGRIMMAR;
            case RACE_BLOODELF:     return CAPITAL_SILVERMOON;

            // DK starting race fallback (déjà mapping ci-dessus mais au cas où)
            default:                return CAPITAL_STORMWIND;
        }
    }

    char const* GetCapitalName(uint8 race)
    {
        switch (race)
        {
            case RACE_HUMAN:        return "Stormwind";
            case RACE_DWARF:        return "Ironforge";
            case RACE_NIGHTELF:     return "Darnassus";
            case RACE_GNOME:        return "Ironforge";
            case RACE_DRAENEI:      return "Exodar";
            case RACE_ORC:          return "Orgrimmar";
            case RACE_UNDEAD_PLAYER:return "Undercity";
            case RACE_TAUREN:       return "Thunder Bluff";
            case RACE_TROLL:        return "Orgrimmar";
            case RACE_BLOODELF:     return "Silvermoon";
            default:                return "Unknown";
        }
    }

    bool IsEnabled()
    {
        return sConfigMgr->GetOption<bool>("ConquestRespawnCapital.Enable", true);
    }

    void TeleportToCapital(Player* player)
    {
        if (!player)
            return;

        CapitalLocation loc = GetCapitalForRace(player->getRace());
        player->TeleportTo(loc.mapId, loc.x, loc.y, loc.z, loc.o);

        LOG_INFO("conquest", "RespawnCapital: '{}' (race {}) teleported to {} ({}, {:.1f}, {:.1f}, {:.1f})",
                 player->GetName(), uint32(player->getRace()),
                 GetCapitalName(player->getRace()),
                 loc.mapId, loc.x, loc.y, loc.z);
    }

    void UpdateHomebindToCapital(Player* player)
    {
        if (!player)
            return;

        CapitalLocation loc = GetCapitalForRace(player->getRace());

        WorldLocation home(loc.mapId, loc.x, loc.y, loc.z, loc.o);
        player->SetHomebind(home, loc.areaId);

        LOG_INFO("conquest", "RespawnCapital: homebind set to {} for '{}'",
                 GetCapitalName(player->getRace()), player->GetName());
    }
} // namespace

class ConquestRespawnCapital : public PlayerScript
{
public:
    ConquestRespawnCapital() : PlayerScript("ConquestRespawnCapital") { }

    // Hook prioritaire : on bloque le graveyard teleport du framework et on tp en capitale.
    // Sans ça, le framework appelle RepopAtGraveyard APRÈS notre tp → double teleport →
    // client coincé en loading screen.
    bool OnPlayerCanRepopAtGraveyard(Player* player) override
    {
        if (!IsEnabled() || !player)
            return true; // laisse le framework gérer normalement

        if (player->InBattleground() || player->InArena())
            return true; // pas d'override en BG/arena

        // Ressuscite directement le joueur (sort du fantôme + corps fantôme retiré)
        // puis téléporte en capitale. Retourne false pour empêcher RepopAtGraveyard
        // d'envoyer un second TeleportTo qui ferait coincer le loading screen.
        player->ResurrectPlayer(1.0f); // 100% HP/Mana
        player->SpawnCorpseBones();    // efface le corps physique
        TeleportToCapital(player);
        return false;
    }

    // Nouveau perso : spawn direct en capitale plutôt que dans la zone starter
    void OnPlayerCreate(Player* player) override
    {
        if (!IsEnabled())
            return;

        if (!sConfigMgr->GetOption<bool>("ConquestRespawnCapital.OnCreate", true))
            return;

        if (!player)
            return;

        TeleportToCapital(player);

        if (sConfigMgr->GetOption<bool>("ConquestRespawnCapital.UpdateHomebind", true))
            UpdateHomebindToCapital(player);
    }

    // Premier login (sécurité au cas où OnPlayerCreate ne couvre pas)
    void OnPlayerFirstLogin(Player* player) override
    {
        if (!IsEnabled())
            return;

        if (!sConfigMgr->GetOption<bool>("ConquestRespawnCapital.OnCreate", true))
            return;

        if (!player)
            return;

        TeleportToCapital(player);

        if (sConfigMgr->GetOption<bool>("ConquestRespawnCapital.UpdateHomebind", true))
            UpdateHomebindToCapital(player);
    }
};

void AddSC_ConquestRespawnCapital()
{
    new ConquestRespawnCapital();
}
