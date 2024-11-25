#ifndef RIGID_BODY_H
#define RIGID_BODY_H

#include "entities/rendered_entity.h"

namespace gigno {

    class RigidBody : public RenderedEntity {
        ENABLE_SERIALIZATION(RigidBody);
    public:
        RigidBody(ModelData_t model);
        ~RigidBody();

        void AddForce(const glm::vec3 &force, const glm::vec3 &application = glm::vec3{0.0f, 0.0f, 0.0f});

        virtual void LatePhysicThink(float dt) override;

        void Stop() {
            m_Velocity = glm::vec3{0.0f};
        }

        float Mass{1.0f};

    private:
        glm::vec3 m_Force{};
        glm::vec3 m_Velocity{};

        glm::vec3 m_Torque{};
        glm::vec3 m_RotationVelocity{};
    };

    DEFINE_SERIALIZATION(RigidBody) {
        SERIALIZE_BASE_CLASS(RenderedEntity);

        SERIALIZE(glm::vec3, m_Velocity);
        SERIALIZE(glm::vec3, m_Force);

        SERIALIZE(glm::vec3, m_Torque);
        SERIALIZE(glm::vec3, m_RotationVelocity);
    }

}

#endif