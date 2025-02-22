#ifndef FP_CONTROLLER_H
#define FP_CONTROLLER_H

#include "../physics/rigid_body.h"
#include "camera.h"

namespace gigno {

    class FPController : public RigidBody {
        ENTITY_DECLARATIONS(FPController, RigidBody);
    public:
        FPController();
        ~FPController();

        virtual void Init() override;

        virtual void Think(float dt) override;
        virtual void PhysicThink(float dt) override;

        float InstantVelocity = 9.0f;
        float Acceleration = 35.0f;
        float MaxVelocity = 15.5f;

        float LookSpeed = 1.2f;
        float LookSensibility = 1.0f;
        
        float DecelMultiplier = 0.025f;

        float Height = 2.0f;

        float JumpPower = 5.0f;
    private:
        int m_MoveForward{};
        int m_MoveRight{};

        float m_LookUp{};
        float m_LookRight{};

        bool m_PressedJump{};

        Camera *m_pCamera;
    };

    BEGIN_KEY_TABLE(FPController)
        DEFINE_KEY_VALUE(float, LookSpeed)
        DEFINE_KEY_VALUE(float, DecelMultiplier)
        DEFINE_KEY_VALUE(float, Height)
    END_KEY_TABLE

}

#endif