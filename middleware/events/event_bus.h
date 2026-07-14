#pragma once

#include "event.h"
#include <vector>


class EventBus {
public:
    void Push(const Event& e);
    const std::vector<Event>& GetEvents() const;
    void Clear();

private:
    std::vector<Event> m_vecEvents;
};