#pragma once

#include "position_estimation.h"
#include <unordered_map>
#include <vector>
#include <string>

class ConsensusLocalization {
public:

    void SetNeighbors(
        const std::string& id,
        const std::vector<std::string>& n);

    PositionEstimate Update(
        const std::string& id,
        double x,
        double y,
        double t);

private:

    std::unordered_map<std::string,
        std::vector<std::string>> m_neighbors;

    std::unordered_map<std::string, PositionEstimate> m_est;
};