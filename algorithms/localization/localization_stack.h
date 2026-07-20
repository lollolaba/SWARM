#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "position_estimation.h"
#include "consensus_localization.h"
#include "ekf_distributed.h"
#include "particle_filter.h"
#include "factor_graph_light.h"
#include "bayesian_distributed_localization.h"
#include "range_observation.h"

enum class LocalizationMode {
    IDEAL,
    NOISY,
    EKF,
    EKF_CI,
    CONSENSUS,
    PARTICLE_FILTER,
    BAYESIAN,
    FACTOR_GRAPH
};

class LocalizationStack {
public:
    void SetMode(
        LocalizationMode mode);

    PositionEstimate Update(
        const std::string& id,
        double true_x,
        double true_y,
        double t);

    PositionEstimate Update(
        const std::string& id,
        double true_x,
        double true_y,
        double measured_x,
        double measured_y,
        double dt);

    void SetNeighbors(
        const std::string& id,
        const std::vector<std::string>& neighbors);

    void SetRangeObservations(
        const std::string& id,
        const std::vector<RangeObservation>& observations);

private:
    PositionEstimate UpdateCooperativeEKF(
        EKFDistributed& filter,
        EKFRangeFusionMode fusion_mode,
        const std::string& id,
        double measured_x,
        double measured_y,
        double dt);

    LocalizationMode m_mode =
        LocalizationMode::NOISY;

    // Separate state maps prevent one mode from
    // contaminating the other during testing.
    EKFDistributed m_ekf;
    EKFDistributed m_ekf_ci;

    ConsensusLocalization m_consensus;
    ParticleFilterLocalization m_pf;
    BayesianDistributedLocalization m_bayes;
    FactorGraphLight m_fg;

    std::unordered_map<
        std::string,
        std::vector<RangeObservation>>
        m_range_observations;
};