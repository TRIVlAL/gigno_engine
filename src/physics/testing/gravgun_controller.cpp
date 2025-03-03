#include "gravgun_controller.h"
#include "../../application.h"

namespace gigno {

    ENTITY_DEFINITIONS(GravgunController, FPController);

    void GravgunController::Init() {

        FPController::Init();
    }

    void GravgunController::Think(float dt) {  
        InputServer *in = GetApp()->GetInputServer();
        RenderingServer *r = GetApp()->GetRenderer();
        PhysicServer *phy = GetApp()->GetPhysicServer();
        
        const glm::vec3 forward = ApplyRotation(m_pCamera->Rotation, glm::vec3{-1.0f, 0.0f, 0.0f});

        Collider_t collider{};
        collider.ColliderType = COLLIDER_HULL;
        collider.Model = GetApp()->GetPhysicServer()->GetCollisionModel("models/cube.obj");
        collider.Position = m_pCamera->Position + forward * PullDistance * 0.5f;
        collider.Rotation = m_pCamera->Rotation;
        collider.Scale = glm::vec3{PullDistance, Height, 1.0f};
        collider.CreateTransformedModel();

        if(in->GetKeyDown(KEY_Q) && !m_pSelected) {
            //Select the closest object
            RigidBody *closest{};
            float closest_dist_sqared = FLT_MAX;

            RigidBody *curr = nullptr;
            CollisionData_t col = phy->GetColliding(collider, &curr);
            while(curr) {
                if(curr != this && !curr->IsStatic) {
                    float dist = LenSquared(curr->Position - Position);
                    if(dist < closest_dist_sqared) {
                        closest_dist_sqared = dist;
                        closest = curr;
                    }
                }
                phy->GetColliding(collider, &curr);
            }

            if(closest) {
                m_pSelected = closest;
                m_JustSelected = true;
            }
        }

        if(!m_pSelected || (!in->GetKey(KEY_Q) && !m_GotClose)) {
            m_pSelected = nullptr;
            m_GotClose = false;
        }
        else {
            //Pull Thoward Target
            glm::vec3 target = m_pCamera->Position + forward * TargetDistance;
            r->DrawPoint(target, glm::vec3{0.0f, 1.0f, 0.0f});

            r->DrawLine(glm::vec3{0.0f}, forward, glm::vec3{0.0f, 0.0f, 1.0f});

            glm::vec3 dist = target - m_pSelected->Position;
            float dist_len_sqrd = LenSquared(dist);
            
            if(dist_len_sqrd > TargetDistance * TargetDistance * 1.5f) {
                glm::vec3 dist_norm = dist / glm::sqrt(dist_len_sqrd);
                m_pSelected->AddForce(dist * FarPullPower * m_pSelected->Mass / glm::max(dist_len_sqrd, 0.00001f), dist * 0.5f); // Add a bit of torque
            }
            else {
                m_pSelected->AddImpulse(dist * dt * ClosePullPower);
                m_GotClose = true;
            }

            r->DrawLine(m_pSelected->Position, m_pSelected->Position + glm::vec3{0.0f, 10.0f, 0.0f}, glm::vec3{0.0f, 1.0f, 0.0f});
            Console::LogInfo("name : %s", m_pSelected->Name);

            if(!m_JustSelected && in->GetKeyDown(KEY_Q)) {
                m_pSelected = nullptr;
            }
        }

        if(in->GetKey(KEY_R) && m_pSelected && m_GotClose)  {
            m_pSelected->AddForce(forward * ThrowPower * glm::sqrt(m_pSelected->Mass));
            m_pSelected = nullptr;
        }

        m_JustSelected = false;

        FPController::Think(dt);
    }

    void GravgunController::PhysicThink(float dt) {


        FPController::PhysicThink(dt);
    }
}
