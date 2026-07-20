#pragma once

#include <cstddef>
#include <string>

#include "channel_result.h"
#include "../../core/communication/packet.h"

#include <argos3/core/utility/math/rng.h>

enum class ChannelMode {
    BASIC,
    REALISTIC,
    ADVANCED_SNR,
    UNDERWATER_EXTREME,
    THORP
};

class AcousticChannel {
public:
    AcousticChannel();

    void SetMode(ChannelMode mode);

    void SetNoiseSigma(double noise);
    void SetRange(double range);
    void SetAttenuation(double attenuation);
    void SetSpeedSound(double speed_sound);

    void SetFrequencyKHz(
        double frequency_khz);

    void SetBitrate(
        double bitrate_bps);

    void SetSpreadingFactor(
        double spreading_factor);

    void SetSourceLevelDb(
        double source_level_db);

    void SetAmbientNoiseDb(
        double ambient_noise_db);

    void SetSnrThresholdDb(
        double snr_threshold_db);

    void SetSnrTransitionDb(
        double snr_transition_db);

    void SetFadingSigmaDb(
        double fading_sigma_db);

    void SetPacketOverheadBytes(
        std::size_t bytes);

    ChannelMode GetMode() const {
        return m_mode;
    }

    std::string GetModeString() const;

    static double ComputeThorpAbsorptionDbPerKm(
        double frequency_khz);

    double ComputeTransmissionLossDb(
        double distance_m) const;

    double ComputeTransmissionTime(
        const Packet& packet) const;

    ChannelResult Transmit(
        const Packet& packet,
        double distance,
        double sim_time);

private:
    ChannelResult TransmitLegacy(
        const Packet& packet,
        double distance,
        double sim_time,
        argos::CRandom::CRNG* rng) const;

    ChannelResult TransmitThorp(
        const Packet& packet,
        double distance,
        double sim_time,
        argos::CRandom::CRNG* rng) const;

    ChannelMode m_mode =
        ChannelMode::REALISTIC;

    // Existing legacy-model parameters
    double m_speed_sound = 1500.0;
    double m_attenuation = 0.05;
    double m_noise_sigma = 0.1;
    double m_range = 50.0;

    // Physical THORP-model parameters
    double m_frequency_khz = 25.0;
    double m_bitrate_bps = 1000.0;
    double m_spreading_factor = 1.5;

    double m_source_level_db = 150.0;
    double m_ambient_noise_db = 90.0;

    double m_snr_threshold_db = 10.0;
    double m_snr_transition_db = 2.0;
    double m_fading_sigma_db = 1.5;

    std::size_t m_packet_overhead_bytes = 32;
};