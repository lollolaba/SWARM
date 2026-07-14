#pragma once

class SimulationClock {
public:
    void SetTime(double t);
    double GetTime() const;

private:
    double m_time = 0.0;
};