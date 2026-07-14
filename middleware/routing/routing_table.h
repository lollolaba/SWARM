#pragma once

#include <unordered_map>
#include <string>
#include "routing_entry.h"
#include "../neighbors/neighbor_table.h"

class RoutingTable {

public:

    void UpdateFromNeighbors(
        const NeighborTable& neighbors,
        const std::string& self_id,
        double sim_time);

    std::string GetNextHop(const std::string& destination) const;

    void RemoveStale(double sim_time, double timeout);

private:

    std::unordered_map<std::string, RoutingEntry> m_table;
};