#ifndef CONSTRAINT_TEST_H
#define CONSTRAINT_TEST_H

#include "../../entities/rendered_entity.h"

namespace gigno
{
    class RaycastTest : public Entity
    {
        ENTITY_DECLARATIONS(RaycastTest, RigidBody);

    public:
        virtual void Init() override;
        virtual void Think(float dt) override;
        virtual void PhysicThink(float dt) override;

        //is the world cursor visible
        bool Visible = true;
    private:
        bool m_PressedLeft;
        bool m_PressedRight;
        RenderedEntity *m_WorldCursor;
    };

    BEGIN_KEY_TABLE(RaycastTest)
        DEFINE_KEY_VALUE(bool, Visible)
    END_KEY_TABLE
}

#endif