#include "underwater_agent_controller.h"
#include "../../globals.h"

#include <argos3/core/simulator/simulator.h>
#include <argos3/core/simulator/space/space.h>
#include <argos3/core/utility/configuration/argos_configuration.h>

#include <algorithm>
#include <vector>

REGISTER_CONTROLLER(CUnderwaterAgentController,"underwater_agent_controller")

void CUnderwaterAgentController::Init(argos::TConfigurationNode& t_node){
    argos::GetNodeAttributeOrDefault(t_node,"broadcast_period_ticks",m_broadcast_period_ticks,m_broadcast_period_ticks);

    if(m_broadcast_period_ticks == 0) m_broadcast_period_ticks = 1;
    Reset();
}

void CUnderwaterAgentController::ControlStep() {
    const argos::UInt32 current_tick =argos::CSimulator::GetInstance().GetSpace().GetSimulationClock();
    if(m_has_broadcast && current_tick - m_last_broadcast_tick <m_broadcast_period_ticks) return;

    const std::string& id = GetId();
    const auto estimate_it = g_robot_est_state.find(id);
    if(estimate_it == g_robot_est_state.end()) return;

    const auto table_it = g_neighbor_tables.find(id);
    if(table_it == g_neighbor_tables.end()) return;
    std::vector<std::string> neighbor_ids;
    neighbor_ids.reserve(table_it->second.GetAll().size());
    for(const auto& entry : table_it->second.GetAll()){
        neighbor_ids.push_back(entry.first);
    }

    if(neighbor_ids.empty()) return;

    // Deterministic neighbor order.
    std::sort(neighbor_ids.begin(),neighbor_ids.end());

    if(m_next_neighbor_index >= neighbor_ids.size()) m_next_neighbor_index = 0;
    Packet packet;
    packet.sender = id;
    packet.receiver = neighbor_ids[m_next_neighbor_index];
    packet.timestamp = estimate_it->second.timestamp;
    packet.est_x = estimate_it->second.x;
    packet.est_y = estimate_it->second.y;
    g_message_bus.Send(packet);

    // Send to another neighbor during the next broadcast.
    m_next_neighbor_index = (m_next_neighbor_index + 1) %neighbor_ids.size();

    m_last_broadcast_tick = current_tick;
    m_has_broadcast = true;
}

void CUnderwaterAgentController::Reset() {
    m_last_broadcast_tick = 0;
    m_has_broadcast = false;
    m_next_neighbor_index = 0;
}