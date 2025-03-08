#include "rigid_body_spawner.h"

#include "../../application.h"

namespace gigno
{
    ENTITY_DEFINITIONS(RigidBodySpawner, Entity);

    void RigidBodySpawner::Init() {
        const glm::vec3 ext = ExtentEnd - ExtentBegin;

        const float x_step = (ext.x - MARGIN - MARGIN) / (float)XCount;
        const float z_step = (ext.z - MARGIN - MARGIN) / (float)ZCount;
        const float y_step = ext.y / VerticalSlice;

        int alternate = 0;

        for (int v = 0; v < VerticalSlice; v++)
        {
            for (int i = 0; i < XCount; i++)
            {
                for(int j = 0; j < ZCount; j++) {
                    RigidBody *rb = GetApp()->GetEntityServer()->CreateEntity<RigidBody>();
                    rb->Position = glm::vec3{ExtentBegin.x + MARGIN + x_step * i,
                                            ExtentBegin.y + y_step * v,
                                            ExtentBegin.z + MARGIN + z_step * j};
                    rb->Mass = 25.0f;
                    
                    switch(alternate) {
                        case 0:
                            rb->ColliderType = COLLIDER_SPHERE;
                            rb->ModelPath = "assets/models/colored_uv_sphere.obj";
                            rb->Radius = 0.75f;
                            rb-> Scale = glm::vec3{0.75f};
                            alternate++;
                            break;
                        case 1:
                            rb->ColliderType = COLLIDER_HULL;
                            rb->ModelPath = "assets/models/cube.obj";
                            rb->CollisionModelPath = "assets/models/cube.obj";
                            rb->Scale = glm::vec3{0.5f};
                            alternate++;
                            break;
                        case 2:
                            rb->ColliderType = COLLIDER_HULL;
                            rb->ModelPath = "assets/models/hull1.obj";
                            rb->CollisionModelPath = "assets/models/hull1.obj";
                            alternate = 0;
                            break;
                    }
                    rb->Init();
                }
            }
        }

        Entity::Init();
    }

}