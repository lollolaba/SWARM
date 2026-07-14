#include "ekf_distributed.h"
#include <cmath>
#include <stdexcept>

void EKFDistributed::Init(double q, double r) {
    m_q = q;
    m_r = r;
}

/*
 * Ensure state exists
 */
void EKFDistributed::EnsureState(const std::string& id) {

    auto& s = m_states[id];

    // lazy covariance init
    if (s.P[0][0] == 0 && s.P[1][1] == 0) {
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                s.P[i][j] = (i == j) ? 1.0 : 0.0;
    }
}

const EKFState& EKFDistributed::GetState(const std::string& id) const {
    return m_states.at(id);
}

/*
 * PREDICTION STEP (constant velocity model)
 */
void EKFDistributed::Predict(const std::string& id, double dt) {

    EnsureState(id);
    auto& s = m_states[id];

    // state propagation
    s.x  += s.vx * dt;
    s.y  += s.vy * dt;

    // covariance growth (diagonal simplified model)
    for (int i = 0; i < 4; i++)
        s.P[i][i] += m_q * dt;
}

/*
 * LOCAL MEASUREMENT UPDATE (x,y only)
 */
void EKFDistributed::UpdateLocalMeasurement(
    const std::string& id,
    double zx,
    double zy)
{
    EnsureState(id);
    auto& s = m_states[id];

    double kx = s.P[0][0] / (s.P[0][0] + m_r);
    double ky = s.P[1][1] / (s.P[1][1] + m_r);

    double ex = zx - s.x;
    double ey = zy - s.y;

    s.x += kx * ex;
    s.y += ky * ey;

    s.P[0][0] *= (1.0 - kx);
    s.P[1][1] *= (1.0 - ky);
}

/*
 * DISTRIBUTED NEIGHBOR UPDATE (gossip EKF-style)
 */
void EKFDistributed::UpdateFromNeighbor(
    const std::string& id,
    const std::string& neighbor,
    double nx,
    double ny,
    double confidence)
{
    EnsureState(id);
    auto& s = m_states[id];

    double alpha = confidence;

    // variance-aware blending (better than raw alpha)
    double w_self = 1.0 / (s.P[0][0] + s.P[1][1] + 1e-6);
    double w_nei  = alpha;

    double w = w_nei / (w_self + w_nei);

    s.x += w * (nx - s.x);
    s.y += w * (ny - s.y);

    // covariance contraction
    s.P[0][0] *= (1.0 - w);
    s.P[1][1] *= (1.0 - w);
}

PositionEstimate EKFDistributed::Update(
    const std::string& id,
    double x,
    double y,
    double t)
{
    Predict(id, 0.1);
    UpdateLocalMeasurement(id, x, y);

    const auto& s = GetState(id);

    return {
        s.x,
        s.y,
        s.P[0][0] + s.P[1][1]
    };
}