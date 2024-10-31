#ifndef RIGID_BODY_H
#define RIGID_BODY_H

#include "../entities/rendered_entity.h"

namespace gigno {

    class RigidBody : public RenderedEntity {
        BEGIN_SERIALIZE(RigidBody, RenderedEntity)
        SERIALIZE(glm::vec3, m_Force);
        SERIALIZE(glm::vec3, m_Velocity);
        END_SERIALIZE
    public:
        RigidBody(ModelData_t model);
        ~RigidBody();

        virtual void Think(float dt) override;

        void AddForce(const glm::vec3& force) {
            m_Force += force;
        }

        void Stop() {
            m_Velocity = glm::vec3{0.0f};
        }

        int TestingInterpolateType{};

    private:
        glm::vec3 m_Velocity{};
        glm::vec3 m_Force{};
    };

}

#endif