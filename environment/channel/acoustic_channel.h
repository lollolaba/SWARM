#pragma once

#include <vector>
#include <string>

#include "channel_result.h"
#include "../../core/communication/packet.h"
#include <argos3/core/utility/math/rng.h>

enum class ChannelMode {
    BASIC,
    REALISTIC,
    ADVANCED_SNR,
    UNDERWATER_EXTREME
};

class AcousticChannel {
public:
    AcousticChannel();
    void SetMode(ChannelMode mode);
    void SetNoiseSigma(double noise);
    void SetRange(double range);
    void SetAttenuation(double attenuation);
    void SetSpeedSound(double speed_sound);
    ChannelMode GetMode() const {
        return m_mode;
    }
    std::string GetModeString() const {
        switch(m_mode) {
            case ChannelMode::BASIC: return "basic";
            case ChannelMode::REALISTIC: return "realistic";
            case ChannelMode::ADVANCED_SNR: return "advanced_snr";
            case ChannelMode::UNDERWATER_EXTREME: return "underwater_extreme";
        }
        return "unknown";
    }
    ChannelResult Transmit(const Packet& packet,
                           double distance,
                           double sim_time);

private:
    ChannelMode m_mode;
    double m_speed_sound = 1500.0;   // underwater m/s
    double m_attenuation = 0.02;     // absorption base
    double m_noise_sigma = 0.1;      // noise
    double m_range = 50.0;
};