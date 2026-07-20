#include "log_utils.h"

std::string EventTypeToString(EventType type){
    switch(type){
        case EventType::TX: return "TX";
        case EventType::RX: return "RX";
        case EventType::DROP: return "DROP";
    }
    return "UNKNOWN";
}

std::string LocalizationModeToString(LocalizationMode mode){
    switch(mode) {
        case LocalizationMode::IDEAL: return "IDEAL";
        case LocalizationMode::NOISY: return "NOISY";
        case LocalizationMode::EKF: return "EKF";
        case LocalizationMode::EKF_CI: return "EKF_CI"
        case LocalizationMode::CONSENSUS: return "CONSENSUS";
        case LocalizationMode::PARTICLE_FILTER: return "PARTICLE_FILTER";
        case LocalizationMode::BAYESIAN: return "BAYESIAN";
        case LocalizationMode::FACTOR_GRAPH: return "FACTOR_GRAPH";
    }
    return "UNKNOWN";
}

std::string ChannelModeToString(ChannelMode mode){
    switch(mode){
        case ChannelMode::BASIC: return "BASIC";
        case ChannelMode::REALISTIC: return "REALISTIC";
        case ChannelMode::ADVANCED_SNR: return "ADVANCED_SNR";
        case ChannelMode::UNDERWATER_EXTREME: return "UNDERWATER_EXTREME";
        case ChannelMode::THORP: return "THORP";
    }
    return "UNKNOWN";
}