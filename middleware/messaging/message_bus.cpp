#include "message_bus.h"

void MessageBus::Send(const Packet& p) {
    m_outbox.push_back(p);
}

void MessageBus::DeliverToInbox(
    const std::string& id,
    const Packet& p
) {
    m_inboxes[id].push_back(p);
}

std::vector<Packet>& MessageBus::GetInbox(const std::string& id) {
    return m_inboxes[id];
}

std::vector<Packet>& MessageBus::GetOutbox() {
    return m_outbox;
}

void MessageBus::ClearOutbox() {
    m_outbox.clear();
}