#ifndef BOWLING_MINIGAME_H
#define BOWLING_MINIGAME_H

#include "../../entities/entity.h"
#include <array>

namespace gigno {
    class RigidBody;
    class RenderedEntity;
    class Camera;

    class BowlingMinigame : public Entity {
        ENTITY_DECLARATIONS(BowlingMinigame, Entity);
    public:
        virtual void Init() override;
        virtual void Think(float dt) override;
        virtual void PhysicThink(float dt) override;

        const char *AlleyBeginPositionName{};
        const char *AlleyEndPositionName{};

    private:
        void SetupPins();
        glm::vec3 BallBasePosition();
        bool ArePinsStable();
        bool IsPinDown(size_t pin_index);

        enum {
            GS_STARTUP,
            GS_SELECTING_THROW,
            GS_THROWN,
            GS_RESETTING
        } m_GameState;

        glm::vec3 m_AlleyBegin{};
        glm::vec3 m_AlleyEnd{};

        glm::vec3 m_Forward{};
        glm::vec3 m_Up{};
        glm::vec3 m_Right{};

        std::array<RigidBody *, 10> m_Pins{};
        RigidBody *m_Ball{};

        RenderedEntity *m_PointingArrow{};

        Camera *m_Camera{};
        Camera *m_SidewayCamera{};
        Camera *m_TopCamera{};

        float m_Charge{}; //current charge (with animation)
        float m_TopCharge{}; //max charge we got this throw (for determining throw power)
        float m_MaxCharge{200.0f}; //max possible charge

        float m_ThrowAngle{}; // -20 to 320 degs (in rads)

        float m_BallStoppedTimer{};

        size_t m_PinDownCount{};
        bool m_PinDownArray[10]{};
    };

    
    BEGIN_KEY_TABLE(BowlingMinigame)
    DEFINE_KEY_VALUE(cstr, AlleyBeginPositionName)
    DEFINE_KEY_VALUE(cstr, AlleyEndPositionName)
    END_KEY_TABLE

}

#endif