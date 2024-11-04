#ifndef RIGID_BODY_H
#define RIGID_BODY_H

#include "physic_entity.h"

namespace gigno {

    class RigidBody : public PhysicEntity {
        ENABLE_SERIALIZATION(RigidBody);
    public:
        RigidBody(ModelData_t model);
        ~RigidBody();

        virtual void PhysicThink(float dt) override;

        void Stop() {
            m_Velocity = glm::vec3{0.0f};
        }

        int TestingInterpolateType{};

        glm::vec3 m_Force{};
    private:
        glm::vec3 m_Velocity{};
    };

    DEFINE_SERIALIZATION(RigidBody) {
        SERIALIZE(glm::vec3, m_Velocity);
        SERIALIZE(glm::vec3, m_Force);
    }

}

#endif