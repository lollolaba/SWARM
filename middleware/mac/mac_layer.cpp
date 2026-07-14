#include "mac_layer.h"
#include <cmath>

MACLayer::MACLayer(){
    m_mode = MACMode::ALOHA;
}

void MACLayer::SetMode(MACMode mode){
    m_mode = mode;
}

bool MACLayer::CanTransmit(const std::string& sender,double sim_time){
    switch(m_mode){
        case MACMode::ALOHA:return CanTransmitALOHA(sender,sim_time);
        case MACMode::TDMA: return CanTransmitTDMA(sender,sim_time);
        case MACMode::CSMA: return CanTransmitCSMA(sender,sim_time);
    }
    return false;
}

void MACLayer::OnTransmit(const std::string& sender,double sim_time){
    switch(m_mode){
        case MACMode::ALOHA:
            OnTransmitALOHA(sender,sim_time);
            break;

        case MACMode::CSMA:
            OnTransmitCSMA(sender,sim_time);
            break;

        case MACMode::TDMA:
            OnTransmitTDMA(sender,sim_time);
            break;
    }
}

void MACLayer::SetALOHAProbability(double p){
    m_aloha_probability = p;
}

void MACLayer::SetCSMABackoff(double b){
    m_csma_backoff = b;
}

void MACLayer::SetCSMARandomWindow(double w){
    m_csma_random_window = w;
}