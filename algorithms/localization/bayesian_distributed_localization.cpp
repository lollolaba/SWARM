#include "bayesian_distributed_localization.h"
#include <cmath>
void BayesianDistributedLocalization::Init(double init_var) {
    m_beliefs.clear();
    m_initial_variance = init_var;
}

const BayesianBelief& BayesianDistributedLocalization::Get(const std::string& id) const {
    static BayesianBelief empty;
    auto it = m_beliefs.find(id);
    if(it == m_beliefs.end()) return empty;
    return it->second;
}

// SIMPLE MOTION MODEL (dead reckoning style)
void BayesianDistributedLocalization::Predict(const std::string& id,double dx,double dy,double motion_noise){
    auto& b = m_beliefs[id];
    b.x += dx;
    b.y += dy;
    b.variance += motion_noise;
}

// RANGE UPDATE (inverse variance weighting)
void BayesianDistributedLocalization::Update(const std::string& id,const std::string& neighbor_id,double range,double measurement_noise){
    auto& bi = m_beliefs[id];
    auto& bj = m_beliefs[neighbor_id];

    double dx = bj.x - bi.x;
    double dy = bj.y - bi.y;
    double d  = std::sqrt(dx*dx + dy*dy);

    if(d < 1e-6) return;

    double z = range;
    double innovation = z - d;

    double K = bi.variance / (bi.variance + measurement_noise);

    bi.x += K * innovation * (dx / d);
    bi.y += K * innovation * (dy / d);
    bi.variance *= (1.0 - K);
}

// CONSENSUS STEP (distributed averaging)
void BayesianDistributedLocalization::ConsensusUpdate(const std::string& id,const std::vector<std::string>& neighbors,double alpha){
    auto& bi = m_beliefs[id];

    if(neighbors.empty()) return;

    double mx = 0, my = 0;
    int count = 0;

    for(const auto& n : neighbors) {
        auto it = m_beliefs.find(n);
        if(it == m_beliefs.end()) continue;
        mx += it->second.x;
        my += it->second.y;
        count++;
    }

    if(count == 0) return;

    mx /= count;
    my /= count;

    bi.x = (1 - alpha) * bi.x + alpha * mx;
    bi.y = (1 - alpha) * bi.y + alpha * my;
}

void BayesianDistributedLocalization::PredictAndUpdateSelf(
    const std::string& id,
    double x,
    double y,
    double motion_noise)
{
    auto& b = m_beliefs[id];

    double dx = x - b.x;
    double dy = y - b.y;

    b.x = x;
    b.y = y;
    b.variance += motion_noise + (dx*dx + dy*dy);
}