#include "constraint_test.h"
#include "../../application.h"

namespace gigno {

    ENTITY_DEFINITIONS(ConstraintTest, RigidBody);

    void ConstraintTest::Init() {

        Preset = glm::min<int>(Preset, 1);

        RigidBody::Init();

        if(Preset == 0) {
            p = Position + glm::vec3{0, 6, 0};
    
            RopeConstraint *constraint = new RopeConstraint(this, p, 6);
            Constraints.emplace_back(constraint);
        } else if(Preset == 1) {
            p = glm::vec3{0.0f, 1.0f, 0.0f};

            HingeConstraint *constraint = new HingeConstraint(this, Position, p);
            Constraints.emplace_back(constraint);
        }

    }

    void ConstraintTest::Think(float dt) {
        if(Preset == 0) {
            GetApp()->GetRenderer()->DrawLine(p, Position, glm::vec3{1.0f, 0.0f, 0.0f});
        } else if(Preset == 1) {
            GetApp()->GetRenderer()->DrawLine(Position - p * 2.0f, Position + p * 2.0f, glm::vec3{1.0f, 0.0f, 0.0f});
        }
    }
}