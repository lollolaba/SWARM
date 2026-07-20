#include "range_sensor.h"

#include <algorithm>
#include <random>

void RangeSensor::SetNoiseSigma(double sigma) {
    m_noise_sigma = std::max(0.0, sigma);
}

double RangeSensor::Measure(double true_distance) {
    static std::default_random_engine generator;
    std::normal_distribution<double> noise(0.0, m_noise_sigma);

    return std::max(0.0, true_distance + noise(generator));
}

double RangeSensor::Measure(double true_distance,argos::CRandom::CRNG* rng) const{
    if(!rng || m_noise_sigma <= 0.0) return std::max(0.0, true_distance);
    return std::max(0.0,true_distance + rng->Gaussian(0.0, m_noise_sigma));
}