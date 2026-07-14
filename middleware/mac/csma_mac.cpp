#include "csma_mac.h"
#include "mac_layer.h"

#include <argos3/core/utility/math/rng.h>

bool MACLayer::CanTransmitCSMA(const std::string& sender,double sim_time){
    auto* rng = argos::CRandom::CreateRNG("argos");
    auto it = m_last_tx_time.find(sender);
    if(it == m_last_tx_time.end()) return true;
    double dt = sim_time - it->second;
    return dt > ( m_csma_backoff + m_csma_random_window*rng->Uniform(argos::CRange<double>(0.0,1.0)));
}


void MACLayer::OnTransmitCSMA(const std::string& sender,double sim_time){
    m_last_tx_time[sender]=sim_time;
}