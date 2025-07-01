#ifndef PLAYER_H
#define PLAYER_H

#include "../physics/rigid_body.h"

namespace gigno {

    class Camera;

    class Player : public RigidBody {
        ENTITY_DECLARATIONS(Player, RigidBody)
    public:
        virtual void Init() override;
        virtual void Think(float dt) override;
        virtual void PhysicThink(float dt) override;

        float Height{3.0f}, Width{1.8f};

    private:
        void UpdateInputs();

        void UpdateLook(float dt);

        void CalculateMovement(float dt);

        void HandleWeaponChange();
        void FireWeapon();

        bool m_DidPhysics{};

        struct InputState_t {
            //updated per PHYSICS tick
            bool Forward{}, Backward{}, Left{}, Right{};
            glm::vec3 MovementDirection{};
            bool Jump{};

            //updated per tick
            bool Fire{};
            size_t NextWeapon{};

            glm::vec2 ViewDiff{};

        } m_Inputs{}, m_OldInputs{};

        struct MovementState_t {

            float LookYaw{}, LookPitch{}; //in rads.
            glm::vec3 LookDir{};
        
            bool        IsGrounded{};
            RigidBody   *Ground{};
            glm::vec3 GroundNormal{};
        } m_Mov{};

        struct MovementVariables_t {

            float Height = 3.0f, Width = 1.8f;

            float LookSpeed = 0.05f;

            float gravity   = -20.0f;

            float ForwardSpeed  = 16.5f;
            float SideSpeed     = 14.0f;

            float Accelerate    = 16.5f;
            float AirAccelerate = 7.5f;
            float Friction      = 20.0f;
            float AirFriction      = 5.0f;

            float JumpSpeed     = 8.0f;
        } const m_Vars{};

        enum WeaponType_t : int {
            WEAPON_BALL,
            WEAPON_GUN,
            WEAPON_MAX_ENUM
        };

        int m_CurrentWeapon = WEAPON_BALL;

        Camera *m_Camera{};
    };

    BEGIN_KEY_TABLE(Player)
        DEFINE_KEY_VALUE(float, Height)
        DEFINE_KEY_VALUE(float, Width)
    END_KEY_TABLE

}

#endif