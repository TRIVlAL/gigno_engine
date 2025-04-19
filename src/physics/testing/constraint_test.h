#ifndef CONSTRAINT_TEST_H
#define CONSTRAINT_TEST_H

#include "../rigid_body.h"

namespace gigno {

    class ConstraintTest : public RigidBody {
        ENTITY_DECLARATIONS(ConstraintTest, RigidBody);
    public:
        virtual void Init() override;
        virtual void Think(float dt) override;

    private:
        glm::vec3 p;
    };

    BEGIN_KEY_TABLE(ConstraintTest)
    END_KEY_TABLE
}

#endif