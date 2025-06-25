#include "collision.h"
#include "../error_macros.h"
#include "rigid_body.h"
#include "../debug/console/convar.h"
#include "../algorithm/geometry.h"
#include <utility>
#include "gjk.h"
#include "physic_server.h"

namespace gigno {

    Collider_t::Collider_t(glm::vec3 position, glm::quat rotation, glm::vec3 scale, float radius) {
        Position = position; Rotation = rotation; Scale = scale; Radius = radius;
        ColliderType = COLLIDER_SPHERE;
        SetBoundingBox();
    }
    
    Collider_t::Collider_t(glm::vec3 position, glm::quat rotation, glm::vec3 scale, float radius, float length) {
        Position = position; Rotation = rotation; Scale = scale; Radius = radius; Length = length;
        ColliderType = COLLIDER_CAPSULE;
        SetBoundingBox();
    }
    
    Collider_t::Collider_t(glm::vec3 position, glm::quat rotation, glm::vec3 scale, glm::vec3 normal) {
        Position = position; Rotation = rotation; Scale = scale; Normal = normal;
        ColliderType = COLLIDER_PLANE;
        SetBoundingBox();
    }
    
    Collider_t::Collider_t(glm::vec3 position, glm::quat rotation, glm::vec3 scale, const CollisionModel_t *model) {
        Position = position; Rotation = rotation; Scale = scale; Model = model;
        ColliderType = COLLIDER_HULL;
        SetTransformedModel();
        SetBoundingBox();
    }

    void Collider_t::SetTransformedModel() {
        ASSERT(ColliderType == COLLIDER_HULL);

        TransformedModel.resize(Model->Vertices.size());

        for (size_t i = 0; i < Model->Vertices.size(); i++) {
            glm::vec3 vert = Model->Vertices[i];
            vert.x *= Scale.x;
            vert.y *= Scale.y;
            vert.z *= Scale.z;
            TransformedModel[i] = ApplyRotation(Rotation, vert);
        }
    }

    void Collider_t::SetBoundingBox() {
        float epsilon = 0.0001f;
        switch(ColliderType) {
            case COLLIDER_SPHERE : {
                AABB.Min = Position + glm::vec3{-Radius};
                AABB.Max = Position + glm::vec3{Radius};
                break;
            }
            case COLLIDER_CAPSULE : {
                glm::vec3 up = ApplyRotation(Rotation, glm::vec3{0.0f, 1.0f, 0.0f});
                glm::vec3 diag = ApplyRotation(Rotation, glm::vec3{0.707f, 0.0f, .707f});
                AABB.Min = Position - (up * Length + Radius) - diag * Radius;
                AABB.Max = Position + (up * Length + Radius) + diag * Radius;
                break;
            }
            case COLLIDER_HULL : {
                AABB.Min = glm::vec3{FLT_MAX};
                AABB.Max = glm::vec3{-FLT_MAX};
                for(glm::vec3 vert : TransformedModel) {
                    const glm::vec3 world_vert = Position + vert;

                    if(world_vert.x < AABB.Min.x) { AABB.Min.x = world_vert.x; } 
                    if(world_vert.y < AABB.Min.y) { AABB.Min.y = world_vert.y; } 
                    if(world_vert.z < AABB.Min.z) { AABB.Min.z = world_vert.z; }

                    if(world_vert.x > AABB.Max.x) { AABB.Max.x = world_vert.x; } 
                    if(world_vert.y > AABB.Max.y) { AABB.Max.y = world_vert.y; } 
                    if(world_vert.z > AABB.Max.z) { AABB.Max.z = world_vert.z; } 
                }
                break;
            }
            case COLLIDER_PLANE : {
                if(Normal == glm::vec3{0.0f, 1.0f, 0.0f} || Normal == glm::vec3{0.0f, -1.0f, 0.0f}) {
                    AABB.Min = glm::vec3{-FLT_MAX, Position.y - epsilon, -FLT_MAX};
                    AABB.Max = glm::vec3{FLT_MAX, Position.y + epsilon, FLT_MAX};
                }
                else {
                    AABB.Min = glm::vec3{-FLT_MAX, -FLT_MAX, -FLT_MAX};
                    AABB.Max = glm::vec3{FLT_MAX, FLT_MAX, FLT_MAX};
                }
                break;
            }
            default : {
                ASSERT(false);
            }
        }
    }

    /*
    -------------------------------------------------------------------------------------------------------
                    COLLISION DETECTION
    -------------------------------------------------------------------------------------------------------
    */

    bool AABBCollision(BoundingBox_t A, BoundingBox_t B) {
        return (A.Min.x <= B.Max.x && A.Max.x >= B.Min.x &&
                A.Min.y <= B.Max.y && A.Max.y >= B.Min.y &&
                A.Min.z <= B.Max.z && A.Max.z >= B.Min.z);
    }

    bool AABBCollisionPoint(BoundingBox_t box, glm::vec3 p){
        const float epsilon = 0.00001f;
        return p.x + epsilon >= box.Min.x && p.x - epsilon <= box.Max.x &&
               p.y + epsilon >= box.Min.y && p.y - epsilon <= box.Max.y &&
               p.z + epsilon >= box.Min.z && p.z - epsilon <= box.Max.z;
        }

    CollisionData_t DetectCollision(const Collider_t &col1, const Collider_t &col2)
    {
        ASSERT_V(col1.ColliderType != COLLIDER_NONE && col2.ColliderType != COLLIDER_NONE, CollisionData_t{});

        // Swaping Bodies to limit possible combinations when dispatching.
        const Collider_t &a = col1.ColliderType <= col2.ColliderType ? col1 : col2;
        const Collider_t &b = col1.ColliderType <= col2.ColliderType ? col2 : col1;

        // COLLIDER_HULL is the first enum entry
        if(a.ColliderType == COLLIDER_HULL) {
            if(b.ColliderType == COLLIDER_PLANE) {
                return DetectCollision_HullPlane(a, b);
            } else {
                return DetectCollision_HullNonPlane(a, b);
            }
        } else if(a.ColliderType == COLLIDER_SPHERE) {
            if(b.ColliderType == COLLIDER_SPHERE) {
                return DetectCollision_SphereSphere(a, b);
            } else if(b.ColliderType == COLLIDER_PLANE) {
                return DetectCollision_SpherePlane(a, b);
            } else if(b.ColliderType == COLLIDER_CAPSULE) {
                return DetectCollision_SphereCapsule(a, b);
            }
        } else if(a.ColliderType == COLLIDER_PLANE) {
            if(b.ColliderType == COLLIDER_CAPSULE) {
                return DetectCollision_PlaneCapsule(a, b);
            }
        } else if(a.ColliderType == COLLIDER_CAPSULE) {
            if(b.ColliderType == COLLIDER_CAPSULE) {
                return DetectCollision_CapsuleCapsule(a, b);
            }
        }

        ASSERT_V(false, CollisionData_t{});
        return CollisionData_t{};
    }

    CollisionData_t DetectCollision_SphereSphere(const Collider_t &col1, const Collider_t &col2) {

        glm::vec3 dis = col2.Position - col1.Position;
        float dis_len = glm::length(dis);
        float col_depth = dis_len - col1.Radius - col2.Radius;

        if(col_depth >= 0) {
            // Not Colliding
            return CollisionData_t{};
        }

        glm::vec3 dis_norm = dis / dis_len;

        return CollisionData_t{true, dis_norm, col_depth, dis_norm * col1.Radius, -dis_norm * col2.Radius};
    }

    CollisionData_t DetectCollision_SpherePlane(const Collider_t &sphere, const Collider_t &plane) {

        float height = glm::dot(sphere.Position - plane.Position, plane.Normal);
        float col_depth = height - sphere.Radius;

        if (col_depth >= 0 ||
            col_depth < -2.0f * sphere.Radius /*Under the plane*/)
        {
            return CollisionData_t{};
        }

        const glm::vec3 point_on_plane = sphere.Position - plane.Normal * (sphere.Radius + col_depth) - plane.Position;

        return CollisionData_t{true, -plane.Normal, col_depth, -plane.Normal * sphere.Radius, point_on_plane};
    }

    CollisionData_t DetectCollision_SphereCapsule(const Collider_t &sphere, const Collider_t &capsule) {
        const glm::vec3 capsule_orientation = ApplyRotation(capsule.Rotation, glm::vec3{0.0f, 1.0f, 0.0f});
        const glm::vec3 capsule_bottom = capsule.Position - capsule.Length * 0.5f * capsule_orientation;
        const glm::vec3 capsule_top = capsule.Position + capsule.Length * 0.5f * capsule_orientation;
        const glm::vec3 sphere_to_capsule = PointToSegment(sphere.Position, capsule_bottom, capsule_top);

        const float distance = glm::length(sphere_to_capsule);
        const float col_depth = distance - sphere.Radius - capsule.Radius;

        if(col_depth >= 0) {
            //Not colliding
            return CollisionData_t{};
        }

        const glm::vec3 sphere_to_capsule_norm = sphere_to_capsule / distance;

        return CollisionData_t{true, sphere_to_capsule / distance, col_depth, sphere_to_capsule_norm * sphere.Radius, -sphere_to_capsule_norm * capsule.Radius};
    }

    CollisionData_t DetectCollision_PlaneCapsule(const Collider_t &plane, const Collider_t &capsule) {
        const glm::vec3 capsule_orientation = ApplyRotation(capsule.Rotation, glm::vec3{0.0f, 1.0f, 0.0f});
        const glm::vec3 capsule_bottom = capsule.Position - capsule.Length * 0.5f * capsule_orientation;
        const glm::vec3 capsule_top = capsule.Position + capsule.Length * 0.5f * capsule_orientation;

        const float bottom_depth = glm::dot(capsule_bottom - plane.Position, plane.Normal) - capsule.Radius;
        const float top_depth = glm::dot(capsule_top - plane.Position, plane.Normal) - capsule.Radius;

        if(bottom_depth >= 0 && top_depth >= 0) {
            return CollisionData_t{};
        }

        glm::vec3 capsule_apply_point{};

        if(bottom_depth < 0 && top_depth < 0) {
            // Both underground, the apply point is thoward the one that is deeper, so we weight by their depth.
            capsule_apply_point = (((capsule_bottom - capsule.Position) * -bottom_depth) + ((capsule_top - capsule.Position) * -top_depth))/(-bottom_depth - top_depth) ;
        } 
        else if(bottom_depth < 0 && top_depth > 0) {
            // On under, one above. Apply point is between the one under, and the point that meets the ground.
            float total_height = top_depth - bottom_depth;
            float t = -bottom_depth / total_height;
            t = t * 0.5f;

            capsule_apply_point = ((capsule_bottom - capsule.Position) * (1-t)) + ((capsule_top - capsule.Position) * (t));
        } 
        else if(top_depth < 0 && top_depth < bottom_depth) {
            // On under, one above. Apply point is between the one under, and the point that meets the ground.
            float total_height = bottom_depth - top_depth;
            float t = -top_depth / total_height;
            t = t * 0.5f;

            capsule_apply_point = ((capsule_top - capsule.Position) * (1 - t)) + ((capsule_bottom - capsule.Position) * (t));
        }

        capsule_apply_point += -plane.Normal * capsule.Radius;

        glm::vec3 plane_apply_point = capsule.Position + capsule_apply_point - plane.Position - plane.Normal * glm::dot(capsule.Position + capsule_apply_point - plane.Position, plane.Normal);

        return CollisionData_t{true, plane.Normal, glm::min(bottom_depth, top_depth), plane_apply_point, capsule_apply_point};
    }

    CollisionData_t DetectCollision_CapsuleCapsule(const Collider_t &col1, const Collider_t &col2) {
        const glm::vec3 orientation1 = ApplyRotation(col1.Rotation, glm::vec3{0.0f, 1.0f, 0.0f});
        const glm::vec3 bottom1 = col1.Position - (col1.Length * 0.5f * orientation1);
        const glm::vec3 top1 = col1.Position + (col1.Length * 0.5f * orientation1);

        const glm::vec3 orientation2 = ApplyRotation(col2.Rotation, glm::vec3{0.0f, 1.0f, 0.0f});
        const glm::vec3 bottom2 = col2.Position - (col2.Length * 0.5f * orientation2);
        const glm::vec3 top2 = col2.Position + (col2.Length * 0.5f * orientation2);

        glm::vec3 point1{};
        glm::vec3 point2{};
        SegmentsClosestPoints(bottom1, top1, bottom2, top2, point1, point2);

        const glm::vec3 dist = point2 - point1;
        const float dist_len = glm::length(dist);

        const float col_depth = dist_len - col1.Radius - col2.Radius;

        if (col_depth >= 0) {
            // Not colliding
            return CollisionData_t{};
        }

        const glm::vec3 dist_norm = dist / dist_len;

        const glm::vec3 edge_point1 = point1 + (dist_norm * col1.Radius);
        const glm::vec3 edge_point2 = point2 - (dist_norm * col2.Radius);

        const glm::vec3 middle = (edge_point1 + edge_point2) * 0.5f;

        return CollisionData_t{true, dist_norm, col_depth, middle - col1.Position, middle - col2.Position - (dist_norm * col2.Radius)};
    }

    CollisionData_t DetectCollision_HullNonPlane(const Collider_t &hull, const Collider_t &nonPlane) {
        Simplex_t simplex{};

        bool collide = GJK(hull, nonPlane, simplex);
        
        if(!collide) {
            return CollisionData_t{};
        } 

        glm::vec3 pointA{};
        glm::vec3 pointB{};
        glm::vec3 dir{};
        float depth;
        EPA(hull, nonPlane, simplex, pointA, pointB, dir, depth);

        return CollisionData_t{true, dir, -depth, pointA - hull.Position, pointB - nonPlane.Position};
    }

    CollisionData_t DetectCollision_HullPlane(const Collider_t &hull, const Collider_t &Plane) {
        /*
        Although it can seem like this method is over-eingeneered,
        from my testing, it gets significatly better looking result compared to
        simple individual vertices testing.
        */

        const std::vector<glm::vec3> &vert = hull.TransformedModel;

        if(!hull.Model) {
            return CollisionData_t{};
        }
        const std::vector<int> &ind = hull.Model->Indices;

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
                    heights[indice_a].second = glm::dot(hull.Position + vert[indice_a] - Plane.Position, Plane.Normal);
                    if(heights[indice_a].second < max_depth) {
                        max_depth = heights[indice_a].second;
                    }
                }
                if(!heights[indice_b].first) {
                    //Need to calculate the height
                    heights[indice_b].first = true;
                    heights[indice_b].second = glm::dot(hull.Position + vert[indice_b] - Plane.Position, Plane.Normal);
                    if(heights[indice_b].second < max_depth) {
                        max_depth = heights[indice_b].second;
                    }
                }

                if(!got_under) {
                    got_under = heights[indice_a].second <= 0.0f || heights[indice_b].second <= 0.0f;
                }

                glm::vec3 slice_pos{};

                if(heights[indice_a].second > 0.0f && heights[indice_b].second > 0.0f /*Above*/) {
                    continue;
                } 
                else if(heights[indice_a].second <= 0.0f && heights[indice_b].second <= 0.0f /*Bellow*/) {
                    slice_pos = (hull.Position + vert[indice_a] + hull.Position + vert[indice_b]) * 0.5f + glm::vec3{0.0f, glm::min(-heights[indice_a].second, -heights[indice_b].second), 0.0f};
                } 
                else { /*Above and bellow*/
                    float total_height = glm::abs(heights[indice_a].second - heights[indice_b].second);
                    float floor_height = glm::abs(heights[indice_a].second);

                    slice_pos = ((total_height - floor_height) * (hull.Position + vert[indice_a])
                                +  floor_height * (hull.Position + vert[indice_b])) / total_height;
                }

                slice_sum += slice_pos;

                slice_vert_count++;
            }
        }

        if(!got_under) {
            return CollisionData_t{};
        }

        glm::vec3 plane_apply_point = ProjectToPlane(slice_sum / (float)slice_vert_count, Plane.Normal) - Plane.Position;

        return CollisionData_t{true, -Plane.Normal, max_depth, slice_sum / (float)slice_vert_count - hull.Position, plane_apply_point};
    }

    /*
    -------------------------------------------------------------------------------------------------------
                    COLLISION RESPONSE
    -------------------------------------------------------------------------------------------------------
    */

    CONVAR(float, phys_response_epsilon, 1e-5f, "Very small value. Object are pushed out of each other with this leaway.");

    void RespondCollision(RigidBody &rb1, RigidBody &rb2, const CollisionData_t &collision, CollisionSoundManager *soundManager)
    {
        //Swaping the object order to reflect the one in DetectCollision function.
        RigidBody &BodyA = rb1.ColliderType <= rb2.ColliderType ? rb1 : rb2;
        RigidBody &BodyB = rb1.ColliderType <= rb2.ColliderType ? rb2 : rb1;

        /*
        I used to check if objects were moving away from each other before responding
        but it seemed to be irrelevent and to cause 'skipping' of some collision reponses.
        This has now been removed.
        */

        float friction_energy_loss = 0.0f;
        float hit_energy_loss = 0.0f;

        if(BodyA.IsStatic && BodyB.IsStatic) {
            return;
        } else {
            const glm::vec3 v_a = BodyA.Velocity + glm::cross(BodyA.AngularVelocity, collision.ApplyPointA);
            const glm::vec3 v_b = BodyB.Velocity + glm::cross(BodyB.AngularVelocity, collision.ApplyPointB);
            const glm::vec3 v_rel = v_b - v_a;
            const float proj_v_rel = glm::dot(v_rel, collision.Normal);

            const float e = GetBounciness(BodyA, BodyB);

            const float inv_m_a = BodyA.IsStatic ? 0.0f : 1.0f / BodyA.Mass;
            const float inv_m_b = BodyB.IsStatic ? 0.0f : 1.0f / BodyB.Mass;
            //const mat3 inv_I_a = glm::inverse(BodyA.InertiaTensor) * inv_m_a;
            //const mat3 inv_I_b = glm::inverse(BodyB.InertiaTensor) * inv_m_b;

            const mat3 inv_I_a = BodyA.GetInverseInertiaTensor();
            const mat3 inv_I_b = BodyB.GetInverseInertiaTensor();

            const glm::vec3 moment_div_a = glm::cross(inv_I_a * glm::cross(collision.ApplyPointA, collision.Normal), collision.ApplyPointA);
            const glm::vec3 moment_div_b = glm::cross(inv_I_b * glm::cross(collision.ApplyPointB, collision.Normal), collision.ApplyPointB);

            float J = (1.0 + e) * proj_v_rel / (inv_m_a + inv_m_b + glm::dot(moment_div_a + moment_div_b, collision.Normal));

            float start_energy = 0.5f * BodyA.Mass * LenSquared(BodyA.Velocity);
            BodyA.AddImpulse(collision.Normal * J, collision.ApplyPointA);
            float end_energy = 0.5f * BodyA.Mass * LenSquared(BodyA.Velocity);

            hit_energy_loss = start_energy - end_energy;

            start_energy = 0.5f * BodyB.Mass * LenSquared(BodyB.Velocity);
            BodyB.AddImpulse(-collision.Normal * J, collision.ApplyPointB);
            end_energy = 0.5f * BodyB.Mass * LenSquared(BodyB.Velocity);

            hit_energy_loss = glm::max<float>(hit_energy_loss + start_energy - end_energy, 0);

            if (collision.Depth < 2.0f)
            {
                // Offset the object out of the collision to avoid double apply on consecutive frame when working witih tiny velocity!
                BodyA.Position += BodyA.IsStatic ? glm::vec3{0.0f} : collision.Normal * (collision.Depth - convar_phys_response_epsilon) * 0.5f;
                BodyB.Position += BodyB.IsStatic ? glm::vec3{0.0f} : -collision.Normal * (collision.Depth - convar_phys_response_epsilon) * 0.5f;
            }

            if(!BodyA.IsStatic) {
                friction_energy_loss += ApplyFriction(J, BodyA, -collision.Normal, collision.ApplyPointA, GetFriction(BodyA, BodyB), GetStaticFriction(BodyA, BodyB));
            }
            if(!BodyB.IsStatic) {
                friction_energy_loss += ApplyFriction(J, BodyB, collision.Normal, collision.ApplyPointB, GetFriction(BodyA, BodyB), GetStaticFriction(BodyA, BodyB));
            }
        }

        {
            CollisionSoundManager::CollisionSoundQuery_t hit_query{};
            hit_query.Type = CollisionSoundManager::COLLISION_SOUND_TYPE_HIT;
            hit_query.ApplyPoint = rb1.Position + collision.ApplyPointA;
            hit_query.EnergyLoss = hit_energy_loss;
            soundManager->NewSoundQuery(hit_query);
            
            CollisionSoundManager::CollisionSoundQuery_t friction_query{};
            friction_query.Type = CollisionSoundManager::COLLISION_SOUND_TYPE_FRICTION;
            friction_query.ApplyPoint = rb1.Position + collision.ApplyPointA;
            friction_query.EnergyLoss = friction_energy_loss;
            friction_query.Velocity = rb1.IsStatic ? rb2.Velocity : 
                                      rb2.IsStatic ? rb1.Velocity : 
                                      rb1.Velocity;
            soundManager->NewSoundQuery(friction_query);
        }
    }

    float ApplyFriction(float normalImpulse, RigidBody &rb, const glm::vec3 &surfaceNormal,
        const glm::vec3 &applyPoint, float frictionCoefficient, float staticFrictionCoefficient)
        {
            
        const glm::vec3 point_velocity = rb.Velocity + glm::cross(rb.AngularVelocity, applyPoint);
        glm::vec3 tangent_velocity = point_velocity - surfaceNormal * glm::dot(surfaceNormal, point_velocity);
        const float tangent_vel_len = glm::length(tangent_velocity);
        
        if(tangent_vel_len != 0.0f) {
            
            float static_threshold = normalImpulse * staticFrictionCoefficient;
            if(tangent_vel_len <= static_threshold) {
                rb.AddImpulse(-tangent_velocity * rb.Mass, applyPoint);
                return tangent_vel_len * tangent_vel_len * rb.Mass;
            } else {
                tangent_velocity = tangent_velocity/tangent_vel_len;
                float impulse = glm::clamp(glm::abs(normalImpulse * frictionCoefficient), 0.0f, tangent_vel_len);
                rb.AddImpulse(-tangent_velocity * impulse, applyPoint);
                return 0.5f * (tangent_vel_len * impulse - impulse * impulse / rb.Mass);
            }

        }

        return 0.0f;
    }

    /*
    -------------------------------------------------------------------------------------------------------
                    MATERIAL PROPERTIES
    -------------------------------------------------------------------------------------------------------
    */

    CONVAR(int, phys_material_property_mix_type, 0, 
        "Defines how Bounciness/firction are calculated in collision, given two objects with each their own coefficient\n"
        "   0 : Multiplies both (Default)\n"
        "   1 : adds both (and clamps)\n"
        "   2 : takes the average.")

    float GetBounciness(RigidBody &rb1, RigidBody &rb2) {
        switch(convar_phys_material_property_mix_type) {
            case 0:
                return rb1.Bounciness * rb2.Bounciness;
            case 1:
                return glm::clamp(rb1.Bounciness + rb2.Bounciness, 0.0f, 1.0f);
            case 2:
                return (rb1.Bounciness + rb2.Bounciness) * 0.5f;
            default:
                return rb1.Bounciness * rb2.Bounciness;
        }
    }
    
    float GetFriction(RigidBody &rb1, RigidBody &rb2) {
        switch(convar_phys_material_property_mix_type) {
            case 0:
                return rb1.Friction * rb2.Friction;
            case 1:
                return glm::clamp(rb1.Friction + rb2.Friction, 0.0f, 1.0f);
            case 2:
                return (rb1.Friction + rb2.Friction) * 0.5f;
            default:
                return rb1.Friction * rb2.Friction;
        }
    }
    
    float GetStaticFriction(RigidBody &rb1, RigidBody &rb2) {
        switch(convar_phys_material_property_mix_type) {
            case 0:
                return rb1.Friction * rb1.StaticFrictionMultiplier * rb2.Friction * rb2.StaticFrictionMultiplier;
            case 1:
                return glm::clamp(rb1.Friction * rb1.StaticFrictionMultiplier + rb2.Friction * rb2.StaticFrictionMultiplier, 0.0f, 1.0f);
            case 2:
                return (rb1.Friction * rb1.StaticFrictionMultiplier + rb2.Friction * rb2.StaticFrictionMultiplier) * 0.5f;
            default:
                return rb1.Friction * rb1.StaticFrictionMultiplier * rb2.Friction * rb2.StaticFrictionMultiplier;
        }
    }

}