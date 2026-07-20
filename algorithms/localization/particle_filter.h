#pragma once

#include "position_estimation.h"
#include "range_observation.h"

#include <vector>
#include <unordered_map>
#include <string>

class ParticleFilterLocalization {
public:
    void Init(int num_particles = 20);
    PositionEstimate Update(const std::string& id,double x,double y,double t);

    void SetNeighbors(const std::string& id,const std::vector<std::string>& neighbors);
    void SetRangeObservations(const std::string& id,const std::vector<RangeObservation>& observations);

    const std::vector<std::string>& GetNeighbors(const std::string& id) const;
    struct Particle {
        double x = 0.0;
        double y = 0.0;
        double w = 1.0;
    };

    void Normalize(std::vector<Particle>& particles);
    void Resample(std::vector<Particle>& particles);

    std::unordered_map<std::string, std::vector<Particle>> m_particles;
    std::unordered_map<std::string, std::vector<std::string>> m_neighbors;
    std::unordered_map<std::string, std::vector<RangeObservation>>m_range_observations;
};