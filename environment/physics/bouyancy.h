#pragma once

#include <argos3/core/utility/math/vector3.h>

class Bouyancy {
public:
    Bouyancy();

    argos::CVector3 ComputeBouyancy() const;

private:
    double m_force = 1.0;
};
///da usare per aggiornamento futuro dove non considero lambiente come 3d, computazionalmente lento e poco interessante
//ma posso proiettare la velocita sul piano xy usando a,lpha anglo tra asse z e piano xy
