#include "log_utils.h"

std::string EventTypeToString(EventType type){
    switch(type){
        case EventType::TX: return "TX";
        case EventType::RX: return "RX";
        case EventType::DROP: return "DROP";
    }
    return "UNKNOWN";
}

std::string ChannelModeToString(ChannelMode mode){
    switch(mode){
        case ChannelMode::BASIC: return "BASIC";
        case ChannelMode::REALISTIC: return "REALISTIC";
        case ChannelMode::ADVANCED_SNR: return "ADVANCED_SNR";
        case ChannelMode::UNDERWATER_EXTREME: return "UNDERWATER_EXTREME";
    }
    return "UNKNOWN";
}