#pragma once

#include "../../core/communication/packet.h"
#include <vector>

class ChannelCollision {
public:

    static bool HasCollision(
        const Packet& p,
        const std::vector<Packet>& packets,
        double threshold = 2.0
    );
};