#include "config_loader.h"
#include <argos3/core/utility/configuration/argos_configuration.h>
#include <argos3/core/utility/logging/argos_log.h>

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

void ConfigLoader::LoadChannel(ExperimentConfig& config,TConfigurationNode& node){
    GetNodeAttributeOrDefault(node,"mode",config.channel.mode,config.channel.mode);
    GetNodeAttributeOrDefault(node,"range",config.channel.range,config.channel.range);
    GetNodeAttributeOrDefault(node,"noise",config.channel.noise,config.channel.noise);
    GetNodeAttributeOrDefault(node,"attenuation",config.channel.attenuation,config.channel.attenuation);
    GetNodeAttributeOrDefault(node,"speed_sound",config.channel.speed_sound,config.channel.speed_sound);
    GetNodeAttributeOrDefault(node,"neighbor_timeout",config.channel.neighbor_timeout,config.channel.neighbor_timeout);
    GetNodeAttributeOrDefault(node,"routing_timeout",config.channel.routing_timeout,config.channel.routing_timeout);
}

void ConfigLoader::LoadLocalization(ExperimentConfig& config,TConfigurationNode& node){
    GetNodeAttributeOrDefault(node,"algorithm",config.localization.algorithm,config.localization.algorithm);
    GetNodeAttributeOrDefault(node,"aloha_probability",config.mac.aloha_probability,config.mac.aloha_probability);
    GetNodeAttributeOrDefault(node,"csma_backoff",config.mac.csma_backoff,config.mac.csma_backoff);
    GetNodeAttributeOrDefault(node,"csma_random_window",config.mac.csma_random_window,config.mac.csma_random_window);
}

void ConfigLoader::LoadMAC(ExperimentConfig& config,TConfigurationNode& node){
    GetNodeAttributeOrDefault(node,"protocol",config.mac.protocol,config.mac.protocol);
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