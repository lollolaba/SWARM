#include "kinematic_model.h"

AgentState KinematicModel::Step(const AgentState& s, double dt) {
    AgentState out = s;
    out.x += s.vx * dt;
    out.y += s.vy * dt;
    return out;
}