#pragma once

#include "../events/event.h"
#include <vector>
#include <fstream>

class ReplayEngine {
public:
    void Record(const  Event& e);
    void Save(const std::string& file);
    
    void Load(const std::string& file);
    void Play();

private:
    std::ofstream m_file;
};