#pragma once
#include "agent/state/agent_state.h"

class KinematicModel {
public:
    AgentState Step(const AgentState& s, double dt);
};