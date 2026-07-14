#include "network_manager.h"
#include "../neighbors/neighbor_table.h"

NeighborTable& NetworkManager::GetNeighbors(const std::string& id) {
    return m_tables[id];
}