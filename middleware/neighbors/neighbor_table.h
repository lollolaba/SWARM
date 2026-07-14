#pragma once
#include <string>
#include <unordered_map>
#include <vector>

struct NeighborEntry {
    std::string id;
    double distance;
    double last_seen;
};

class NeighborTable {
public:
    void Update(const std::string& id, double distance, double t);
    void RemoveStale(double current_time, double timeout);
    const std::vector<NeighborEntry>& GetNeighbors() const;
    const std::unordered_map<std::string, NeighborEntry>& GetAll() const{
        return m_table;
    }
private:
    std::unordered_map<std::string, NeighborEntry> m_table;
};