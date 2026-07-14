#include "swarm_loop_functions.h"
#include <argos3/core/utility/math/rng.h>
#include "../../globals.h"
#include "../../environment/channel/acoustic_channel.h"
#include "../../middleware/system.h"
#include "../../middleware/events/channel_collision.h"
#include <argos3/core/simulator/space/space.h>
#include <argos3/core/simulator/entity/controllable_entity.h>
#include <argos3/plugins/robots/foot-bot/simulator/footbot_entity.h>
#include "../../core/config/config_loader.h"
#include <argos3/core/utility/datatypes/any.h>
#include <fstream>
#include <cmath>

REGISTER_LOOP_FUNCTIONS(CSwarmLoopFunctions, "swarm_loopfunctions")

CSwarmLoopFunctions::CSwarmLoopFunctions() {}

void CSwarmLoopFunctions::Init(argos::TConfigurationNode& t_node) {
    m_config.ResetDefaults();
    if(argos::NodeExists(t_node,"experiment_config")){
        ConfigLoader::Load(m_config,t_node);
        if(m_config.mac.protocol == "ALOHA"){
            g_mac_layer.SetMode(MACMode::ALOHA);
        }
        if(m_config.mac.protocol == "TDMA"){
            g_mac_layer.SetMode(MACMode::TDMA);
        }
        else if(m_config.mac.protocol == "CSMA"){
            g_mac_layer.SetMode(MACMode::CSMA);
        }
        g_channel.SetNoiseSigma(m_config.channel.noise);
        g_channel.SetRange(m_config.channel.range);
        g_channel.SetAttenuation(m_config.channel.attenuation);
        g_channel.SetSpeedSound(m_config.channel.speed_sound);
        g_mac_layer.SetALOHAProbability(m_config.mac.aloha_probability);
        g_mac_layer.SetCSMABackoff(m_config.mac.csma_backoff);
        g_mac_layer.SetCSMARandomWindow(m_config.mac.csma_random_window);
        argos::LOG << "[CONFIG] Acoustic range = "
            << m_config.channel.range
            << std::endl;
        argos::LOG<< "[CONFIG] MAC = "
            << m_config.mac.protocol
            << std::endl;
        argos::LOG<< "[CONFIG] duration = "
            << m_config.simulation.duration
            << " s"
            << std::endl;
        argos::LOG<< "[CONFIG] seed = "
            << m_config.simulation.seed
            << std::endl;
    }
    std::ofstream cfg("experiment_config.log");
    cfg << "seed="
    << m_config.simulation.seed
    << "\n";
    cfg << "duration="
    << m_config.simulation.duration
    << "\n";
    cfg << "channel="
    << m_config.channel.mode
    << "\n";
    cfg << "range="
    << m_config.channel.range
    << "\n";
    cfg<< "mac="
    << m_config.mac.protocol
    << "\n";
    cfg<< "localization="
    << m_config.localization.algorithm
    << "\n";
    cfg.close();

    if(!g_rng) g_rng = argos::CRandom::CreateRNG("argos");
    
    m_state.time = 0.0;

    // CHANNEL MODE
    std::string mode = m_config.channel.mode;
    argos::GetNodeAttributeOrDefault(t_node, "channel_mode", mode, mode);

    if(mode == "BASIC") 
        g_channel.SetMode(ChannelMode::BASIC);
    else if(mode == "REALISTIC") 
        g_channel.SetMode(ChannelMode::REALISTIC);
    else if(mode == "ADVANCED_SNR") 
        g_channel.SetMode(ChannelMode::ADVANCED_SNR);
  /*else if(mode == "UNDERWATER_EXTREME") 
        g_channel.SetMode(ChannelMode::UNDERWATER_EXTREME);*/
    // LOCALIZATION MODE
    std::string loc_mode = m_config.localization.algorithm;
    argos::GetNodeAttributeOrDefault(t_node,"localization_mode",loc_mode,loc_mode);
    if(loc_mode == "EKF")
        g_localization_mode = LocalizationMode::EKF;
    else if(loc_mode == "CONSENSUS")
        g_localization_mode = LocalizationMode::CONSENSUS;
    else if(loc_mode == "PARTICLE_FILTER")
        g_localization_mode = LocalizationMode::PARTICLE_FILTER;
    else if(loc_mode == "FACTOR_GRAPH")
        g_localization_mode = LocalizationMode::FACTOR_GRAPH;
    else if(loc_mode == "BAYESIAN")
        g_localization_mode = LocalizationMode::BAYESIAN;
    // HEADLESS MODE
    bool headless = m_config.simulation.headless;
    argos::GetNodeAttributeOrDefault(t_node, "headless", headless, headless);
    g_headless_mode = headless;

    if(g_headless_mode) {
        g_network_logger.Init("network_log.csv");
    }
}
void CSwarmLoopFunctions::Reset() {
    m_state = SwarmLoopState();
    g_message_bus.ClearOutbox();
}

const SwarmLoopState& CSwarmLoopFunctions::GetState() const {
    return m_state;
}

void CSwarmLoopFunctions::PostStep() {
    const double sim_time = GetSpace().GetSimulationClock();
    if(sim_time >= m_config.simulation.duration){
        GetSimulator().Terminate();
        return;
    }
    m_state.time = sim_time;
    //UPDATE GROUND TRUTH
    auto& space = GetSpace();
    auto& entities = space.GetEntitiesByType("foot-bot");

    for(const auto& it : entities) {
        auto* fb = argos::any_cast<argos::CFootBotEntity*>(it.second);
        if(!fb) continue;

        const std::string& id = it.first;
        const auto& pos = fb->GetEmbodiedEntity().GetOriginAnchor().Position;
        auto& truth = g_robot_truth_state[id];
        truth.x = pos.GetX();
        truth.y = pos.GetY();
        truth.timestamp = sim_time;
    }
    // MOBILITY REALISM LAYER
    // current field
    //auto& current = g_current_field.Sample(sim_time);
    double current = m_config.mobility.current_strength;
    // drag + inertia correction (conceptual)
    for(const auto& it : entities) {
        auto* fb = argos::any_cast<argos::CFootBotEntity*>(it.second);
        if(!fb) continue;
        const std::string& id = it.first;
        //auto vel = fb->GetEmbodiedEntity().GetOriginAnchor().Velocity;
        auto& truth = g_robot_truth_state[id];
        // drift induced by current
        double drift_x = current * m_config.mobility.current_factor;
        double drift_y = current * m_config.mobility.current_factor;
        // anisotropic drag
        double drag_x = m_config.mobility.drag_x ; // da cambiare!
        double drag_y = m_config.mobility.drag_y ;
        // IMU noise (affects localization, NOT physics)
        truth.x += drift_x + drag_x;
        truth.y += drift_y + drag_y;
        // optional:sensor  noise
        truth.est_x = truth.x + g_rng->Gaussian(0, m_config.mobility.imu_noise);
        truth.est_y = truth.y + g_rng->Gaussian(0, m_config.mobility.imu_noise);
    }

    // NEIGHBOR DISCOVERY 
    for(const auto& a : entities) {
        auto* fb_a = argos::any_cast<argos::CFootBotEntity*>(a.second);
        if(!fb_a) continue;
        const std::string& id_a = a.first;
        const auto& pos_a = fb_a->GetEmbodiedEntity().GetOriginAnchor().Position;
        for(const auto& b : entities) {
            if(a.first == b.first) continue;
            auto* fb_b = argos::any_cast<argos::CFootBotEntity*>(b.second);
            if(!fb_b) continue;
            const std::string& id_b = b.first;
            const auto& pos_b = fb_b->GetEmbodiedEntity().GetOriginAnchor().Position;

            double dx = pos_a.GetX() - pos_b.GetX();
            double dy = pos_a.GetY() - pos_b.GetY();
            double dist = std::sqrt(dx*dx + dy*dy);
            // communication range (underwater acoustic)
            if(dist < m_config.channel.range) g_neighbor_tables[std::string(id_a)].Update(id_b,dist,sim_time);
        }
        g_neighbor_tables[std::string(id_a)].RemoveStale(sim_time,  m_config.channel.neighbor_timeout);
    }
    // ROUTING LAYER
    for(const auto& it : g_neighbor_tables) {
        const std::string& id = it.first;
        g_routing_tables[std::string(id)].UpdateFromNeighbors(it.second,id,sim_time);
        g_routing_tables[std::string(id)].RemoveStale(sim_time,  m_config.channel.routing_timeout);
    }
    //TX PIPELINE
    auto& outbox =g_message_bus.GetOutbox();
    for(const Packet& p : outbox) {
        // MAC LAYER
        if(!g_mac_layer.CanTransmit(p.sender, sim_time)) continue;
        g_mac_layer.OnTransmit(p.sender, sim_time);
        // COLLISION CHECK
        auto pending = g_scheduler.GetPacketsAtTime(sim_time);
        if(ChannelCollision::HasCollision(p, pending)) {
            Event drop;
            drop.time = sim_time;
            drop.type = EventType::DROP;
            drop.packet = p;
            drop.sender = p.sender;
            drop.receiver = p.receiver;
            drop.reason = "collision";
            drop.channel_mode = g_channel.GetModeString();
            drop.localization_mode = "unknown";
            g_event_bus.Push(drop);
            continue;
        }

        //DISTANCE ESTIMATION
        auto it = g_robot_truth_state.find(p.receiver);
        if(it == g_robot_truth_state.end()) continue;

        const double dx = p.est_x - it->second.x;
        const double dy = p.est_y - it->second.y;
        const double distance = std::sqrt(dx*dx + dy*dy);

        //PHYSICAL CHANNEL
        ChannelResult result = g_channel.Transmit(p, distance, sim_time);

        if(!result.delivered) {
            Event drop;
            drop.time = sim_time;
            drop.timestamp = result.timestamp;
            drop.type = EventType::DROP;
            drop.packet = p;
            drop.sender = p.sender;
            drop.receiver = p.receiver;
            drop.distance = distance;
            drop.delay= result.delay;
            drop.snr = result.snr;
            drop.reason = result.reason;
            drop.channel_mode = result.channel_mode;
            drop.localization_mode = result.localization_mode;
            g_event_bus.Push(drop);
            continue;
        }

        //SCHEDULING DELIVERY
        ScheduledPacket sp;
        sp.packet = p;
        sp.delivery_time = sim_time + result.delay;
        sp.distance = result.distance;
        sp.delay = result.delay;
        sp.snr = result.snr;
        sp.channel_mode = result.channel_mode;
        sp.localization_mode = result.localization_mode;
        sp.timestamp = sim_time;
        sp.delivered = true;
        g_scheduler.Schedule(sp);

        //TX EVENT (VALID ONLY)
        Event tx;
        tx.time = sim_time;
        tx.timestamp = sim_time;
        tx.packet = p;
        tx.type = EventType::TX;
        tx.sender = p.sender;
        tx.receiver = p.receiver;
        tx.distance = distance;
        tx.delay = result.delay;
        tx.snr = result.snr;
        tx.channel_mode = result.channel_mode;
        tx.localization_mode = result.localization_mode;
        g_event_bus.Push(tx);
    }

    g_message_bus.ClearOutbox();
}