#include "config_loader.h"
#include <argos3/core/utility/configuration/argos_configuration.h>
#include <argos3/core/utility/logging/argos_log.h>

#include <algorithm>
#include <cctype>
#include <string>

std::string ToUpper(std::string value){
    std::transform(value.begin(),value.end(),value.begin(),[](unsigned char character){
        return static_cast<char>(std::toupper(character));
    });
    return value;
}

using namespace argos;

void ConfigLoader::Load(ExperimentConfig& config,TConfigurationNode& node){
    config.ResetDefaults();

    try
    {
        TConfigurationNode exp =GetNode(node,"experiment_config");
        if(NodeExists(exp,"simulation")){
            LoadSimulation(config,GetNode(exp,"simulation"));
        }
        if(NodeExists(exp,"robots")){
            LoadRobots(config,GetNode(exp,"robots"));
        }
        if(NodeExists(exp,"channel")){
            LoadChannel(config,GetNode(exp,"channel"));
        }
        if(NodeExists(exp,"localization")){
            LoadLocalization(config,GetNode(exp,"localization"));
        }
        if(NodeExists(exp,"mac")){
            LoadMAC(config,GetNode(exp,"mac"));
        }
        if(NodeExists(exp,"mobility")){
            LoadMobility(config,GetNode(exp,"mobility"));
        }
    }
    catch(CARGoSException& ex){
        LOGERR << "[CONFIG] "
               << ex.what()
               << std::endl;
    }
    Validate(config);
}

void ConfigLoader::LoadSimulation(ExperimentConfig& config,TConfigurationNode& node){
    GetNodeAttributeOrDefault(node,"seed",config.simulation.seed,config.simulation.seed);
    GetNodeAttributeOrDefault(node,"duration",config.simulation.duration,config.simulation.duration);
    GetNodeAttributeOrDefault(node,"timestep",config.simulation.timestep,config.simulation.timestep);
    GetNodeAttributeOrDefault(node,"headless",config.simulation.headless,config.simulation.headless);
}

void ConfigLoader::LoadRobots(ExperimentConfig& config,TConfigurationNode& node){
    GetNodeAttributeOrDefault(node,"number",config.robots.number,config.robots.number);
    GetNodeAttributeOrDefault(node,"distribution",config.robots.distribution,config.robots.distribution);
    GetNodeAttributeOrDefault(node,"min_distance",config.robots.min_distance,config.robots.min_distance);
    GetNodeAttributeOrDefault(node,"placement_seed",config.robots.placement_seed,config.robots.placement_seed);
    GetNodeAttributeOrDefault(node,"csv",config.robots.csv,config.robots.csv);
}

void ConfigLoader::LoadChannel(
    ExperimentConfig& config,
    TConfigurationNode& node)
{
    GetNodeAttributeOrDefault(
        node,
        "mode",
        config.channel.mode,
        config.channel.mode);

    GetNodeAttributeOrDefault(
        node,
        "range",
        config.channel.range,
        config.channel.range);

    GetNodeAttributeOrDefault(
        node,
        "noise",
        config.channel.noise,
        config.channel.noise);

    GetNodeAttributeOrDefault(
        node,
        "attenuation",
        config.channel.attenuation,
        config.channel.attenuation);

    GetNodeAttributeOrDefault(
        node,
        "speed_sound",
        config.channel.speed_sound,
        config.channel.speed_sound);

    GetNodeAttributeOrDefault(
        node,
        "neighbor_timeout",
        config.channel.neighbor_timeout,
        config.channel.neighbor_timeout);

    GetNodeAttributeOrDefault(
        node,
        "routing_timeout",
        config.channel.routing_timeout,
        config.channel.routing_timeout);

    GetNodeAttributeOrDefault(
        node,
        "frequency_khz",
        config.channel.frequency_khz,
        config.channel.frequency_khz);

    GetNodeAttributeOrDefault(
        node,
        "bitrate_bps",
        config.channel.bitrate_bps,
        config.channel.bitrate_bps);

    GetNodeAttributeOrDefault(
        node,
        "spreading_factor",
        config.channel.spreading_factor,
        config.channel.spreading_factor);

    GetNodeAttributeOrDefault(
        node,
        "source_level_db",
        config.channel.source_level_db,
        config.channel.source_level_db);

    GetNodeAttributeOrDefault(
        node,
        "ambient_noise_db",
        config.channel.ambient_noise_db,
        config.channel.ambient_noise_db);

    GetNodeAttributeOrDefault(
        node,
        "snr_threshold_db",
        config.channel.snr_threshold_db,
        config.channel.snr_threshold_db);

    GetNodeAttributeOrDefault(
        node,
        "snr_transition_db",
        config.channel.snr_transition_db,
        config.channel.snr_transition_db);

    GetNodeAttributeOrDefault(
        node,
        "fading_sigma_db",
        config.channel.fading_sigma_db,
        config.channel.fading_sigma_db);

    GetNodeAttributeOrDefault(
        node,
        "packet_overhead_bytes",
        config.channel.packet_overhead_bytes,
        config.channel.packet_overhead_bytes);
}

void ConfigLoader::LoadLocalization(ExperimentConfig& config,TConfigurationNode& node){
    GetNodeAttributeOrDefault(node,"algorithm",config.localization.algorithm,config.localization.algorithm);
}

void ConfigLoader::LoadMAC(ExperimentConfig& config,TConfigurationNode& node){
    GetNodeAttributeOrDefault(node,"protocol",config.mac.protocol,config.mac.protocol);
    GetNodeAttributeOrDefault(node,"aloha_probability",config.mac.aloha_probability,config.mac.aloha_probability);
    GetNodeAttributeOrDefault(node,"csma_backoff",config.mac.csma_backoff,config.mac.csma_backoff);
    GetNodeAttributeOrDefault(node,"csma_random_window",config.mac.csma_random_window,config.mac.csma_random_window);
}

void ConfigLoader::LoadMobility(ExperimentConfig& config,TConfigurationNode& node){
    if(NodeExists(node,"current")){
        TConfigurationNode current =GetNode(node,"current");
        GetNodeAttributeOrDefault(current,"strength",config.mobility.current_strength,config.mobility.current_strength);
        GetNodeAttributeOrDefault(current,"factor",config.mobility.current_factor,config.mobility.current_factor);
    }
    if(NodeExists(node,"drag")){
        TConfigurationNode drag =GetNode(node,"drag");
        GetNodeAttributeOrDefault(drag,"x",config.mobility.drag_x,config.mobility.drag_x);
        GetNodeAttributeOrDefault(drag,"y",config.mobility.drag_y,config.mobility.drag_y);
    }
    if(NodeExists(node,"imu_noise")){
        TConfigurationNode imu = GetNode(node,"imu_noise");
        GetNodeAttributeOrDefault(imu,"sigma",config.mobility.imu_noise,config.mobility.imu_noise);
    }
}

void ConfigLoader::Validate(ExperimentConfig& config){
    const ExperimentConfig defaults;
    config.channel.mode = ToUpper(config.channel.mode);
    config.localization.algorithm = ToUpper(config.localization.algorithm);
    config.mac.protocol =ToUpper(config.mac.protocol);
    if(config.simulation.duration <= 0.0) {
        LOGERR
            << "[CONFIG] Invalid duration. Using "
            << defaults.simulation.duration
            << " seconds."
            << std::endl;

        config.simulation.duration =
            defaults.simulation.duration;
    }

    if(config.simulation.timestep <= 0.0) {
        LOGERR
            << "[CONFIG] Invalid timestep. Using "
            << defaults.simulation.timestep
            << " seconds."
            << std::endl;

        config.simulation.timestep =
            defaults.simulation.timestep;
    }

    if(config.robots.number == 0) {
        LOGERR
            << "[CONFIG] Robot number must be greater "
            << "than zero. Using 1 robot."
            << std::endl;

        config.robots.number = 1;
    }

    if(config.robots.min_distance < 0.0) {
        LOGERR
            << "[CONFIG] min_distance cannot be negative. "
            << "Using 0."
            << std::endl;

        config.robots.min_distance = 0.0;
    }

    if(config.channel.range <= 0.0) {
        LOGERR
            << "[CONFIG] Invalid acoustic range. Using "
            << defaults.channel.range
            << "."
            << std::endl;

        config.channel.range =
            defaults.channel.range;
    }

    if(config.channel.range <= config.robots.min_distance){
        LOGERR
            << "[CONFIG WARNING] Acoustic range ("
            << config.channel.range
            << ") is not greater than robot "
            << "min_distance ("
            << config.robots.min_distance
            << "). Neighbor discovery may produce "
            << "no communication links."
            << std::endl;
    }

    if(config.channel.noise < 0.0) {
        LOGERR
            << "[CONFIG] Channel noise cannot be "
            << "negative. Using 0."
            << std::endl;

        config.channel.noise = 0.0;
    }

    if(config.channel.attenuation < 0.0) {
        LOGERR
            << "[CONFIG] Channel attenuation cannot be "
            << "negative. Using 0."
            << std::endl;

        config.channel.attenuation = 0.0;
    }

    if(config.channel.speed_sound <= 0.0) {
        LOGERR
            << "[CONFIG] Invalid sound speed. Using "
            << defaults.channel.speed_sound
            << " m/s."
            << std::endl;

        config.channel.speed_sound =
            defaults.channel.speed_sound;
    }
    if(config.channel.frequency_khz <= 0.0) {
        LOGERR
            << "[CONFIG] Acoustic frequency must be "
            << "greater than zero. Using "
            << defaults.channel.frequency_khz
            << " kHz."
            << std::endl;

        config.channel.frequency_khz =
            defaults.channel.frequency_khz;
    }

    if(config.channel.bitrate_bps <= 0.0) {
        LOGERR
            << "[CONFIG] Acoustic bitrate must be "
            << "greater than zero. Using "
            << defaults.channel.bitrate_bps
            << " bit/s."
            << std::endl;

        config.channel.bitrate_bps =
            defaults.channel.bitrate_bps;
    }

    if(config.channel.spreading_factor < 1.0 ||
    config.channel.spreading_factor > 2.0)
    {
        LOGERR
            << "[CONFIG] Spreading factor must be "
            << "between 1.0 and 2.0. Clamping value."
            << std::endl;

        config.channel.spreading_factor =
            std::clamp(
                config.channel.spreading_factor,
                1.0,
                2.0);
    }

    if(config.channel.snr_transition_db <= 0.0) {
        LOGERR
            << "[CONFIG] snr_transition_db must be "
            << "greater than zero. Using "
            << defaults.channel.snr_transition_db
            << " dB."
            << std::endl;

        config.channel.snr_transition_db =
            defaults.channel.snr_transition_db;
    }

    if(config.channel.fading_sigma_db < 0.0) {
        LOGERR
            << "[CONFIG] fading_sigma_db cannot be "
            << "negative. Using 0."
            << std::endl;

        config.channel.fading_sigma_db = 0.0;
    }

    if(config.channel.source_level_db <=
    config.channel.ambient_noise_db)
    {
        LOGERR
            << "[CONFIG WARNING] source_level_db is not "
            << "greater than ambient_noise_db. THORP "
            << "links may experience very high loss."
            << std::endl;
    }
    if(config.channel.neighbor_timeout <= 0.0) {
        config.channel.neighbor_timeout =
            defaults.channel.neighbor_timeout;
    }

    if(config.channel.routing_timeout <= 0.0) {
        config.channel.routing_timeout =
            defaults.channel.routing_timeout;
    }

    const bool valid_channel =
        config.channel.mode == "BASIC" ||
        config.channel.mode == "REALISTIC" ||
        config.channel.mode == "ADVANCED_SNR" ||
        config.channel.mode == "UNDERWATER_EXTREME" ||
        config.channel.mode == "THORP" ;

    if(!valid_channel) {
        LOGERR
            << "[CONFIG] Unsupported channel mode: "
            << config.channel.mode
            << ". Using REALISTIC."
            << std::endl;

        config.channel.mode = "REALISTIC";
    }

    const bool valid_localization =
        config.localization.algorithm == "IDEAL" ||
        config.localization.algorithm == "NOISY" ||
        config.localization.algorithm == "NONE" ||
        config.localization.algorithm == "EKF" ||
        config.localization.algorithm == "EKF_CI" ||
        config.localization.algorithm == "CONSENSUS" ||
        config.localization.algorithm ==
            "PARTICLE_FILTER" ||
        config.localization.algorithm == "BAYESIAN" ||
        config.localization.algorithm ==
            "FACTOR_GRAPH";

    if(!valid_localization) {
        LOGERR
            << "[CONFIG] Unsupported localization "
            << "algorithm: "
            << config.localization.algorithm
            << ". Using NOISY."
            << std::endl;

        config.localization.algorithm = "NOISY";
    }

    if(config.mac.protocol == "TDMA") {
        LOGERR
            << "[CONFIG WARNING] TDMA is not currently "
            << "implemented. Using CSMA instead."
            << std::endl;

        config.mac.protocol = "CSMA";
    }

    if(config.mac.protocol != "ALOHA" &&
       config.mac.protocol != "CSMA")
    {
        LOGERR
            << "[CONFIG] Unsupported MAC protocol: "
            << config.mac.protocol
            << ". Using CSMA."
            << std::endl;

        config.mac.protocol = "CSMA";
    }

    config.mac.aloha_probability =std::clamp(config.mac.aloha_probability,0.0,1.0);
    config.mac.csma_backoff = std::max(0.0,config.mac.csma_backoff);
    config.mac.csma_random_window = std::max(0.0,config.mac.csma_random_window);
}