#pragma once
#include <argos3/core/utility/math/rng.h>

class MultipathModel {
public:
    static double Compute(argos::CRandom::CRNG* rng,double distance);
    double DelaySpread(double distance);
};