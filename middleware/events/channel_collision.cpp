#include "channel_collision.h"

bool ChannelCollision::HasCollision(
    const Packet& p,
    const std::vector<Packet>& packets,
    double threshold
) {

    int overlaps = 0;

    for(const auto& other : packets) {

        if(other.sender == p.sender)
            continue;

        double dt =
            std::abs(other.timestamp - p.timestamp);

        if(dt < threshold)
            overlaps++;
    }

    return overlaps > 0;
}