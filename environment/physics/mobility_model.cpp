#include "mobility_model.h"

MobilityModel::MobilityModel() {}

argos::CVector3 MobilityModel::Step(
    const argos::CVector3& pos,
    const argos::CVector3& vel,
    double dt,
    double time) {

    argos::CVector3 current = m_current.GetCurrent(pos, time);
    argos::CVector3 drag = m_drag.ComputeDrag(vel);

    argos::CVector3 accel = current + drag + m_bouyancy.ComputeBouyancy();

    return vel + accel * dt;
}