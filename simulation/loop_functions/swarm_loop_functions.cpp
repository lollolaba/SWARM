#include "swarm_loop_functions.h"
#include <argos3/core/utility/math/rng.h>
#include "../../globals.h"
#include "../../environment/channel/acoustic_channel.h"
#include "../../middleware/system.h"
#include "../../middleware/events/channel_collision.h"
#include "../../middleware/logging/log_utils.h"
#include <argos3/core/simulator/space/space.h>
#include <argos3/core/simulator/entity/controllable_entity.h>
#include <argos3/plugins/robots/foot-bot/simulator/footbot_entity.h>
#include "../../core/config/config_loader.h"
#include <argos3/core/utility/datatypes/any.h>
#include <fstream>
#include <cmath>
#include <vector>
#include <algorithm>

REGISTER_LOOP_FUNCTIONS(CSwarmLoopFunctions, "swarm_loopfunctions")

CSwarmLoopFunctions::CSwarmLoopFunctions() {}

void CSwarmLoopFunctions::Init(argos::TConfigurationNode& t_node) {
    m_config.ResetDefaults();
    if(argos::NodeExists(t_node,"experiment_config")){
        ConfigLoader::Load(m_config,t_node);
        std::string mac_protocol = m_config.mac.protocol;
        argos::GetNodeAttributeOrDefault(t_node,"mac_protocol",mac_protocol,mac_protocol);
        if(mac_protocol == "TDMA") {
            argos::LOGERR
                << "[CONFIG WARNING] TDMA is not currently "
                << "implemented. Using CSMA instead."
                << std::endl;
            mac_protocol = "CSMA";
        }
        if(mac_protocol == "ALOHA") {
            g_mac_layer.SetMode(MACMode::ALOHA);
        }
        else if(mac_protocol == "CSMA") {
            g_mac_layer.SetMode(MACMode::CSMA);
        }
        else {
            argos::LOGERR
                << "[CONFIG] Unsupported MAC protocol: "
                << mac_protocol
                << ". Using CSMA."
                << std::endl;
            mac_protocol = "CSMA";
            g_mac_layer.SetMode(MACMode::CSMA);
        }

        m_config.mac.protocol =mac_protocol;
        g_channel.SetNoiseSigma(m_config.channel.noise);
        g_channel.SetRange(m_config.channel.range);
        g_channel.SetAttenuation(m_config.channel.attenuation);
        g_channel.SetSpeedSound(m_config.channel.speed_sound);
        g_acoustic_bitrate = m_config.channel.bitrate_bps;
        g_channel.SetFrequencyKHz(m_config.channel.frequency_khz);
        g_channel.SetBitrate(g_acoustic_bitrate);
        g_channel.SetSpreadingFactor(m_config.channel.spreading_factor);
        g_channel.SetSourceLevelDb(m_config.channel.source_level_db);
        g_channel.SetAmbientNoiseDb(m_config.channel.ambient_noise_db);
        g_channel.SetSnrThresholdDb(m_config.channel.snr_threshold_db);
        g_channel.SetSnrTransitionDb(m_config.channel.snr_transition_db);
        g_channel.SetFadingSigmaDb(m_config.channel.fading_sigma_db);
        g_channel.SetPacketOverheadBytes(m_config.channel.packet_overhead_bytes);
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
    cfg << "frequency_khz="
    << m_config.channel.frequency_khz
    << "\n";
    cfg << "bitrate_bps="
    << m_config.channel.bitrate_bps
    << "\n";
    cfg << "spreading_factor="
    << m_config.channel.spreading_factor
    << "\n";
    cfg << "source_level_db="
    << m_config.channel.source_level_db
    << "\n";
    cfg << "ambient_noise_db="
    << m_config.channel.ambient_noise_db
    << "\n";
    cfg << "snr_threshold_db="
    << m_config.channel.snr_threshold_db
    << "\n";
    cfg<< "mac="
    << m_config.mac.protocol
    << "\n";
    cfg<< "localization="
    << m_config.localization.algorithm
    << "\n";
    cfg.close();

    if(!g_rng) g_rng = argos::CRandom::CreateRNG("argos");
    m_range_sensor.SetNoiseSigma(m_config.channel.noise);
    
    m_state.time = 0.0;

    // CHANNEL MODE
    std::string mode = m_config.channel.mode;

    argos::GetNodeAttributeOrDefault(
        t_node,
        "channel_mode",
        mode,
        mode);

    if(mode == "BASIC") {
        g_channel.SetMode(
            ChannelMode::BASIC);
    }
    else if(mode == "REALISTIC") {
        g_channel.SetMode(
            ChannelMode::REALISTIC);
    }
    else if(mode == "ADVANCED_SNR") {
        g_channel.SetMode(
            ChannelMode::ADVANCED_SNR);
    }
    else if(mode == "UNDERWATER_EXTREME") {
        g_channel.SetMode(
            ChannelMode::UNDERWATER_EXTREME);
    }
    else if(mode == "THORP") {
        g_channel.SetMode(
            ChannelMode::THORP);
    }
    else {
        argos::LOGERR
            << "[CONFIG] Unknown channel mode: "
            << mode
            << ". Falling back to REALISTIC."
            << std::endl;

        mode = "REALISTIC";

        g_channel.SetMode(
            ChannelMode::REALISTIC);
    }

    m_config.channel.mode = mode;

    // LOCALIZATION MODE
    std::string loc_mode = m_config.localization.algorithm;
    argos::GetNodeAttributeOrDefault(t_node,"localization_mode",loc_mode,loc_mode);
    if(loc_mode == "IDEAL")
        g_localization_mode = LocalizationMode::IDEAL;
    else if(loc_mode=="NOISY" || loc_mode=="NONE")
        g_localization_mode = LocalizationMode::NOISY;
    else if(loc_mode == "EKF")
        g_localization_mode = LocalizationMode::EKF;
    else if(loc_mode =="EKF_CI")
        g_localization_mode = LocalizationMode::EKF_CI,
    else if(loc_mode == "CONSENSUS")
        g_localization_mode = LocalizationMode::CONSENSUS;
    else if(loc_mode == "PARTICLE_FILTER")
        g_localization_mode = LocalizationMode::PARTICLE_FILTER;
    else if(loc_mode == "FACTOR_GRAPH")
        g_localization_mode = LocalizationMode::FACTOR_GRAPH;
    else if(loc_mode == "BAYESIAN")
        g_localization_mode = LocalizationMode::BAYESIAN;
    else { argos::LOGERR << "Falling back to NOISY" << std::endl; 
        g_localization_mode=LocalizationMode::NOISY;}
    g_localization_stack.SetMode(g_localization_mode);
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
    g_message_bus.ClearInboxes();
    g_scheduler.Clear();
    g_event_bus.Clear();
    g_mac_layer.Reset();
    g_robot_truth_state.clear();
    g_robot_est_state.clear();
    g_robot_inboxes.clear();
    g_neighbor_tables.clear();
    g_routing_tables.clear();
    g_localization_stack = LocalizationStack();
    g_localization_stack.SetMode(g_localization_mode);
}

const SwarmLoopState& CSwarmLoopFunctions::GetState() const {
    return m_state;
}

void CSwarmLoopFunctions::PostStep() {
    const double sim_time = static_cast<double>(GetSpace().GetSimulationClock()) * m_config.simulation.timestep;
    if(sim_time >= m_config.simulation.duration){
        GetSimulator().Terminate();
        return;
    }
    m_state.time = sim_time;
    const std::string localization_mode = LocalizationModeToString(g_localization_mode);
    g_scheduler.Step(sim_time,g_event_bus);
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
    // LOCALIZATION LAYER
    // Ensure that every robot has an initial noisy estimate
    // before cooperative localization starts.
    for(const auto& it : g_robot_truth_state) {
        const std::string& id = it.first;
        const AgentState& truth = it.second;

        if(g_robot_est_state.find(id) == g_robot_est_state.end()){
            AgentState initial_estimate;
            initial_estimate.x = truth.est_x;
            initial_estimate.y = truth.est_y;

            initial_estimate.est_x = truth.est_x;
            initial_estimate.est_y = truth.est_y;
            initial_estimate.timestamp = sim_time;
            initial_estimate.covariance = 0.1;

            g_robot_est_state.emplace(id,initial_estimate);
        }
    }

    // Use a snapshot so all robots use estimates from the
    // previous localization step. This prevents update-order bias.
    const auto prior_estimates = g_robot_est_state;
    const double range_variance = std::max(m_config.channel.noise * m_config.channel.noise + m_config.mobility.imu_noise * m_config.mobility.imu_noise,1e-9);
    for(const auto& it : g_robot_truth_state) {
        const std::string& id = it.first;
        const AgentState& truth = it.second;

        std::vector<std::string> neighbor_ids;
        std::vector<RangeObservation> range_observations;

        const auto table_it = g_neighbor_tables.find(id);
        if(table_it != g_neighbor_tables.end()) {
            const auto& neighbors = table_it->second.GetNeighbors();
            neighbor_ids.reserve(neighbors.size());
            range_observations.reserve(neighbors.size());

            for(const auto& neighbor : neighbors) {
                const auto estimate_it = prior_estimates.find(neighbor.id);

                if(estimate_it == prior_estimates.end()) continue;

                neighbor_ids.push_back(neighbor.id);
                RangeObservation observation;
                observation.neighbor_id = neighbor.id;
                observation.measured_range = m_range_sensor.Measure(neighbor.distance,g_rng);
                observation.neighbor_x =estimate_it->second.x;
                observation.neighbor_y =estimate_it->second.y;

                observation.measurement_variance =range_variance;
                observation.neighbor_position_variance = std::max(0.5 * estimate_it->second.covariance, 1e-9);
                range_observations.push_back(observation);
            }
        }
        g_localization_stack.SetNeighbors(id,neighbor_ids);
        g_localization_stack.SetRangeObservations(id,range_observations);
        const PositionEstimate estimate = g_localization_stack.Update(id,truth.x,truth.y,truth.est_x,truth.est_y,m_config.simulation.timestep);
        auto& estimated_state = g_robot_est_state[id];

        estimated_state.x = estimate.x;
        estimated_state.y = estimate.y;
        estimated_state.est_x = estimate.x;
        estimated_state.est_y = estimate.y;
        estimated_state.timestamp = sim_time;
        estimated_state.covarince = estimate.cov;
    }
    // ROUTING LAYER
    for(const auto& it : g_neighbor_tables) {
        const std::string& id = it.first;
        g_routing_tables[std::string(id)].UpdateFromNeighbors(it.second,id,sim_time);
        g_routing_tables[std::string(id)].RemoveStale(sim_time,  m_config.channel.routing_timeout);
    }
    // TX PIPELINE
    auto& outbox = g_message_bus.GetOutbox();
    std::vector<Packet> transmitting_packets;
    transmitting_packets.reserve(outbox.size());
    // Apply the configured MAC protocol first.
    for(const Packet& packet : outbox) {
        if(!g_mac_layer.CanTransmit(packet.sender,sim_time))continue;
        g_mac_layer.OnTransmit(packet.sender,sim_time);
        transmitting_packets.push_back(packet);
    }
    // Process only MAC-approved packets.
    for(const Packet& packet : transmitting_packets){
        const auto sender_it = g_robot_truth_state.find(packet.sender);
        const auto receiver_it = g_robot_truth_state.find(packet.receiver);
        if(sender_it == g_robot_truth_state.end() || receiver_it == g_robot_truth_state.end()) continue;
        // Physical channel distance must use
        // sender and receiver ground-truth positions.
        const double dx = sender_it->second.x - receiver_it->second.x;
        const double dy = sender_it->second.y -receiver_it->second.y;
        const double distance = std::sqrt(dx * dx + dy * dy);
        // Same-time transmissions collide only when
        // they target the same receiver.
        if(ChannelCollision::HasCollision(packet,transmitting_packets,m_config.simulation.timestep)){
            Event drop{};
            drop.time = sim_time;
            drop.timestamp = packet.timestamp;
            drop.type = EventType::DROP;

            drop.packet = packet;
            drop.sender = packet.sender;
            drop.receiver = packet.receiver;
            drop.distance = distance;
            drop.reason = "collision";

            drop.channel_mode = g_channel.GetModeString();

            drop.localization_mode = localization_mode;
            drop.metric1 =
                result.metric1;

            drop.metric2 =
                result.metric2;

            drop.metric3 =
                result.metric3;
            g_event_bus.Push(drop);
            continue;
        }

        const ChannelResult result = g_channel.Transmit(packet,distance,sim_time);
        if(!result.delivered) {
            Event drop{};
            drop.time = sim_time;
            drop.timestamp = result.timestamp;
            drop.type = EventType::DROP;
            drop.packet = packet;
            drop.sender = packet.sender;
            drop.receiver = packet.receiver;
            drop.distance = distance;
            drop.delay = result.delay;
            drop.snr = result.snr;
            drop.reason = result.reason;
            drop.channel_mode = result.channel_mode;
            drop.localization_mode = localization_mode;
            drop.metric1 =
                result.metric1;

            drop.metric2 =
                result.metric2;

            drop.metric3 =
                result.metric3;
            g_event_bus.Push(drop);
            continue;
        }

        ScheduledPacket scheduled;
        scheduled.packet = packet;
        scheduled.delivery_time = sim_time + result.delay;
        scheduled.distance = distance;
        scheduled.delay = result.delay;
        scheduled.snr = result.snr;
        scheduled.channel_mode = result.channel_mode;
        scheduled.localization_mode = localization_mode;
        scheduled.metric1 =
            result.metric1;

        scheduled.metric2 =
            result.metric2;

        scheduled.metric3 =
            result.metric3;
        scheduled.timestamp = sim_time;
        scheduled.delivered = true;
        g_scheduler.Schedule(scheduled);

        Event tx{};
        tx.time = sim_time;
        tx.timestamp = sim_time;
        tx.packet = packet;
        tx.type = EventType::TX;
        tx.sender = packet.sender;
        tx.receiver = packet.receiver;
        tx.distance = distance;
        tx.delay = result.delay;
        tx.snr = result.snr;
        tx.channel_mode = result.channel_mode;
        tx.localization_mode = localization_mode;
        tx.metric1 =
            result.metric1;

        tx.metric2 =
            result.metric2;

        tx.metric3 =
            result.metric3;
        g_event_bus.Push(tx);
    }
    g_message_bus.ClearOutbox();
    // NETWORK EVENT LOGGING
    const auto& network_events = g_event_bus.GetEvents();
    for(const Event& event : network_events) {
        switch(event.type) {
            case EventType::TX:
                g_network_logger.LogTx(
                    event.time,
                    event.packet,
                    event.distance,
                    event.delay,
                    event.snr,
                    event.localization_mode,
                    event.channel_mode,
                    event.metric1,
                    event.metric2,
                    event.metric3);
                break;

            case EventType::RX:
                g_network_logger.LogRx(
                    event.time,
                    event.packet,
                    event.distance,
                    event.delay,
                    event.snr,
                    event.localization_mode,
                    event.channel_mode,
                    event.metric1,
                    event.metric2,
                    event.metric3);
                break;

            case EventType::DROP:
                g_network_logger.LogDrop(
                    event.time,
                    event.packet,
                    event.reason,
                    event.distance,
                    event.delay,
                    event.snr,
                    event.localization_mode,
                    event.channel_mode,
                    event.metric1,
                    event.metric2,
                    event.metric3);
                break;
        }
    }
    g_network_logger.Flush();
    // Events have now been consumed.
    // Clear them to prevent accumulation in memory.
    g_event_bus.Clear();
}