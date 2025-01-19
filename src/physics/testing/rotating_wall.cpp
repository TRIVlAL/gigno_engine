#include "rotating_wall.h"
#include "../../application.h"

namespace gigno {

    ENTITY_DEFINITIONS(RotatingWall, RigidBody);

    void RotatingWall::Think(float dt) {
        RigidBody::Think(dt);
    }

    void RotatingWall::PhysicThink(float dt) {
       Rotation += glm::vec3{0.0f, 1.0f, 0.0f} * dt * SPEED;


        RigidBody::PhysicThink(dt);
    }
}