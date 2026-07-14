#include "aloha_mac.h"
#include "mac_layer.h"

#include <argos3/core/utility/math/rng.h>

bool MACLayer::CanTransmitALOHA(const std::string& sender,double sim_time){
    auto* rng = argos::CRandom::CreateRNG("argos");
    return (rng->Uniform(argos::CRange<double>(0.0,1.0)) < m_aloha_probability);
}


void MACLayer::OnTransmitALOHA(const std::string& sender,double sim_time){
    m_last_tx_time[sender] = sim_time;
}