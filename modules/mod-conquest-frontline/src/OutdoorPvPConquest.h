/*
 * Conquest Frontline — OutdoorPvP custom pour le système de capture H vs A
 */

#ifndef OUTDOOR_PVP_CONQUEST_H
#define OUTDOOR_PVP_CONQUEST_H

#include "OutdoorPvP.h"
#include "OutdoorPvPScript.h"
#include <unordered_map>
#include <unordered_set>

// Entry du GameObject capture point custom (cf SQL conquest_banner_template.sql)
constexpr uint32 CONQUEST_BANNER_ENTRY = 400010;

// TypeId custom (cf OutdoorPvP.h: OUTDOOR_PVP_CONQUEST = 8)
// MAX_OUTDOORPVP_TYPES a été bumpé à 9 pour réserver ce slot.
constexpr uint32 OUTDOOR_PVP_CONQUEST_TYPE = 8;

class ConquestCapturePoint;
class OutdoorPvPConquest;

// Accès global à l'instance unique de OutdoorPvPConquest pour le hook d'auto-détection
extern OutdoorPvPConquest* g_conquestInstance;

// Manager principal — détient toutes les capture points spawned dans Azeroth
class OutdoorPvPConquest : public OutdoorPvP
{
public:
    OutdoorPvPConquest();
    ~OutdoorPvPConquest() override;

    bool SetupOutdoorPvP() override;

    // Hook custom : appelé quand un GO type 29 d'entry 400010 spawn dans le monde
    // Crée dynamiquement une ConquestCapturePoint associée
    void RegisterCapturePoint(GameObject* go);

    // Idempotent : ajoute zoneId à m_OutdoorPvPMap si pas déjà attaché.
    // Sans cet enregistrement, OutdoorPvPMgr::HandlePlayerEnterZone ne rattache
    // pas les joueurs à cette OutdoorPvP, et OPvPCapturePoint::Update ne tourne
    // jamais dans cette zone (= banner spawné mais inert).
    void EnsureZoneRegistered(uint32 zoneId);

    // Recherche la bannière la plus proche de (mapId, x, y) dans un rayon `radius`
    // qui N'EST PAS encore fully-locked pour `myTeam`. Renvoie {found=true, ...}
    // si trouvée. Utilisé par MoveToTravelTargetAction pour le détour opportuniste
    // (bots qui passent près d'une bannière à capturer/contester).
    struct OpportunisticBanner { bool found = false; float x = 0, y = 0, z = 0; uint32 mapId = 0; };
    OpportunisticBanner FindOpportunisticBanner(uint32 mapId, float x, float y,
                                                 uint32 teamId, float radius) const;

    // Rescan toutes les maps charges : pour chaque GO d'entry 400010 trouve,
    // appelle RegisterCapturePoint. Si force=true, on vide d'abord
    // _registeredPositions pour forcer un re-traitement des banners deja vus.
    // Retourne le nombre de capture points enregistres apres scan.
    uint32 RescanAllBanners(bool force);

    // Snapshot de toutes les bannières pour smart dispatch.
    struct BannerSnapshot {
        uint32 mapId = 0;
        float x = 0, y = 0, z = 0;
        float slider = 0.0f;    // -maxValue (Horde lock) à +maxValue (Alliance lock)
        float maxValue = 0.0f;
        std::string zoneName;
    };
    std::vector<BannerSnapshot> GetAllBanners() const;

private:
    // Dédup par position arrondie (map:x:y) pour ignorer les "ghost banners"
    // créés en interne par OPvPCapturePoint::SetCapturePointData → AddGOData,
    // qui re-déclencheraient le hook OnGameObjectAddWorld → boucle infinie.
    std::unordered_set<std::string> _registeredPositions;

    // Zones déjà enregistrées via EnsureZoneRegistered (pour skip rapide).
    std::unordered_set<uint32> _registeredZones;

    // Tracker custom des ConquestCapturePoint pour itération rapide (la base class
    // OutdoorPvP les stocke dans m_capturePoints map<keyed-by-spawnId>, mais on
    // garde aussi notre propre vecteur pour FindOpportunisticBanner).
    std::vector<ConquestCapturePoint*> _conquestPoints;
public:
    void AddConquestPoint(ConquestCapturePoint* cp) { _conquestPoints.push_back(cp); }
};

// Un point de capture custom
class ConquestCapturePoint : public OPvPCapturePoint
{
public:
    explicit ConquestCapturePoint(OutdoorPvP* pvp, GameObject* go);

    bool HandlePlayerEnter(Player* player) override;
    bool Update(uint32 diff) override;
    void ChangeState() override;
    void ChangeTeam(TeamId oldTeam) override;

    std::string const& GetZoneName() const { return _zoneName; }
    uint32 GetMapId() const { return _mapId; }
    float  GetX() const { return _x; }
    float  GetY() const { return _y; }
    float  GetZ() const { return _z; }
    float  GetSliderValue() const { return _value; }
    float  GetSliderMax() const { return _maxValue; }
    bool   IsBannerAlive() const { return _capturePoint != nullptr; }

private:
    void AwardCaptureRewards(TeamId capturingTeam);
    void BroadcastProgress();
    void BroadcastZoneAnnounce(std::string const& message);
    void SpawnFireFX();
    void DespawnFireFX();

    std::string _zoneName;
    uint32 _zoneId{};
    uint32 _areaId{};
    uint32 _mapId{};
    float _x{}, _y{}, _z{};
    float _captureRadius{};
    uint32 _progressTickMs{};
    bool _rewardsFired{false}; // true tant qu'un FULL LOCK est en cours, reset quand slider redescend
    ObjectGuid _fireGuid;       // GUID du GO "feu" spawné pendant CHALLENGE (vide quand pas de feu)

    // Spawn id of the source banner row in acore_world.gameobject. Persisted
    // as `conquest_zone_control.zone_guid` so out-of-process tooling
    // (conquest-map dashboard) can join captures back to their banner. We
    // can't reuse m_capturePointSpawnId because the OutdoorPvP framework
    // generates a ghost GameObject during SetCapturePointData() with a
    // different guid that does not exist in the gameobject table.
    uint32 _sourceSpawnId{};
};

#endif // OUTDOOR_PVP_CONQUEST_H
