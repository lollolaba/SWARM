#pragma once
#include "../../core/communication/packet.h"
#include <string>

enum class EventType {
    TX,
    RX,
    DROP
};

struct Event {
    double time;
    EventType type;
    Packet packet;
    std::string sender;
    std::string receiver;
    std::string reason; // for drops
    double distance; // for logging
    double timestamp = 0.0;
    double delay = 0.0;
    double snr = 0.0;
    std::string channel_mode = "unknown";
    std::string localization_mode = "unknown";
    double metric1 = 0.0;
    double metric2 = 0.0;
    double metric3 = 0.0;
};