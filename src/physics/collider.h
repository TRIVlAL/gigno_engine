#ifndef COLLIDER_H
#define COLLIDER_H

#include <glm/glm.hpp>

namespace gigno {

    class RigidBody;

    enum ColliderType_t {
        COLLIDER_NONE = 0,
        COLLIDER_SPHERE,
        COLLIDER_PLANE,
        COLLIDER_MAX_ENUM
    };

    struct Collider {
        ColliderType_t type = COLLIDER_NONE;

        union ColliderParameter {
            // Sphere
            float Radius;

            // Plane
            glm::vec3 Normal;
        } parameters;

        // Values to poll from the rigid body
        void PollRigidBodyValues();
        glm::vec3 Position;
        glm::vec3 Rotation;
        glm::vec3 Velocity;
        float Mass;
        float Bounciness;
        bool IsStatic;

        // Impulsion to be applied in the end.
        void ApplyImpulse();
        glm::vec3 Impulse;
        glm::vec3 AngularImpulse;
        glm::vec3 PosOffset;

        // To avoid memory fragmentation, values needed from the 
        // rigidbody are all polled once at the beggining
        // and impulsions are all applied in one batch.

        RigidBody *BoundRigidBody{};
    };

    

    bool ResolveCollision(Collider &col1, Collider &col2);

    bool ResolveCollision_SphereSphere(Collider &col1, Collider &col2);
    bool ResolveCollision_SpherePlane(Collider &col1, Collider &col2);

    /*
    colNormal : unit vector from col1 to col2
    colDepth : NEGATIVE number representing how deep inside objects are.
    col1ApplyPoint, col2ApplyPoint : where the collision force is apply (local space)
    */
    void RespondCollision(Collider &col1, Collider &col2, const glm::vec3 &colNormal, const float &colDepth, 
                        const glm::vec3 &col1ApplyPoint, const glm::vec3 &col2ApplyPoint);
}

#endif