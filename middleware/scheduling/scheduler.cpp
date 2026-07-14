#include "scheduler.h"
#include "../../globals.h"
#include "../messaging/message_bus.h"

#include "../events/event_bus.h"
#include "../events/event.h"
#include "../../environment/channel/acoustic_channel.h"

extern MessageBus g_message_bus;

void Scheduler::Schedule(const ScheduledPacket& p) {
    m_queue.push_back(p);
}

std::vector<Packet> Scheduler::GetPacketsAtTime(double sim_time) {
    std::vector<Packet> out;
    auto it = m_queue.begin();
    while(it != m_queue.end()) {
        if(it->delivery_time <= sim_time) {
            out.push_back(it->packet);
            it = m_queue.erase(it);
        } else {
            ++it;
        }
    }
    return out;
}


void Scheduler::Step(double sim_time, EventBus& bus) {
    auto delivered = GetPacketsAtTime(sim_time);
    for(const auto& p : delivered) {
        Event rx;
        rx.time = sim_time;
        rx.type = EventType::RX;
        rx.packet = p;
        rx.sender = p.sender;
        rx.receiver = p.receiver;
        for(const auto& scheduled : m_queue) {


            if(scheduled.packet.sender == p.sender && scheduled.packet.receiver == p.receiver) {
                rx.distance =scheduled.distance;
                rx.delay =scheduled.delay;
                rx.snr =scheduled.snr;
                rx.channel_mode =scheduled.channel_mode;
                rx.localization_mode =scheduled.localization_mode;
                rx.reason =scheduled.drop_reason;
                rx.timestamp =scheduled.timestamp;
                rx.metric1 =scheduled.metric1;
                rx.metric2 =scheduled.metric2;
                rx.metric3 =scheduled.metric3;
                break;
            }
        }
        bus.Push(rx);
    }
}

void Scheduler::Clear() {
    m_queue.clear();
}  