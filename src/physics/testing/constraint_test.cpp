#include "constraint_test.h"
#include "../../application.h"

namespace gigno {

    ENTITY_DEFINITIONS(ConstraintTest, RigidBody);

    void ConstraintTest::Init() {

        RigidBody::Init();

        p = Position + glm::vec3{0, 5, 0};

        RopeConstraint *constraint = new RopeConstraint(this, p, 6);
        Constraints.emplace_back(constraint);
    }

    void ConstraintTest::Think(float dt) {
        GetApp()->GetRenderer()->DrawLine(p, Position, glm::vec3{1.0f, 0.0f, 0.0f});
    }
}