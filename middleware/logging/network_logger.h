#pragma once

#include <fstream>
#include <string>

#include "../../core/communication/packet.h"
#include "log_record.h"

class NetworkLogger {
public:
    void Init(
        const std::string& path)
    {
        m_log.open(path);

        if(m_log.is_open()) {
            m_log
                << "timestamp,"
                << "event,"
                << "sender,"
                << "receiver,"
                << "est_x,"
                << "est_y,"
                << "distance,"
                << "delay,"
                << "snr,"
                << "localization_mode,"
                << "channel_mode,"
                << "drop_reason,"
                << "metric1,"
                << "metric2,"
                << "metric3"
                << "\n";
        }
    }

    void LogTx(
        double time,
        const Packet& packet,
        double distance,
        double delay = 0.0,
        double snr = 0.0,
        const std::string& localization_mode =
            "unknown",
        const std::string& channel_mode =
            "unknown",
        double metric1 = 0.0,
        double metric2 = 0.0,
        double metric3 = 0.0)
    {
        if(!m_log.is_open()) {
            return;
        }

        m_log
            << time << ","
            << "TX,"
            << packet.sender << ","
            << packet.receiver << ","
            << packet.est_x << ","
            << packet.est_y << ","
            << distance << ","
            << delay << ","
            << snr << ","
            << localization_mode << ","
            << channel_mode << ","
            << "none,"
            << metric1 << ","
            << metric2 << ","
            << metric3
            << "\n";
    }

    void LogDrop(
        double time,
        const Packet& packet,
        const std::string& reason,
        double distance = 0.0,
        double delay = 0.0,
        double snr = 0.0,
        const std::string& localization_mode =
            "unknown",
        const std::string& channel_mode =
            "unknown",
        double metric1 = 0.0,
        double metric2 = 0.0,
        double metric3 = 0.0)
    {
        if(!m_log.is_open()) {
            return;
        }

        m_log
            << time << ","
            << "DROP,"
            << packet.sender << ","
            << packet.receiver << ","
            << packet.est_x << ","
            << packet.est_y << ","
            << distance << ","
            << delay << ","
            << snr << ","
            << localization_mode << ","
            << channel_mode << ","
            << reason << ","
            << metric1 << ","
            << metric2 << ","
            << metric3
            << "\n";
    }

    void LogRx(
        double time,
        const Packet& packet,
        double distance = 0.0,
        double delay = 0.0,
        double snr = 0.0,
        const std::string& localization_mode =
            "unknown",
        const std::string& channel_mode =
            "unknown",
        double metric1 = 0.0,
        double metric2 = 0.0,
        double metric3 = 0.0)
    {
        if(!m_log.is_open()) {
            return;
        }

        m_log
            << time << ","
            << "RX,"
            << packet.sender << ","
            << packet.receiver << ","
            << packet.est_x << ","
            << packet.est_y << ","
            << distance << ","
            << delay << ","
            << snr << ","
            << localization_mode << ","
            << channel_mode << ","
            << "none,"
            << metric1 << ","
            << metric2 << ","
            << metric3
            << "\n";
    }

    void Flush() {
        if(m_log.is_open()) {
            m_log.flush();
        }
    }

private:
    std::ofstream m_log;
};