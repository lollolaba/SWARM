#include "multipath_model.h"
#include <cmath>
#include <argos3/core/simulator/simulator.h>

double MultipathModel::Compute(argos::CRandom::CRNG* rng, double distance)
{
    double noise = rng->Gaussian(0.0,0.03);
    double echo = 0.3*std::exp(-distance/10.0);
    /*double echo_strength = 0.3 * exp(-distance / 10.0) * sin(t * 2.0);
    return std::abs(echo_strength);*/
    return std::abs(echo+noise);
}

double MultipathModel::DelaySpread(double distance)
{
    return 0.02 * log(distance + 1.0);
}