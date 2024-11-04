#include "rigid_body.h"
#include "application.h"

namespace gigno {

    RigidBody::RigidBody(ModelData_t model)
    : PhysicEntity(model) {

    }

    RigidBody::~RigidBody() {

    }

    void RigidBody::PhysicThink(float dt) {
        if(TestingInterpolateType == 0) {
            m_Velocity += dt * m_Force;
            Transform.Position += dt * m_Velocity;
        } else if(TestingInterpolateType == 1) {
            glm::vec3 avrg_vel = m_Velocity;
            m_Velocity += dt * m_Force;
            avrg_vel += m_Velocity;
            avrg_vel *= 0.5f;
            Transform.Position += dt * avrg_vel;
        }
    }

}