#include "collider.h"
#include "../error_macros.h"
#include "rigid_body.h"

namespace gigno {

    void ResolveCollision(Collider &col1, Collider &col2) {
        if(col2.type < col1.type) {
            Collider &intermediate = col1;
            col1 = col2;
            col2 = intermediate;
        }

        ASSERT_MSG(col1.type != COLLIDER_NONE && col2.type != COLLIDER_NONE, "Physics ResolveCollision : A collider was of the base class !");

        if(col1.type == COLLIDER_SPHERE) {
            if(col2.type == COLLIDER_SPHERE) {
                ResolveCollision_SphereSphere(col1, col2);
            } else if(col2.type == COLLIDER_PLANE) {
                ResolveCollision_SpherePlane(col1, col2);
            }
        } else if(col1.type == COLLIDER_PLANE) {

        }
        
    }

    void ResolveCollision_SphereSphere(Collider &col1, Collider &col2) {
        glm::vec3 pos1 = col1.boundRigidBody->Transform.Position;
        glm::vec3 pos2 = col2.boundRigidBody->Transform.Position;

        glm::vec3 dis = pos2 - pos1;
        float dis_len = glm::length(dis);
        float col_depth = dis_len - col1.parameters.Radius - col2.parameters.Radius;

        if(col_depth < 0) {
            // It is colliding
            glm::vec3 dis_norm = dis / dis_len;

            glm::vec3 vr = col1.boundRigidBody->GetVelocity() - col2.boundRigidBody->GetVelocity();
            float J = glm::abs(glm::dot(-(1.0f)*vr, dis_norm))/((1.0f/col1.boundRigidBody->Mass) + (1.0f/col2.boundRigidBody->Mass));

            col1.boundRigidBody->AddImpulse(-dis_norm * J / col1.boundRigidBody->Mass, pos1 + dis_norm * col1.parameters.Radius);
            col2.boundRigidBody->AddImpulse(dis_norm * J / col2.boundRigidBody->Mass, pos2 - dis_norm * col2.parameters.Radius);
        }
    }

    void ResolveCollision_SpherePlane(Collider &col1, Collider &col2) {

    }
}