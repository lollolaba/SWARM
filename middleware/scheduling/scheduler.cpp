#include "scheduler.h"
#include "../../globals.h"
#include "../events/event.h"

void Scheduler::Schedule(const ScheduledPacket& packet){
    m_queue.push_back(packet);
}

std::vector<Packet>
Scheduler::GetPacketsAtTime(double sim_time){
    std::vector<Packet> packets;
    auto it = m_queue.begin();

    while(it != m_queue.end()) {
        if(it->delivery_time <= sim_time) {
            packets.push_back(it->packet);
            it = m_queue.erase(it);
        }
        else ++it;
    }
    return packets;
}

void Scheduler::Step(double sim_time,EventBus& bus){
    auto it = m_queue.begin();
    while(it != m_queue.end()) {
        if(it->delivery_time > sim_time) {
            ++it;
            continue;
        }
        const ScheduledPacket delivered = *it;
        // Make the received packet available to the robot.
        g_message_bus.DeliverToInbox(delivered.packet.receiver,delivered.packet);

        Event rx{};
        rx.time = sim_time;
        rx.timestamp = delivered.timestamp;
        rx.type = EventType::RX;

        rx.packet = delivered.packet;
        rx.sender = delivered.packet.sender;
        rx.receiver =delivered.packet.receiver;
        rx.distance = delivered.distance;
        rx.delay = delivered.delay;
        rx.snr = delivered.snr;
        rx.channel_mode =delivered.channel_mode;
        rx.localization_mode = delivered.localization_mode;
        rx.reason = delivered.drop_reason;

        rx.metric1 = delivered.metric1;
        rx.metric2 = delivered.metric2;
        rx.metric3 = delivered.metric3;
        bus.Push(rx);
        it = m_queue.erase(it);
    }
}
void Scheduler::Clear() {
    m_queue.clear();
}