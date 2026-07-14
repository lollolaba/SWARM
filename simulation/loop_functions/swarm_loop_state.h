#pragma once
#include <vector>
#include <map>
#include "core/communication/packet.h"

struct SwarmLoopState {
    double time = 0.0;

    std::vector<Packet> outbox_snapshot;
    std::vector<Packet> pending_snapshot;
    std::unordered_map<std::string, std::vector<Packet>> inbox_snapshot;
    std::string last_event;
};