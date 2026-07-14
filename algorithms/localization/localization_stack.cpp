#include "localization_stack.h"
#include <cmath>

void LocalizationStack::SetMode(LocalizationMode m) {
    m_mode = m;
}

void LocalizationStack::SetNeighbors(
    const std::string& id,
    const std::vector<std::string>& neighbors)
{
    m_consensus.SetNeighbors(id, neighbors);
    m_pf.SetNeighbors(id, neighbors);
    m_fg.SetNeighbors(id, neighbors);
}

PositionEstimate LocalizationStack::Update(
    const std::string& id,
    double x,
    double y,
    double t)
{
    PositionEstimate out;

    switch(m_mode) {

        case LocalizationMode::IDEAL:
            return {x, y, 0.0};

        case LocalizationMode::NOISY:
            return {x + 0.02, y + 0.02, 0.1};

        case LocalizationMode::EKF:
            return m_ekf.Update(id, x, y, t);

        case LocalizationMode::CONSENSUS:
            return m_consensus.Update(id, x, y, t);

        case LocalizationMode::PARTICLE_FILTER:
            return m_pf.Update(id, x, y, t);

        case LocalizationMode::BAYESIAN: {
            // 1. self update
            m_bayes.PredictAndUpdateSelf(id, x, y, 0.01);
            // 2. get neighbors correctly
            const auto& neighbours = m_pf.GetNeighbors(id);
            for (const auto& n_id : neighbours) {
                const auto& nb = m_bayes.Get(n_id);
                const auto& me = m_bayes.Get(id);
                double dx = nb.x - me.x;
                double dy = nb.y - me.y;
                double range = std::sqrt(dx*dx + dy*dy);
                if (range < 1e-6) continue;
                m_bayes.Update(id, n_id, range, 0.05);
            }
            const auto& est = m_bayes.Get(id);
            return {est.x, est.y, est.variance};
        }
        case LocalizationMode::FACTOR_GRAPH:
            return m_fg.Update(id, x, y, t);
    }

    return {x, y, 0.1};
}