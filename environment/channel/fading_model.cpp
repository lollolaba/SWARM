#include "fading_model.h"
#include <argos3/core/simulator/simulator.h>

double FadingModel::Compute(argos::CRandom::CRNG* rng) {
    return rng->Gaussian(1.0, 0.2);
}
/*double FadingModel::Apply(double snr)
{
    static argos::CRandom::CRNG* rng =
        argos::CSimulator::GetInstance().GetRNG();

    double rayleigh = rng->Gaussian(0.0, 0.2);
    double slow_fading = rng->Uniform(argos::CRange<Real>(0.8, 1.2));

    return (snr + rayleigh) * slow_fading;
}*/