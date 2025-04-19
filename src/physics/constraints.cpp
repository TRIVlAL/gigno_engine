#include "constraints.h"
#include "rigid_body.h"
#include "../debug/console/convar.h"

namespace gigno {

    CONVAR(float, phys_constraint_bias_factor, 0.01f, "");

    void RopeConstraint::Solve(float dt) {

        const glm::vec3 velocity = m_Body->Velocity;

        const glm::vec3 delta = m_Body->Position - m_Point;
        const float delta_mag = glm::length(delta);
        const glm::vec3 delta_dir = delta / delta_mag;

        const float offset = delta_mag - m_Distance;
        
        if(offset <= 0.0f) {
            return;
        }
        
        const glm::vec3 jacobian = -delta_dir;
        
        const float effective_mass = m_Body->Mass;

        const float bias =  -((float)convar_phys_constraint_bias_factor / dt) * offset;

        const float lambda = -effective_mass * (glm::dot(jacobian, velocity) + bias);

        m_Body->AddImpulse(jacobian * lambda);
    }

}
