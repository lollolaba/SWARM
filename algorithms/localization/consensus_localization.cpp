#include "consensus_localization.h"
#include <cmath>

void ConsensusLocalization::SetNeighbors(
    const std::string& id,
    const std::vector<std::string>& n)
{
    m_neighbors[id] = n;
}

/*
 * WEIGHTED CONSENSUS (variance-aware)
 */
PositionEstimate ConsensusLocalization::Update(
    const std::string& id,
    double x,
    double y,
    double /*t*/)
{
    auto& est = m_est[id];

    // initialize / update self measurement
    est.x = x;
    est.y = y;

    double weight_sum = 1.0;
    double sum_x = x;
    double sum_y = y;

    auto it = m_neighbors.find(id);
    if (it == m_neighbors.end()) {
        est.cov = 1.0;
        return est;
    }

    for (const auto& n : it->second) {

        auto jt = m_est.find(n);
        if (jt == m_est.end()) continue;

        const auto& en = jt->second;

        // convert covariance → weight
        double w = 1.0 / (en.cov + 1e-6);

        sum_x += w * en.x;
        sum_y += w * en.y;
        weight_sum += w;
    }

    est.x = sum_x / weight_sum;
    est.y = sum_y / weight_sum;

    // updated uncertainty (information fusion approximation)
    est.cov = 1.0 / weight_sum;

    return est;
}