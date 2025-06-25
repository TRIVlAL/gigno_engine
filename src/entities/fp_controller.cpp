#include "fp_controller.h"
#include "../application.h"
#include "../algorithm/geometry.h"

namespace gigno {

    ENTITY_DEFINITIONS(FPController, RigidBody);

    FPController::FPController() : RigidBody() {
        //ModelPath = "assets/models/capsule.obj";
    }

    FPController::~FPController() {
        if(GetApp() && GetApp()->GetInputServer()) {
            GetApp()->GetInputServer()->SetMouseMode(MOUSE_DEFAULT);
        }
    }

    void FPController::Init() {
        m_pCamera = GetApp()->GetEntityServer()->CreateEntity<Camera>();
        m_pCamera->LookMode = LOOK_MODE_TRANSFORM_FORWARD;
        m_pCamera->ProjMode = PROJECTION_MODE_PERSPECTIVE;
        m_pCamera->Init();
        
        Friction = 0.3f;
        Drag = 0.0f;

        DoRender = false;

        ColliderType = COLLIDER_HULL;
        CollisionModelPath = "assets/models/cube.obj";

        Scale = glm::vec3{1.0f, Height * 0.5f + 0.2, 1.0f};
        Position += Height * 0.5f + 0.1;

        LockRotation = true;

        Mass = 70.0f;

        RigidBody::Init();

        GetApp()->GetInputServer()->SetMouseMode(MOUSE_MOVE_INFINITE);
    }

    void FPController::Think(float dt) {
        InputServer *in = GetApp()->GetInputServer();

        //Query Movement Inputs
        if(in->GetKey(KEY_W)) {
            if(m_MoveForward != 1){
                m_MoveForward++;
            }
        }
        if(in->GetKey(KEY_S)) {
            if(m_MoveForward != -1){
                m_MoveForward--;
            }
        }
        if(in->GetKey(KEY_D)) {
            if(m_MoveRight != 1){
                m_MoveRight++;
            }
        }
        if(in->GetKey(KEY_A)) {
            if(m_MoveRight != -1){
                m_MoveRight--;
            }
        }

        //Query Look Inputs
        if(in->GetKey(KEY_UP)) {
            if(m_LookUp != 1) {
                m_LookUp += 1.0f;
            }
        }
        if(in->GetKey(KEY_DOWN)) {
            if(m_LookUp != -1.0f) {
                m_LookUp -= 1.0f;
            }
        }
        if(in->GetKey(KEY_RIGHT)) {
            if(m_LookRight != 1.0f){
                m_LookRight += 1.0f;
            }
        }
        if(in->GetKey(KEY_LEFT)) {
            if(m_LookRight != -1){
                m_LookRight -= 1.0f;
            }
        }

        //Query Jump Input
        m_PressedJump = m_PressedJump || in->GetKeyDown(KEY_SPACE);

        //Update Camera
        m_pCamera->Position = Position + glm::vec3{0.0f, Height * 0.5f, 0.0f};

        glm::vec2 mouse = in->GetMouseDelta();
        mouse *= LookSensibility;
        m_LookRight += mouse.x;
        m_LookUp += mouse.y;
        
        m_LookUp = -m_LookUp;
        glm::vec3 forward = ApplyRotation(m_pCamera->Rotation, glm::vec3{-1.0f, 0.0f, 0.0f});
        if( forward.y > 0.98f) {
            m_LookUp = glm::min(0.0f, m_LookUp); //Already looking max up
        } else if(forward.y < -0.98f) {
            m_LookUp = glm::max(0.0f, m_LookUp); //Already looking max down
        }

        glm::vec3 right = ApplyRotation(m_pCamera->Rotation, glm::vec3{0.0f, 0.0f, 1.0f});

        m_pCamera->AddRotation((glm::vec3{0.0f, -m_LookRight, 0.0f} - m_LookUp * right) * LookSpeed * dt);

        m_LookRight = 0;
        m_LookUp = 0;

        //Shoot
        if(in->GetKeyDown(KEY_E)) {
            RigidBody *bullet = GetApp()->GetEntityServer()->CreateEntity<RigidBody>();
            bullet->ModelPath = "assets/models/uv_sphere.obj";
            bullet->ColliderType = COLLIDER_SPHERE;
            bullet->Radius = 0.15f;
            bullet->Scale = glm::vec3{0.15f};
            bullet->Velocity = forward * 37.0f;
            bullet->Mass = 15.0f;
            bullet->Bounciness = 0.7f;
            bullet->Position = Position + glm::vec3{0.0f, Height * 0.5f, 0.0f} + forward * 1.5f;
            bullet->Name = "Bullet";
            bullet->Init();
        }

        RigidBody::Think(dt);
    }

    void FPController::PhysicThink(float dt) {
        //Query input in Think to not miss any

        glm::vec3 forward = ApplyRotation(m_pCamera->Rotation, glm::vec3{-1.0f, 0.0f, 0.0f});
        forward.y = 0;
        forward = glm::normalize(forward);
        glm::vec3 right{-forward.z, 0.0f, forward.x};

        glm::vec3 movement = glm::normalize(forward * (float)m_MoveForward + right * (float)m_MoveRight);
        if(movement != movement) {
            movement = glm::vec3{0.0f};
        }

        float head_vel = glm::dot(movement, Velocity);

        bool isMoving = (m_MoveForward != 0 || m_MoveRight != 0);

        if(isMoving) {
            if(head_vel < InstantVelocity)  {
                Velocity += movement * (InstantVelocity - head_vel + 1.0f);
            } else if(head_vel <= MaxVelocity) {
                Velocity += movement * Acceleration * dt;
            } else {
                float extra_vel = MaxVelocity - head_vel;
                Velocity += movement * extra_vel;
            }
        }

        //Apply Deceleration
        if(m_MoveForward == 0) {
            float forward_vel = glm::dot(forward, Velocity);
            Velocity -= forward * (1.0f - DecelMultiplier) * forward_vel;
        }
        if(m_MoveRight == 0) {
            float right_vel = glm::dot(right, Velocity);
            Velocity -= right * (1.0f - DecelMultiplier) * right_vel;
        }

        if(m_MoveForward != 0 && m_MoveRight != 0) {
            float forward_vel = glm::dot(forward * (float)m_MoveForward, Velocity);
            float right_vel = glm::dot(right * (float)m_MoveRight, Velocity);

            if(glm::abs(forward_vel) > glm::abs(right_vel)) {
                Velocity -= forward * (float)m_MoveForward * (glm::abs(forward_vel) - glm::abs(right_vel));
            } else if(glm::abs(forward_vel) < glm::abs(right_vel)) {
                Velocity -= right * (float)m_MoveRight * (glm::abs(right_vel) - glm::abs(forward_vel));
            }

        }

        //Apply Jump
        if(m_PressedJump) {
            AddImpulse(glm::vec3{0.0f, JumpPower * Mass, 0.0f});
        }

        m_MoveForward = 0;
        m_MoveRight = 0;
        m_PressedJump = false;

        RigidBody::PhysicThink(dt);
    }
}
