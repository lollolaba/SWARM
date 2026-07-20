#include "localization_stack.h"

void LocalizationStack::SetMode(
    LocalizationMode mode)
{
    m_mode = mode;
}

void LocalizationStack::SetNeighbors(
    const std::string& id,
    const std::vector<std::string>& neighbors)
{
    m_consensus.SetNeighbors(
        id,
        neighbors);

    m_pf.SetNeighbors(
        id,
        neighbors);

    m_fg.SetNeighbors(
        id,
        neighbors);
}

void LocalizationStack::SetRangeObservations(
    const std::string& id,
    const std::vector<RangeObservation>& observations)
{
    m_range_observations[id] =
        observations;

    m_pf.SetRangeObservations(
        id,
        observations);
}

PositionEstimate LocalizationStack::Update(
    const std::string& id,
    double true_x,
    double true_y,
    double /*t*/)
{
    return Update(
        id,
        true_x,
        true_y,
        true_x,
        true_y,
        0.1);
}

PositionEstimate
LocalizationStack::UpdateCooperativeEKF(
    EKFDistributed& filter,
    EKFRangeFusionMode fusion_mode,
    const std::string& id,
    double measured_x,
    double measured_y,
    double dt)
{
    // Existing prediction and local-position update.
    filter.Update(
        id,
        measured_x,
        measured_y,
        dt);

    const auto observations_it =
        m_range_observations.find(id);

    if(observations_it !=
       m_range_observations.end())
    {
        for(const auto& observation :
            observations_it->second)
        {
            filter.UpdateFromRange(
                id,
                observation.neighbor_x,
                observation.neighbor_y,
                observation.measured_range,
                observation.measurement_variance,
                observation.neighbor_position_variance,
                fusion_mode);
        }
    }

    const auto& state =
        filter.GetState(id);

    return {
        state.x,
        state.y,
        state.P[0][0] +
        state.P[1][1]
    };
}

PositionEstimate LocalizationStack::Update(
    const std::string& id,
    double true_x,
    double true_y,
    double measured_x,
    double measured_y,
    double dt)
{
    switch(m_mode) {
        case LocalizationMode::IDEAL:
            return {
                true_x,
                true_y,
                0.0
            };

        case LocalizationMode::NOISY:
            return {
                measured_x,
                measured_y,
                0.1
            };

        case LocalizationMode::EKF:
            return UpdateCooperativeEKF(
                m_ekf,
                EKFRangeFusionMode::STANDARD,
                id,
                measured_x,
                measured_y,
                dt);

        case LocalizationMode::EKF_CI:
            return UpdateCooperativeEKF(
                m_ekf_ci,
                EKFRangeFusionMode::
                    COVARIANCE_INTERSECTION,
                id,
                measured_x,
                measured_y,
                dt);

        case LocalizationMode::CONSENSUS:
            return m_consensus.Update(
                id,
                measured_x,
                measured_y,
                dt);

        case LocalizationMode::PARTICLE_FILTER:
            return m_pf.Update(
                id,
                measured_x,
                measured_y,
                dt);

        case LocalizationMode::BAYESIAN: {
            m_bayes.PredictAndUpdateSelf(
                id,
                measured_x,
                measured_y,
                0.01);

            const auto observations_it =
                m_range_observations.find(id);

            if(observations_it !=
               m_range_observations.end())
            {
                for(const auto& observation :
                    observations_it->second)
                {
                    m_bayes.UpdateFromNeighborEstimate(
                        id,
                        observation.neighbor_x,
                        observation.neighbor_y,
                        observation.measured_range,
                        observation.measurement_variance);
                }
            }

            const auto& estimate =
                m_bayes.Get(id);

            return {
                estimate.x,
                estimate.y,
                estimate.variance
            };
        }

        case LocalizationMode::FACTOR_GRAPH:
            return m_fg.Update(
                id,
                measured_x,
                measured_y,
                dt);
    }

    return {
        measured_x,
        measured_y,
        0.1
    };
}