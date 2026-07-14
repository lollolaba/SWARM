#include "snr_model.h"
#include <cmath>

double SNRModel::Compute(double signal_power,double noise_power,double rng_value){
    double noise_sigma = noise_power;
    double noise = noise_sigma * (rng_value - 0.5);
    return signal_power / (noise_power + noise + 1e-9);
}