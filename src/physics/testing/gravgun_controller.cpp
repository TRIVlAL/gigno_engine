#include "gravgun_controller.h"
#include "../../application.h"

namespace gigno {

    ENTITY_DEFINITIONS(GravgunController, FPController);

    void GravgunController::Init() {
        FPController::Init();
    }

    void GravgunController::Think(float dt) {  
        InputServer *in = GetApp()->GetInputServer();
        
        m_GotPullKeyDown = m_GotPullKeyDown || in->GetKeyDown(KEY_G);
        m_GotPullKeyUp = m_GotPullKeyUp || in->GetKeyUp(KEY_G);
        m_GotPullKey = m_GotPullKey || in->GetKey(KEY_G);
        m_GotThrowKeyDown = m_GotThrowKeyDown || in->GetKeyDown(KEY_T);

        FPController::Think(dt);
    }

    void GravgunController::PhysicThink(float dt) {
        PhysicServer *phy = GetApp()->GetPhysicServer();

        const glm::vec3 forward = ApplyRotation(m_pCamera->Rotation, glm::vec3{-1.0f, 0.0f, 0.0f});

        Collider_t collider{ m_pCamera->Position + forward * PullDistance * 0.5f,
                             m_pCamera->Rotation,
                             glm::vec3{PullDistance, Height, 1.0f,},
                             GetApp()->GetPhysicServer()->GetCollisionModel("models/cube.obj")};

        if (m_GotPullKeyDown && !m_pSelected)
        {
            // Select the closest object
            RigidBody *closest{};
            float closest_dist_sqared = FLT_MAX;

            RigidBody *curr = nullptr;
            CollisionData_t col = phy->GetColliding(collider, &curr);
            while (curr)
            {
                if (curr != this && !curr->IsStatic)
                {
                    float dist = LenSquared(curr->Position - Position);
                    if (dist < closest_dist_sqared)
                    {
                        closest_dist_sqared = dist;
                        closest = curr;
                    }
                }
                phy->GetColliding(collider, &curr);
            }

            if (closest)
            {
                m_pSelected = closest;
                m_JustSelected = true;
            }
        }

        if (!m_pSelected || (!m_GotPullKey && !m_GotClose)) {
            m_pSelected = nullptr;
            m_GotClose = false;
        }
        else {
            // Pull Thoward Target
            glm::vec3 target = m_pCamera->Position + forward * TargetDistance;

            glm::vec3 dist = target - m_pSelected->Position;
            float dist_len_sqrd = LenSquared(dist);

            m_pSelected->DisableGravity();

            if (dist_len_sqrd > TargetDistance * TargetDistance * 1.25f) {
                glm::vec3 dist_norm = dist / glm::sqrt(dist_len_sqrd);
                m_pSelected->AddForce(dist * FarPullPower * m_pSelected->Mass / glm::max(dist_len_sqrd, 0.00001f), dist * 0.5f); // Add a bit of torque
            }
            else {
                m_pSelected->Velocity = glm::vec3{0.0f};
                m_pSelected->AddImpulse(dist * dt * ClosePullPower);
                m_GotClose = true;
            }

            if (!m_JustSelected && m_GotPullKeyDown) {
                m_pSelected = nullptr;
            }
        }

        if (m_GotThrowKeyDown && m_pSelected && m_GotClose) {
            m_pSelected->AddForce(forward * ThrowPower * glm::sqrt(m_pSelected->Mass));
            m_pSelected = nullptr;
        }

        m_JustSelected = false;

        m_GotPullKeyDown = false;
        m_GotPullKeyUp = false;
        m_GotPullKey = false;
        m_GotThrowKeyDown = false;


        FPController::PhysicThink(dt);
    }
}
