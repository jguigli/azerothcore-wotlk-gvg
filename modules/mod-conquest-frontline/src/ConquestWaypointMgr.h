/*
 * Conquest Frontline — Waypoint navigation graph for bots.
 *
 * Loads all GameObjects of entry CONQUEST_WAYPOINT_ENTRY (400100) at world boot,
 * auto-builds edges by proximity (<500y same-map, navmesh validated),
 * exposes getRoute(start, end) via Dijkstra.
 *
 * Used by mod-playerbots MoveToTravelTargetAction to route bots banner-to-banner
 * along curated paths instead of relying on the generic TravelNodeMap (which
 * often returns trivial paths, causing bots to fall back to straight-line
 * splines that cut through terrain).
 */

#ifndef _CONQUEST_WAYPOINT_MGR_H_
#define _CONQUEST_WAYPOINT_MGR_H_

#include "Define.h"
#include "Position.h"
#include <unordered_map>
#include <vector>

constexpr uint32 CONQUEST_WAYPOINT_ENTRY = 400100;

struct ConquestWaypoint
{
    uint32 id = 0;       // gameobject guid (unique)
    uint32 mapId = 0;
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

struct ConquestWaypointEdge
{
    uint32 toId = 0;
    float distance = 0.0f;
};

class ConquestWaypointMgr
{
public:
    static ConquestWaypointMgr* instance();

    /// Scans all GO of entry 400100, loads positions, auto-builds edges
    /// by proximity (<500y same-map). Called after world init.
    void Load();

    /// Returns ordered waypoint positions from nearest WP to `start` to
    /// nearest WP to `end`, via Dijkstra. Empty vector if no path.
    /// Only includes waypoints on the same map as `start`.
    std::vector<Position> GetRoute(uint32 mapId, float startX, float startY, float startZ,
                                   float endX, float endY, float endZ) const;

    /// Returns nearest waypoint id (or 0 if none on this map).
    uint32 GetNearestWaypointId(uint32 mapId, float x, float y, float z) const;

    size_t GetWaypointCount() const { return _waypoints.size(); }
    size_t GetEdgeCount() const;

private:
    ConquestWaypointMgr() = default;

    std::unordered_map<uint32, ConquestWaypoint> _waypoints;          // id → WP
    std::unordered_map<uint32, std::vector<ConquestWaypointEdge>> _adjacency; // id → edges
};

#define sConquestWaypointMgr ConquestWaypointMgr::instance()

#endif // _CONQUEST_WAYPOINT_MGR_H_
