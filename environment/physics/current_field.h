#pragma once

#include <argos3/core/utility/math/vector3.h>

class CurrentField {
public:
    CurrentField();

    argos::CVector3 GetCurrent(const argos::CVector3& position, double time) const;

private:
    double m_base_strength;
};