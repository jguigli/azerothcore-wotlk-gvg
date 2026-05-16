/*
 * Conquest Frontline — Waypoint navigation graph for bots.
 * See ConquestWaypointMgr.h for design notes.
 */

#include "ConquestWaypointMgr.h"

#include "DatabaseEnv.h"
#include "Field.h"
#include "Log.h"
#include "QueryResult.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <queue>
#include <utility>

namespace
{
    constexpr float MAX_EDGE_DIST = 500.0f; // auto-link WPs closer than this
}

ConquestWaypointMgr* ConquestWaypointMgr::instance()
{
    static ConquestWaypointMgr inst;
    return &inst;
}

void ConquestWaypointMgr::Load()
{
    _waypoints.clear();
    _adjacency.clear();

    // Scan gameobject table for waypoint spawns. spawnMask=1 → standard difficulty
    // (avoid duplicates from heroic/etc).
    QueryResult result = WorldDatabase.Query(
        "SELECT `guid`, `map`, `position_x`, `position_y`, `position_z` "
        "FROM `gameobject` WHERE `id` = {}", CONQUEST_WAYPOINT_ENTRY);

    if (!result)
    {
        LOG_INFO("conquest", "ConquestWaypointMgr::Load — 0 waypoints found.");
        return;
    }

    do
    {
        Field* f = result->Fetch();
        ConquestWaypoint wp;
        wp.id    = f[0].Get<uint32>();
        wp.mapId = f[1].Get<uint16>();
        wp.x     = f[2].Get<float>();
        wp.y     = f[3].Get<float>();
        wp.z     = f[4].Get<float>();
        _waypoints[wp.id] = wp;
    } while (result->NextRow());

    // Auto-build edges by proximity, same-map only.
    // O(n^2) — fine for n < 1000.
    uint32 edgeCount = 0;
    for (auto const& [idA, wpA] : _waypoints)
    {
        for (auto const& [idB, wpB] : _waypoints)
        {
            if (idA == idB || wpA.mapId != wpB.mapId)
                continue;
            float dx = wpA.x - wpB.x;
            float dy = wpA.y - wpB.y;
            float dz = wpA.z - wpB.z;
            float d  = std::sqrt(dx * dx + dy * dy + dz * dz);
            if (d > MAX_EDGE_DIST)
                continue;
            _adjacency[idA].push_back({idB, d});
            ++edgeCount;
        }
    }

    LOG_INFO("conquest", "ConquestWaypointMgr::Load — {} waypoints, {} directed edges (max edge {}y)",
             _waypoints.size(), edgeCount, (uint32)MAX_EDGE_DIST);
}

size_t ConquestWaypointMgr::GetEdgeCount() const
{
    size_t n = 0;
    for (auto const& [id, edges] : _adjacency)
        n += edges.size();
    return n;
}

uint32 ConquestWaypointMgr::GetNearestWaypointId(uint32 mapId, float x, float y, float z) const
{
    uint32 bestId = 0;
    float bestDistSq = std::numeric_limits<float>::max();
    for (auto const& [id, wp] : _waypoints)
    {
        if (wp.mapId != mapId)
            continue;
        float dx = wp.x - x;
        float dy = wp.y - y;
        float dz = wp.z - z;
        float dSq = dx * dx + dy * dy + dz * dz;
        if (dSq < bestDistSq)
        {
            bestDistSq = dSq;
            bestId = id;
        }
    }
    return bestId;
}

std::vector<Position> ConquestWaypointMgr::GetRoute(uint32 mapId, float startX, float startY, float startZ,
                                                    float endX, float endY, float endZ) const
{
    std::vector<Position> route;

    uint32 startId = GetNearestWaypointId(mapId, startX, startY, startZ);
    uint32 endId   = GetNearestWaypointId(mapId, endX,   endY,   endZ);
    if (!startId || !endId || startId == endId)
        return route;

    // Dijkstra : O((V + E) log V). For our sizes (~200 nodes), instant.
    using PQEntry = std::pair<float, uint32>; // (distance, node id)
    std::priority_queue<PQEntry, std::vector<PQEntry>, std::greater<>> pq;
    std::unordered_map<uint32, float> dist;
    std::unordered_map<uint32, uint32> prev;

    dist[startId] = 0.0f;
    pq.push({0.0f, startId});

    while (!pq.empty())
    {
        auto [d, u] = pq.top();
        pq.pop();
        if (u == endId)
            break;
        if (d > dist[u])
            continue;
        auto it = _adjacency.find(u);
        if (it == _adjacency.end())
            continue;
        for (auto const& edge : it->second)
        {
            float nd = d + edge.distance;
            auto dit = dist.find(edge.toId);
            if (dit == dist.end() || nd < dit->second)
            {
                dist[edge.toId] = nd;
                prev[edge.toId] = u;
                pq.push({nd, edge.toId});
            }
        }
    }

    if (!dist.count(endId))
        return route; // no path

    // Reconstruct path
    std::vector<uint32> chain;
    for (uint32 cur = endId; cur != startId; cur = prev[cur])
    {
        chain.push_back(cur);
        if (!prev.count(cur))
            return std::vector<Position>(); // broken chain
    }
    chain.push_back(startId);
    std::reverse(chain.begin(), chain.end());

    route.reserve(chain.size());
    for (uint32 id : chain)
    {
        auto it = _waypoints.find(id);
        if (it == _waypoints.end())
            continue;
        Position p;
        p.Relocate(it->second.x, it->second.y, it->second.z, 0.0f);
        route.push_back(p);
    }
    return route;
}
