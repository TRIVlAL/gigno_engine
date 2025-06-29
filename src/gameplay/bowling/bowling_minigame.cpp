#include "../../error_macros.h"
#include "bowling_minigame.h"
#include "../../application.h"
#include "../../physics/rigid_body.h"
#include "../../entities/camera.h"
#include "../../entities/rendered_entity.h"

namespace gigno {

    ENTITY_DEFINITIONS(BowlingMinigame, Entity);

    void BowlingMinigame::Init() {
        EntityServer *ent = GetApp()->GetEntityServer();

        m_GameState     = GS_STARTUP;

        m_AlleyBegin    = ent->GetEntityByName(AlleyBeginPositionName)->Position;
        m_AlleyEnd      = ent->GetEntityByName(AlleyEndPositionName)->Position;

        m_Forward       = glm::normalize(m_AlleyEnd - m_AlleyBegin);
        m_Up            = glm::vec3{0.0f, 1.0f, 0.0f};
        m_Right         = glm::cross(m_Forward, m_Up);

        for(size_t i = 0; i < 10; i++) {
            m_Pins[i] = ent->CreateEntity<RigidBody>();

            m_Pins[i]->Name                 = "pin";

            m_Pins[i]->ModelPath            = "assets/models/bowling_pin.obj";
            m_Pins[i]->ColliderType         = COLLIDER_HULL;
            m_Pins[i]->CollisionModelPath   = "assets/models/bowling_pin_collision.obj";

            m_Pins[i]->Mass                 = 50.0f;
            m_Pins[i]->InertiaTensor        = glm::mat3{    0.5f,   0.0f,   0.0f,
                                                            0.0f,   0.2f,   0.0f ,
                                                            0.0f,   0.0f,   0.5f};
            
            m_Pins[i]->Friction = 0.9f;

            m_Pins[i]->Scale    = glm::vec3{0.8f};

            m_Pins[i]->Init();
        }
        SetupPins();


        {
            m_Ball                          = ent->CreateEntity<RigidBody>();
            m_Ball->Name                    = "ball";
            m_Ball->ModelPath               = "assets/models/colored_uv_sphere.obj";

            m_Ball->ColliderType            = COLLIDER_SPHERE;
            m_Ball->Scale                   = glm::vec3{0.75f};
            m_Ball->Radius                  = 0.75f;

            m_Ball->Mass                    = 450.0f;
            m_Ball->InertiaTensor           = glm::mat3{    2.0f/3.0f * 0.25f,   0.0f,               0.0f,
                                                            0.0f,               2.0f/3.0f * 0.25f,   0.0f,
                                                            0.0f,               0.0f,               2.0f/3.0f * 0.25f };
            
            m_Ball->Bounciness = 0.01f;
            m_Ball->Friction = 0.3f;
            m_Ball->Drag = 0.1f;
            m_Ball->AngularDrag = 1.0f;

            m_Ball->Position = BallBasePosition();

            m_Ball->Init();
        }

        {
            m_PointingArrow                     = ent->CreateEntity<RenderedEntity>();

            m_PointingArrow->Name               = "arrow";
            m_PointingArrow->ModelPath          = "assets/models/arrow.obj";
            m_PointingArrow->Scale              = glm::vec3{1.0f, 0.5f, 0.5f};

            m_PointingArrow->Init();
        }

        {
            m_Camera = ent->CreateEntity<Camera>();

            m_Camera->Init();

            m_Camera->AddRotation(glm::vec3{0.0f, 0.0f, 15.0f * glm::two_pi<float>() / 360.0f});
        }
        
        {
            m_SidewayCamera = ent->CreateEntity<Camera>();
            m_SidewayCamera->Position = m_AlleyEnd - m_Forward * 3.5f - m_Right * 8.0f + m_Up * 4.0f;

            m_SidewayCamera->LookMode = LOOK_MODE_TRANSFORM_FORWARD;
            
            m_SidewayCamera->AddRotation(glm::vec3{0.0f, -90.0f, 0.0f} * glm::two_pi<float>() / 360.0f);
            m_SidewayCamera->AddRotation(glm::vec3{-15.0f, 0.0f, 0.0f} * glm::two_pi<float>() / 360.0f);

            m_SidewayCamera->Init();

        }

        {
            m_TopCamera = ent->CreateEntity<Camera>();
            m_TopCamera->Position = m_AlleyEnd - m_Forward * 1.0f + m_Up * 2.5f;

            m_TopCamera->LookMode = LOOK_MODE_TRANSFORM_FORWARD;

            m_TopCamera->AddRotation(glm::vec3{0.0f, 180.0f, 0.0f} * glm::two_pi<float>() / 360.0f);
            m_TopCamera->AddRotation(glm::vec3{0.0f, 0.0f, -20.0f} * glm::two_pi<float>() / 360.0f);

            m_TopCamera->Init();
        }

        m_GameState = GS_SELECTING_THROW;

        Entity::Init();
    }

    void BowlingMinigame::SetupPins() {

        size_t line_count{1}; //line 1 has 1 pin, line 2 has 2 pin, etc...
        size_t row_count{};
        float line_spacing{1.5f}, row_spacing{1.4f};
        for(size_t i = 0; i < 10; i++) {

            ASSERT(m_Pins[i] != nullptr);

            glm::vec3 Position = m_AlleyEnd - (float)(4 - line_count) * line_spacing    * m_Forward
                                - ((float)(line_count-1) / 2.0f) * row_spacing              * m_Right
                                + row_count * row_spacing                               * m_Right
                                + 0.75f                                                 * m_Up;

            
            if(!m_PinDownArray[i]) {
                m_Pins[i]->Position = Position;
            }              

            row_count++;
            if(row_count == line_count) {
                line_count++;
                row_count = 0;
            }
        }

    }

    glm::vec3 BowlingMinigame::BallBasePosition() {
        return m_AlleyBegin - m_Forward * 1.0f + m_Up * 1.0f;
    }

    bool BowlingMinigame::ArePinsStable() {
        bool ret = true;
        for(size_t i = 0; i < 10; i++) {
            ret = glm::length2(m_Pins[i]->Velocity) < 0.01f;
            if(!ret) break;
        }
        return ret;
    }

    bool BowlingMinigame::IsPinDown(size_t pin_index) {
        glm::vec3 pin_up = ApplyRotation(m_Pins[pin_index]->Rotation, m_Up);

        if(glm::dot(pin_up, m_Up) <= 0.7f) return true;

        return false;
    }

    void BowlingMinigame::Think(float dt) {

        InputServer *in = GetApp()->GetInputServer();
        RenderingServer *r = GetApp()->GetRenderer();

        if(in->GetKey(KEY_1)) {
            m_SidewayCamera->SetAsCurrentCamera();
        } else if(in->GetKey(KEY_2)) {
            m_TopCamera->SetAsCurrentCamera();
        } else {
            m_Camera->SetAsCurrentCamera();
        }

        if(m_GameState == GS_SELECTING_THROW) {

            if(in->GetKey(KEY_A)) {
                m_ThrowAngle -= dt * 0.5f;
            }
            if(in->GetKey(KEY_D)) {
                m_ThrowAngle += dt * 0.5f;
            }
            m_ThrowAngle = glm::clamp<float>(m_ThrowAngle, -20.0f * glm::two_pi<float>() / 360.0f, 20.0f * glm::two_pi<float>() / 360.0f);

            m_PointingArrow->DoRender = true;
            m_PointingArrow->Position = BallBasePosition() + m_Up * 0.75f - m_Forward * 0.5f;
            m_PointingArrow->Rotation = glm::quat{1.0f, 0.0f, 0.0f, 0.0f};
            m_PointingArrow->AddRotation(glm::vec3{0.0f, -m_ThrowAngle, 0.0f});

            if(in->GetKey(KEY_SPACE)) {
                m_Charge += dt * 150.0f;
                if(m_Charge > m_MaxCharge) {
                    m_Charge = m_MaxCharge;
                }
                m_TopCharge = m_Charge;
            }
            if(in->GetKeyUp(KEY_SPACE)) {
                m_GameState = GS_THROWN;
                m_PointingArrow->DoRender = false;
            }
        }

        if(m_GameState == GS_THROWN) {
            if(m_Charge > 0.0f) {
                m_Charge -= dt * 2.0f * m_TopCharge;
                if(m_Charge <= 0.0f) {
                    m_Charge = 0.0f;

                    glm::vec3 throw_dir = m_Forward * glm::cos(m_ThrowAngle) + m_Right * glm::sin(m_ThrowAngle);

                    m_Ball->AddImpulse(throw_dir * 50.0f * m_TopCharge - m_Up * 1.5f * m_TopCharge, m_Up * 0.1f);
                    m_TopCharge = 0.0f;
                }
            }

            if( m_Charge == 0.0f && glm::length2(m_Ball->Velocity) <= 0.01f) {
                m_BallStoppedTimer += dt;
            }

            if(m_BallStoppedTimer >= 1.5f || 
                m_Ball->Position.y < m_AlleyEnd.y) //fell down the hole at the end.
            {
                if(ArePinsStable()) {
                    m_GameState = GS_RESETTING;
                }
            }
        } 
        
        if(m_GameState == GS_RESETTING) {
            for(size_t i = 0; i < 10; i++) {
                if(!m_PinDownArray[i]) {
                    if(IsPinDown(i)) {
                        m_PinDownArray[i] = true;
                        m_PinDownCount++;
                        m_Pins[i]->Rotation = glm::quat{1.0f, 0.0f, 0.0f, 0.0f};
                        m_Pins[i]->Position = m_AlleyBegin + m_Right * 7.0f + m_Forward * (3.0f + (float)m_PinDownCount) * 1.5f;
                    }
                }
            }
            SetupPins();

            m_BallStoppedTimer = 0.0f;

            m_Ball->Velocity = glm::vec3{0.0f};
            m_Ball->AngularVelocity = glm::vec3{0.0f};
            m_Ball->Position = BallBasePosition();

            m_GameState = GS_SELECTING_THROW;
        }

        if(!(m_GameState == GS_THROWN && m_Charge == 0.0f)) {
            //animate ball swing (yes, this is indeed crude)

            float angle = (m_Charge / m_MaxCharge) * 0.18f * glm::two_pi<float>();
            float radius = 3.0f;
            m_Ball->Position = (BallBasePosition() + m_Up * radius) - m_Up * glm::cos(angle) * radius - m_Forward * glm::sin(angle) * radius;
            m_Camera->Position = BallBasePosition() - m_Forward * 7.0f + m_Up * 3.0f;
        }


        Entity::Think(dt);
    }

    void BowlingMinigame::PhysicThink(float dt) {

        if(!(m_GameState == GS_THROWN && m_Charge == 0.0f)) {
            m_Ball->DisableGravity();
            m_Ball->Velocity = glm::vec3{0.0f};
        }

        Entity::PhysicThink(dt);
    }

}