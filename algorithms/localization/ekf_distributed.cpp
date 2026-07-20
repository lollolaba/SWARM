#include "ekf_distributed.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

void EKFDistributed::Init(
    double process_noise,
    double measurement_noise)
{
    m_q = process_noise;
    m_r = measurement_noise;
}

void EKFDistributed::EnsureState(
    const std::string& id)
{
    auto& state = m_states[id];

    if(state.P[0][0] == 0.0 &&
       state.P[1][1] == 0.0)
    {
        for(int row = 0; row < 4; ++row) {
            for(int column = 0;
                column < 4;
                ++column)
            {
                state.P[row][column] =
                    (row == column)
                    ? 1.0
                    : 0.0;
            }
        }
    }
}

const EKFState& EKFDistributed::GetState(
    const std::string& id) const
{
    return m_states.at(id);
}

void EKFDistributed::Predict(
    const std::string& id,
    double dt)
{
    EnsureState(id);

    auto& state = m_states[id];

    state.x += state.vx * dt;
    state.y += state.vy * dt;

    for(int index = 0;
        index < 4;
        ++index)
    {
        state.P[index][index] +=
            m_q * dt;
    }
}

void EKFDistributed::UpdateLocalMeasurement(
    const std::string& id,
    double zx,
    double zy)
{
    EnsureState(id);

    auto& state = m_states[id];

    const double kx =
        state.P[0][0] /
        (state.P[0][0] + m_r);

    const double ky =
        state.P[1][1] /
        (state.P[1][1] + m_r);

    const double error_x =
        zx - state.x;

    const double error_y =
        zy - state.y;

    state.x += kx * error_x;
    state.y += ky * error_y;

    state.P[0][0] *=
        (1.0 - kx);

    state.P[1][1] *=
        (1.0 - ky);
}

void EKFDistributed::UpdateFromNeighbor(
    const std::string& id,
    const std::string& /*neighbor*/,
    double nx,
    double ny,
    double confidence)
{
    EnsureState(id);

    auto& state = m_states[id];

    const double self_weight =
        1.0 /
        (state.P[0][0] +
         state.P[1][1] +
         1e-6);

    const double neighbor_weight =
        confidence;

    const double blend_weight =
        neighbor_weight /
        (self_weight + neighbor_weight);

    state.x +=
        blend_weight *
        (nx - state.x);

    state.y +=
        blend_weight *
        (ny - state.y);

    state.P[0][0] *=
        (1.0 - blend_weight);

    state.P[1][1] *=
        (1.0 - blend_weight);
}

EKFDistributed::RangeUpdateCandidate
EKFDistributed::BuildRangeUpdateCandidate(
    const EKFState& state,
    double neighbor_x,
    double neighbor_y,
    double measured_range,
    double effective_variance,
    double prior_scale,
    double measurement_scale) const
{
    RangeUpdateCandidate candidate;

    const double dx =
        state.x - neighbor_x;

    const double dy =
        state.y - neighbor_y;

    const double predicted_range =
        std::sqrt(
            dx * dx +
            dy * dy);

    if(predicted_range < 1e-9) {
        return candidate;
    }

    const double hx =
        dx / predicted_range;

    const double hy =
        dy / predicted_range;

    const double p00 =
        state.P[0][0] * prior_scale;

    const double p01 =
        state.P[0][1] * prior_scale;

    const double p10 =
        state.P[1][0] * prior_scale;

    const double p11 =
        state.P[1][1] * prior_scale;

    const double measurement_variance =
        effective_variance *
        measurement_scale;

    const double ph0 =
        p00 * hx +
        p01 * hy;

    const double ph1 =
        p10 * hx +
        p11 * hy;

    const double innovation_variance =
        hx * ph0 +
        hy * ph1 +
        measurement_variance;

    if(innovation_variance <= 1e-12) {
        return candidate;
    }

    const double gain_x =
        ph0 / innovation_variance;

    const double gain_y =
        ph1 / innovation_variance;

    const double innovation =
        measured_range -
        predicted_range;

    candidate.x =
        state.x +
        gain_x * innovation;

    candidate.y =
        state.y +
        gain_y * innovation;

    /*
     * Joseph covariance update:
     *
     * P = (I-KH) P (I-KH)^T + K R K^T
     */

    const double a00 =
        1.0 - gain_x * hx;

    const double a01 =
        -gain_x * hy;

    const double a10 =
        -gain_y * hx;

    const double a11 =
        1.0 - gain_y * hy;

    const double ap00 =
        a00 * p00 +
        a01 * p10;

    const double ap01 =
        a00 * p01 +
        a01 * p11;

    const double ap10 =
        a10 * p00 +
        a11 * p10;

    const double ap11 =
        a10 * p01 +
        a11 * p11;

    candidate.p00 =
        ap00 * a00 +
        ap01 * a01 +
        gain_x *
        measurement_variance *
        gain_x;

    candidate.p01 =
        ap00 * a10 +
        ap01 * a11 +
        gain_x *
        measurement_variance *
        gain_y;

    candidate.p10 =
        ap10 * a00 +
        ap11 * a01 +
        gain_y *
        measurement_variance *
        gain_x;

    candidate.p11 =
        ap10 * a10 +
        ap11 * a11 +
        gain_y *
        measurement_variance *
        gain_y;

    const double symmetric_value =
        0.5 *
        (candidate.p01 +
         candidate.p10);

    candidate.p00 =
        std::max(
            candidate.p00,
            1e-9);

    candidate.p01 =
        symmetric_value;

    candidate.p10 =
        symmetric_value;

    candidate.p11 =
        std::max(
            candidate.p11,
            1e-9);

    candidate.valid = true;

    return candidate;
}

void EKFDistributed::UpdateFromRange(
    const std::string& id,
    double neighbor_x,
    double neighbor_y,
    double measured_range,
    double range_variance,
    double neighbor_position_variance,
    EKFRangeFusionMode fusion_mode)
{
    EnsureState(id);

    auto& state = m_states[id];

    const double effective_variance =
        std::max(
            range_variance +
            neighbor_position_variance,
            1e-9);

    RangeUpdateCandidate selected;

    if(fusion_mode ==
       EKFRangeFusionMode::STANDARD)
    {
        selected =
            BuildRangeUpdateCandidate(
                state,
                neighbor_x,
                neighbor_y,
                measured_range,
                effective_variance,
                1.0,
                1.0);
    }
    else {
        double best_trace =
            std::numeric_limits<double>
                ::infinity();

        /*
         * Conservative covariance-intersection-style
         * weighting.
         *
         * omega scales the prior information.
         * 1-omega scales the range information.
         */

        for(int step = 1;
            step < 20;
            ++step)
        {
            const double omega =
                0.05 * step;

            const auto candidate =
                BuildRangeUpdateCandidate(
                    state,
                    neighbor_x,
                    neighbor_y,
                    measured_range,
                    effective_variance,
                    1.0 / omega,
                    1.0 / (1.0 - omega));

            if(!candidate.valid) {
                continue;
            }

            const double trace =
                candidate.p00 +
                candidate.p11;

            if(trace < best_trace) {
                best_trace = trace;
                selected = candidate;
            }
        }
    }

    if(!selected.valid) {
        return;
    }

    state.x = selected.x;
    state.y = selected.y;

    state.P[0][0] = selected.p00;
    state.P[0][1] = selected.p01;
    state.P[1][0] = selected.p10;
    state.P[1][1] = selected.p11;
}

PositionEstimate EKFDistributed::Update(
    const std::string& id,
    double x,
    double y,
    double dt)
{
    const double safe_dt =
        (dt > 0.0)
        ? dt
        : 0.1;

    Predict(
        id,
        safe_dt);

    UpdateLocalMeasurement(
        id,
        x,
        y);

    const auto& state =
        GetState(id);

    return {
        state.x,
        state.y,
        state.P[0][0] +
        state.P[1][1]
    };
}