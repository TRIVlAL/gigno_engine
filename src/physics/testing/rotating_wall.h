#ifndef ROTATING_WALL_H
#define ROTATING_WALL_H

#include "../rigid_body.h"

namespace gigno {

    class RotatingWall : public RigidBody {
        ENTITY_DECLARATIONS(RotatingWall, RigidBody);

    public:

        virtual void Init() override {
            ModelPath = "models/wall.obj";
            ColliderType = COLLIDER_HULL;
            CollisionModelPath = "models/wall.obj";
            IsStatic = true;

            RigidBody::Init();
        }

        virtual void Think(float dt) override;

        virtual void PhysicThink(float dt) override;

    private:
        const float SPEED = 0.25;
    };

    BEGIN_KEY_TABLE(RotatingWall)
    END_KEY_TABLE


}
#endif