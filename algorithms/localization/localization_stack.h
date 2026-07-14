#pragma once

#include <string>
#include <memory>

#include "position_estimation.h"
#include "consensus_localization.h"
#include "ekf_distributed.h"
#include "particle_filter.h"
#include "factor_graph_light.h"
#include "bayesian_distributed_localization.h"

enum class LocalizationMode {
    IDEAL,
    NOISY,
    EKF,
    CONSENSUS,
    PARTICLE_FILTER,
    BAYESIAN,
    FACTOR_GRAPH
};

class LocalizationStack {
public:

    void SetMode(LocalizationMode m);

    PositionEstimate Update(
        const std::string& id,
        double true_x,
        double true_y,
        double t);

    void SetNeighbors(const std::string& id,
                      const std::vector<std::string>& neighbors);

private:

    LocalizationMode m_mode = LocalizationMode::NOISY;

    EKFDistributed m_ekf;
    ConsensusLocalization m_consensus;
    ParticleFilterLocalization m_pf;
    BayesianDistributedLocalization m_bayes;
    FactorGraphLight m_fg;
};