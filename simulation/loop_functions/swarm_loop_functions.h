#pragma once

#include <argos3/core/simulator/loop_functions.h>
#include "swarm_loop_state.h"
#include "../../core/config/experiment_config.h"
#include "../../environment/sensing/range_sensor.h"

class CSwarmLoopFunctions : public argos::CLoopFunctions {
public:
    CSwarmLoopFunctions();
    void Init(argos::TConfigurationNode& t_tree) override;
    void Reset() override;
    void PostStep() override;

    const SwarmLoopState& GetState() const;

private:
    SwarmLoopState m_state;
    ExperimentConfig m_config;
    RangeSensor m_range_sensor;
};