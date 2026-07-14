#pragma once

#include <unordered_map>
#include <vector>
#include <string>

#include "core/communication/packet.h"
#include "middleware/messaging/message_bus.h"
#include "middleware/scheduling/scheduler.h"
#include "middleware/events/event_bus.h"
#include "middleware/logging/network_logger.h"
#include "agent/state/agent_state.h"
#include "environment/channel/acoustic_channel.h"
#include "middleware/mac/mac_layer.h"
#include "environment/channel/acoustic_channel.h"
#include "middleware/networking/network_manager.h"
#include "algorithms/localization/ekf_distributed.h"
#include "middleware/neighbors/neighbor_table.h" 
#include <argos3/core/utility/math/rng.h>
#include "middleware/routing/routing_table.h"
#include "argos3/core/utility/math/rng.h"
#include "algorithms/localization/localization_stack.h"
#include <argos3/core/utility/math/rng.h>

class MessageBus;
class Scheduler;
class EventBus;
class NetworkLogger;

// GLOBALS
extern MessageBus g_message_bus;
extern Scheduler g_scheduler;
extern NetworkLogger g_network_logger;
extern EventBus g_event_bus;
extern AcousticChannel g_channel;
extern NetworkManager g_network_manager;
extern MACLayer g_mac_layer;
extern AcousticChannel g_channel;
extern EKFDistributed g_ekf;
extern LocalizationMode g_localization_mode;

extern bool g_headless_mode;
extern bool g_log_network;
extern double g_acoustic_bitrate;
extern argos::CRandom::CRNG* g_rng;

extern std::unordered_map<std::string, AgentState> g_robot_truth_state;
extern std::unordered_map<std::string, AgentState> g_robot_est_state;
extern std::unordered_map<std::string, std::vector<Packet>> g_robot_inboxes;
extern std::unordered_map<std::string, NeighborTable> g_neighbor_tables;
extern std::unordered_map<std::string, RoutingTable> g_routing_tables;