#include "constraints.h"
#include "rigid_body.h"
#include "../debug/console/convar.h"

namespace gigno {

    CONVAR(float, phys_constraint_bias_factor, 0.05f, "");

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

    void HingeConstraint::Solve(float dt) {
        //Constraint position
        {
            const glm::vec3 velocity = m_Body->Velocity;

            const glm::vec3 delta = m_Body->Position - m_Point;
            const float delta_mag = glm::length(delta);
            const glm::vec3 delta_dir = delta / delta_mag;

            const float offset = delta_mag;

            if (offset == 0.0f) {
                return;
            }

            const glm::vec3 jacobian = glm::normalize(-delta_dir);

            const float effective_mass = m_Body->Mass;

            const float bias = -((float)convar_phys_constraint_bias_factor / dt) * offset;

            const float lambda = -effective_mass * (glm::dot(jacobian, velocity) + bias);

            m_Body->AddImpulse(jacobian * lambda);
        }

        //Constraint rotation
        {
            const glm::vec3 ang_vel = m_Body->AngularVelocity;
    
            const float total = glm::length(ang_vel);
            const float along_axis = glm::dot(ang_vel, m_Axis);

            const glm::vec3 dir = ApplyRotation(m_Body->Rotation, m_Axis);
            const float angle_offset = glm::acos(glm::clamp(glm::dot(dir, m_Axis), -1.0f, 1.0f));
    
            const float offset = angle_offset;

            Console::LogInfo("angle_offset = %f", angle_offset);
    
            if (offset == 0.0f) {
                return;
            }
    
            const glm::vec3 jacobian = -glm::normalize(glm::cross(dir, m_Axis));
    
            const mat3 effective_mass = m_Body->Mass * m_Body->InertiaTensor;
    
            const float bias = ((float)convar_phys_constraint_bias_factor / dt) * offset;
    
            const mat3 lambda = -effective_mass * (bias);
    
            m_Body->AddRotationImpulse(jacobian * lambda);

            m_Body->AddRotationImpulse(-(ang_vel - m_Axis * along_axis) * effective_mass);
        }
    }

}
