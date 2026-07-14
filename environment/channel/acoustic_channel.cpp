#include "acoustic_channel.h"
#include <cmath>
#include <argos3/core/utility/math/rng.h>

#include "multipath_model.h"
#include "fading_model.h"
#include "snr_model.h"

AcousticChannel::AcousticChannel() :
    m_mode(ChannelMode::REALISTIC),
    m_speed_sound(1500.0),
    m_attenuation(0.05),
    m_range(50.0),
    m_noise_sigma(0.1) {}

ChannelResult AcousticChannel::Transmit(
    const Packet& p,
    double distance,
    double t) {

    ChannelResult res;
        
    res.distance = distance;
    res.timestamp = t;
    switch(m_mode) { // Salvataggio modalità canale
        case ChannelMode::BASIC:
            res.channel_mode = "basic";
            break;
        case ChannelMode::REALISTIC:
            res.channel_mode = "realistic";
            break;
        case ChannelMode::ADVANCED_SNR:
            res.channel_mode = "advanced_snr";
            break;
        case ChannelMode::UNDERWATER_EXTREME:
            res.channel_mode = "underwater_extreme";
            break;
    }
    static auto* rng = argos::CRandom::CreateRNG("argos");
    if(distance > m_range) {
        res.delivered = false;
        res.reason = "out_of_range";
        return res;
    }

    // propagation delay
    double delay = distance / m_speed_sound;
    res.delay = delay;

    // MODELS
    if(m_mode == ChannelMode::BASIC) {
        res.delivered = (rng->Uniform(argos::CRange<double>(0.0, 1.0)) > 0.1);
        res.reason = res.delivered ? "ok" : "loss";
        return res;
    }   

    // attenuation
    double attenuation_factor = std::exp(-m_attenuation * distance);

    // fading (MODEL)
    double fading = FadingModel::Compute(rng);

    // received power
    double rx_power = attenuation_factor * fading;

    // SNR (MODEL)
    double r = rng->Uniform(argos::CRange<argos::Real>(0.0,1.0));
    double snr = SNRModel::Compute(rx_power, m_noise_sigma,r);
    res.snr = snr;

    // packet loss
    double p_loss = 1.0 - std::exp(-distance / 25.0);
    p_loss += std::max(0.0, 0.25 - snr * 0.05);
    
    if(m_mode == ChannelMode::ADVANCED_SNR) {
        p_loss += 0.1*(1.0/(snr+0.1));
    }

    if(m_mode == ChannelMode::UNDERWATER_EXTREME) {
        p_loss += 0.2;
        delay *= 1.3; 
    }
    
    p_loss = std::min(0.95, p_loss);

    double rnd = rng->Uniform(argos::CRange<double>(0.0, 1.0));

    if(rnd < p_loss) {
        res.delivered = false;
        res.reason = "loss";
        res.delay = delay;
        return res;
    }

    // multipath + jitter
    double jitter = rng->Gaussian(0.0, 0.03);
    double multipath = MultipathModel::Compute(rng,distance);

    res.delivered = true;
    res.delay = std::max(0.0, delay + jitter + multipath);
    res.reason = "ok";
    return res;
}

void AcousticChannel::SetMode(ChannelMode mode){
    m_mode = mode;
}

void AcousticChannel::SetNoiseSigma(double noise){
    m_noise_sigma = noise;
}

void AcousticChannel::SetRange(double range){
    m_range = range;
}

void AcousticChannel::SetAttenuation(double attenuation){
    m_attenuation = attenuation;
}

void AcousticChannel::SetSpeedSound(double speed){
    m_speed_sound = speed;
}