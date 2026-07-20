#pragma once
#include "position_estimation.h"
#include <string>
#include <unordered_map>
#include <argos3/core/utility/math/rng.h>
#include <vector>

struct BayesianBelief {
    double x = 0.0;
    double y = 0.0;
    double variance = 1.0;   // uncertainty
};

class BayesianDistributedLocalization {

public:

    void Init(double initial_variance = 1.0);
    //motion model
    void Predict(const std::string& id,double motion_dx,double motion_dy,double motion_noise = 0.01);

    // range based
    void Update(const std::string& id,const std::string& neighbor_id,double range,double measurment_noise);
    void UpdateFromNeighborEstimate(const std::string& id,double neighbor_x,double neighbor_y,double range,double measurement_variance);

    void ConsensusUpdate(const std::string& id, const std::vector<std::string>& neighbors, double alpha = 0.5);

    const BayesianBelief& Get(const std::string& id) const;

    std::unordered_map<std::string, BayesianBelief>& GetAll() {
        return m_beliefs;
    }
    void PredictAndUpdateSelf(const std::string& id,double x,double y,double motion_noise = 0.01);

private:
    double m_initial_variance = 1.0;
    std::unordered_map<std::string, BayesianBelief> m_beliefs;
};