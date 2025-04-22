#include "raycast_test.h"
#include "../../application.h"
#include "../rigid_body.h"

namespace gigno
{

    ENTITY_DEFINITIONS(RaycastTest, Entity);

    void RaycastTest::Init(){
        m_WorldCursor = GetApp()->GetEntityServer()->CreateEntity<RenderedEntity>();
        m_WorldCursor->ModelPath = "assets/models/uv_sphere.obj";
        m_WorldCursor->Scale = glm::vec3{0.3f};
        m_WorldCursor->DoRender = Visible;
        m_WorldCursor->Init();
    }

    void RaycastTest::Think(float dt){
        if(GetApp()->GetInputServer()->GetMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            m_PressedLeft = true;
        }
        if(GetApp()->GetInputServer()->GetMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            m_PressedRight = true;
        }
    }

    void RaycastTest::PhysicThink(float dt){

        glm::vec2 mouse_pos = GetApp()->GetInputServer()->GetMousePosition();
        const Camera *cam = GetApp()->GetRenderer()->GetCameraHandle();

        Ray_t ray{};
        ray.Point = cam->Position;
        ray.Direction = cam->RayFromScreenPos(mouse_pos);
        ray.MaxRange = 500.0f;

        RaycastHit_t hit;
        if(GetApp()->GetPhysicServer()->RaycastSingle(ray, RAYCAST_COLLIDE_COLLIDER, &hit)) {
            m_WorldCursor->Position = m_WorldCursor->Position * 0.2f + hit.Position * 0.8f;
            m_WorldCursor->DoRender = Visible;

            if(m_PressedLeft && hit.EntityHitType == RAYCAST_HIT_ENTITY_RIGIDBODY) {
                RigidBody *rb = dynamic_cast<RigidBody*>(hit.EntityHit);

                rb->AddImpulse(ray.Direction * 30.0f * rb->Mass, hit.Position - rb->Position);
            }
            if(m_PressedRight && hit.EntityHitType == RAYCAST_HIT_ENTITY_RIGIDBODY) {
                RigidBody *rb = dynamic_cast<RigidBody*>(hit.EntityHit);

                rb->AddImpulse(glm::vec3{0.0f, 10.0f * rb->Mass, 0.0f}, hit.Position - rb->Position);
            }
        } else {
            m_WorldCursor->DoRender = false;
        }

        m_PressedLeft = m_PressedRight = false;
    }
}