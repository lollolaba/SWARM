#pragma once
#include <unordered_map>
#include <string>
#include "position_estimation.h"

struct EKFState {
    double x = 0;
    double y = 0;
    double vx = 0;
    double vy = 0;

    double P[4][4] = {{0}}; // covariance
};

class EKFDistributed {

public:

    void Init(double process_noise, double meas_noise);

    void Predict(const std::string& id, double dt);

    void UpdateLocalMeasurement(const std::string& id,
                                double zx,
                                double zy);

    void UpdateFromNeighbor(const std::string& id,
                             const std::string& neighbor,
                             double nx,
                             double ny,
                             double confidence);

    const EKFState& GetState(const std::string& id) const;

    PositionEstimate Update(const std::string& id, double x, double y, double t);

private:

    void EnsureState(const std::string& id);

    std::unordered_map<std::string, EKFState> m_states;

    double m_q = 0.01;
    double m_r = 0.1;
};