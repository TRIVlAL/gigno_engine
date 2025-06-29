#ifndef RIGID_BODY_SPAWNER
#define RIGID_BODY_SPAWNER

#include "../../physics/rigid_body.h"

namespace gigno
{

    class RigidBodySpawner : public Entity
    {
        ENTITY_DECLARATIONS(RigidBodySpawner, Entity);

    public:
        virtual void Init() override;

        glm::vec3 ExtentBegin = {-5.0f, 3.0f, -5.0f};
        glm::vec3 ExtentEnd = {5.0f, 0.0f, 5.0f};

        int VerticalSlice = 2;
        int XCount = 5;
        int ZCount = 5;

    private:
        const float MARGIN = -0.5f;
    };

    BEGIN_KEY_TABLE(RigidBodySpawner)
        DEFINE_KEY_VALUE(vec3, ExtentBegin)
        DEFINE_KEY_VALUE(vec3, ExtentEnd)
        DEFINE_KEY_VALUE(int, VerticalSlice)
        DEFINE_KEY_VALUE(int, XCount)
        DEFINE_KEY_VALUE(int, ZCount)
    END_KEY_TABLE

}

#endif