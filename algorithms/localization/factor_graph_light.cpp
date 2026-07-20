#include "factor_graph_light.h"
#include <cmath>

void FactorGraphLight::AddNode(const std::string& id, double x, double y) {
    m_nodes[id] = {x, y};
}

void FactorGraphLight::SetNode(const std::string& id, double x, double y) {
    m_nodes[id].x = x;
    m_nodes[id].y = y;
}

const FGNode& FactorGraphLight::Get(const std::string& id) const {
    static FGNode empty;
    auto it = m_nodes.find(id);
    if (it == m_nodes.end()) return empty;

    return it->second;
}

//RANGE CONSTRAINT
void FactorGraphLight::AddRangeConstraint(const std::string& a,const std::string& b,double distance,double weight){
    m_edges.push_back({a, b, distance, weight});
}

//GRADIENT DESCENT OPTIMIZATION
void FactorGraphLight::Optimize(int iterations, double step){
    for (int it = 0; it < iterations; ++it) {
        for (const auto& e : m_edges) {
            auto& na = m_nodes[e.a];
            auto& nb = m_nodes[e.b];

            double dx = na.x - nb.x;
            double dy = na.y - nb.y;

            double d = std::sqrt(dx *dx + dy *dy + 1e-9);

            double error = d - e.distance;

            double gx = (dx / d) * error * e.weight;
            double gy = (dy / d) * error * e.weight;

            na.x -= step * gx;
            na.y -= step * gy;

            nb.x += step * gx;
            nb.y += step * gy;
        }
    }
}

void FactorGraphLight::ClearEdges() {
    m_edges.clear();
}

std::unordered_map<std::string, FGNode>& FactorGraphLight::GetNodes() {
    return m_nodes;
}

void FactorGraphLight::SetNeighbors(const std::string&, const std::vector<std::string>&){
    
}