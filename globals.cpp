#include "globals.h"
#include <unordered_map>
#include <vector>
#include <string>

// forward declarations ONLY
class MessageBus;
class Scheduler;
class EventBus;
class NetworkLogger;
class AcousticChannel;
class NetworkManager;
class MACLayer;
class EKFDistributed;
class NeighborTable;
class RoutingTable;
class RobotState;
class AgentState;
class Packet;

argos::CRandom::CRNG* g_rng = nullptr;
MessageBus g_message_bus;
Scheduler g_scheduler;
NetworkLogger g_network_logger;
EventBus g_event_bus;
AcousticChannel g_channel;
NetworkManager g_network_manager;
MACLayer g_mac_layer;
EKFDistributed g_ekf;
LocalizationMode g_localization_mode =LocalizationMode::EKF;
LocalizationStack g_localization_stack;
bool g_headless_mode=false;
bool g_log_network=false;
double g_acoustic_bitrate=1000;
std::unordered_map<std::string,AgentState> g_robot_truth_state;
std::unordered_map<std::string,AgentState> g_robot_est_state;
std::unordered_map<std::string,std::vector<Packet>> g_robot_inboxes;
std::unordered_map<std::string,NeighborTable> g_neighbor_tables;
std::unordered_map<std::string,RoutingTable> g_routing_tables;