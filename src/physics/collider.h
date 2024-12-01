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
            float Radius; // Sphere

            glm::vec3 Normal; //Plane
        } parameters;

        RigidBody *boundRigidBody{}; // The rigidbody to which apply the forces when resolving collisions.
    };

    

    void ResolveCollision(Collider &col1, Collider &col2);

    void ResolveCollision_SphereSphere(Collider &col1, Collider &col2);
    void ResolveCollision_SpherePlane(Collider &col1, Collider &col2);

}

#endif