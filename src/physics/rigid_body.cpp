#include "rigid_body.h"
#include "application.h"

namespace gigno {

    RigidBody::RigidBody(ModelData_t model)
    : RenderedEntity(model) {

    }

    RigidBody::~RigidBody() {

    }

    void RigidBody::AddForce(const glm::vec3& force, const glm::vec3& application) {
        m_Force += force;

        m_Torque += glm::cross(application, force);
    }

    void RigidBody::LatePhysicThink(float dt) {
        glm::vec3 avrg_vel = m_Velocity;
        m_Velocity += dt * m_Force / Mass;
        avrg_vel += m_Velocity;
        avrg_vel *= 0.5f;
        Transform.Position += dt * avrg_vel;

        glm::vec3 avrg_rot_vel = m_RotationVelocity;
        m_RotationVelocity += dt * m_Torque / Mass;
        avrg_rot_vel += m_RotationVelocity;
        avrg_rot_vel *= 0.5f;
        Transform.Rotation += dt * avrg_rot_vel;

        Transform.Rotation.x = glm::mod<float>(Transform.Rotation.x, glm::pi<float>()*2);
        Transform.Rotation.y = glm::mod<float>(Transform.Rotation.y, glm::pi<float>()*2);
        Transform.Rotation.z = glm::mod<float>(Transform.Rotation.z, glm::pi<float>()*2);

        m_Force = glm::vec3{0.0f};
        m_Torque = glm::vec3{0.0f};
    }

}