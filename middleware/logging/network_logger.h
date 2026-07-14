#pragma once

#include <fstream>
#include <string>
#include "../../core/communication/packet.h"
#include "log_record.h"

class NetworkLogger {
public:
    void Init(const std::string& path) {
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

    void LogTx(double t,const Packet& p,double dist,double delay=0.0,double snr=0.0,const std::string& localization_mode = "unknown",const std::string& channel_mode = "unknown"){
        if(!m_log.is_open())return;
        m_log
            << t << ","
            << "TX,"
            << p.sender << ","
            << p.receiver << ","
            << p.est_x << ","
            << p.est_y << ","
            << dist << ","
            << delay << ","
            << snr << ","
            << localization_mode << ","
            << channel_mode << ","
            << "none,"
            << 0 << ","
            << 0 << ","
            << 0
            << "\n";
    }

void LogDrop(double t,const Packet& p,const std::string& reason,double dist = 0.0,double delay = 0.0,double snr = 0.0,const std::string& localization_mode = "unknown",const std::string& channel_mode = "unknown"){
        if(!m_log.is_open())
            return;
        m_log
            << t << ","
            << "DROP,"
            << p.sender << ","
            << p.receiver << ","
            << p.est_x << ","
            << p.est_y << ","
            << dist << ","
            << delay << ","
            << snr << ","
            << localization_mode << ","
            << channel_mode << ","
            << reason << ","
            << 0 << ","
            << 0 << ","
            << 0
            << "\n";
    }


 void LogRx(double t,const Packet& p,double dist = 0.0,double delay = 0.0,double snr = 0.0,const std::string& localization_mode = "unknown",const std::string& channel_mode = "unknown"){
        if(!m_log.is_open()) return;
        m_log
            << t << ","
            << "RX,"
            << p.sender << ","
            << p.receiver << ","
            << p.est_x << ","
            << p.est_y << ","
            << dist << ","
            << delay << ","
            << snr << ","
            << localization_mode << ","
            << channel_mode << ","
            << "none,"
            << 0 << ","
            << 0 << ","
            << 0
            << "\n";
}


private:
    std::ofstream m_log;
};