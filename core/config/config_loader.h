#ifndef CONFIG_LOADER_H
#define CONFIG_LOADER_H
#include "experiment_config.h"
#include <argos3/core/utility/configuration/argos_configuration.h>

class ConfigLoader{

public:
    /**
     * Load experiment parameters from ARGoS XML node.
     *
     * Missing values preserve defaults.
     */
    static void Load(ExperimentConfig& config,argos::TConfigurationNode& node);

private:
    static void LoadSimulation(ExperimentConfig& config,argos::TConfigurationNode& node);

    static void LoadRobots(ExperimentConfig& config,argos::TConfigurationNode& node);

    static void LoadChannel(ExperimentConfig& config,argos::TConfigurationNode& node);

    static void LoadLocalization(ExperimentConfig& config,argos::TConfigurationNode& node);

    static void LoadMAC(ExperimentConfig& config,argos::TConfigurationNode& node);

    static void LoadMobility(ExperimentConfig& config,argos::TConfigurationNode& node);
};


#endif