#ifndef RAYCAST_H
#define RAYCAST_H

/*
    Internal implementations of the Raycasting Algorithms. 
    Raycasting checks collision between a line segment (Ray) and another objects.
*/

#include "collision.h"

namespace gigno {

    class Entity;

    /*
    Represents a line segment, 
    with an endpoint a direction and a length.
    */
    struct Ray_t{
        glm::vec3 Point{};
        //expected to be a unit vector (len of 1)
        glm::vec3 Direction{};
        //if zero, no max range.
        float MaxRange{0.0f};
    };

    // Specifies with what kind of colliders does the ray collide.
    enum RaycastCollisionType_t {
        RAYCAST_COLLIDE_AABB,               // Collides with the AABB of every Rigidbodies in scene 
        RAYCAST_COLLIDE_COLLIDER            // Colliders with the Collider of every Rigidbodies in scene
    };

    // Specifies what kind of entity 'owns' the collider the ray hit
    // Set by the server.
    enum RaycastHitEntityType_t {
        RAYCAST_HIT_ENTITY_NONE,
        RAYCAST_HIT_ENTITY_RIGIDBODY,
        RAYCAST_HIT_ENTITY_RENDERED_ENTITY
    };

    struct RaycastHit_t {
        // What collider type did the ray hit (only one set)
        RaycastCollisionType_t ColliderHit{};
        // Where did the ray hit in world space
        glm::vec3 Position{};
        //The distance between the ray source and the hit point
        float Distance{};
        // The RigidBody / RenderedEntity the ray hit
        Entity* EntityHit{};
        // The kind of entity EntityHit is. 
        // as such, dynamic_cast of EntityHit to that type is safe. 
        RaycastHitEntityType_t EntityHitType{};
    };

    /* -----------------------------------------------
    INTERNAL IMPLEMENTATIONS

    You should use the PhysicsServer interfaces to use these functionalities. See :
        - PhysicServer::Raycast
        - PhysicServer::RaycastSingle
        - PhysicServer::RaycastHas                  in physic_server.h
    --------------------------------------------- */

    /*
    Returns whether the ray collides with the axis aligned bounding box, while considering only this object
    If true, outputs the RaycastHit informations.
    */
    bool Raycast_AABB(Ray_t ray, BoundingBox_t bb, RaycastHit_t *outHit);

    /*
    Returns whether the ray collides with the collider, while considering only this object
    If true, outputs the RaycastHit informations.
    */
    bool Raycast_Collider(Ray_t ray, Collider_t collider, RaycastHit_t *outHit);
    bool Raycast_Sphere(Ray_t ray, glm::vec3 position, float radius, RaycastHit_t *outHit);
    bool Raycast_Plane(Ray_t ray, glm::vec3 position, glm::vec3 normal, RaycastHit_t *outHit);
    bool Raycast_Hull(Ray_t ray, glm::vec3 position, const CollisionModel_t *model, std::vector<glm::vec3> &transformedModel, BoundingBox_t bb, RaycastHit_t *outHit);
}

#endif