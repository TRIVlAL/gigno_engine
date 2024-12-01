#ifndef TEST_ENTITY_H
#define TEST_ENTITY_h

#include "entities/entity.h"

#include "physics/rigid_body.h"

#include "algorithm/geometry.h"

namespace gigno {

    class TestEntity : Entity {
        ENABLE_SERIALIZATION(TestEntity);
    public:
        TestEntity(RigidBody *rb1, RigidBody *rb2) : Entity(), m_RB1{rb1}, m_RB2{rb2} {
            m_RB1->Mass = 2.0f;
        }
        ~TestEntity() {

        }

        glm::vec3 a1{1.0f, 1.0f, 1.0f};
        glm::vec3 a2{0.0f, 1.0f, 1.0f};
        glm::vec3 a3{0.0f, 1.0f, 0.0f};

        glm::vec3 b1{0.0f, 0.2f, 1.0f};
        glm::vec3 b2{1.0f, 0.0f, 1.0f};
        glm::vec3 b3{1.0f, 0.0f, 2.0f};

        void Think(float dt) override {
            /*
            glm::vec3 an = glm::cross(a2 - a1, a3 - a1);
            glm::vec3 bn = glm::cross(b2 - b1, b3 - b1);

            InputServer *input = Application::Singleton()->GetInputServer();

            float speed = 5.0f;

            if(input->GetKey(KEY_U)) {
                //a1 += glm::vec3{0.0f, 1.0f, 0.0f} * speed * dt;
                a2 += glm::vec3{0.0f, 1.0f, 0.0f} * speed * dt;
                a3 += glm::vec3{0.0f, 1.0f, 0.0f} * speed * dt;
            }
            if(input->GetKey(KEY_J)) {
                //a1 += glm::vec3{0.0f, -1.0f, 0.0f} * speed * dt;
                a2 += glm::vec3{0.0f, -1.0f, 0.0f} * speed * dt;
                a3 += glm::vec3{0.0f, -1.0f, 0.0f} * speed * dt;
            }
            if (input->GetKey(KEY_H))
            {
                //a1 += glm::vec3{-1.0f, 0.0f, 0.0f} * speed * dt;
                a2 += glm::vec3{-1.0f, 0.0f, 0.0f} * speed * dt;
                a3 += glm::vec3{-1.0f, 0.0f, 0.0f} * speed * dt;
            }
            if (input->GetKey(KEY_K))
            {
                //a1 += glm::vec3{1.0f, 0.0f, 0.0f} * speed * dt;
                a2 += glm::vec3{1.0f, 0.0f, 0.0f} * speed * dt;
                a3 += glm::vec3{1.0f, 0.0f, 0.0f} * speed * dt;
            }

            an = glm::normalize(an);
            bn = glm::normalize(bn);

            Application::Singleton()->GetRenderer()->DrawPoint(a1, glm::vec3{1.0f, 0.0f, 0.0f}, UNIQUE_NAME);
            Application::Singleton()->GetRenderer()->DrawPoint(a2, glm::vec3{1.0f, 0.0f, 0.0f}, UNIQUE_NAME);
            Application::Singleton()->GetRenderer()->DrawPoint(a3, glm::vec3{1.0f, 0.0f, 0.0f}, UNIQUE_NAME);

            Application::Singleton()->GetRenderer()->DrawLine(a1, a2, glm::vec3{0.8f, 0.0f, 0.0f}, UNIQUE_NAME);
            Application::Singleton()->GetRenderer()->DrawLine(a2, a3, glm::vec3{0.8f, 0.0f, 0.0f}, UNIQUE_NAME);
            Application::Singleton()->GetRenderer()->DrawLine(a3, a1, glm::vec3{0.8f, 0.0f, 0.0f}, UNIQUE_NAME);

            Application::Singleton()->GetRenderer()->DrawPoint(b1, glm::vec3{0.0f, 0.0f, 1.0f}, UNIQUE_NAME);
            Application::Singleton()->GetRenderer()->DrawPoint(b2, glm::vec3{0.0f, 0.0f, 1.0f}, UNIQUE_NAME);
            Application::Singleton()->GetRenderer()->DrawPoint(b3, glm::vec3{0.0f, 0.0f, 1.0f}, UNIQUE_NAME);

            Application::Singleton()->GetRenderer()->DrawLine(b1, b2, glm::vec3{0.0f, 0.0f, 0.8f}, UNIQUE_NAME);
            Application::Singleton()->GetRenderer()->DrawLine(b2, b3, glm::vec3{0.0f, 0.0f, 0.8f}, UNIQUE_NAME);
            Application::Singleton()->GetRenderer()->DrawLine(b3, b1, glm::vec3{0.0f, 0.0f, 0.8f}, UNIQUE_NAME);

            Line line{};
            line = PlaneIntersect(a1, an, b1, bn);

            glm::vec3 intersect{};

            LineIntersect(line.Point, line.Dir, a1, glm::normalize(a2 - a1), intersect);

            Application::Singleton()->GetRenderer()->DrawLine(line.Point - line.Dir * 1000.0f, line.Point + line.Dir * 1000.0f, glm::vec3{0.0f, 0.8f, 0.2f}, UNIQUE_NAME);
            */
        }

        void PhysicThink(float dt) override {
            static int i = 0;
            //m_RB1->AddForce(glm::vec3{0.0f, -2.0f, 0.0f});
            //m_RB2->AddForce(glm::vec3{0.0f, 2.0f, 0.0f});
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