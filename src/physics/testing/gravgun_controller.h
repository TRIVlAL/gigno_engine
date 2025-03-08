#ifndef GRAVGUN_CONTROLLER_H
#define GRAVGUN_CONTROLLER_H

#include "../../entities/fp_controller.h"

namespace gigno {

    struct Sound_t;

    // Player controller with gravity gun.
    class GravgunController : public FPController {
        ENTITY_DECLARATIONS(GravgunController, FPController);
    public:
        GravgunController() = default;
        ~GravgunController() = default;

        virtual void Init() override;
        virtual void Think(float dt) override;
        virtual void PhysicThink(float dt) override;
    
        float PullDistance{30.0f};
        float TargetDistance{5.0f};
        float FarPullPower{350.0f};
        float ClosePullPower{900.0f};
        float ThrowPower{80000.0f};
    private:
        
        RigidBody *m_pSelected;
        bool m_GotClose = false;
        bool m_JustSelected = false;

        bool m_GotPullKeyDown = false;
        bool m_GotPullKey = false;
        bool m_GotPullKeyUp = false;
        bool m_GotThrowKeyDown = false;
    };

    BEGIN_KEY_TABLE(GravgunController)
    END_KEY_TABLE

}

#endif