#ifndef COLLISION_H
#define COLLISION_H

#include <glm/glm.hpp>
#include "rigid_body.h"

namespace gigno {

    /*
    Checks collision of the rigidbodies (by dispatching according to the ColliderType Keyvalue)
    Responds to the collision if they are colliding (add collision impulses and friction)
    Returns whether or not the two bodies did collide. 
    */
    bool ResolveCollision(RigidBody &rb1, RigidBody &rb2);

    bool ResolveCollision_SphereSphere(RigidBody &rb1, RigidBody &rb2);
    bool ResolveCollision_SpherePlane(RigidBody &sphere, RigidBody &plane);
    bool ResolveCollision_SphereCapsule(RigidBody &sphere, RigidBody &capsule);
    bool ResolveCollision_PlaneCapsule(RigidBody &plane, RigidBody &capsule);
    bool ResolveCollision_CapsuleCapsule(RigidBody &rb1, RigidBody &rb2);

    /*
    colNormal : unit vector from rb1 to rb2
    colDepth : NEGATIVE number representing how deep inside objects are.
    rb1ApplyPoint : Position where the collision is applied, relative to rb1's center of rotation (i.e. world position)
    */
    void RespondCollision(RigidBody &rb1, RigidBody &rb2, const glm::vec3 &colNormal, const float &colDepth, 
                        const glm::vec3 &rb1ApplyPoint, const glm::vec3 &rb2ApplyPoint);

    /*
    applyPoint : Position where the collision is appening, relative to the rigidbody's center of rotation (i.e. world position).
                It is thus where the frictional force is applied.
    */
    void ApplyFriction(float normalImpulse, RigidBody &rb, const glm::vec3 &surfaceNormal, const glm::vec3 &applyPoint, float frictionCoefficient);
}

#endif