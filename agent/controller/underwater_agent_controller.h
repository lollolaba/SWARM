#pragma once

#include <argos3/core/control_interface/ci_controller.h>
#include <argos3/core/utility/datatypes/datatypes.h>

#include <cstddef>

class CUnderwaterAgentController :
    public argos::CCI_Controller
{
public:
    void Init(
        argos::TConfigurationNode& t_node) override;

    void ControlStep() override;

    void Reset() override;

private:
    argos::UInt32 m_broadcast_period_ticks = 10;
    argos::UInt32 m_last_broadcast_tick = 0;

    bool m_has_broadcast = false;

    std::size_t m_next_neighbor_index = 0;
};