#include "collision.h"
#include "../error_macros.h"
#include "rigid_body.h"
#include "../debug/console/convar.h"
#include "../algorithm/geometry.h"
#include <utility>
#include "gjk.h"

namespace gigno {

    bool ResolveCollision(RigidBody &rb1, RigidBody &rb2) {
        if(rb1.ColliderType == COLLIDER_NONE || rb2.ColliderType == COLLIDER_NONE) {
            return false;
        }

        RigidBody &a = rb1.ColliderType <= rb2.ColliderType ? rb1 : rb2;
        RigidBody &b = rb1.ColliderType <= rb2.ColliderType ? rb2 : rb1;

        // COLLIDER_HULL is the first enum entry
        if(a.ColliderType == COLLIDER_HULL) {
            if(b.ColliderType == COLLIDER_PLANE) {
                return ResolveCollision_HullPlane(a, b);
            } else {
                return ResolveCollision_HullNonPlane(a, b);
            }
        } else if(a.ColliderType == COLLIDER_SPHERE) {
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

        RespondCollision(plane, capsule, plane.Normal, glm::min(bottom_depth, top_depth),
                        capsule.Position + capsule_application_point - plane.Position - plane.Normal * glm::dot(capsule.Position + capsule_application_point - plane.Position, plane.Normal),
                        capsule_application_point);

        return true;
    }

    bool ResolveCollision_CapsuleCapsule(RigidBody &rb1, RigidBody &rb2) {
        const glm::vec3 orientation1 = ApplyRotation(rb1.Rotation, glm::vec3{0.0f, 1.0f, 0.0f});
        const glm::vec3 bottom1 = rb1.Position - (rb1.Length * 0.5f * orientation1);
        const glm::vec3 top1 = rb1.Position + (rb1.Length * 0.5f * orientation1);

        const glm::vec3 orientation2 = ApplyRotation(rb2.Rotation, glm::vec3{0.0f, 1.0f, 0.0f});
        const glm::vec3 bottom2 = rb2.Position - (rb2.Length * 0.5f * orientation2);
        const glm::vec3 top2 = rb2.Position + (rb2.Length * 0.5f * orientation2);

        glm::vec3 point1{};
        glm::vec3 point2{};
        SegmentsClosestPoints(bottom1, top1, bottom2, top2, point1, point2);

        const glm::vec3 dist = point2 - point1;
        const float dist_len = glm::length(dist);

        const float col_depth = dist_len - rb1.Radius - rb2.Radius;

        if (col_depth >= 0) {
            // Not colliding
            return false;
        }

        const glm::vec3 dist_norm = dist / dist_len;

        const glm::vec3 edge_point1 = point1 + (dist_norm * rb1.Radius);
        const glm::vec3 edge_point2 = point2 - (dist_norm * rb2.Radius);

        const glm::vec3 middle = (edge_point1 + edge_point2) * 0.5f;

        RespondCollision(rb1, rb2, dist_norm, col_depth,
                        middle - rb1.Position, middle - rb2.Position - (dist_norm * rb2.Radius));

        return true;
    }

    bool ResolveCollision_HullNonPlane(RigidBody &hull, RigidBody &nonPlane) {
        Simplex_t simplex{};
        bool collide = GJK(hull, nonPlane, simplex);

        //TODO : test if SAT is faster than GJK?
        //TODO : try out GJK with a margin to avoid calling EPA for small penetration...
        
        if(!collide) {
            return false;
        }

        glm::vec3 pointA{};
        glm::vec3 pointB{};
        glm::vec3 dir{};
        float depth;
        EPA(hull, nonPlane, simplex, pointA, pointB, dir, depth);

        dir = glm::dot(pointB - pointA, dir) >= 0 ? -dir : dir;

        RespondCollision(hull, nonPlane, glm::normalize(pointB - pointA), depth, pointA - hull.Position, pointB - nonPlane.Position);
        
        return true;
    }

    bool ResolveCollision_HullPlane(RigidBody &hull, RigidBody &Plane) {

        //TODO : Compute hull vertices in world space ONE per iteraration instead of on the fly.

        std::vector<glm::vec3> &vert = hull.Hull.Vertices;
        std::vector<int> &ind = hull.Hull.Indices;

        std::vector<std::pair<bool, float>> heights(vert.size());

        size_t slice_vert_count{};
        glm::vec3 slice_sum{};

        float max_depth = 0;
        bool got_under = false;

        for(size_t i = 0; i < ind.size(); i += 3) {
            for(int j = 0; j < 3; j++) {
                int indice_a = ind[i+j];
                int indice_b = ind[j == 2 ? i : i+j+1];
                

                if(!heights[indice_a].first) {
                    //Need to calculate the height
                    heights[indice_a].first = true;
                    heights[indice_a].second = glm::dot(hull.Position + ApplyRotation(hull.Rotation, vert[indice_a]) - Plane.Position, Plane.Normal);
                    if(heights[indice_a].second < max_depth) {
                        max_depth = heights[indice_a].second;
                    }
                }
                if(!heights[indice_b].first) {
                    //Need to calculate the height
                    heights[indice_b].first = true;
                    heights[indice_b].second = glm::dot(hull.Position + ApplyRotation(hull.Rotation, vert[indice_b]) - Plane.Position, Plane.Normal);
                    if(heights[indice_b].second < max_depth) {
                        max_depth = heights[indice_b].second;
                    }
                }

                if(!got_under) {
                    got_under = heights[indice_a].second <= 0.0f || heights[indice_b].second <= 0.0f;
                }

                if((heights[indice_a].second > 0.0f && heights[indice_b].second > 0.0f /*Above*/)
                || (heights[indice_a].second <= 0.0f && heights[indice_b].second <= 0.0f /*Bellow*/)) {
                    continue;
                }

                if(i== 0) {
                    int k = 0;
                }


                float total_height = glm::abs(heights[indice_a].second - heights[indice_b].second);
                float floor_height = glm::abs(heights[indice_a].second);

                glm::vec3 slice_pos = ((total_height - floor_height) * (hull.Position + ApplyRotation(hull.Rotation, vert[indice_a]))
                             +  floor_height * (hull.Position + ApplyRotation(hull.Rotation, vert[indice_b]))) / total_height;

                slice_sum += slice_pos;

                slice_vert_count++;
            }
        }

        if(!got_under) {
            return false;
        }

        RespondCollision(hull, Plane, Plane.Normal, -max_depth, slice_sum / (float)slice_vert_count - hull.Position, glm::vec3{0.0f});

        return false;
    }

    void RespondCollision(RigidBody &rb1, RigidBody &rb2, const glm::vec3 &colNormal, const float &colDepth,
                          const glm::vec3 &col1ApplyPoint, const glm::vec3 &col2ApplyPoint)
    {
        const float epsilon = 0.0000000001;

        //Is this ok???
        const glm::vec3 point_velocity1 = rb1.Velocity + glm::cross(rb1.AngularVelocity, col1ApplyPoint);
        const glm::vec3 point_velocity2 = rb2.Velocity + glm::cross(rb2.AngularVelocity, col2ApplyPoint);
        if (glm::dot(point_velocity1, colNormal) < -epsilon && glm::dot(point_velocity2, -colNormal) < -epsilon)
        {
            // Object are moving away from each other,
            // Don't re-add any impulse.
            return;
        }

        if(rb1.IsStatic && rb2.IsStatic) {
            return;
        } else {
            glm::vec3 Delta_v = rb2.Velocity - rb1.Velocity;
            float ProjD_v = glm::dot(Delta_v, colNormal);

            float e = GetBounciness((PhysicsMaterial_t)rb1.Material) * GetBounciness((PhysicsMaterial_t)rb2.Material);
    
            float J = (1.0 + e) * ProjD_v / (rb1.Mass + rb2.Mass);

            if(rb1.IsStatic) {
                rb2.AddImpulse(-colNormal * J * (rb1.Mass + rb2.Mass), col2ApplyPoint);

                if(colDepth < 2.0f) {
                    rb2.PositionOffset += -colNormal * colDepth;
                    if(glm::length(rb1.PositionOffset) > 5.0f || glm::length(rb2.PositionOffset) > 5.0f) {
                        int i = 0;
                    }
                }

                ApplyFriction(J * (rb1.Mass + rb2.Mass), rb2, colNormal, col2ApplyPoint, GetDynamicFrictionCoefficient((PhysicsMaterial_t)rb1.Material, (PhysicsMaterial_t)rb2.Material));
            } 
            else if(rb2.IsStatic) {
                rb1.AddImpulse(colNormal * J * (rb1.Mass + rb2.Mass), col1ApplyPoint);

                if(colDepth < 2.0f) {
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

                if (colDepth < 2.0f)
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
        const glm::vec3 point_velocity = rb.Velocity + glm::cross(rb.AngularVelocity, applyPoint);
        glm::vec3 tangent_velocity = point_velocity - surfaceNormal * glm::dot(surfaceNormal, point_velocity);
        const float tangent_vel_len = glm::length(tangent_velocity);
        
        if(tangent_vel_len != 0.0f) { 
            tangent_velocity = tangent_velocity/tangent_vel_len;
            float impulse = glm::clamp(glm::abs(normalImpulse * frictionCoefficient), 0.0f, tangent_vel_len);
            rb.AddImpulse(-tangent_velocity * impulse, applyPoint);
        }

    }
}