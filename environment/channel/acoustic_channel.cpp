#include "acoustic_channel.h"

#include <algorithm>
#include <cmath>

#include "fading_model.h"
#include "multipath_model.h"
#include "snr_model.h"

namespace {

double LogisticSuccessProbability(
    double snr_margin_db,
    double transition_width_db)
{
    const double safe_width =
        std::max(
            transition_width_db,
            1e-9);

    const double normalized =
        snr_margin_db /
        safe_width;

    if(normalized >= 60.0) {
        return 1.0;
    }

    if(normalized <= -60.0) {
        return 0.0;
    }

    return
        1.0 /
        (1.0 + std::exp(-normalized));
}

}

AcousticChannel::AcousticChannel() = default;

std::string AcousticChannel::GetModeString() const {
    switch(m_mode) {
        case ChannelMode::BASIC:
            return "basic";

        case ChannelMode::REALISTIC:
            return "realistic";

        case ChannelMode::ADVANCED_SNR:
            return "advanced_snr";

        case ChannelMode::UNDERWATER_EXTREME:
            return "underwater_extreme";

        case ChannelMode::THORP:
            return "thorp";
    }

    return "unknown";
}

double AcousticChannel::ComputeThorpAbsorptionDbPerKm(
    double frequency_khz)
{
    const double frequency =
        std::max(
            frequency_khz,
            0.0);

    const double frequency_squared =
        frequency *
        frequency;

    if(frequency >= 0.4) {
        return
            0.11 *
                frequency_squared /
                (1.0 + frequency_squared) +

            44.0 *
                frequency_squared /
                (4100.0 + frequency_squared) +

            2.75e-4 *
                frequency_squared +

            0.003;
    }

    return
        0.002 +

        0.11 *
            frequency_squared /
            (1.0 + frequency_squared) +

        0.011 *
            frequency_squared;
}

double AcousticChannel::ComputeTransmissionLossDb(
    double distance_m) const
{
    // Spreading is referenced to 1 metre.
    const double safe_distance_m =
        std::max(
            distance_m,
            1.0);

    const double spreading_loss_db =
        10.0 *
        m_spreading_factor *
        std::log10(
            safe_distance_m);

    const double absorption_loss_db =
        ComputeThorpAbsorptionDbPerKm(
            m_frequency_khz) *
        (safe_distance_m / 1000.0);

    return
        spreading_loss_db +
        absorption_loss_db;
}

double AcousticChannel::ComputeTransmissionTime(
    const Packet& packet) const
{
    /*
     * Current packets store estimate fields directly,
     * therefore a fixed modem/protocol overhead is
     * included even when payload is empty.
     */
    const std::size_t total_bytes =
        packet.payload.size() +
        m_packet_overhead_bytes;

    const double total_bits =
        static_cast<double>(
            total_bytes) *
        8.0;

    return
        total_bits /
        std::max(
            m_bitrate_bps,
            1e-9);
}

ChannelResult AcousticChannel::Transmit(
    const Packet& packet,
    double distance,
    double sim_time)
{
    static auto* rng =
        argos::CRandom::CreateRNG(
            "argos");

    if(m_mode == ChannelMode::THORP) {
        return TransmitThorp(
            packet,
            distance,
            sim_time,
            rng);
    }

    return TransmitLegacy(
        packet,
        distance,
        sim_time,
        rng);
}

ChannelResult AcousticChannel::TransmitLegacy(
    const Packet& packet,
    double distance,
    double sim_time,
    argos::CRandom::CRNG* rng) const
{
    ChannelResult result;

    result.distance = distance;
    result.timestamp = sim_time;
    result.channel_mode =
        GetModeString();

    const double propagation_delay =
        distance /
        std::max(
            m_speed_sound,
            1e-9);

    const double transmission_time =
        ComputeTransmissionTime(
            packet);

    /*
     * Existing generic metrics are now:
     *
     * metric1 = transmission time
     * metric2 = propagation delay
     * metric3 = transmission loss
     */
    result.metric1 =
        transmission_time;

    result.metric2 =
        propagation_delay;

    // Legacy modes do not use Thorp loss.
    result.metric3 = 0.0;

    result.delay =
        propagation_delay +
        transmission_time;

    if(distance > m_range) {
        result.delivered = false;
        result.reason = "out_of_range";

        return result;
    }

    /*
     * Existing BASIC delivery behaviour is
     * deliberately preserved.
     */
    if(m_mode == ChannelMode::BASIC) {
        result.delivered =
            rng->Uniform(
                argos::CRange<double>(
                    0.0,
                    1.0)) >
            0.1;

        result.reason =
            result.delivered
            ? "ok"
            : "loss";

        return result;
    }

    /*
     * Existing phenomenological channel behaviour
     * is preserved for the legacy modes.
     */
    const double attenuation_factor =
        std::exp(
            -m_attenuation *
            distance);

    const double fading =
        FadingModel::Compute(
            rng);

    const double received_power =
        attenuation_factor *
        fading;

    const double random_value =
        rng->Uniform(
            argos::CRange<argos::Real>(
                0.0,
                1.0));

    const double snr =
        SNRModel::Compute(
            received_power,
            m_noise_sigma,
            random_value);

    result.snr = snr;

    double loss_probability =
        1.0 -
        std::exp(
            -distance / 25.0);

    loss_probability +=
        std::max(
            0.0,
            0.25 -
            snr * 0.05);

    if(m_mode ==
       ChannelMode::ADVANCED_SNR)
    {
        loss_probability +=
            0.1 *
            (1.0 /
             (snr + 0.1));
    }

    double adjusted_propagation_delay =
        propagation_delay;

    if(m_mode ==
       ChannelMode::UNDERWATER_EXTREME)
    {
        loss_probability += 0.2;

        adjusted_propagation_delay *=
            1.3;
    }

    loss_probability =
        std::min(
            0.95,
            loss_probability);

    const double draw =
        rng->Uniform(
            argos::CRange<double>(
                0.0,
                1.0));

    if(draw < loss_probability) {
        result.delivered = false;
        result.reason = "loss";

        result.delay =
            adjusted_propagation_delay +
            transmission_time;

        return result;
    }

    const double jitter =
        rng->Gaussian(
            0.0,
            0.03);

    const double multipath =
        MultipathModel::Compute(
            rng,
            distance);

    result.delivered = true;

    result.delay =
        std::max(
            0.0,
            adjusted_propagation_delay +
            transmission_time +
            jitter +
            multipath);

    result.reason = "ok";

    return result;
}

ChannelResult AcousticChannel::TransmitThorp(
    const Packet& packet,
    double distance,
    double sim_time,
    argos::CRandom::CRNG* rng) const
{
    ChannelResult result;

    result.distance = distance;
    result.timestamp = sim_time;
    result.channel_mode =
        GetModeString();

    const double propagation_delay =
        distance /
        std::max(
            m_speed_sound,
            1e-9);

    const double transmission_time =
        ComputeTransmissionTime(
            packet);

    const double transmission_loss_db =
        ComputeTransmissionLossDb(
            distance);

    result.metric1 =
        transmission_time;

    result.metric2 =
        propagation_delay;

    result.metric3 =
        transmission_loss_db;

    result.delay =
        propagation_delay +
        transmission_time;

    if(distance > m_range) {
        result.delivered = false;
        result.reason = "out_of_range";

        return result;
    }

    const double fading_db =
        (rng &&
         m_fading_sigma_db > 0.0)
        ? rng->Gaussian(
            0.0,
            m_fading_sigma_db)
        : 0.0;

    /*
     * Simplified link budget:
     *
     * SNR = source level
     *       - transmission loss
     *       - ambient noise
     *       + fading
     */
    const double snr_db =
        m_source_level_db -
        transmission_loss_db -
        m_ambient_noise_db +
        fading_db;

    result.snr = snr_db;

    /*
     * Smooth modem-threshold abstraction.
     * This avoids a discontinuous perfect/fail threshold.
     */
    const double success_probability =
        LogisticSuccessProbability(
            snr_db -
                m_snr_threshold_db,
            m_snr_transition_db);

    const double draw =
        rng
        ? rng->Uniform(
            argos::CRange<double>(
                0.0,
                1.0))
        : 0.0;

    result.delivered =
        draw <=
        success_probability;

    result.reason =
        result.delivered
        ? "ok"
        : "snr_loss";

    return result;
}

void AcousticChannel::SetMode(
    ChannelMode mode)
{
    m_mode = mode;
}

void AcousticChannel::SetNoiseSigma(
    double noise)
{
    m_noise_sigma =
        std::max(
            0.0,
            noise);
}

void AcousticChannel::SetRange(
    double range)
{
    m_range =
        std::max(
            0.0,
            range);
}

void AcousticChannel::SetAttenuation(
    double attenuation)
{
    m_attenuation =
        std::max(
            0.0,
            attenuation);
}

void AcousticChannel::SetSpeedSound(
    double speed)
{
    m_speed_sound =
        std::max(
            speed,
            1e-9);
}

void AcousticChannel::SetFrequencyKHz(
    double frequency_khz)
{
    m_frequency_khz =
        std::max(
            frequency_khz,
            0.0);
}

void AcousticChannel::SetBitrate(
    double bitrate_bps)
{
    m_bitrate_bps =
        std::max(
            bitrate_bps,
            1e-9);
}

void AcousticChannel::SetSpreadingFactor(
    double spreading_factor)
{
    m_spreading_factor =
        std::clamp(
            spreading_factor,
            1.0,
            2.0);
}

void AcousticChannel::SetSourceLevelDb(
    double source_level_db)
{
    m_source_level_db =
        source_level_db;
}

void AcousticChannel::SetAmbientNoiseDb(
    double ambient_noise_db)
{
    m_ambient_noise_db =
        ambient_noise_db;
}

void AcousticChannel::SetSnrThresholdDb(
    double snr_threshold_db)
{
    m_snr_threshold_db =
        snr_threshold_db;
}

void AcousticChannel::SetSnrTransitionDb(
    double snr_transition_db)
{
    m_snr_transition_db =
        std::max(
            snr_transition_db,
            1e-9);
}

void AcousticChannel::SetFadingSigmaDb(
    double fading_sigma_db)
{
    m_fading_sigma_db =
        std::max(
            0.0,
            fading_sigma_db);
}

void AcousticChannel::SetPacketOverheadBytes(
    std::size_t bytes)
{
    m_packet_overhead_bytes =
        bytes;
}