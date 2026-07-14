#pragma once
#include <string>

struct LogRecord {
    double timestamp = 0.0;
    std::string event_type;
    std::string sender;
    std::string receiver;
    double est_x = 0.0;
    double est_y = 0.0;
    double distance = 0.0;
    double delay = 0.0;
    double snr = 0.0;
    std::string localization_mode;
    std::string channel_mode;
    std::string drop_reason;
    double metric1 = 0.0;
    double metric2 = 0.0;
    double metric3 = 0.0;
};