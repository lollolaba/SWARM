#pragma once

#include <string>
#include "../messaging/message_bus.h"

enum class MACMode {
    ALOHA,
    TDMA,
    CSMA
};

class MACLayer {

public:
    MACLayer();
    void SetMode(MACMode mode);
    
    bool CanTransmit(const std::string& sender,double sim_time);
    void OnTransmit(const std::string& sender,double sim_time);

    void SetALOHAProbability(double p);
    void SetCSMABackoff(double b);
    void SetCSMARandomWindow(double w);
private:
    double m_aloha_probability = 0.7;
    double m_csma_backoff = 0.2;
    double m_csma_random_window = 0.2;
    MACMode m_mode;
    std::unordered_map<std::string, double> m_last_tx_time;
    bool CanTransmitALOHA(const std::string& sender,double sim_time);
    bool CanTransmitCSMA(const std::string& sender,double sim_time);
    void OnTransmitALOHA(const std::string& sender,double sim_time);
    void OnTransmitCSMA(const std::string& sender,double sim_time);
    bool CanTransmitTDMA(const std::string& sender, double sim_time);
    void OnTransmitTDMA(const std::string& sender, double sim_time);
};