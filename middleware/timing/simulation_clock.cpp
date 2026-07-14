#include "simulation_clock.h"

void SimulationClock::SetTime(double t) {
    m_time = t;
}

double SimulationClock::GetTime() const {
    return m_time;
}