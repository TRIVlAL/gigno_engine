#include "player.h"
#include "../debug/console/convar.h"
#include "../application.h"
#include "../utils/geometry.h"

namespace gigno {

    ENTITY_DEFINITIONS(Player, Entity)

    CONVAR(float, player_speed, 0.5f, "");

    void Player::Init() {

        EntityServer *ent = GetApp()->GetEntityServer();

        ColliderType = COLLIDER_HULL;
        CollisionModelPath = "assets/models/cube.obj";
        Scale = glm::vec3{m_Vars.Width, m_Vars.Height, m_Vars.Width};
        LockRotation = true;
        DoRender = false;
        Friction = 0.0f;
        GravityOverride = glm::vec3{0.0f, m_Vars.gravity, 0.0f};
        Bounciness = 0.1f;
        
        m_Camera = ent->CreateEntity<Camera>();
        m_Camera->Init();

        GetApp()->GetInputServer()->SetMouseMode(MOUSE_MOVE_INFINITE);

        RigidBody::Init();
    }

    void Player::Think(float dt) {
        UpdateInputs();

        UpdateLook(dt);

        HandleWeaponChange();

        if(m_CurrentWeapon == WEAPON_GUN) {
            GetApp()->GetRenderer()->DrawPoint(m_Camera->Position + m_Mov.LookDir * 0.1f, glm::vec3{0.0f, 0.0f, 0.0f}); 
        }
        
        if(m_Inputs.Fire) {
            FireWeapon();
        }
    }

    void Player::PhysicThink(float dt) {
        m_DidPhysics = true;

        CalculateMovement(dt);

        m_Camera->Position = Position + glm::vec3{0.0f, m_Vars.Height - 0.25f, 0.0f};
        m_Camera->Rotation = glm::quat{1.0f, 0.0f, 0.0f, 0.0f};
        m_Camera->AddRotation(glm::vec3(0.0f, 0.0f, m_Mov.LookPitch));
        m_Camera->AddRotation(glm::vec3{0.0f, m_Mov.LookYaw, 0.0f});
    }

    void Player::UpdateInputs() {

        InputServer *in = GetApp()->GetInputServer();

        if(m_DidPhysics) {
            m_Inputs.Forward = false;
            m_Inputs.Backward = false;
            m_Inputs.Left = false;
            m_Inputs.Right = false;
            m_Inputs.Jump = false;

            m_DidPhysics = false;
        }

        m_Inputs.Forward     = m_Inputs.Forward     || in->GetKey(KEY_W);
        m_Inputs.Backward    = m_Inputs.Backward    || in->GetKey(KEY_S);
        m_Inputs.Left        = m_Inputs.Left        || in->GetKey(KEY_A);
        m_Inputs.Right       = m_Inputs.Right       || in->GetKey(KEY_D);

        m_Inputs.MovementDirection = glm::vec3{m_Inputs.Left ? -1.0f : m_Inputs.Right ? 1.0f : 0.0f,
                                                0.0f,
                                                m_Inputs.Forward ? 1.0f : m_Inputs.Backward ? -1.0f : 0.0f};

        m_Inputs.Jump = m_Inputs.Jump || in->GetKeyDown(KEY_SPACE); 

        m_Inputs.ViewDiff   = in->GetMouseDelta();

        m_Inputs.Fire = in->GetMouseButtonDown(MOUSE_BUTTON_LEFT);

        m_Inputs.NextWeapon = 0;
        if(in->GetKeyDown(KEY_1)) {
            m_Inputs.NextWeapon--;
        }
        if(in->GetKeyDown(KEY_2)) {
            m_Inputs.NextWeapon++;
        }
    }

    void Player::UpdateLook(float dt) {

        m_Mov.LookYaw   = glm::mod<float>(m_Mov.LookYaw - m_Vars.LookSpeed * m_Inputs.ViewDiff.x * dt, glm::two_pi<float>());
        m_Mov.LookPitch = glm::clamp<float>(m_Mov.LookPitch + m_Vars.LookSpeed * m_Inputs.ViewDiff.y * dt, -1.55f, 1.55f);

        m_Mov.LookDir = glm::vec3{-glm::cos(m_Mov.LookYaw), -glm::sin(m_Mov.LookPitch), glm::sin(m_Mov.LookYaw)};
        m_Mov.LookDir = glm::normalize(m_Mov.LookDir);
    }

    void Player::CalculateMovement(float dt) {

        if(m_Mov.IsGrounded) {
            DisableGravity();
        }

        PhysicServer *phys = GetApp()->GetPhysicServer();

        if(m_Mov.LookDir == glm::vec3{0.0f}) { return; } //input has not yet been initialized !

        glm::vec3 floor_vel = glm::vec3{ Velocity.x, 0.0f, Velocity.z };
        glm::vec3 forward = glm::normalize(glm::vec3{m_Mov.LookDir.x, 0.0f, m_Mov.LookDir.z});
        glm::vec3 right = glm::cross(forward, glm::vec3{0.0f, 1.0f, 0.0f});

        glm::vec3 wish_vel = forward * m_Inputs.MovementDirection.z * m_Vars.ForwardSpeed + right * m_Inputs.MovementDirection.x * m_Vars.SideSpeed;

        if(m_Inputs.Jump && m_Mov.IsGrounded) {
            Velocity.y = m_Vars.JumpSpeed;
            //speed boost when jumping
            wish_vel *= 1.33f;
        }

        glm::vec3 delta_vel = wish_vel - floor_vel;

        float accel{};
        if(wish_vel == glm::vec3{0.0f}) {
            accel = m_Mov.IsGrounded ? m_Vars.Friction : m_Vars.AirFriction;
        } else {
            accel = (m_Mov.IsGrounded ? m_Vars.Accelerate : m_Vars.AirAccelerate);
        }
        accel *= dt;
        if(accel > 1.0f) { accel = 1.0f; }

        Velocity += delta_vel * accel;

        


        m_Mov.IsGrounded = false;
        m_Mov.Ground = nullptr;
        Collider_t collider = Collider_t{Position + glm::vec3{0.0f, -0.05f, 0.0f},
                              FromEuler(glm::vec3{0.0f, m_Mov.LookYaw, 0.0f}),
                              glm::vec3{m_Vars.Width - 0.05f, m_Vars.Height + 0.025f, m_Vars.Width - 0.05f},
                              phys->GetCollisionModel("assets/models/cube.obj")};
        RigidBody *colliding = nullptr;
        CollisionData_t collision{};
        do {
            collision = phys->GetColliding(collider, &colliding);
            if(colliding == this) {
                continue;
            }
            
            if(collision.Collision && glm::abs(collision.Normal.y) > 0.4f) {
                m_Mov.IsGrounded = collision.Collision;
                m_Mov.Ground = colliding;
                m_Mov.GroundNormal = collision.Normal;
                break;
            }

        } while(colliding != nullptr);
        
    }

    void Player::HandleWeaponChange() {
        m_CurrentWeapon += m_Inputs.NextWeapon;
        if(m_CurrentWeapon < 0) {
            m_CurrentWeapon = WEAPON_MAX_ENUM - 1;
        }
        if(m_CurrentWeapon >= WEAPON_MAX_ENUM) {
            m_CurrentWeapon = 0;
        }
    }

    void Player::FireWeapon() {

        PhysicServer *phys = GetApp()->GetPhysicServer();

        switch(m_CurrentWeapon) {
            case WEAPON_BALL: {
                RigidBody *bullet = GetApp()->GetEntityServer()->CreateEntity<RigidBody>();
                bullet->ModelPath = "assets/models/uv_sphere.obj";
                bullet->ColliderType = COLLIDER_SPHERE;
                bullet->Radius = 0.15f;
                bullet->Scale = glm::vec3{0.15f};
                bullet->Velocity = m_Mov.LookDir * 37.0f;
                bullet->Mass = 15.0f;
                bullet->Bounciness = 0.9f;
                bullet->Position = m_Camera->Position + m_Mov.LookDir * (m_Vars.Width + 0.5f);
                bullet->Name = "Bullet";
                bullet->Init();
            }
            case WEAPON_GUN: {
                Console::LogInfo("Fired gun !");

                Ray_t ray{};
                ray.Direction = m_Mov.LookDir;
                ray.Point = m_Camera->Position;
                ray.MaxRange = 400.0f;

                RaycastHit_t hit{};
                if(phys->RaycastSingle (ray, RAYCAST_COLLIDE_COLLIDER, &hit)) {

                    RigidBody *target = (RigidBody*)(hit.EntityHit);
                    target->AddImpulse(ray.Direction * 600.0f, ray.Point - target->Position);

                    Console::LogInfo("HIT !");
                }

                break;
            }
            default:
                break;
        }

    }
}