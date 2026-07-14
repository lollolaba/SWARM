#pragma once
#include <cmath>
#include <string>
#include <unordered_map>
#include <vector>
#include "position_estimation.h"

struct FGNode {
    double x = 0.0;
    double y = 0.0;
};

struct FGEdge {
    std::string a;
    std::string b;
    double distance = 0.0;
    double weight = 1.0;
};

class FactorGraphLight {

public:
    void AddNode(const std::string& id,double x,double y);

    void SetNode(const std::string& id,double x,double y);

    const FGNode& Get(const std::string& id) const;

    const FGNode& GetNode(const std::string& id) const;

    void AddRangeConstraint(const std::string& a,const std::string& b,double distance,double weight = 1.0);

    void Optimize(int iterations = 1,double step = 0.01);

    void ClearEdges();
    std::unordered_map<std::string, FGNode>& GetNodes();
    
    void SetNeighbors(const std::string&, const std::vector<std::string>&);
    PositionEstimate Update(const std::string&, double, double, double){return {};}

private:
    std::unordered_map<std::string, FGNode> m_nodes;
    std::vector<FGEdge> m_edges;
};