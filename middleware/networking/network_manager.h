#pragma once
#include <unordered_map>
#include <string>
#include "middleware/neighbors/neighbor_table.h"

class NetworkManager {
public:
    NeighborTable& GetNeighbors(const std::string& id);

private:
    std::unordered_map<std::string, NeighborTable> m_tables;
};