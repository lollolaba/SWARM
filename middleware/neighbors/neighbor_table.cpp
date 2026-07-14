#include "neighbor_table.h"

void NeighborTable::Update(const std::string& id, double distance, double t) {
    m_table[id] = {id, distance, t};
}

void NeighborTable::RemoveStale(double current_time, double timeout) {
    for(auto it = m_table.begin(); it != m_table.end(); ) {
        if(current_time - it->second.last_seen > timeout)
            it = m_table.erase(it);
        else
            ++it;
    }
}

const std::vector<NeighborEntry>& NeighborTable::GetNeighbors() const {
    static std::vector<NeighborEntry> out;
    out.clear();
    for(const auto& p : m_table)
        out.push_back(p.second);
    return out;
}