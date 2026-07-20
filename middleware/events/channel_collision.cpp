#include "channel_collision.h"
#include <cmath>

bool ChannelCollision::HasCollision(const Packet& packet,const std::vector<Packet>& packets,double threshold){
    for(const auto& other : packets) {
        if(other.sender == packet.sender) continue;
        // Two packets interfere only if they target the same receiving robot.
        if(other.receiver != packet.receiver) continue;
        const double dt = std::abs(other.timestamp - packet.timestamp);
        if(dt < threshold) return true;
    }
    return false;
}