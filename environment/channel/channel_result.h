#pragma once
#include <string>

enum class DropReason {
    LOSS,
    OUT_OF_RANGE,
    COLLISION
};

struct ChannelResult {
    bool delivered=false;
    double delay=0.0;
    double snr=0.0;
    double distance=0.0;
    std::string reason; // "collision", "snr_drop", "range"
    std::string channel_mode = "unknown";
    std::string localization_mode = "unknown";
    double timestamp = 0.0;
    int event_id = -1;
    double metric1 = 0.0;
    double metric2 = 0.0;
    double metric3 = 0.0;
};