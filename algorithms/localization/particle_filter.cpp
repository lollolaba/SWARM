#include "particle_filter.h"

#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <string>

void ParticleFilterLocalization::Init(int num_particles) {
    // Optional global initialization hook.
}

void ParticleFilterLocalization::Normalize(std::vector<Particle>& particles){
    if(particles.empty()) return;
    double sum = 0.0;

    for(const auto& particle : particles) {
        sum += particle.w;
    }

    if(sum < 1e-12) {
        const double uniform_weight = 1.0 / particles.size();

        for(auto& particle : particles) {
            particle.w = uniform_weight;
        }
        return;
    }

    for(auto& particle : particles) {
        particle.w /= sum;
    }
}

void ParticleFilterLocalization::Resample(std::vector<Particle>& particles){
    if(particles.empty()) return;

    std::vector<Particle> new_particles;
    new_particles.reserve(particles.size());
    const double step = 1.0 / particles.size();
    const double random_start = (static_cast<double>(rand()) / RAND_MAX) * step;

    double cumulative_weight = particles[0].w;
    std::size_t particle_index = 0;

    for(std::size_t sample_index = 0;sample_index < particles.size();++sample_index){
        const double target = random_start + sample_index * step;

        while( target > cumulative_weight && particle_index < particles.size() - 1){
            ++particle_index;
            cumulative_weight += particles[particle_index].w;
        }
        Particle particle = particles[particle_index];

        // Existing particle-motion jitter.
        particle.x += ((rand() % 100) / 500.0) - 0.1;
        particle.y += ((rand() % 100) / 500.0) - 0.1;
        particle.w = 1.0 / particles.size();
        new_particles.push_back(particle);
    }
    particles = new_particles;
}

PositionEstimate ParticleFilterLocalization::Update(const std::string& id,double measured_x,double measured_y,double t){
    auto& particles = m_particles[id];

    if(particles.empty()) {
        for(int i = 0; i < 20; ++i) {
            particles.push_back({
                measured_x + ((rand() % 100) / 500.0),
                measured_y + ((rand() % 100) / 500.0),
                1.0
            });
        }
    }

    const double local_sigma = 0.5;
    const auto observations_it = m_range_observations.find(id);
    const bool has_range_observations = observations_it != m_range_observations.end() && !observations_it->second.empty();

    for(auto& particle : particles) {
        const double local_dx = measured_x - particle.x;
        const double local_dy = measured_y - particle.y;
        const double local_distance_squared = local_dx * local_dx + local_dy * local_dy;

        double log_weight = -local_distance_squared / (2.0 * local_sigma * local_sigma);

        if(has_range_observations) {
            for(const auto& observation : observations_it->second){
                const double range_dx = particle.x - observation.neighbor_x;
                const double range_dy = particle.y - observation.neighbor_y;
                const double predicted_range = std::sqrt(range_dx * range_dx + range_dy * range_dy);

                const double residual = observation.measured_range -predicted_range;

                const double variance = std::max(observation.measurement_variance,1e-9);

                log_weight += -(residual * residual) / (2.0 * variance);
            }
        }

        particle.w = std::exp(std::max(log_weight, -700.0));
    }

    Normalize(particles);
    Resample(particles);

    double sum_x = 0.0;
    double sum_y = 0.0;
    double count = 0.0;

    for(const auto& particle : particles) {
        sum_x += particle.x;
        sum_y += particle.y;
        count += 1.0;
    }

    return {
        sum_x / count,
        sum_y / count,
        1.0
    };
}

void ParticleFilterLocalization::SetNeighbors(const std::string& id,const std::vector<std::string>& neighbors){
    m_neighbors[id] = neighbors;
}
const std::vector<std::string>& ParticleFilterLocalization::GetNeighbors(const std::string& id) const{
    static const std::vector<std::string> empty;
    const auto it = m_neighbors.find(id);
    if(it == m_neighbors.end()) return empty;
    return it->second;
}

void ParticleFilterLocalization::SetRangeObservations(const std::string& id,const std::vector<RangeObservation>& observations){
    m_range_observations[id] = observations;
}