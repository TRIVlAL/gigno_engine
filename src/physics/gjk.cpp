#include "gjk.h"
#include "rigid_body.h"
#include "error_macros.h"

namespace gigno {

    bool GJK(const RigidBody &A, const RigidBody &B, Simplex_t &outSimplex) {

        glm::vec3 dir = A.Position - B.Position; //Can be any default direction.
        glm::vec3 extreme = Support(dir, A) - Support(-dir, B);
        outSimplex.a = extreme;

        dir = -extreme;
        extreme = Support(dir, A) - Support(-dir, B);
        outSimplex.b = extreme;
        if(glm::dot(extreme, dir) < 0) {
            return false;
        }

        //Line case
        const glm::vec3 ab = outSimplex.b - outSimplex.a;
        const glm::vec3 ao = -extreme;
        dir = glm::cross(glm::cross(ab, ao), ab);
        extreme = Support(dir, A) - Support(-dir, B);
        const glm::vec3 s1 = Support(dir, A); const glm::vec3 s2 = Support(-dir, B);
        outSimplex.c = extreme;
        if(glm::dot(extreme, dir) < 0) {
            return false;
        }

        dir = glm::cross(outSimplex.b - outSimplex.a, outSimplex.c - outSimplex.a);
        if(glm::dot(outSimplex.a, dir) > 0) {
            dir = -dir;
        }

        while(true) {
            extreme = Support(dir, A) - Support(-dir, B);
            if(glm::dot(extreme, dir) < 0) {
                return false;
            }
            outSimplex.d = extreme;

            const glm::vec3 d0 = -outSimplex.d;

            const glm::vec3 da = outSimplex.a - outSimplex.d;
            const glm::vec3 db = outSimplex.b - outSimplex.d;
            const glm::vec3 dc = outSimplex.c - outSimplex.d;

            const glm::vec3 dab = glm::cross(da, db);
            const glm::vec3 dbc = glm::cross(db, dc);
            const glm::vec3 dca = glm::cross(dc, da);

            if(glm::dot(dab, d0) > 0) {
                outSimplex.c = outSimplex.d;
                dir = dab;
            } else if(glm::dot(dbc, d0) > 0) {
                outSimplex.a = outSimplex.d;
                dir = dbc;
            } else if(glm::dot(dca, d0) > 0) {
                outSimplex.b = outSimplex.d;
                dir = dca;
            } else {
                RenderingServer *r = Application::Singleton()->GetRenderer();
                const glm::vec3 color{0.0f, 1.0f, 0.0f};
                return true; //Origin is inside simplex.
            }
        }
    }

    glm::vec3 Support(glm::vec3 direction, const RigidBody &rb) {
        if(rb.ColliderType == COLLIDER_PLANE) {
            ERR_MSG_V(glm::vec3{}, "Physics collision : Support function for plane collider not implemented !");
        } 
        else if(rb.ColliderType == COLLIDER_SPHERE) {
            return rb.Position + glm::normalize(direction) * rb.Radius;
        } 
        else if(rb.ColliderType == COLLIDER_CAPSULE) {
            const glm::vec3 up = ApplyRotation(rb.Rotation, glm::vec3{0.0f, 1.0f, 0.0f});
            return rb.Position 
                    + (glm::dot(up, direction) >= 0 ? up : -up) * rb.Length * 0.5f 
                    + glm::normalize(direction) * rb.Radius;
        } 
        else if(rb.ColliderType == COLLIDER_HULL) {
            glm::vec3 best_point = ApplyRotation(rb.Rotation, rb.Hull.Vertices[0]);
            float max_dot = glm::dot(best_point, direction);
            for(int i = 1; i < rb.Hull.Vertices.size(); i++) {
                const glm::vec3 new_point = ApplyRotation(rb.Rotation, rb.Hull.Vertices[i]);
                const float new_dot = glm::dot(new_point, direction);
                if ( new_dot > max_dot) {
                    max_dot = new_dot;
                    best_point = new_point;
                }
            }
            return rb.Position + best_point;
        }
        else {
            return glm::vec3{};
        }
    }
}