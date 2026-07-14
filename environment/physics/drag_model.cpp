#include "drag_model.h"

DragModel::DragModel() :
    m_drag_xy(0.8),
    m_drag_z(1.5) {}

argos::CVector3 DragModel::ComputeDrag(
    const argos::CVector3& v) const {

    return argos::CVector3(
        -m_drag_xy * v.GetX(),
        -m_drag_xy * v.GetY(),
        -m_drag_z  * v.GetZ()
    );
}