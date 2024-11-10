#ifndef RIGID_BODY_H
#define RIGID_BODY_H

#include "entities/rendered_entity.h"

namespace gigno {

    class RigidBody : public RenderedEntity {
        ENABLE_SERIALIZATION(RigidBody);
    public:
        RigidBody(ModelData_t model);
        ~RigidBody();

        void AddForce(const glm::vec3 &force) {
            m_Force += force;
        }

        virtual void LatePhysicThink(float dt) override;

        void Stop() {
            m_Velocity = glm::vec3{0.0f};
        }

        int TestingInterpolateType{};

    private:
        glm::vec3 m_Force{};
        glm::vec3 m_Velocity{};
    };

    DEFINE_SERIALIZATION(RigidBody) {
        SERIALIZE_BASE_CLASS(RenderedEntity);

        SERIALIZE(glm::vec3, m_Velocity);
        SERIALIZE(glm::vec3, m_Force);
    }

}

#endif