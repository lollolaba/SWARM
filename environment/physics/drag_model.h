#pragma once
#include <argos3/core/utility/math/vector3.h>

class DragModel {
public:
    DragModel();
    argos::CVector3 ComputeDrag(const argos::CVector3& velocity ) const;
private:
    double m_drag_xy;
    double m_drag_z;
};
