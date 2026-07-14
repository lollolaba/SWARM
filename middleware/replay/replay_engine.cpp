#include "replay_engine.h"

void ReplayEngine::Record(const Event& e) {
    if(!m_file.is_open()) return;

    m_file << "EVENT\n"; // placeholder
}

void ReplayEngine::Save(const std::string& file) {
    if(m_file.is_open()) m_file.close();
    m_file.open(file);
}

void ReplayEngine::Load(const std::string& file) {
    // placeholder (future upgrade)
}

void ReplayEngine::Play() {
    // placeholder (replay execution engine)
}