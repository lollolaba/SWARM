#pragma once
#include "../../core/communication/packet.h"
#include "../events/event_bus.h"
#include <vector>
#include "scheduled_packet.h"

class Scheduler {
public:
    void Schedule(const ScheduledPacket& p);
    void Step(double sim_time, EventBus& bus);
    std::vector<Packet> GetPacketsAtTime(double sim_time);
    void Clear();

private:
    std::vector<ScheduledPacket> m_queue;
};