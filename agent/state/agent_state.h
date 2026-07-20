#pragma once
struct AgentState {
    double x = 0.0;
    double y = 0.0;
    double covariance = 0.0;
    double vx=0;
    double vy=0;
    double timestamp = 0.0;
    double est_x = 0.0;
    double est_y = 0.0;
};