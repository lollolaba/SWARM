#pragma once
#include <string>

struct RoutingEntry {

    std::string destination;
    std::string next_hop;

    double cost = 0.0;
    double last_update = 0.0;

    int hop_count = 1;
};