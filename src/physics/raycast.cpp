#include "raycast.h"
#include "../utils/geometry.h"
#include <array>
#include "../error_macros.h"
#include "../debug/profiler/profiler.h"

namespace gigno {

    bool Raycast_AABB(Ray_t ray, BoundingBox_t bb, RaycastHit_t *outHit) {

        Profiler::CreateScope{"Raycast_AABB"};

        std::vector<glm::vec3> points{};
        points.reserve(6);

        for(size_t dim = 0; dim < 3; dim++) {
            {
                float t = (bb.Min[dim] - ray.Point[dim]) / ray.Direction[dim];
                if(t >= 0.0f) {
                    points.emplace_back(ray.Point + t * ray.Direction);
                }
            }
            {
                float t = (bb.Max[dim] - ray.Point[dim]) / ray.Direction[dim];
                if(t >= 0.0f) {
                    points.emplace_back(ray.Point + t * ray.Direction);
                }
            }
        }

        // points now contains all points that MAY be on the edge of the bb

        //We work with distance squared here.
        const float max_distance = ray.MaxRange * ray.MaxRange;

        float distance = FLT_MAX;
        size_t point_hit_index = -1;
        for(size_t i = 0; i < points.size(); i++) {
            glm::vec3 p = points[i];
            if(AABBCollisionPoint(bb, p)) {
                float new_distance = LenSquared(p - ray.Point);
                if(new_distance < distance && new_distance <= max_distance) {
                    distance = new_distance;
                    point_hit_index = i;
                }
            }
        }

        if(point_hit_index == -1) {
            return false;
        } else {

            RaycastHit_t hit{};
            hit.ColliderHit = RAYCAST_COLLIDE_AABB;
            hit.EntityHit = nullptr; //modified by server if need be
            hit.EntityHitType = RAYCAST_HIT_ENTITY_NONE;
            hit.Position = points[point_hit_index];
            hit.Distance = glm::sqrt(distance);

            *outHit = hit;

            return true;
        }
    }

    bool Raycast_Collider(Ray_t ray, Collider_t collider, RaycastHit_t *outHit) {
        switch(collider.ColliderType) {
            case(COLLIDER_SPHERE):
                return Raycast_Sphere(ray, collider.Position, collider.Radius, outHit);
            case(COLLIDER_PLANE):
                return Raycast_Plane(ray, collider.Position, collider.Normal, outHit);
            case(COLLIDER_HULL):
                return Raycast_Hull(ray, collider.Position, collider.Model, collider.GetTransformedModel(), collider.AABB, outHit);
            default:
                // todo : implement other collider types
                return false;
        }
    }

    bool Raycast_Sphere(Ray_t ray, glm::vec3 position, float radius, RaycastHit_t *outHit) {

        const float project = glm::dot(ray.Direction, ray.Point - position);

        const float delta = radius * radius - (LenSquared(ray.Point - position) - project * project);

        if(delta < 0.0f) {
            return false;
        }

        float d = -project - delta;
        if(d > ray.MaxRange) {
            return false;
        }
        if(d < 0.0f) {
            d = -project + delta;
        }
        if(d < 0.0f) {
            return false;
        }

        glm::vec3 point = ray.Point + ray.Direction * d;

        RaycastHit_t hit{};
        hit.ColliderHit = RAYCAST_COLLIDE_COLLIDER;
        hit.EntityHit = nullptr; // modified by server if need be
        hit.EntityHitType = RAYCAST_HIT_ENTITY_NONE;
        hit.Position = point;
        hit.Distance = glm::length(point - ray.Point);

        *outHit = hit;

        return true;
    }

    bool Raycast_Plane(Ray_t ray, glm::vec3 position, glm::vec3 normal, RaycastHit_t *outHit) {

        if(glm::dot(ray.Direction, position - ray.Point) < 0.0f) {
            return false;
        }

        normal = glm::normalize(normal);

        float distance_to_plane = glm::dot(position - ray.Point, normal);
        float along_normal = glm::dot(ray.Direction, normal);
        glm::vec3 on_plane = ray.Point + ray.Direction * distance_to_plane / along_normal;

        RaycastHit_t hit{};
        hit.ColliderHit = RAYCAST_COLLIDE_COLLIDER;
        hit.EntityHit = nullptr; //modified by server if need be.
        hit.EntityHitType = RAYCAST_HIT_ENTITY_NONE;
        hit.Position = on_plane;
        hit.Distance = glm::length(on_plane - ray.Point);

        if(hit.Distance > ray.MaxRange) {
            return false;
        }

        *outHit = hit;

        return true;
    }

    bool Raycast_Hull(Ray_t ray, glm::vec3 position, const CollisionModel_t *model, std::vector<glm::vec3>& transformedModel, BoundingBox_t bb, RaycastHit_t *outHit) {

        Profiler::CreateScope("Raycast_Hull");

        RaycastHit_t ignored{};
        if(!Raycast_AABB(ray, bb, &ignored)) {
            return false;
        }

        for(size_t i = 0; i < model->Indices.size(); i+=3) {
            //Loop over each face.

            glm::vec3 A = position + transformedModel[model->Indices[i]];
            glm::vec3 B = position + transformedModel[model->Indices[i+1]];
            glm::vec3 C = position + transformedModel[model->Indices[i+2]];
            
            if(glm::dot(A - ray.Point, ray.Direction) < 0.0f) {
                continue;
            }

            const glm::vec3 normal = glm::normalize(glm::cross(B - A, C - A));

            if(glm::dot(ray.Direction, normal) > 0.0f) {
                continue;
            }

            const float dist_to_plane = glm::dot(A - ray.Point, normal);

            if(dist_to_plane > 0.0f) {
                continue;
            }

            const float along_normal = glm::dot(ray.Direction, normal);

            const glm::vec3 on_plane = ray.Point + ray.Direction * dist_to_plane / along_normal;

            //Check if inside triangle
            if (glm::dot(glm::cross(normal, B - A), on_plane - A) >= 0 &&
                glm::dot(glm::cross(normal, C - B), on_plane - B) >= 0 &&
                glm::dot(glm::cross(normal, A - C), on_plane - C) >= 0)
            {
                RaycastHit_t hit{};
                hit.ColliderHit = RAYCAST_COLLIDE_COLLIDER;
                hit.EntityHit = nullptr; // modified by server if need be
                hit.EntityHitType = RAYCAST_HIT_ENTITY_NONE;
                hit.Position = on_plane;
                hit.Distance = glm::length(on_plane - ray.Point);

                if(hit.Distance > ray.MaxRange) {
                    return false;
                }
                
                *outHit = hit;

                return true;
            }
        }

        return false;

    }
}