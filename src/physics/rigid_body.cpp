#include "rigid_body.h"
#include "application.h"
#include "../debug/console/convar.h"

namespace gigno {
    #define DEFAULT_GRAVITY glm::vec3{0.0f, -9.81f, 0.0f}
    CONVAR(glm::vec3, phys_gravity, DEFAULT_GRAVITY, "");

    RigidBody::RigidBody(ModelData_t model)
    : RenderedEntity(model) {
    }

    RigidBody::~RigidBody() {

    }

    void RigidBody::GiveSphereCollider(float radius) {
        if(hasCollider) {
            GetApp()->GetPhysicServer()->RemoveCollider(this);
        }
        Collider::ColliderParameter param;
        param.Radius = radius;
        GetApp()->GetPhysicServer()->AddCollider(this, COLLIDER_SPHERE, param);
        hasCollider = true;
    }

    void RigidBody::GivePlaneCollider(glm::vec3 normal) {
        if(hasCollider) {
            GetApp()->GetPhysicServer()->RemoveCollider(this);
        }
        Collider::ColliderParameter param;
        param.Normal = normal;
        GetApp()->GetPhysicServer()->AddCollider(this, COLLIDER_PLANE, param);
    }

    void RigidBody::AddForce(const glm::vec3 &force, const glm::vec3 &application) {
        m_Force += force;

        m_Torque += glm::cross(application, force);
    }

    void RigidBody::AddImpulse(const glm::vec3 &impulse, const glm::vec3 &application) {
        m_Velocity += impulse;

        m_RotationVelocity += glm::cross(application, impulse);
    }

    void RigidBody::AddRotationImpulse(const glm::vec3 &rotation) {
        m_RotationVelocity += rotation;
    }

    void RigidBody::LatePhysicThink(float dt) {
        if(IsStaitc) {
            return;
        }

        AddForce((glm::vec3)convar_phys_gravity);

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