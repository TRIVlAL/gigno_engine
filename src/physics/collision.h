#ifndef COLLISION_H
#define COLLISION_H

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include "collider_type.h"

namespace gigno {
    struct CollisionModel_t;
    class RigidBody;
    class CollisionSoundManager;

    struct BoundingBox_t {
        //Represents an axis aligned bounding box.
        glm::vec3 Min{}, Max{};
    };

    struct Collider_t {
        friend class RigidBody;
    private:
        Collider_t() = default; //Only trust RigidBody to not be dumb with the empty constructor
    public:
        Collider_t(glm::vec3 position, glm::quat rotation, glm::vec3 scale, float radius); //Sphere
        Collider_t(glm::vec3 position, glm::quat rotation, glm::vec3 scale, float radius, float length); //Capsule
        Collider_t(glm::vec3 position, glm::quat rotation, glm::vec3 scale, glm::vec3 normal); //Plane
        Collider_t(glm::vec3 position, glm::quat rotation, glm::vec3 scale, const CollisionModel_t *model); //Hull

        void SetTransformedModel();
        void SetBoundingBox();

        ColliderType_t ColliderType;

        glm::vec3 Position{};
        glm::quat Rotation{};
        glm::vec3 Scale{};

        float Radius{}; // COLLIDER_SPHERE / COLLIDER_CAPSULE

        float Length{}; // COLLIDER_CAPSULE

        glm::vec3 Normal{}; // COLLIDER_PLANE

        const CollisionModel_t *Model{};  // COLLIDER_HULL
        // Vertices with Scale and Rotation applied.
        std::vector<glm::vec3> TransformedModel{};  // COLLIDER_HULL

        BoundingBox_t AABB;

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

    /*
    Returns whether the axis aligned bounding boxes intersect.
    */
    bool AABBCollision(BoundingBox_t A, BoundingBox_t B);

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
    void RespondCollision(RigidBody &rb1, RigidBody &rb2, const CollisionData_t &collision, CollisionSoundManager *soundManager );

    /*
    Returns the kinetic enery loss.
    */
    float ApplyFriction(float normalImpulse, RigidBody &rb, const glm::vec3 &surfaceNormal, 
                        const glm::vec3 &applyPoint, float frictionCoefficient, float staticFrictionCoefficient);

    float GetBounciness(RigidBody &rb1, RigidBody &rb2);
    float GetFriction(RigidBody &rb1, RigidBody &rb2);
    float GetStaticFriction(RigidBody &rb1, RigidBody &rb2);
}

#endif