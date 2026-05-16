/*
 * Conquest Frontline — OutdoorPvP custom pour le système de capture H vs A
 */

#include "OutdoorPvPConquest.h"
#include "ConquestPoints.h"
#include "ConquestWaypointMgr.h"
#include "CharacterDatabase.h"
#include "Chat.h"
#include "Config.h"
#include "DBCStores.h"
#include "GameObject.h"
#include "GameTime.h"
#include "Log.h"
#include "Map.h"
#include "MapMgr.h"
#include "ObjectAccessor.h"
#include "OutdoorPvPMgr.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "RandomPlayerbotMgr.h"
#include "ScriptMgr.h"
#include <unordered_map>

// Map des bots en attente de redispatch après capture (défense 30s).
// Accédée depuis OutdoorPvPConquest.cpp (write) et un PlayerScript (read).
namespace
{
    static std::unordered_map<uint64, uint32> s_pendingRedispatch;
}

void ConquestScheduleDefenseRedispatch(uint64 botGuid, uint32 atMs)
{
    s_pendingRedispatch[botGuid] = atMs;
}

bool ConquestPopRedispatchIfReady(uint64 botGuid, uint32 nowMs)
{
    auto it = s_pendingRedispatch.find(botGuid);
    if (it == s_pendingRedispatch.end()) return false;
    if (nowMs < it->second) return false;
    s_pendingRedispatch.erase(it);
    return true;
}

// Singleton global pour le hook d'auto-détection
OutdoorPvPConquest* g_conquestInstance = nullptr;

OutdoorPvPConquest::OutdoorPvPConquest()
{
    _typeId = OUTDOOR_PVP_CONQUEST_TYPE;
    g_conquestInstance = this;
}

OutdoorPvPConquest::~OutdoorPvPConquest()
{
    if (g_conquestInstance == this)
        g_conquestInstance = nullptr;
}

void OutdoorPvPConquest::EnsureZoneRegistered(uint32 zoneId)
{
    if (zoneId == 0)
        return;
    if (!_registeredZones.insert(zoneId).second)
        return; // déjà enregistrée
    RegisterZone(zoneId);
    LOG_INFO("conquest", "EnsureZoneRegistered: zone {} attached dynamically", zoneId);
}

// Attache rétroactivement tous les joueurs déjà présents dans `zoneId` sur `map`.
// Sans ça, un banner ajouté à chaud via .gob add ne verrait pas les joueurs
// déjà sur place tant qu'ils ne quittent pas et reviennent.
static void ReattachPlayersInZone(Map* map, uint32 zoneId)
{
    if (!map || zoneId == 0)
        return;
    for (auto itr = map->GetPlayers().begin(); itr != map->GetPlayers().end(); ++itr)
    {
        Player* p = itr->GetSource();
        if (!p)
            continue;
        if (p->GetZoneId() != zoneId)
            continue;
        sOutdoorPvPMgr->HandlePlayerEnterZone(p, zoneId);
    }
}

bool OutdoorPvPConquest::SetupOutdoorPvP()
{
    // Enregistrement des zones où les banners MVP peuvent spawn.
    // CRITIQUE : sans ça, GameObject::SetZoneScript() retourne nullptr →
    // OnGameObjectCreate n'est jamais appelé → _capturePoint reste nullptr →
    // OPvPCapturePoint::Update sort early → aucun HandlePlayerEnter.
    // Cette liste statique couvre les zones bootstrappées au démarrage ; toute
    // zone supplémentaire est ajoutée à chaud par EnsureZoneRegistered() lors
    // du spawn d'un banner (via RegisterCapturePoint), donc placer un banner
    // n'importe où via .gob add l'active automatiquement.
    static const uint32 BANNER_ZONES[] = {
        // ============ MVP zones (5 banners originaux) ============
         17, // The Barrens (Crossroads)
        331, // Ashenvale (Astranaar + Splintertree Post)
        267, // Hillsbrad Foothills (Southshore + Tarren Mill)
        440, // Tanaris (Gadgetzan)
        // ============ Alliance Eastern Kingdoms ============
         12, // Elwynn Forest (Goldshire)
         40, // Westfall (Sentinel Hill)
         44, // Redridge Mountains (Lakeshire)
         10, // Duskwood (Darkshire)
         38, // Loch Modan (Thelsamar)
         11, // Wetlands (Menethil Harbor)
         45, // Arathi Highlands (Refuge Pointe + Hammerfall)
         47, // The Hinterlands (Aerie Peak + Revantusk)
        // ============ Alliance Kalimdor ============
        148, // Darkshore (Auberdine)
        357, // Feralas (Feathermoon + Camp Mojache)
        405, // Dustwallow Marsh (Theramore + Brackenwall)
        // ============ Horde Eastern Kingdoms ============
         85, // Tirisfal Glades (Brill)
        130, // Silverpine Forest (Sepulcher)
         33, // Stranglethorn Vale (Grom'gol)
          8, // Swamp of Sorrows (Stonard)
        // ============ Horde Kalimdor ============
         14, // Durotar (Razor Hill)
        215, // Mulgore (Bloodhoof Village)
    };
    for (uint32 zoneId : BANNER_ZONES)
    {
        RegisterZone(zoneId);
        _registeredZones.insert(zoneId);
    }

    LOG_INFO("conquest", "OutdoorPvPConquest::SetupOutdoorPvP - manager initialized, {} zones registered", sizeof(BANNER_ZONES) / sizeof(BANNER_ZONES[0]));

    // Charge le graphe de waypoints (GO entry 400100) après que la WorldDatabase
    // est prête. SetupOutdoorPvP s'exécute pendant OutdoorPvPMgr::InitOutdoorPvP
    // qui tourne après le chargement du monde (gameobject table déjà loaded).
    sConquestWaypointMgr->Load();
    return true;
}

void OutdoorPvPConquest::RegisterCapturePoint(GameObject* go)
{
    if (!go)
        return;

    if (go->GetEntry() != CONQUEST_BANNER_ENTRY)
        return;

    // Dédup par position arrondie : SetCapturePointData() crée un "ghost banner"
    // à la même position que l'original, qui re-déclenche ce hook. Sans cette garde,
    // chaque ghost en spawne un nouveau → boucle infinie de ConquestCapturePoint.
    std::string posKey = std::to_string(go->GetMapId()) + ":" +
                         std::to_string(int(go->GetPositionX())) + ":" +
                         std::to_string(int(go->GetPositionY()));
    if (!_registeredPositions.insert(posKey).second)
        return; // déjà enregistré pour cette position

    LOG_INFO("conquest", "RegisterCapturePoint: GO entry={} spawn_id={} at ({:.1f},{:.1f},{:.1f})",
             go->GetEntry(), go->GetSpawnId(),
             go->GetPositionX(), go->GetPositionY(), go->GetPositionZ());

    // Garantit que la zone de ce banner est attachée à l'OutdoorPvP. Couvre
    // les banners placés via .gob add dans des zones non-listées au boot.
    uint32 zoneId = go->GetZoneId();
    bool zoneWasNew = (_registeredZones.find(zoneId) == _registeredZones.end());
    EnsureZoneRegistered(zoneId);
    // Si la zone vient d'être attachée à chaud, rattache les joueurs déjà présents.
    if (zoneWasNew)
        ReattachPlayersInZone(go->GetMap(), zoneId);

    // Crée et associe le capture point
    ConquestCapturePoint* cp = new ConquestCapturePoint(this, go);

    G3D::Quat rot = go->GetWorldRotation();
    if (!cp->SetCapturePointData(go->GetEntry(),
                                  go->GetMapId(),
                                  go->GetPositionX(),
                                  go->GetPositionY(),
                                  go->GetPositionZ(),
                                  go->GetOrientation(),
                                  rot.x, rot.y, rot.z, rot.w))
    {
        LOG_ERROR("conquest", "RegisterCapturePoint: SetCapturePointData failed for entry={}", go->GetEntry());
        delete cp;
        return;
    }

    // Enregistre dans le manager (map interne d'OutdoorPvP)
    AddCapturePoint(cp);
    AddConquestPoint(cp); // tracker custom pour FindOpportunisticBanner

    LOG_INFO("conquest", "RegisterCapturePoint: capture point ready '{}' (zone={})",
             cp->GetZoneName().c_str(), go->GetSpawnId());
}

uint32 OutdoorPvPConquest::RescanAllBanners(bool force)
{
    if (force)
    {
        // On reconstruit le dedupe set a partir des cp vivants au lieu de
        // le vider sechement. Vider+re-iterer cree des doublons : chaque cp
        // genere son propre "ghost GO" via SetCapturePointData, donc l'ancien
        // cp (avec son slider en cours) ET le nouveau cp neutre coexistent a
        // la meme position. Cette version laisse les cp deja enregistres
        // intacts et ne re-register que les banners qui n'ont pas encore de
        // cp (typiquement spawnes apres le boot via .gob add ou SQL).
        _registeredPositions.clear();
        for (ConquestCapturePoint* cp : _conquestPoints)
        {
            if (!cp || !cp->IsBannerAlive()) continue;
            std::string posKey = std::to_string(cp->GetMapId()) + ":" +
                                 std::to_string(int(cp->GetX())) + ":" +
                                 std::to_string(int(cp->GetY()));
            _registeredPositions.insert(posKey);
        }
    }

    uint32 found = 0;
    sMapMgr->DoForAllMaps([&](Map* map)
    {
        if (!map) return;
        auto& store = map->GetGameObjectBySpawnIdStore();
        for (auto const& [spawnId, go] : store)
        {
            if (!go || go->GetEntry() != CONQUEST_BANNER_ENTRY) continue;
            RegisterCapturePoint(go);
            ++found;
        }
    });
    LOG_INFO("conquest", "RescanAllBanners: scanned {} live banner GOs (force={})", found, force);
    return found;
}

std::vector<OutdoorPvPConquest::BannerSnapshot> OutdoorPvPConquest::GetAllBanners() const
{
    std::vector<BannerSnapshot> out;
    out.reserve(_conquestPoints.size());
    for (ConquestCapturePoint* cp : _conquestPoints)
    {
        if (!cp) continue;
        // Filtre les banners phantom (GO supprimé via .gob delete mais cp encore en mémoire)
        if (!cp->IsBannerAlive()) continue;
        BannerSnapshot s;
        s.mapId    = cp->GetMapId();
        s.x        = cp->GetX();
        s.y        = cp->GetY();
        s.z        = cp->GetZ();
        s.slider   = cp->GetSliderValue();
        s.maxValue = cp->GetSliderMax();
        s.zoneName = cp->GetZoneName();
        out.push_back(s);
    }
    return out;
}

OutdoorPvPConquest::OpportunisticBanner OutdoorPvPConquest::FindOpportunisticBanner(
    uint32 mapId, float x, float y, uint32 teamId, float radius) const
{
    OpportunisticBanner result;
    float bestDistSq = radius * radius;
    for (ConquestCapturePoint* cp : _conquestPoints)
    {
        if (!cp || cp->GetMapId() != mapId)
            continue;
        // Skip si la bannière est déjà fully-locked pour notre équipe (rien à gagner)
        float sliderVal = cp->GetSliderValue();
        float sliderMax = cp->GetSliderMax();
        bool lockedForUs = (teamId == TEAM_ALLIANCE && sliderVal >= sliderMax - 0.001f) ||
                           (teamId == TEAM_HORDE    && sliderVal <= -sliderMax + 0.001f);
        if (lockedForUs)
            continue;
        float dx = cp->GetX() - x;
        float dy = cp->GetY() - y;
        float dSq = dx * dx + dy * dy;
        if (dSq < bestDistSq)
        {
            bestDistSq = dSq;
            result.found = true;
            result.x = cp->GetX();
            result.y = cp->GetY();
            result.z = cp->GetZ();
            result.mapId = cp->GetMapId();
        }
    }
    return result;
}

// ============= ConquestCapturePoint =============

ConquestCapturePoint::ConquestCapturePoint(OutdoorPvP* pvp, GameObject* go)
    : OPvPCapturePoint(pvp)
{
    if (!go)
        return;

    _mapId = go->GetMapId();
    _x = go->GetPositionX();
    _y = go->GetPositionY();
    _z = go->GetPositionZ();
    _sourceSpawnId = go->GetSpawnId();

    if (GameObjectTemplate const* goinfo = go->GetGOInfo())
        _captureRadius = float(goinfo->capturePoint.radius);
    if (_captureRadius <= 0.0f)
        _captureRadius = 45.0f;

    if (Map* map = go->GetMap())
        map->GetZoneAndAreaId(go->GetPhaseMask(), _zoneId, _areaId, _x, _y, _z);

    if (AreaTableEntry const* area = sAreaTableStore.LookupEntry(_areaId))
        _zoneName = area->area_name[0]; // English fallback
    else
        _zoneName = "Unknown";
}

bool ConquestCapturePoint::HandlePlayerEnter(Player* player)
{
    bool added = OPvPCapturePoint::HandlePlayerEnter(player);
    // Note : la base class fait un double-insert, donc `added` retourne toujours false
    // côté override. On envoie le message inconditionnellement — cette fonction est appelée
    // par OPvPCapturePoint::Update UNIQUEMENT quand le joueur vient d'être ajouté.
    if (player && player->GetSession())
    {
        ChatHandler(player->GetSession()).PSendSysMessage(
            "|cffffd000[Conqu\xC3\xAAte]|r Tu es entr\xC3\xA9 en zone de capture : |cff00ff00{}|r.",
            _zoneName);
        LOG_INFO("conquest", "[{}] HandlePlayerEnter fired for '{}'",
                 _zoneName.c_str(), player->GetName());
    }

    // CONQUEST PATCH — Redispatch immédiat si bot arrive sur banner DÉJÀ locked pour
    // sa faction. Sans ça, les bots arrivant après le full-lock atFullLock event
    // restent stuck à attendre 60-300s leur prochain tick périodique pour bouger.
    if (player && std::abs(_value) >= _maxValue - 0.001f)
    {
        TeamId lockedTeam = (_value > 0) ? TEAM_ALLIANCE : TEAM_HORDE;
        if (player->GetTeamId() == lockedTeam)
        {
            if (PlayerbotAI* ai = GET_PLAYERBOT_AI(player))
            {
                (void)ai;
                sRandomPlayerbotMgr.RandomTeleportForLevel(player);
                LOG_INFO("conquest", "[{}] Late-arrival redispatch: '{}' (banner already locked)",
                         _zoneName.c_str(), player->GetName());
            }
        }
    }
    return added;
}

bool ConquestCapturePoint::Update(uint32 diff)
{
    bool changed = OPvPCapturePoint::Update(diff);

    // FULL LOCK detection + fire FX driven par la valeur du slider :
    //   - inFlux = slider bouge (|val| > 0) et pas encore au full lock → FIRE ON
    //   - atFullLock = slider à ±maxValue → FIRE OFF + rewards distribués (one-shot)
    //   - balanced (val = 0) → FIRE OFF (état idle neutre)
    if (_maxValue > 0.0f)
    {
        float absVal = std::abs(_value);
        bool atFullLock = absVal >= _maxValue - 0.001f;
        bool inFlux     = absVal > 0.001f && !atFullLock;

        // Sync le feu avec l'état "contesté"
        if (inFlux && !_fireGuid)
            SpawnFireFX();
        else if (!inFlux && _fireGuid)
            DespawnFireFX();

        // FULL LOCK rewards (one-shot par cycle de lock)
        if (atFullLock && !_rewardsFired)
        {
            _rewardsFired = true;
            TeamId capturingTeam = (_value > 0) ? TEAM_ALLIANCE : TEAM_HORDE;
            char const* newName = (capturingTeam == TEAM_ALLIANCE) ? "Alliance" : "Horde";
            char const* color   = (capturingTeam == TEAM_ALLIANCE) ? "|cff00aaff" : "|cffff4040";

            LOG_INFO("conquest", "[{}] FULL LOCK by {} (slider={:.1f}/{:.1f}) — awarding +5 PC",
                     _zoneName.c_str(), newName, _value, _maxValue);

            BroadcastZoneAnnounce(
                std::string("|cffffd000[Conqu") + "\xC3\xAAte] " + _zoneName +
                " verrouill" + "\xC3\xA9" + "e par " + color + newName + "|r|r ! +5 PC distribu" + "\xC3\xA9" + "s.");
            AwardCaptureRewards(capturingTeam);

            // CONQUEST PATCH — Capture trigger : schedule un redispatch DIFFÉRÉ
            // de 30s pour les bots de la faction gagnante autour du banner. Pendant
            // ces 30s ils restent immobiles (défense fictive). Au tick après les 30s,
            // un PlayerScript hook (OnPlayerBeforeUpdate) déclenche la redispatch.
            if (Map* m = sMapMgr->FindBaseNonInstanceMap(_mapId))
            {
                uint32 nowMs = GameTime::GetGameTimeMS().count();
                uint32 defenseDelay = sConfigMgr->GetOption<uint32>("ConquestFrontline.DefenseDurationMs", 30000);
                int scheduled = 0;
                Map::PlayerList const& players = m->GetPlayers();
                for (auto itr = players.begin(); itr != players.end(); ++itr)
                {
                    Player* p = itr->GetSource();
                    if (!p || !p->IsInWorld() || p->GetTeamId() != capturingTeam)
                        continue;
                    PlayerbotAI* ai = GET_PLAYERBOT_AI(p);
                    if (!ai)
                        continue;
                    // Toute la zone : on schedule tous les bots de la team
                    // capturante dans la zone du banner. Couvre les bots qui
                    // patrouillent (>200y du banner) mais sont quand meme
                    // affectes par la capture. ConquestDefenseRedispatch va
                    // ensuite reveiller leur leader de groupe si applicable.
                    if (p->GetZoneId() != _zoneId)
                        continue;
                    ConquestScheduleDefenseRedispatch(p->GetGUID().GetRawValue(),
                                                      nowMs + defenseDelay);
                    ++scheduled;
                }
                LOG_INFO("conquest", "[{}] Capture trigger: {} bots de {} en defense {}ms",
                         _zoneName.c_str(), scheduled, newName, defenseDelay);
            }
        }
        else if (!atFullLock && _rewardsFired)
        {
            _rewardsFired = false;
            LOG_INFO("conquest", "[{}] Lock broken (slider back below maxValue) — rewards re-armed",
                     _zoneName.c_str());
        }
    }

    _progressTickMs += diff;
    uint32 broadcastEvery = sConfigMgr->GetOption<uint32>("ConquestFrontline.ProgressBroadcastMs", 5000);
    if (_progressTickMs >= broadcastEvery)
    {
        _progressTickMs = 0;
        BroadcastProgress();
    }
    return changed;
}

void ConquestCapturePoint::BroadcastProgress()
{
    if (_maxValue <= 0.0f)
        return;

    // Skip UNIQUEMENT quand le slider est au full lock (atteint ±maxValue).
    // Ne pas se baser sur _state : la framework AC set _state = ALLIANCE dès
    // slider > minValue (CHALLENGE et LOCK partagent le même state), donc
    // un check sur _state cacherait toute la phase de consolidation.
    if (std::abs(_value) >= _maxValue - 0.001f)
        return;

    // Affichage per-phase 0-100% :
    //   - Slider en [-minValue, +minValue] (zone NEUTRE) : "Neutre 0%..100%"
    //     où 0% = parfaitement balanced (slider=0) et 100% = sur le point de basculer
    //   - Slider en [+minValue, +maxValue] : "Alliance 0%..100%"
    //     où 0% = vient de basculer en CHALLENGE et 100% = FULL LOCK
    //   - Slider en [-maxValue, -minValue] : "Horde 0%..100%" (symétrique)
    float absVal = std::abs(_value);
    int percent = 0;
    char const* phaseLabel = "Neutre";
    char const* color = "|cffaaaaaa"; // gris par défaut (neutre)

    if (absVal < _minValue)
    {
        // Phase NEUTRE — proximité du basculement faction
        percent = (_minValue > 0.0f) ? int((absVal / _minValue) * 100.0f) : 0;
        if (_value > 0.1f)      phaseLabel = "Neutre \xE2\x86\x92 Alliance"; // → Alliance
        else if (_value < -0.1f) phaseLabel = "Neutre \xE2\x86\x92 Horde";    // → Horde
        else                     phaseLabel = "Neutre (balanced)";
        color = "|cffffd700"; // jaune-or
    }
    else
    {
        // Phase CHALLENGE — consolidation de l'ownership
        float range = _maxValue - _minValue;
        if (range <= 0.0f) range = 1.0f;
        percent = int(((absVal - _minValue) / range) * 100.0f);
        if (_value > 0)
        {
            phaseLabel = "Alliance";
            color = "|cff00aaff"; // bleu
        }
        else
        {
            phaseLabel = "Horde";
            color = "|cffff4040"; // rouge
        }
    }

    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;

    for (uint8 team = 0; team < 2; ++team)
    {
        for (ObjectGuid const& guid : _activePlayers[team])
        {
            if (Player* p = ObjectAccessor::FindPlayer(guid))
            {
                if (p->GetSession())
                    ChatHandler(p->GetSession()).PSendSysMessage(
                        "|cffffd000[{}]|r {} {}|r : {}%",
                        _zoneName, color, phaseLabel, percent);
            }
        }
    }
}

void ConquestCapturePoint::SpawnFireFX()
{
    if (_fireGuid)
        return; // déjà spawn
    Map* map = sMapMgr->FindBaseNonInstanceMap(_mapId);
    if (!map)
        return;
    // Entry 185319 = vrai feu visuel
    // respawnTime=0 → persistant jusqu'à Delete() explicite
    constexpr uint32 FIRE_GO_ENTRY = 185319;
    if (GameObject* fire = map->SummonGameObject(FIRE_GO_ENTRY, _x, _y, _z, 0.0f, 0, 0, 0, 0, 0))
    {
        _fireGuid = fire->GetGUID();
        LOG_INFO("conquest", "[{}] Fire FX spawned (GUID={})", _zoneName.c_str(), _fireGuid.ToString());
    }
}

void ConquestCapturePoint::DespawnFireFX()
{
    if (!_fireGuid)
        return;
    Map* map = sMapMgr->FindBaseNonInstanceMap(_mapId);
    if (!map)
    {
        _fireGuid.Clear();
        return;
    }
    if (GameObject* fire = map->GetGameObject(_fireGuid))
    {
        fire->Delete();
        LOG_INFO("conquest", "[{}] Fire FX despawned", _zoneName.c_str());
    }
    _fireGuid.Clear();
}

void ConquestCapturePoint::BroadcastZoneAnnounce(std::string const& message)
{
    Map* map = sMapMgr->FindBaseNonInstanceMap(_mapId);
    if (!map)
    {
        LOG_WARN("conquest", "BroadcastZoneAnnounce: map {} not found", _mapId);
        return;
    }

    uint32 reached = 0;
    Map::PlayerList const& players = map->GetPlayers();
    for (auto itr = players.begin(); itr != players.end(); ++itr)
    {
        Player* p = itr->GetSource();
        if (!p || !p->GetSession())
            continue;
        if (p->GetZoneId() != _zoneId)
            continue;
        ChatHandler(p->GetSession()).PSendSysMessage("{}", message);
        ++reached;
    }
    LOG_INFO("conquest", "BroadcastZoneAnnounce [{}] reached {} players (zone={})",
             _zoneName.c_str(), reached, _zoneId);
}

void ConquestCapturePoint::ChangeState()
{
    char const* stateName = "?";
    uint8 artkit = 21; // neutre par défaut
    switch (_state)
    {
        case OBJECTIVESTATE_NEUTRAL:                    stateName = "NEUTRAL";          artkit = 21; break;
        case OBJECTIVESTATE_ALLIANCE:                   stateName = "ALLIANCE";         artkit = 2;  break;
        case OBJECTIVESTATE_HORDE:                      stateName = "HORDE";            artkit = 1;  break;
        case OBJECTIVESTATE_NEUTRAL_ALLIANCE_CHALLENGE: stateName = "NEUTRAL->ALLIANCE"; artkit = 21; break;
        case OBJECTIVESTATE_NEUTRAL_HORDE_CHALLENGE:    stateName = "NEUTRAL->HORDE";    artkit = 21; break;
        case OBJECTIVESTATE_ALLIANCE_HORDE_CHALLENGE:   stateName = "ALLIANCE->HORDE";   artkit = 2;  break;
        case OBJECTIVESTATE_HORDE_ALLIANCE_CHALLENGE:   stateName = "HORDE->ALLIANCE";   artkit = 1;  break;
        default: break;
    }

    // Swap visual artKit du banner selon état (vanilla pattern: 1=Horde, 2=Alliance, 21=Neutral)
    if (Map* map = sMapMgr->FindBaseNonInstanceMap(_mapId))
    {
        auto bounds = map->GetGameObjectBySpawnIdStore().equal_range(m_capturePointSpawnId);
        for (auto itr = bounds.first; itr != bounds.second; ++itr)
            itr->second->SetGoArtKit(artkit);
    }

    LOG_INFO("conquest", "[{}] State changed from {} to {} (artKit={})",
             _zoneName.c_str(),
             (_oldState == OBJECTIVESTATE_ALLIANCE ? "ALLIANCE" : _oldState == OBJECTIVESTATE_HORDE ? "HORDE" : "other"),
             stateName, artkit);

    // Note: la détection FULL LOCK + rewards est faite dans Update() qui surveille
    // la valeur du slider (vs maxValue). Le state OBJECTIVESTATE_ALLIANCE bascule
    // dès slider > minValue donc on ne peut pas s'y fier pour distinguer CHALLENGE de LOCK.
}

void ConquestCapturePoint::ChangeTeam(TeamId oldTeam)
{
    // ChangeTeam fire au passage en CHALLENGE (slider franchit ±minValue).
    // Ici on émet l'**annonce d'ownership flip** ("Gadgetzan capturée par Alliance").
    // Les rewards (+5 PC) sont distribués plus tard dans ChangeState sur FULL LOCK.
    char const* oldName = (oldTeam == TEAM_ALLIANCE ? "Alliance" : (oldTeam == TEAM_HORDE ? "Horde" : "Neutral"));
    char const* newName = (_team == TEAM_ALLIANCE ? "Alliance" : (_team == TEAM_HORDE ? "Horde" : "Neutral"));
    LOG_INFO("conquest", "[{}] Team shift {} -> {} (challenge phase = ownership flip)",
             _zoneName.c_str(), oldName, newName);

    // Persist banner ownership so out-of-process tooling (e.g. the conquest-map
    // dashboard) can read live faction state. faction column convention:
    //   0 = neutral / disputed, 1 = Alliance, 2 = Horde.
    //
    // zone_guid must be the source banner's gameobject.guid (i.e. the row
    // visible in acore_world.gameobject), NOT m_capturePointSpawnId — the
    // OutdoorPvP framework spawns a separate ghost GameObject during
    // SetCapturePointData() with its own runtime-only guid that has no row
    // in the gameobject table, so joining captures back to a banner would
    // always miss.
    //
    // Preserves last_reward_time on existing rows.
    uint8 const dbFaction =
        (_team == TEAM_ALLIANCE) ? 1 :
        (_team == TEAM_HORDE)    ? 2 : 0;
    if (_sourceSpawnId != 0)
    {
        CharacterDatabase.Execute(
            "INSERT INTO conquest_zone_control "
            "(zone_guid, guild_id, faction, original_entry, captured_at, last_reward_time) "
            "VALUES ({}, 0, {}, {}, UNIX_TIMESTAMP(), 0) "
            "ON DUPLICATE KEY UPDATE faction = VALUES(faction), "
            "captured_at = VALUES(captured_at)",
            _sourceSpawnId,
            dbFaction,
            CONQUEST_BANNER_ENTRY);
    }

    // Annonce zone-wide sur tout changement vers une faction (flip ennemi ou capture neutre).
    // Le feu FX est géré directement dans Update() en fonction du slider (plus fiable que d'attendre ChangeTeam).
    bool isFactionFlip = (_team == TEAM_ALLIANCE || _team == TEAM_HORDE) && oldTeam != _team;
    if (isFactionFlip)
    {
        char const* color = (_team == TEAM_ALLIANCE) ? "|cff00aaff" : "|cffff4040";
        BroadcastZoneAnnounce(
            std::string("|cffffd000[Conqu") + "\xC3\xAAte] " + _zoneName +
            " captur" + "\xC3\xA9" + "e par " + color + newName + "|r|r!");
    }
}

void ConquestCapturePoint::AwardCaptureRewards(TeamId capturingTeam)
{
    Map* map = sMapMgr->FindBaseNonInstanceMap(_mapId);
    if (!map)
        return;

    uint32 reward = sConfigMgr->GetOption<uint32>("ConquestFrontline.ConquestPointsPerCapture", 5);
    if (reward == 0)
        return;

    constexpr uint32 ITEM_MARQUE_CONQUETE = 400301;

    uint32 awarded = 0;
    Map::PlayerList const& mapPlayers = map->GetPlayers();
    for (auto itr = mapPlayers.begin(); itr != mapPlayers.end(); ++itr)
    {
        Player* p = itr->GetSource();
        if (!p || !p->IsInWorld() || p->IsGameMaster())
            continue;
        if (p->GetTeamId() != capturingTeam)
            continue;
        if (p->GetDistance2d(_x, _y) > _captureRadius)
            continue;

        // 1. Wallet DB (utilisé par les vendors pour la balance)
        ConquestPoints::AddConquestPoints(p, reward);

        // 2. Items visibles dans le sac (UX — le joueur voit ses gains)
        ItemPosCountVec dest;
        InventoryResult msg = p->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, ITEM_MARQUE_CONQUETE, reward);
        if (msg == EQUIP_ERR_OK)
        {
            if (Item* it = p->StoreNewItem(dest, ITEM_MARQUE_CONQUETE, true))
                p->SendNewItem(it, reward, true, false);
        }
        // Si inventaire plein, on garde le wallet DB intact (pas de refund)

        ++awarded;
    }

    LOG_INFO("conquest", "[{}] AwardCaptureRewards: {} player(s) de la faction capturante ont reçu {} PC (wallet + bag)",
             _zoneName.c_str(), awarded, reward);
}

// ============= Script registration =============

class OutdoorPvPScript_Conquest : public OutdoorPvPScript
{
public:
    OutdoorPvPScript_Conquest() : OutdoorPvPScript("outdoorpvp_conquest") {}

    OutdoorPvP* GetOutdoorPvP() const override
    {
        return new OutdoorPvPConquest();
    }
};

void AddSC_outdoorpvp_conquest()
{
    new OutdoorPvPScript_Conquest();
}
