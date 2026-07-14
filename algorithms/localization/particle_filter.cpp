#include "particle_filter.h"
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <string>

void ParticleFilterLocalization::Init(int num_particles){
    // optional global init hook (kept simple)
}

//NORMALIZE WEIGHTS
void ParticleFilterLocalization::Normalize(std::vector<Particle>& ps){
    double sum = 0.0;
    for (auto& p : ps) sum += p.w;
    if (sum < 1e-9) return;
    for (auto& p : ps) p.w /= sum;
}

// SIMPLE RESAMPLING (systematic-ish)
void ParticleFilterLocalization::Resample(std::vector<Particle>& ps){
    std::vector<Particle> new_ps;
    new_ps.reserve(ps.size());

    double step = 1.0 / ps.size();
    double r = ((double)rand() / RAND_MAX) * step;
    if (ps.empty()) return;
    double c = ps[0].w;
    int i = 0;

    for (int m = 0; m < ps.size(); m++) {
        double U = r + m * step;
        while (U > c && i < ps.size() - 1) {
            i++;
            c += ps[i].w;
        }
        Particle p = ps[i];

        // jitter (motion model noise)
        p.x += ((rand() % 100) / 500.0) - 0.1;
        p.y += ((rand() % 100) / 500.0) - 0.1;
        p.w = 1.0 / ps.size();
        new_ps.push_back(p);
    }
    ps = new_ps;
}

//UPDATE STEP
PositionEstimate ParticleFilterLocalization::Update(const std::string& id,double x,double y,double /*t*/)
{
    auto& ps = m_particles[id];
    if (ps.empty()) {
        for (int i = 0; i < 20; i++) {
            ps.push_back({
                x + ((rand() % 100) / 500.0),
                y + ((rand() % 100) / 500.0),
                1.0
            });
        }
    }

    // measurement update (likelihood)
    double sigma = 0.5;
    for (auto& p : ps) {
        double dx = x - p.x;
        double dy = y - p.y;
        double dist2 = dx*dx + dy*dy;
        // Gaussian likelihood
        p.w = std::exp(-dist2 / (2 * sigma * sigma));
    }
    Normalize(ps);
    Resample(ps);

    //estimate
    double sx = 0.0, sy = 0.0;
    double sw = 0.0;
    for (auto& p : ps) {
        sx += p.x;
        sy += p.y;
        sw += 1.0;
    }
    return {sx / sw, sy / sw, 1.0};
}

//NEIGHBORS (placeholder hook for swarm coupling)
void ParticleFilterLocalization::SetNeighbors(const std::string& id,const std::vector<std::string>& neighbors){
    m_neighbors[id] = neighbors;
}

const std::vector<std::string>& ParticleFilterLocalization::GetNeighbors(const std::string& id) const {
    static std::vector<std::string> empty;
    auto it = m_neighbors.find(id);
    if(it == m_neighbors.end()) return empty;
    return it->second;
}