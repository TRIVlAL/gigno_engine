#ifndef COLLISION_H
#define COLLISION_H

#include <glm/glm.hpp>
#include "rigid_body.h"

namespace gigno {

    struct CapsuleParameters{
        float Radius, Length;
    };

    void ApplyDrag(RigidBody &rb);
    float GetDragCoefficient(RigidBody &rb);
    float GetAreaCrossSection(RigidBody &rb, const glm::vec3 &direction);

    bool ResolveCollision(RigidBody &rb1, RigidBody &rb2);

    bool ResolveCollision_SphereSphere(RigidBody &rb1, RigidBody &rb2);
    bool ResolveCollision_SpherePlane(RigidBody &sphere, RigidBody &plane);
    bool ResolveCollision_SphereCapsule(RigidBody &sphere, RigidBody &capsule);
    bool ResolveCollision_PlaneCapsule(RigidBody &plane, RigidBody &capsule);
    bool ResolveCollision_CapsuleCapsule(RigidBody &rb1, RigidBody &rb2);

    /*
    colNormal : unit vector from rb1 to rb2
    colDepth : NEGATIVE number representing how deep inside objects are.
    rb1ApplyPoint, rb2ApplyPoint : where the collision force is apply (local space)
    */
    void RespondCollision(RigidBody &rb1, RigidBody &rb2, const glm::vec3 &colNormal, const float &colDepth, 
                        const glm::vec3 &rb1ApplyPoint, const glm::vec3 &rb2ApplyPoint);

    void ApplyFriction(float normalImpulse, RigidBody &col, const glm::vec3 &surfaceNormal, const glm::vec3 &applyPoint, float frictionCoefficient);
}

#endif