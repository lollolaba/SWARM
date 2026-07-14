#pragma once
#include <argos3/core/utility/math/vector3.h>
#include "current_field.h"
#include "drag_model.h"
#include "bouyancy.h"

class MobilityModel {
public:
    MobilityModel();

    argos::CVector3 Step(
        const argos::CVector3& position,
        const argos::CVector3& velocity,
        double dt,
        double time);

private:
    CurrentField m_current;
    DragModel m_drag;
    Bouyancy m_bouyancy;
};