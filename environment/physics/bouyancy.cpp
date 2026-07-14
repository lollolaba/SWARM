#include "bouyancy.h"

Bouyancy::Bouyancy() {}

argos::CVector3 Bouyancy::ComputeBouyancy() const {
    return argos::CVector3(0, 0, m_force);
}