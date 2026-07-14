#include "routing_table.h"
#include <cmath>

void RoutingTable::UpdateFromNeighbors(
    const NeighborTable& neighbors,
    const std::string& self_id,
    double sim_time)
{
    for(const auto& entry : neighbors.GetAll()) {
        const auto& n = entry.second;
        RoutingEntry e;
        e.destination = n.id;
        e.next_hop = n.id;
        e.hop_count = 1;
        e.last_update = sim_time;
        e.cost = n.distance; //+ (1.0 - n.link_quality) * 2.0;
        m_table[n.id] = e;
    }

    // OPTIONAL: simple reinforcement from existing routes
    for(auto& [dest, entry] : m_table) {
        entry.cost *= 0.99; // aging
    }
}

std::string RoutingTable::GetNextHop(const std::string& dest) const {
    auto it = m_table.find(dest);
    if(it == m_table.end())
        return ""; // unknown

    return it->second.next_hop;
}

void RoutingTable::RemoveStale(double sim_time, double timeout)
{
    for(auto it = m_table.begin(); it != m_table.end(); ) {

        if(sim_time - it->second.last_update > timeout)
            it = m_table.erase(it);
        else
            ++it;
    }
}