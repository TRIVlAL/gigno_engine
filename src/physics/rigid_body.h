#ifndef RIGID_BODY_H
#define RIGID_BODY_H

#include "entities/rendered_entity.h"
#include "physics_material.h"
#include "collider_type.h"
#include "algorithm/geometry.h"


namespace gigno {

    struct Hull_t {
        std::vector<glm::vec3> Vertices{};
        std::vector<int> Indices{};
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
        virtual void Think(float dt) override;
        virtual void LatePhysicThink(float dt) override;

        ColliderType_t ColliderType = COLLIDER_PLANE;

        /* -------------------------------------------
         Collider properties
        --------------------------------------------*/

        float Radius{};     // For sphere or capsule
        vec3 Normal{};      // normalized plane normal.
        float Length{};     // Full-length of the capsule
        const char *CollisionModelPath = nullptr; // Uses this path to load the convex hull 
                                                  //  if ColliderType is set to COLLIDER_HULL
        Hull_t Hull{}; // Points of the convex hull (in local space)

        float Mass{1.0f};
        bool IsStatic{false};
        int Material = (int)MAT_PLASTIC;

        float Drag = 3.0f;
        float AngularDrag = 3.0f;

        glm::vec3 StartVelocity{};

        RigidBody *pNextRigidBody{};

        glm::vec3 PositionOffset{};

        glm::vec3 Force{};
        glm::vec3 Velocity{};

        glm::vec3 Torque{};
        glm::vec3 AngularVelocity{};
    private:
        bool hasCollider = false;

        void LoadColliderModel(const char *path);
    };

    BEGIN_KEY_TABLE(RigidBody)
        DEFINE_KEY_VALUE(float, Mass)
        DEFINE_KEY_VALUE(bool, IsStatic)
        DEFINE_KEY_VALUE(int, Material)
        DEFINE_KEY_VALUE(float, Drag)
        DEFINE_KEY_VALUE(vec3, StartVelocity)
        DEFINE_KEY_VALUE(ColliderType_t, ColliderType)
        DEFINE_KEY_VALUE(float, Radius)
        DEFINE_KEY_VALUE(vec3, Normal)
        DEFINE_KEY_VALUE(float, Length)
        DEFINE_KEY_VALUE(cstr, CollisionModelPath)

        DEFINE_KEY_VALUE(vec3, Force)
        DEFINE_KEY_VALUE(vec3, Velocity)
        DEFINE_KEY_VALUE(vec3, Torque)
        DEFINE_KEY_VALUE(vec3, AngularVelocity)
    END_KEY_TABLE

    void DrawRigidbodyCollider(RigidBody &rb);

}

#endif