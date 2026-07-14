#pragma once

#include "../../core/communication/packet.h"

struct ScheduledPacket {
    Packet packet;
    double delivery_time = 0.0;
    double distance = 0.0; 
    double delay = 0.0; 
    bool delivered = true;
    double snr = 0.0;
    std::string channel_mode = "unknown";
    std::string localization_mode = "unknown";
    std::string drop_reason = "none";
    double timestamp = 0.0;
    int event_id = -1;  //identifier optional event useful for correlating TX/RX/DROP
    // Metriche aggiuntive future
    double metric1 = 0.0;
    double metric2 = 0.0;
    double metric3 = 0.0;
};