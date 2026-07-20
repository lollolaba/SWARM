#pragma once
#include <argos3/core/utility/math/rng.h>

class RangeSensor {
public:
    void SetNoiseSigma(double sigma);
    double Measure(double true_distance);
    double Measure(double true_distance,argos::CRandom::CRNG* rng) const;

private:
    double m_noise_sigma = 0.05;
};