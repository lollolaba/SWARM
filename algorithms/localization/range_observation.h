#pragma once
#include <string>

struct RangeObservation {
    std::string neighbor_id;
    double measured_range = 0.0;
    double neighbor_x = 0.0;
    double neighbor_y = 0.0;
    double measurement_variance = 0.0025;
    double neighbor_position_variance = 0.1;
};