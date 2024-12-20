#include "collision.h"
#include "../error_macros.h"
#include "rigid_body.h"
#include "../debug/console/convar.h"
#include "../algorithm/geometry.h"
#include <utility>

namespace gigno {

    CONVAR(float, phys_air_density, 1.225f, "Higher means higher drag");

    void ApplyDrag(RigidBody &rb) {
        float velocity_len = glm::length(rb.GetVelocity());
        rb.AddForce(-rb.GetVelocity() * velocity_len * 0.5f * 
                                (float)convar_phys_air_density * GetDragCoefficient(rb) * 
                                GetAreaCrossSection(rb, -rb.GetVelocity()/velocity_len));

        // TODO : Also apply angular drag.
        
    }

    float GetDragCoefficient(RigidBody &rb) {
        switch(rb.ColliderType) {
            case COLLIDER_SPHERE:
                return 0.48f;
            case COLLIDER_PLANE:
                return 0.0f; // Planes area static so we wont use it anyway.
            case COLLIDER_CAPSULE:
                return 0.7f;
            case COLLIDER_NONE:
                return 0.0;
        }

        ERR_MSG_V(0.0f, "Collider with no type is querrying DragCoefficient !");
    }

    float GetAreaCrossSection(RigidBody &rb, const glm::vec3 &direction)
    {
        if(rb.ColliderType == COLLIDER_SPHERE) {
            return glm::pi<float>() * rb.Radius *rb.Radius;
        } else if(rb.ColliderType == COLLIDER_PLANE) {
            return INFINITY; // Planes are static so we wont use it anyway.
        } else if(rb.ColliderType == COLLIDER_CAPSULE) {
            // If moving vertically, only pi * r^2. If moving horizontally, add r * length.
            const float t = glm::dot(direction, ApplyRotation(rb.Rotation, glm::vec3{0.0f, 1.0f, 0.0f}));
            return rb.Length * rb.Radius * 2.0f * t 
                    + glm::pi<float>() * rb.Radius * rb.Radius;  
        } else {
            ERR_MSG_V(0.0f, "Collider with no type is querrying GetAreaCrossSection !");
        }
    }

    bool ResolveCollision(RigidBody &rb1, RigidBody &rb2) {
        if(rb1.ColliderType == COLLIDER_NONE || rb2.ColliderType == COLLIDER_NONE) {
            return false;
        }

        RigidBody &a = rb1.ColliderType <= rb2.ColliderType ? rb1 : rb2;
        RigidBody &b = rb1.ColliderType <= rb2.ColliderType ? rb2 : rb1;

        if(a.ColliderType == COLLIDER_SPHERE) {
            if(b.ColliderType == COLLIDER_SPHERE) {
                return ResolveCollision_SphereSphere(a, b);
            } else if(b.ColliderType == COLLIDER_PLANE) {
                return ResolveCollision_SpherePlane(a, b);
            } else if(b.ColliderType == COLLIDER_CAPSULE) {
                return ResolveCollision_SphereCapsule(a, b);
            }
        } else if(a.ColliderType == COLLIDER_PLANE) {
            if(b.ColliderType == COLLIDER_CAPSULE) {
                return ResolveCollision_PlaneCapsule(a, b);
            }
        } else if(a.ColliderType == COLLIDER_CAPSULE) {
            if(b.ColliderType == COLLIDER_CAPSULE) {
                return ResolveCollision_CapsuleCapsule(a, b);
            }
        }
        
        return false;
    }

    bool ResolveCollision_SphereSphere(RigidBody &rb1, RigidBody &rb2) {

        glm::vec3 dis = rb2.Position - rb1.Position;
        float dis_len = glm::length(dis);
        float col_depth = dis_len - rb1.Radius - rb2.Radius;

        if(col_depth >= 0) {
            // Not Colliding
            return false; 
        }

        // Apply collision response only if Rigidbodies are bound.
        glm::vec3 dis_norm = dis / dis_len;
        RespondCollision(rb1, rb2, dis_norm, col_depth, dis_norm * rb1.Radius, -dis_norm * rb2.Radius);

        return true;
    }

    bool ResolveCollision_SpherePlane(RigidBody &sphere, RigidBody &plane) {

        float height = glm::dot(sphere.Position - plane.Position, plane.Normal);
        float col_depth = height - sphere.Radius;

        if (col_depth >= 0 ||
            col_depth < -2.0f * sphere.Radius /*Under the plane*/)
        {
            return false;
        }

        RespondCollision(sphere, plane, -plane.Normal, col_depth, -plane.Normal * sphere.Radius, glm::vec3{0.0f});

        return true;
    }

    bool ResolveCollision_SphereCapsule(RigidBody &sphere, RigidBody &capsule) {
        const glm::vec3 capsule_orientation = ApplyRotation(capsule.Rotation, glm::vec3{0.0f, 1.0f, 0.0f});
        const glm::vec3 capsule_bottom = capsule.Position - capsule.Length * 0.5f * capsule_orientation;
        const glm::vec3 capsule_top = capsule.Position + capsule.Length * 0.5f * capsule_orientation;
        const glm::vec3 sphere_to_capsule = PointToSegment(sphere.Position, capsule_bottom, capsule_top);

        Application::Singleton()->GetRenderer()->DrawLine(sphere.Position, sphere.Position + sphere_to_capsule, glm::vec3{0.0f, 1.0f, 0.0f});

        const float distance = glm::length(sphere_to_capsule);
        const float col_depth = distance - sphere.Radius - capsule.Radius;

        if(col_depth >= 0) {
            //Not colliding
            return false;
        }

         const glm::vec3 sphere_to_capsule_norm = sphere_to_capsule / distance;

        RespondCollision(sphere, capsule, sphere_to_capsule / distance, col_depth, 
                        sphere_to_capsule_norm * sphere.Radius, -sphere_to_capsule_norm * capsule.Radius);

        return true;
    }

    bool ResolveCollision_PlaneCapsule(RigidBody &plane, RigidBody &capsule) {
        const glm::vec3 capsule_orientation = ApplyRotation(capsule.Rotation, glm::vec3{0.0f, 1.0f, 0.0f});
        const glm::vec3 capsule_bottom = capsule.Position - capsule.Length * 0.5f * capsule_orientation;
        const glm::vec3 capsule_top = capsule.Position + capsule.Length * 0.5f * capsule_orientation;

        const float bottom_depth = glm::dot(capsule_bottom - plane.Position, plane.Normal) - capsule.Radius;
        const float top_depth = glm::dot(capsule_top - plane.Position, plane.Normal) - capsule.Radius;

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

        capsule_application_point += -plane.Normal * capsule.Radius;

        Application::Singleton()->GetRenderer()->DrawLine(capsule.Position + capsule_application_point + glm::vec3{0.0f, -100.0f, 0.0f}, capsule.Position + capsule_application_point + glm::vec3{0.0f, 100.0f, 0.0f}, glm::vec3{1.0f, 0.0f, 0.0f});

        RespondCollision(plane, capsule, plane.Normal, glm::min(bottom_depth, top_depth),
                        capsule.Position + capsule_application_point - plane.Position - plane.Normal * glm::dot(capsule.Position + capsule_application_point - plane.Position, plane.Normal),
                        capsule_application_point);

        return true;
    }

    bool ResolveCollision_CapsuleCapsule(RigidBody &rb1, RigidBody &rb2) {
        const glm::vec3 orientation1 = ApplyRotation(rb1.Rotation, glm::vec3{0.0f, 1.0f, 0.0f});
        const glm::vec3 bottom1 = rb1.Position - rb1.Length * 0.5f * orientation1;
        const glm::vec3 top1 = rb1.Position + rb1.Length * 0.5f * orientation1;

        const glm::vec3 orientation2 = ApplyRotation(rb2.Rotation, glm::vec3{0.0f, 1.0f, 0.0f});
        const glm::vec3 bottom2 = rb2.Position - rb2.Length * 0.5f * orientation2;
        const glm::vec3 top2 = rb2.Position + rb2.Length * 0.5f * orientation2;

        glm::vec3 point1{};
        glm::vec3 point2{};
        SegmentsClosestPoints(bottom1, top1, bottom2, top2, point1, point2);

        Application::Singleton()->GetRenderer()->DrawLine(point1 + glm::vec3{2.0f, 0.0f, 2.0f}, point2 + glm::vec3{2.0f, 0.0f, 2.0f}, glm::vec3{0.0f, 0.0f, 1.0f});

        const glm::vec3 dist = point2 - point1;
        const float dist_len = glm::length(dist);

        const float col_depth = dist_len - rb1.Radius - rb2.Radius;

        if (col_depth >= 0)
        {
            // Not colliding
            return false;
        }

        const glm::vec3 dist_norm = dist / dist_len;

        RespondCollision(rb1, rb2, dist_norm, col_depth,
                        point1 - rb1.Position + dist_norm * rb1.Radius, point2 - rb2.Position - dist_norm * rb2.Radius);

        return true;
    }

    void RespondCollision(RigidBody &rb1, RigidBody &rb2, const glm::vec3 &colNormal, const float &colDepth,
                          const glm::vec3 &col1ApplyPoint, const glm::vec3 &col2ApplyPoint)
    {

        if (glm::dot(rb1.GetVelocity(), colNormal) <= 0 && glm::dot(rb2.GetVelocity(), -colNormal) <= 0)
        {
            // Object are moving away from each other,
            // Don't re-add any impulse.
            return;
        }

        if(rb1.IsStatic && rb2.IsStatic) {
            return;
        } else {
            glm::vec3 Delta_v = rb2.GetVelocity() - rb1.GetVelocity();
            float ProjD_v = glm::dot(Delta_v, colNormal);

            float e = GetBounciness((PhysicsMaterial_t)rb1.Material) * GetBounciness((PhysicsMaterial_t)rb2.Material);
    
            float J = (1.0 + e) * ProjD_v / (rb1.Mass + rb2.Mass);

            if(rb1.IsStatic) {
                rb2.AddImpulse(-colNormal * J * (rb1.Mass + rb2.Mass), col2ApplyPoint);

                if(colDepth < 0.2f) {
                    rb2.PositionOffset += -colNormal * colDepth;
                    if(glm::length(rb1.PositionOffset) > 5.0f || glm::length(rb2.PositionOffset) > 5.0f) {
                        int i = 0;
                    }
                }

                ApplyFriction(J * (rb1.Mass + rb2.Mass), rb2, colNormal, col2ApplyPoint, GetDynamicFrictionCoefficient((PhysicsMaterial_t)rb1.Material, (PhysicsMaterial_t)rb2.Material));
            } 
            else if(rb2.IsStatic) {
                rb1.AddImpulse(colNormal * J * (rb1.Mass + rb2.Mass), col1ApplyPoint);

                if(colDepth < 0.2f) {
                    rb1.PositionOffset += colNormal * colDepth;
                    if(glm::length(rb1.PositionOffset) > 5.0f || glm::length(rb2.PositionOffset) > 5.0f) {
                        int i = 0;
                    }
                }

                ApplyFriction(J * (rb1.Mass + rb2.Mass), rb1, -colNormal, col1ApplyPoint,  GetDynamicFrictionCoefficient((PhysicsMaterial_t)rb1.Material, (PhysicsMaterial_t)rb2.Material));
            } 
            else {
                rb1.AddImpulse(colNormal * J * rb2.Mass, col1ApplyPoint);
                /*col1ApplyPoint * J * col2.Mass*/

                rb2.AddImpulse(-colNormal * J * rb1.Mass, col2ApplyPoint);
                /*col2ApplyPoint * J * col2.Mass*/

                if (colDepth < 0.2f)
                {
                    // Offset the object out of the collision to avoid double apply on consecutive frame when working witih tiny velocity!
                    rb1.PositionOffset += colNormal * colDepth / 2.0f;
                    rb2.PositionOffset += -colNormal * colDepth / 2.0f;
                    if(glm::length(rb1.PositionOffset) > 5.0f || glm::length(rb2.PositionOffset) > 5.0f) {
                        int i = 0;
                    }
                }

                ApplyFriction(J * rb2.Mass, rb1, -colNormal, col1ApplyPoint,  GetDynamicFrictionCoefficient((PhysicsMaterial_t)rb1.Material, (PhysicsMaterial_t)rb2.Material));

                ApplyFriction(J * rb1.Mass, rb2, colNormal, col2ApplyPoint,  GetDynamicFrictionCoefficient((PhysicsMaterial_t)rb1.Material, (PhysicsMaterial_t)rb2.Material));
            }

        }
    }

    void ApplyFriction(float normalImpulse, RigidBody &rb, const glm::vec3 &surfaceNormal, const glm::vec3 &applyPoint, float frictionCoefficient) {
        glm::vec3 tangent_move = rb.GetVelocity() - surfaceNormal * glm::dot(surfaceNormal, rb.GetVelocity());
        float tangent_move_len = glm::length(tangent_move);
        
        if(tangent_move_len != 0.0f) { 
            tangent_move = tangent_move/tangent_move_len;
            float impulse = glm::clamp(glm::abs(normalImpulse * frictionCoefficient), 0.0f, tangent_move_len);
            rb.AddImpulse(-tangent_move * impulse, applyPoint);
        }

    }
}