#include "mac_layer.h"

#include <algorithm>
#include <cmath>

void MACLayer::ConfigureTDMA(
    double transmission_time,
    double propagation_guard,
    double simulation_timestep)
{
    m_tdma_transmission_time =
        std::max(
            transmission_time,
            1e-9);

    m_tdma_guard_interval =
        std::max(
            propagation_guard,
            0.0);

    m_tdma_timestep =
        std::max(
            simulation_timestep,
            1e-9);
}

void MACLayer::SetTDMASenders(
    const std::vector<std::string>& senders)
{
    std::vector<std::string> ordered =
        senders;

    std::sort(
        ordered.begin(),
        ordered.end());

    ordered.erase(
        std::unique(
            ordered.begin(),
            ordered.end()),
        ordered.end());

    if(ordered == m_tdma_senders) {
        return;
    }

    m_tdma_senders = ordered;

    m_tdma_slot_by_sender.clear();
    m_last_tdma_frame.clear();

    for(std::size_t index = 0;
        index < m_tdma_senders.size();
        ++index)
    {
        m_tdma_slot_by_sender[
            m_tdma_senders[index]] =
            index;
    }
}

double MACLayer::GetTDMATransmissionTime() const {
    return m_tdma_transmission_time;
}

double MACLayer::GetTDMAGuardInterval() const {
    return m_tdma_guard_interval;
}

double MACLayer::GetTDMASlotDuration() const {
    return
        m_tdma_transmission_time +
        m_tdma_guard_interval +
        m_tdma_timestep;
}

double MACLayer::GetTDMAFrameDuration() const {
    return
        GetTDMASlotDuration() *
        static_cast<double>(
            m_tdma_senders.size());
}

bool MACLayer::CanTransmitTDMA(
    const std::string& sender,
    double sim_time)
{
    const auto slot_it =
        m_tdma_slot_by_sender.find(
            sender);

    if(slot_it ==
           m_tdma_slot_by_sender.end() ||
       m_tdma_senders.empty() ||
       sim_time < 0.0)
    {
        return false;
    }

    const double slot_duration =
        GetTDMASlotDuration();

    const double frame_duration =
        GetTDMAFrameDuration();

    if(slot_duration <= 0.0 ||
       frame_duration <= 0.0)
    {
        return false;
    }

    const std::uint64_t frame_index =
        static_cast<std::uint64_t>(
            std::floor(
                sim_time /
                frame_duration));

    const double slot_start =
        static_cast<double>(
            frame_index) *
            frame_duration +

        static_cast<double>(
            slot_it->second) *
            slot_duration;

    const double elapsed =
        sim_time -
        slot_start;

    const double epsilon = 1e-9;

    /*
     * ARGoS advances in discrete ticks.
     *
     * Therefore, the first tick after the exact
     * mathematical slot boundary is accepted.
     */
    if(elapsed < -epsilon ||
       elapsed >
           m_tdma_timestep + epsilon)
    {
        return false;
    }

    const auto previous_it =
        m_last_tdma_frame.find(
            sender);

    if(previous_it !=
           m_last_tdma_frame.end() &&
       previous_it->second ==
           frame_index)
    {
        return false;
    }

    return true;
}

void MACLayer::OnTransmitTDMA(
    const std::string& sender,
    double sim_time)
{
    const double frame_duration =
        GetTDMAFrameDuration();

    if(frame_duration <= 0.0 ||
       sim_time < 0.0)
    {
        return;
    }

    const std::uint64_t frame_index =
        static_cast<std::uint64_t>(
            std::floor(
                sim_time /
                frame_duration));

    m_last_tdma_frame[sender] =
        frame_index;

    m_last_tx_time[sender] =
        sim_time;
}