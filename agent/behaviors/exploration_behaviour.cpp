#include "exploration_behaviour.h"

void ExplorationBehaviour::Compute(AgentState& s) {
    s.vx = 0.5;
    s.vy = 0.2;
}