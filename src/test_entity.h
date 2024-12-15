#ifndef TEST_ENTITY_H
#define TEST_ENTITY_h

#include "entities/entity.h"

#include "physics/rigid_body.h"

#include "algorithm/geometry.h"

namespace gigno {

    class TestEntity : Entity {
    public:
        TestEntity(RigidBody *rb1, RigidBody *rb2) : Entity(), m_RB1{rb1}, m_RB2{rb2} {
            m_RB1->Mass = 1.0f;
        }
        ~TestEntity() {

        }

        glm::vec3 a1{1.0f, 1.0f, 1.0f};
        glm::vec3 a2{0.0f, 0.5f, 0.8f};

        glm::vec3 b1{0.0f, 0.2f, 1.0f};
        glm::vec3 b2{1.0f, 0.0f, 0.4f};

        void Think(float dt) override {
        }

        void PhysicThink(float dt) override {
            //m_RB1->AddForce(glm::vec3{0.0f, -2.0f, 0.0f});
            //m_RB2->AddForce(glm::vec3{0.0f, 2.0f, 0.0f});
        }
    private:
        RigidBody *m_RB1{};
        RigidBody *m_RB2{};
    };


}

#endif