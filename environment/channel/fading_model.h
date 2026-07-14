#pragma once
#include <argos3/core/utility/math/rng.h>

class FadingModel {
public:
    /*double Compute(double signal_strength, double distance, double t) {
        // simple Rayleigh-like approximation (stub)
        return signal_strength * (0.5 + 0.5 * sin(t + distance));
    }*/
    static double Compute(argos::CRandom::CRNG* rng);
};