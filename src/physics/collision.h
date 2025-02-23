#ifndef COLLISION_H
#define COLLISION_H

#include <glm/glm.hpp>
#include "rigid_body.h"

namespace gigno {

    struct CollisionData_t {
        bool Collision;     // Do the objects overlap

        /*
        The Following are set IF Collision == true

        Object A is the object with the smallest ColliderType, or rb1 when calling DetectCollision
        if both ColliderType are the same.
        */

        glm::vec3 Normal;   // Best direction to separate the objects (normalized from A to B)
        float Depth;        // Distance required to separate the objects (negative)
        glm::vec3 ApplyPointA; // Position where the collision is appening on A (object A local space)
        glm::vec3 ApplyPointB; // Position where the collision is appening on B (object B local space)
    };

    CollisionData_t DetectCollision(RigidBody &rb1, RigidBody &rb2);

    CollisionData_t DetectCollision_SphereSphere(RigidBody &rb1, RigidBody &rb2);
    CollisionData_t DetectCollision_SpherePlane(RigidBody &sphere, RigidBody &plane);
    CollisionData_t DetectCollision_SphereCapsule(RigidBody &sphere, RigidBody &capsule);
    CollisionData_t DetectCollision_PlaneCapsule(RigidBody &plane, RigidBody &capsule);
    CollisionData_t DetectCollision_CapsuleCapsule(RigidBody &rb1, RigidBody &rb2);
    CollisionData_t DetectCollision_HullNonPlane(RigidBody &hull, RigidBody &nonPlane);
    CollisionData_t DetectCollision_HullPlane(RigidBody &hull, RigidBody &Plane);

    /*
    rb1         : A RigidBody COLLIDING with rb2
    rb2         : A RigidBody COLLIDING with rb1
    collision   : The Collision Data describing the rb1-rb2 collision

    rb1 and rb2 MUST CORRESPOND TO the rb1 and rb2 used when calling DetectCollision to query the Collision Data.
    */
    void RespondCollision(RigidBody &rb1, RigidBody &rb2, const CollisionData_t &collision );

    void ApplyFriction(float normalImpulse, RigidBody &rb, const glm::vec3 &surfaceNormal, 
                        const glm::vec3 &applyPoint, float frictionCoefficient, float staticFrictionCoefficient);

    float GetBounciness(RigidBody &rb1, RigidBody &rb2);
    float GetFriction(RigidBody &rb1, RigidBody &rb2);
    float GetStaticFriction(RigidBody &rb1, RigidBody &rb2);
}

#endif