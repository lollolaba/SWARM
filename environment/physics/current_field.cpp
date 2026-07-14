#include "current_field.h"
#include <cmath>

CurrentField::CurrentField() :
    m_base_strength(0.05) {}

argos::CVector3 CurrentField::GetCurrent(
    const argos::CVector3& pos,
    double t) const {

    // corrente spaziale + temporale (oscillante)
    double cx = m_base_strength * std::sin(0.2 * t + pos.GetY());
    double cy = m_base_strength * std::cos(0.15 * t + pos.GetX());

    return argos::CVector3(cx, cy, 0.0);
}