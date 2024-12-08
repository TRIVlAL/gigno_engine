#include "collider.h"
#include "../error_macros.h"
#include "rigid_body.h"

namespace gigno {

    void Collider::PollRigidBodyValues() {
        if(BoundRigidBody)  {
            Position = BoundRigidBody->Transform.Position;
            Rotation = BoundRigidBody->Transform.Rotation;
            Velocity = BoundRigidBody->GetVelocity();
            Mass = BoundRigidBody->Mass;
            Bounciness = BoundRigidBody->Bounciness;
            IsStatic = BoundRigidBody->IsStaitc;
        }
    }

    void Collider::ApplyImpulse() {
        if(BoundRigidBody) {
            BoundRigidBody->AddImpulse(Impulse);
            BoundRigidBody->AddRotationImpulse(AngularImpulse);
            BoundRigidBody->Transform.Position += PosOffset;
        }

        Impulse = glm::vec3{0};
        AngularImpulse = glm::vec3{0};
        PosOffset = glm::vec3{0};
    }

    bool ResolveCollision(Collider &col1, Collider &col2) {
        if(col2.type < col1.type) {
            Collider &intermediate = col1;
            col1 = col2;
            col2 = intermediate;
        }

        ASSERT_MSG_V(col1.type != COLLIDER_NONE && col2.type != COLLIDER_NONE, false, "Physics ResolveCollision : A collider was of the base class !");

        if(col1.type == COLLIDER_SPHERE) {
            if(col2.type == COLLIDER_SPHERE) {
                return ResolveCollision_SphereSphere(col1, col2);
            } else if(col2.type == COLLIDER_PLANE) {
                return ResolveCollision_SpherePlane(col1, col2);
            }
        } else if(col1.type == COLLIDER_PLANE) {

        }
        
        return false;
    }

    bool ResolveCollision_SphereSphere(Collider &col1, Collider &col2) {

        glm::vec3 dis = col2.Position - col1.Position;
        float dis_len = glm::length(dis);
        float col_depth = dis_len - col1.parameters.Radius - col2.parameters.Radius;

        if(col_depth >= 0) {
            // Not Colliding
            return false; 
        }

        if(col1.BoundRigidBody && col2.BoundRigidBody) {
            // Apply collision response only if Rigidbodies are bound.
            glm::vec3 dis_norm = dis / dis_len;

            RespondCollision(col1, col2, dis_norm, col_depth, dis_norm * col1.parameters.Radius, -dis_norm * col2.parameters.Radius);
        }

        return true;
    }

    bool ResolveCollision_SpherePlane(Collider &col1, Collider &col2) {

        float height = glm::dot(col1.Position - col2.Position, col2.parameters.Normal);
        float col_depth = height - col1.parameters.Radius;


        if (col_depth >= 0 ||
            col_depth < -2.0f * col1.parameters.Radius /*Under the plane*/ )
        {
            return false;
        }

        // col2 Plane are always constidered static !
        if(col1.BoundRigidBody) {
            /*float e = (col1.Bounciness + col2.Bounciness) / 2.0f;
            float J = glm::dot(-col1.Velocity, col2.parameters.Normal) * (1+e);
            col1.Impulse += J * col2.parameters.Normal;
            col1.AngularImpulse += glm::cross(-col2.parameters.Normal * col1.parameters.Radius, col2.parameters.Normal * J);

            glm::vec3 new_velocity = col1.Velocity + J * col2.parameters.Normal;
            glm::vec3 movement_along_plane_dir = glm::normalize(new_velocity - col2.parameters.Normal * glm::dot(col2.parameters.Normal, new_velocity));
            col1.Impulse += -movement_along_plane_dir * glm::length(J * col2.parameters.Normal) * 0.1f;
            col1.AngularImpulse += glm::cross(-col2.parameters.Normal * col1.parameters.Radius, -movement_along_plane_dir * glm::length(J * col2.parameters.Normal) * 0.1f);

            if (col_depth < 0.2f)
            {
                // Offset the object to the edge of the plane!
                col1.BoundRigidBody->Transform.Position += col2.parameters.Normal * -col_depth / 2.0f;
            }*/
           RespondCollision(col1, col2, -col2.parameters.Normal, col_depth, -col2.parameters.Normal * col1.parameters.Radius, glm::vec3{0.0f});
        }

        return true;
    }

    void RespondCollision(Collider &col1, Collider &col2, const glm::vec3 &colNormal, const float &colDepth,
                          const glm::vec3 &col1ApplyPoint, const glm::vec3 &col2ApplyPoint)
    {

        if (glm::dot(col1.Velocity, colNormal) <= 0 && glm::dot(col2.Velocity, -colNormal) <= 0)
        {
            // Object are moving away from each other,
            // Don't re-add any impulse.
            return;
        }

        if(col1.IsStatic && col2.IsStatic) {
            return;
        } else {
            glm::vec3 Delta_v = col2.Velocity - col1.Velocity;
            float ProjD_v = glm::dot(Delta_v, colNormal);

            float e = (col1.Bounciness + col2.Bounciness) / 2.0f;
    
            float J = (1.0 + e) * ProjD_v / (col1.Mass + col2.Mass);

            if(col1.IsStatic) {
                col2.Impulse += -colNormal * J * (col1.Mass + col2.Mass);

                if(colDepth < 0.2f) {
                    col2.PosOffset += -colNormal * colDepth;
                }
            
                glm::vec3 tangent_move = glm::normalize(col2.Velocity + glm::dot(colNormal, col2.Velocity));
                col2.Impulse += tangent_move * J * (col1.Mass + col2.Mass) * 0.5f;
                col2.AngularImpulse += glm::cross(col2ApplyPoint, tangent_move * J * (col1.Mass + col2.Mass));
            } 
            else if(col2.IsStatic) {
                col1.Impulse += colNormal * J * (col1.Mass + col2.Mass);

                if(colDepth < 0.2f) {
                    col1.PosOffset += colNormal * colDepth;
                }

                glm::vec3 tangent_move = glm::normalize(col1.Velocity + glm::dot(-colNormal, col1.Velocity));
                col1.Impulse += tangent_move * J * (col1.Mass + col2.Mass) * 0.5f;
                col1.AngularImpulse += glm::cross(col1ApplyPoint, tangent_move * J * (col1.Mass + col2.Mass));
            } 
            else {
                col1.Impulse += colNormal * J * col2.Mass;
                col1.AngularImpulse += glm::cross(col1ApplyPoint * J * col2.Mass, colNormal * J * col2.Mass);

                col2.Impulse += -colNormal * J * col1.Mass;
                col2.AngularImpulse += glm::cross(col2ApplyPoint * J * col2.Mass, colNormal * J * col1.Mass);

                if (colDepth < 0.2f)
                {
                    // Offset the object out of the collision to avoid double apply on consecutive frame when working witih tiny velocity!
                    col1.PosOffset += colNormal * colDepth / 2.0f;
                    col2.PosOffset += -colNormal * colDepth / 2.0f;
                }

                glm::vec3 tangent_move = glm::normalize(col1.Velocity + glm::dot(colNormal, col1.Velocity));
                col1.Impulse += tangent_move * J * col2.Mass * 0.5f;
                col1.AngularImpulse += glm::cross(col1ApplyPoint, tangent_move * J * col2.Mass);

                tangent_move = glm::normalize(col2.Velocity + glm::dot(-colNormal, col2.Velocity));
                col2.Impulse += tangent_move * J * col1.Mass * 0.5f;
                col2.AngularImpulse += glm::cross(col2ApplyPoint, tangent_move * J * col1.Mass);
            }

        }
    }
}