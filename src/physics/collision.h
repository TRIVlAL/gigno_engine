#ifndef COLLISION_H
#define COLLISION_H

#include <glm/glm.hpp>
#include "collider_type.h"

namespace gigno {
    struct CollisionModel_t;
    class RigidBody;

    struct Collider_t {
        ColliderType_t ColliderType;

        glm::vec3 Position{};
        glm::vec3 Rotation{};

        float Radius{}; // COLLIDER_SPHERE / COLLIDER_CAPSULE

        float Length{}; // COLLIDER_CAPSULE

        glm::vec3 Normal{}; // COLLIDER_PLANE

        const CollisionModel_t *Model{};  // COLLIDER_HULL
        std::vector<glm::vec3> TransformedModel{};  // COLLIDER_HULL
    };

    struct CollisionData_t {
        bool Collision;     // Do the colliders overlap

        /*
        ------------------ The Following are set IF Collision == true ------------------------------
        */
       
       glm::vec3 Normal;   // Best direction to separate the objects (normalized from A to B)
       float Depth;        // Distance required to separate the objects (negative)
       glm::vec3 ApplyPointA; // Position where the collision is appening on A (object A local space)
       glm::vec3 ApplyPointB; // Position where the collision is appening on B (object B local space)

       /*
       Object A is the object with the smallest ColliderType, or rb1 when calling DetectCollision
       if both ColliderType are the same.
       */
    };

    CollisionData_t DetectCollision(const Collider_t &col1, const Collider_t &col2);

    CollisionData_t DetectCollision_SphereSphere(const Collider_t &col1, const Collider_t &col2);
    CollisionData_t DetectCollision_SpherePlane(const Collider_t &sphere, const Collider_t &plane);
    CollisionData_t DetectCollision_SphereCapsule(const Collider_t &sphere, const Collider_t &capsule);
    CollisionData_t DetectCollision_PlaneCapsule(const Collider_t &plane, const Collider_t &capsule);
    CollisionData_t DetectCollision_CapsuleCapsule(const Collider_t &col1, const Collider_t &col2);
    CollisionData_t DetectCollision_HullNonPlane(const Collider_t &hull, const Collider_t &nonPlane);
    CollisionData_t DetectCollision_HullPlane(const Collider_t &hull, const Collider_t &Plane);

    /*
    rb1         : A Collider COLLIDING with rb2
    rb2         : A Collider COLLIDING with rb1
    collision   : The Collision Data describing the rb1-rb2 collision

    rb1 and rb2 MUST CORRESPOND TO the col1 and col2 used when calling DetectCollision to query the Collision Data.
    */
    void RespondCollision(RigidBody &rb1, RigidBody &rb2, const CollisionData_t &collision );

    void ApplyFriction(float normalImpulse, RigidBody &rb, const glm::vec3 &surfaceNormal, 
                        const glm::vec3 &applyPoint, float frictionCoefficient, float staticFrictionCoefficient);

    float GetBounciness(RigidBody &rb1, RigidBody &rb2);
    float GetFriction(RigidBody &rb1, RigidBody &rb2);
    float GetStaticFriction(RigidBody &rb1, RigidBody &rb2);
}

#endif