#pragma once
#include <argos3/core/utility/math/rng.h>


class SNRModel {
public:
    static double Compute(double rx_power, double noise_sigma, double rng_value);
};