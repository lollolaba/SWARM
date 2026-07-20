#pragma once

#include "../../core/communication/packet.h"

#include <vector>
#include <unordered_map>
#include <string>

class MessageBus {

public:
    void Send(const Packet& p);
    void DeliverToInbox(
        const std::string& id,
        const Packet& p
    );

    std::vector<Packet>& GetOutbox();
    std::vector<Packet>& GetInbox(const std::string& id);
    void ClearOutbox();
    void ClearInboxes();

private:
    std::vector<Packet> m_outbox;
    std::unordered_map<
        std::string,
        std::vector<Packet>
    > m_inboxes;
};