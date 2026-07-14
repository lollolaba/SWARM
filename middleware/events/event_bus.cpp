#include "event_bus.h"

void EventBus::Push(const Event& e) {
    m_vecEvents.push_back(e);
}

const std::vector<Event>& EventBus::GetEvents() const {
    return m_vecEvents;
}

void EventBus::Clear() {
    m_vecEvents.clear();
}