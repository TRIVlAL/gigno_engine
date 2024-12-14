#include "collider.h"
#include "../error_macros.h"
#include "rigid_body.h"
#include "../debug/console/convar.h"
#include "../algorithm/geometry.h"
#include <utility>

namespace gigno {

    CONVAR(float, phys_air_density, 1.225f, "Higher means higher drag");

    void Collider::PollRigidBodyValues() {
        if(BoundRigidBody)  {
            Position = BoundRigidBody->Transform.Position;
            Rotation = BoundRigidBody->Transform.Rotation;
            Velocity = BoundRigidBody->GetVelocity();
            Mass = BoundRigidBody->Mass;
            IsStatic = BoundRigidBody->IsStaitc;
            Material = BoundRigidBody->Material;
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

    void Collider::ApplyDrag() {
        float velocity_len = glm::length(Velocity);
        BoundRigidBody->AddForce(-Velocity * velocity_len * 0.5f * 
                                (float)convar_phys_air_density * GetDragCoefficient() * 
                                GetAreaCrossSection(-Velocity/velocity_len));

        // TODO : Also apply angular drag.
        
    }

    float Collider::GetDragCoefficient() {
        switch(type) {
            case COLLIDER_SPHERE:
                return 0.48f;
            case COLLIDER_PLANE:
                return 0.0f; // Planes area static so we wont use it anyway.
            case COLLIDER_CAPSULE:
                return 0.7f;
        }

        ERR_MSG_V(0.0f, "Collider with no type is querrying DragCoefficient !");
    }

    float Collider::GetAreaCrossSection(const glm::vec3 &direction)
    {
        if(type == COLLIDER_SPHERE) {
            return glm::pi<float>() * parameters.Radius *parameters.Radius;
        } else if(type == COLLIDER_PLANE) {
            return INFINITY; // Planes area static so we wont use it anyway.
        } else if(type == COLLIDER_CAPSULE) {
            // If moving vertically, only pi * r^2. If moving horizontally, add r * length.
            const float t = glm::dot(direction, ApplyRotation(Rotation, glm::vec3{0.0f, 1.0f, 0.0f}));
            return parameters.Capsule.Length * parameters.Capsule.Radius * 2.0f * t 
                    + glm::pi<float>() * parameters.Capsule.Radius * parameters.Capsule.Radius;  
        } else {
            ERR_MSG_V(0.0f, "Collider with no type is querrying GetAreaCrossSection !");
        }
    }

    bool ResolveCollision(Collider &col1, Collider &col2) {
        if(col2.type < col1.type) {
            std::swap(col1, col2);
        }

        ASSERT_MSG_V(col1.type != COLLIDER_NONE && col2.type != COLLIDER_NONE, false, "Physics ResolveCollision : A collider was of the base class !");

        if(col1.type == COLLIDER_SPHERE) {
            if(col2.type == COLLIDER_SPHERE) {
                return ResolveCollision_SphereSphere(col1, col2);
            } else if(col2.type == COLLIDER_PLANE) {
                return ResolveCollision_SpherePlane(col1, col2);
            } else if(col2.type == COLLIDER_CAPSULE) {
                return ResolveCollision_SphereCapsule(col1, col2);
            }
        } else if(col1.type == COLLIDER_PLANE) {
            if(col2.type == COLLIDER_CAPSULE) {
                return ResolveCollision_PlaneCapsule(col1, col2);
            }
        } else if(col1.type == COLLIDER_CAPSULE) {
            if(col2.type == COLLIDER_CAPSULE) {
                return ResolveCollision_CapsuleCapsule(col1, col2);
            }
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

    bool ResolveCollision_SpherePlane(Collider &sphere, Collider &plane) {

        float height = glm::dot(sphere.Position - plane.Position, plane.parameters.Normal);
        float col_depth = height - sphere.parameters.Radius;

        if (col_depth >= 0 ||
            col_depth < -2.0f * sphere.parameters.Radius /*Under the plane*/)
        {
            return false;
        }

        // col2 Plane are always constidered static !
        if(sphere.BoundRigidBody) {
            RespondCollision(sphere, plane, -plane.parameters.Normal, col_depth, -plane.parameters.Normal * sphere.parameters.Radius, glm::vec3{0.0f});
        }

        return true;
    }

    bool ResolveCollision_SphereCapsule(Collider &sphere, Collider &capsule) {
        const glm::vec3 capsule_orientation = ApplyRotation(capsule.Rotation, glm::vec3{0.0f, 1.0f, 0.0f});
        const glm::vec3 capsule_bottom = capsule.Position - capsule.parameters.Capsule.Length * 0.5f * capsule_orientation;
        const glm::vec3 capsule_top = capsule.Position + capsule.parameters.Capsule.Length * 0.5f * capsule_orientation;
        const glm::vec3 sphere_to_capsule = PointToSegment(sphere.Position, capsule_bottom, capsule_top);

        Application::Singleton()->GetRenderer()->DrawLine(sphere.Position, sphere.Position + sphere_to_capsule, glm::vec3{0.0f, 1.0f, 0.0f});

        const float distance = glm::length(sphere_to_capsule);
        const float col_depth = distance - sphere.parameters.Radius - capsule.parameters.Capsule.Radius;

        if(col_depth >= 0) {
            //Not colliding
            return false;
        }

        if(sphere.BoundRigidBody && capsule.BoundRigidBody) {
            const glm::vec3 sphere_to_capsule_norm = sphere_to_capsule / distance;

            RespondCollision(sphere, capsule, sphere_to_capsule / distance, col_depth, 
                            sphere_to_capsule_norm * sphere.parameters.Radius, -sphere_to_capsule_norm * capsule.parameters.Radius);
        }

        return true;
    }

    bool ResolveCollision_PlaneCapsule(Collider &plane, Collider &capsule) {
        const glm::vec3 capsule_orientation = ApplyRotation(capsule.Rotation, glm::vec3{0.0f, 1.0f, 0.0f});
        const glm::vec3 capsule_bottom = capsule.Position - capsule.parameters.Capsule.Length * 0.5f * capsule_orientation;
        const glm::vec3 capsule_top = capsule.Position + capsule.parameters.Capsule.Length * 0.5f * capsule_orientation;

        const float bottom_depth = glm::dot(capsule_bottom - plane.Position, plane.parameters.Normal) - capsule.parameters.Capsule.Radius;
        const float top_depth = glm::dot(capsule_top - plane.Position, plane.parameters.Normal) - capsule.parameters.Capsule.Radius;

        if(bottom_depth >= 0 && top_depth >= 0) {
            return false;
        }

        glm::vec3 capsule_application_point{};

        if(bottom_depth < 0 && top_depth < 0) {
            // Both underground, the apply point is thoward the one that is deeper, so we weight by their depth.
            capsule_application_point = (((capsule_bottom - capsule.Position) * -bottom_depth) + ((capsule_top - capsule.Position) * -top_depth))/(-bottom_depth - top_depth) ;
        } 
        else if(bottom_depth < 0 && top_depth > 0) {
            // On under, one above. Apply point is between the one under, and the point that meets the ground.
            float total_height = top_depth - bottom_depth;
            float t = -bottom_depth / total_height;
            t = t * 0.5f;

            capsule_application_point = ((capsule_bottom - capsule.Position) * (1-t)) + ((capsule_top - capsule.Position) * (t));
        } 
        else if(top_depth < 0 && top_depth < bottom_depth) {
            // On under, one above. Apply point is between the one under, and the point that meets the ground.
            float total_height = bottom_depth - top_depth;
            float t = -top_depth / total_height;
            t = t * 0.5f;

            capsule_application_point = ((capsule_top - capsule.Position) * (1 - t)) + ((capsule_bottom - capsule.Position) * (t));
        }

        capsule_application_point += -plane.parameters.Normal * capsule.parameters.Radius;

        Application::Singleton()->GetRenderer()->DrawLine(capsule.Position + capsule_application_point + glm::vec3{0.0f, -100.0f, 0.0f}, capsule.Position + capsule_application_point + glm::vec3{0.0f, 100.0f, 0.0f}, glm::vec3{1.0f, 0.0f, 0.0f});

        if(capsule.BoundRigidBody) {
            RespondCollision(plane, capsule, plane.parameters.Normal, glm::min(bottom_depth, top_depth),
                             capsule.Position + capsule_application_point - plane.Position - plane.parameters.Normal * glm::dot(capsule.Position + capsule_application_point - plane.Position, plane.parameters.Normal),
                             capsule_application_point);
        }

        return true;
    }

    bool ResolveCollision_CapsuleCapsule(Collider &col1, Collider &col2) {
        const glm::vec3 orientation1 = ApplyRotation(col1.Rotation, glm::vec3{0.0f, 1.0f, 0.0f});
        const glm::vec3 bottom1 = col1.Position - col1.parameters.Capsule.Length * 0.5f * orientation1;
        const glm::vec3 top1 = col1.Position + col1.parameters.Capsule.Length * 0.5f * orientation1;

        const glm::vec3 orientation2 = ApplyRotation(col2.Rotation, glm::vec3{0.0f, 1.0f, 0.0f});
        const glm::vec3 bottom2 = col2.Position - col2.parameters.Capsule.Length * 0.5f * orientation2;
        const glm::vec3 top2 = col2.Position + col2.parameters.Capsule.Length * 0.5f * orientation2;

        glm::vec3 point1{};
        glm::vec3 point2{};
        SegmentsClosestPoints(bottom1, top1, bottom2, top2, point1, point2);

        Application::Singleton()->GetRenderer()->DrawLine(point1 + glm::vec3{2.0f, 0.0f, 2.0f}, point2 + glm::vec3{2.0f, 0.0f, 2.0f}, glm::vec3{0.0f, 0.0f, 1.0f});

        const glm::vec3 dist = point2 - point1;
        const float dist_len = glm::length(dist);

        const float col_depth = dist_len - col1.parameters.Capsule.Radius - col2.parameters.Capsule.Radius;

        if (col_depth >= 0)
        {
            // Not colliding
            return false;
        }

        if (col1.BoundRigidBody && col2.BoundRigidBody)
        {
            const glm::vec3 dist_norm = dist / dist_len;

            RespondCollision(col1, col2, dist_norm, col_depth,
                             point1 - col1.Position + dist_norm * col1.parameters.Capsule.Radius, point2 - col2.Position - dist_norm * col2.parameters.Capsule.Radius);
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

            float e = GetBounciness(col1.Material) * GetBounciness(col2.Material);
    
            float J = (1.0 + e) * ProjD_v / (col1.Mass + col2.Mass);

            if(col1.IsStatic) {
                col2.Impulse += -colNormal * J * (col1.Mass + col2.Mass);
                col2.AngularImpulse += glm::cross(col2ApplyPoint, -colNormal * J * (col1.Mass + col2.Mass));

                if(colDepth < 2.0f) {
                    col2.PosOffset += -colNormal * colDepth;
                }

                ApplyFriction(J * (col1.Mass + col2.Mass), col2, colNormal, col2ApplyPoint, GetDynamicFrictionCoefficient(col1.Material, col2.Material));
            } 
            else if(col2.IsStatic) {
                col1.Impulse += colNormal * J * (col1.Mass + col2.Mass);
                col1.AngularImpulse += glm::cross(col1ApplyPoint, colNormal * J * (col1.Mass + col2.Mass));

                if(colDepth < 0.2f) {
                    col1.PosOffset += colNormal * colDepth;
                }

                ApplyFriction(J * (col1.Mass + col2.Mass), col1, -colNormal, col1ApplyPoint,  GetDynamicFrictionCoefficient(col1.Material, col2.Material));
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

                ApplyFriction(J * col2.Mass, col1, -colNormal, col1ApplyPoint,  GetDynamicFrictionCoefficient(col1.Material, col2.Material));

                ApplyFriction(J * col1.Mass, col2, colNormal, col2ApplyPoint,  GetDynamicFrictionCoefficient(col1.Material, col2.Material));
            }

        }
    }

    void ApplyFriction(float normalImpulse, Collider &col, const glm::vec3 &surfaceNormal, const glm::vec3 &applyPoint, float frictionCoefficient) {
        glm::vec3 tangent_move = col.Velocity - surfaceNormal * glm::dot(surfaceNormal, col.Velocity);
        float tangent_move_len = glm::length(tangent_move);
        
        if(tangent_move_len != 0.0f) { 
            tangent_move = tangent_move/tangent_move_len;
            float impulse = glm::clamp(glm::abs(normalImpulse * frictionCoefficient), 0.0f, tangent_move_len);
            col.Impulse += -tangent_move * impulse;
            col.AngularImpulse += glm::cross(applyPoint, -tangent_move * impulse);
        }

    }
}