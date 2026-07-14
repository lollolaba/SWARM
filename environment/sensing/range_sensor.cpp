#include "range_sensor.h"
#include <random>

static std::default_random_engine gen;

double RangeSensor::Measure(double true_distance) {
    std::normal_distribution<double> noise(0.0, 0.05);
    return true_distance + noise(gen);
}