#ifndef RIGID_BODY_H
#define RIGID_BODY_H

#include "entities/rendered_entity.h"
#include "physics_material.h"


namespace gigno {

    enum ColliderType_t {
        COLLIDER_NONE = 0,
        COLLIDER_SPHERE = 1,
        COLLIDER_PLANE = 2,
        COLLIDER_CAPSULE = 3,
        COLLIDER_MAX_ENUM = 4
    };

    class RigidBody : public RenderedEntity {
        ENTITY_DECLARATIONS(RigidBody, RenderedEntity)
    public:
        RigidBody();
        ~RigidBody();

        void AddForce(const glm::vec3 &force, const glm::vec3 &application = glm::vec3{0.0f, 0.0f, 0.0f});
        void AddImpulse(const glm::vec3 &impulse, const glm::vec3 &application = glm::vec3{0.0f, 0.0f, 0.0f});
        void AddTorque(const glm::vec3 &torque);
        void AddRotationImpulse(const glm::vec3 &rotation);

        virtual void Init() override;

        virtual void LatePhysicThink(float dt) override;

        glm::vec3 GetVelocity() { return m_Velocity; };
        glm::vec3 GetForce() { return m_Force; };

        int ColliderType = (int)COLLIDER_PLANE;
        // Collider properties
        float Radius{}; //Sphere / Capsule
        vec3 Normal{}; // PLane
        float Length{}; //Capsule

        float Mass{1.0f};
        bool IsStatic{false};
        int Material = (int)MAT_PLASTIC;

        glm::vec3 StartVelocity{};

        RigidBody *pNextRigidBody{};

        glm::vec3 PositionOffset{};
    private:
        bool hasCollider = false;

        glm::vec3 m_Force{};
        glm::vec3 m_Velocity{};

        glm::vec3 m_Torque{};
        glm::vec3 m_RotationVelocity{};
    };

    BEGIN_KEY_TABLE(RigidBody)
        DEFINE_KEY_VALUE(float, Mass)
        DEFINE_KEY_VALUE(bool, IsStatic)
        DEFINE_KEY_VALUE(int, Material)
        DEFINE_KEY_VALUE(vec3, StartVelocity)
        DEFINE_KEY_VALUE(int, ColliderType)
        DEFINE_KEY_VALUE(float, Radius)
        DEFINE_KEY_VALUE(vec3, Normal)
        DEFINE_KEY_VALUE(float, Length)
    END_KEY_TABLE

}

#endif