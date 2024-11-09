#ifndef TEST_ENTITY_H
#define TEST_ENTITY_h

#include "entities/entity.h"

#include "physics/rigid_body.h"

namespace gigno {

    class TestEntity : Entity {
        ENABLE_SERIALIZATION(TestEntity);
    public:
        TestEntity(RigidBody *rb1, RigidBody *rb2) : Entity(), m_RB1{rb1}, m_RB2{rb2} {
            m_RB1->TestingInterpolateType = 0;
            m_RB2->TestingInterpolateType = 1;
        }
        ~TestEntity() {

        }

        void PhysicThink(float dt) override {
            m_RB1->AddForce(glm::vec3{5.0f, 0.0f, 0.0f});
            m_RB2->AddForce(glm::vec3{5.0f, 0.0f, 0.0f});
        }
    private:
        RigidBody *m_RB1{};
        RigidBody *m_RB2{};
    };

    DEFINE_SERIALIZATION(TestEntity) {
        SERIALIZE_BASE_CLASS(Entity);
    }

}

#endif