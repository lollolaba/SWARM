#pragma once
#include <string>
#include <vector>

struct Packet {
    std::string sender;
    std::string receiver;
    std::vector<uint8_t> payload;

    double timestamp=0.0; //creation time  

    double est_x = 0.0;
    double est_y = 0.0;
};