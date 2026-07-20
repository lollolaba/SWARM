#pragma once
#include <string>
#include <unordered_map>
#include "position_estimation.h"

struct EKFState {
    double x = 0.0;
    double y = 0.0;

    double vx = 0.0;
    double vy = 0.0;

    double P[4][4] = {{0.0}};
};

enum class EKFRangeFusionMode {
    STANDARD,
    COVARIANCE_INTERSECTION
};

class EKFDistributed {
public:
    void Init(double process_noise,double measurement_noise);
    void Predict(const std::string& id,double dt);
    void UpdateLocalMeasurement(const std::string& id,double zx,double zy);

    // Retained for compatibility with the original code.
    void UpdateFromNeighbor(const std::string& id,const std::string& neighbor,double nx,double ny,double confidence);
    void UpdateFromRange(const std::string& id,double neighbor_x,double neighbor_y,double measured_range,double range_variance,double neighbor_position_variance,EKFRangeFusionMode fusion_mode);

    const EKFState& GetState(const std::string& id) const;
    PositionEstimate Update(const std::string& id,double x,double y,double dt);

private:
    struct RangeUpdateCandidate {
        double x = 0.0;
        double y = 0.0;

        double p00 = 0.0;
        double p01 = 0.0;
        double p10 = 0.0;
        double p11 = 0.0;
        bool valid = false;
    };
    void EnsureState(const std::string& id);

    RangeUpdateCandidate BuildRangeUpdateCandidate(
        const EKFState& state,
        double neighbor_x,
        double neighbor_y,
        double measured_range,
        double effective_variance,
        double prior_scale,
        double measurement_scale) const;

    std::unordered_map<std::string, EKFState> m_states;

    double m_q = 0.01;
    double m_r = 0.1;
};