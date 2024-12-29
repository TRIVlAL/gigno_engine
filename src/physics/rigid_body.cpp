#include "rigid_body.h"
#include "application.h"
#include "../debug/console/convar.h"

namespace gigno {
    ENTITY_DEFINITIONS(RigidBody, RenderedEntity)

    #define DEFAULT_GRAVITY glm::vec3{0.0f, -9.81f, 0.0f}
    CONVAR(glm::vec3, phys_gravity, DEFAULT_GRAVITY, "");

    RigidBody::RigidBody()
    : RenderedEntity() {
        if(GetApp() && GetApp()->GetPhysicServer()) {
            GetApp()->GetPhysicServer()->SubscribeRigidBody(this);
        }
    }

    RigidBody::~RigidBody() {
        if(GetApp() && GetApp()->GetPhysicServer()) {
            GetApp()->GetPhysicServer()->UnsubscribeRigidBody(this);
        }
    }

    void RigidBody::AddForce(const glm::vec3 &force, const glm::vec3 &application) {
        m_Force += force;

        m_Torque += glm::cross(application, force);
    }

    void RigidBody::AddImpulse(const glm::vec3 &impulse, const glm::vec3 &application) {
        m_Velocity += impulse;

        m_RotationVelocity += glm::cross(application, impulse);
    }

    void RigidBody::AddTorque(const glm::vec3 &torque) {
        m_Torque += torque;
    }

    void RigidBody::AddRotationImpulse(const glm::vec3 &rotation) {
        m_RotationVelocity += rotation;
    }

    void RigidBody::Init() {
        RenderedEntity::Init();
        m_Velocity += StartVelocity;
    }

    void RigidBody::LatePhysicThink(float dt) {
        if(IsStatic) {
            return;
        }

        //Gravity
        AddForce((glm::vec3)convar_phys_gravity * Mass);

        //Drag
        // Uses a linear approximation as can be seen in Unity or Godot.
        float ldrag = glm::clamp(1.0f - (Drag * dt), 0.0f, 1.0f);
        m_Velocity *= ldrag;
        float rdrag = glm::clamp(1.0f - (AngularDrag * dt), 0.0f, 1.0f);
        m_RotationVelocity *= rdrag;

        glm::vec3 avrg_vel = m_Velocity;
        m_Velocity += dt * m_Force / Mass;
        avrg_vel += m_Velocity;
        avrg_vel *= 0.5f;
        Position += dt * avrg_vel;
        
        Position += PositionOffset;
        PositionOffset = glm::vec3{0.0f};

        glm::vec3 avrg_rot_vel = m_RotationVelocity;
        m_RotationVelocity += dt * m_Torque / Mass;
        avrg_rot_vel += m_RotationVelocity;
        avrg_rot_vel *= 0.5f;
        Rotation += dt * avrg_rot_vel;

        Rotation.x = glm::mod<float>(Rotation.x, glm::pi<float>()*2);
        Rotation.y = glm::mod<float>(Rotation.y, glm::pi<float>()*2);
        Rotation.z = glm::mod<float>(Rotation.z, glm::pi<float>()*2);

        m_Force = glm::vec3{0.0f};
        m_Torque = glm::vec3{0.0f};
    }
}