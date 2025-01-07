#include "gjk.h"
#include "rigid_body.h"
#include "../algorithm/geometry.h"
#include "error_macros.h"
#include <utility>

namespace gigno {

    bool GJK(const RigidBody &A, const RigidBody &B, Simplex_t &outSimplex) {

        glm::vec3 dir = A.Position - B.Position; //Can be any default direction.
        glm::vec3 SupA = Support(dir, A);
        glm::vec3 SupB = Support(-dir, B);
        glm::vec3 extreme = SupA - SupB;
        outSimplex.a.Point = extreme;
        outSimplex.a.ASupport = SupA;
        outSimplex.a.BSupport = SupB;

        dir = -extreme;
        SupA = Support(dir, A);
        SupB = Support(-dir, B);
        extreme = SupA - SupB;
        outSimplex.b.Point = extreme;
        outSimplex.b.ASupport = SupA;
        outSimplex.b.BSupport = SupB;
        if(glm::dot(extreme, dir) < 0) {
            return false;
        }

        //Line case
        const glm::vec3 ab = outSimplex.b.Point - outSimplex.a.Point;
        const glm::vec3 ao = -extreme;
        dir = glm::cross(glm::cross(ab, ao), ab);
        SupA = Support(dir, A);
        SupB = Support(-dir, B);
        extreme = SupA - SupB;
        outSimplex.c.Point = extreme;
        outSimplex.c.ASupport = SupA;
        outSimplex.c.BSupport = SupB;
        if(glm::dot(extreme, dir) < 0) {
            return false;
        }

        dir = glm::cross(outSimplex.b.Point - outSimplex.a.Point, outSimplex.c.Point - outSimplex.a.Point);
        if(glm::dot(outSimplex.a.Point, dir) > 0) {
            dir = -dir;
        }

        while(true) {
            SupA = Support(dir, A);
            SupB = Support(-dir, B);
            extreme = SupA - SupB;
            if(glm::dot(extreme, dir) < 0) {
                return false;
            }
            outSimplex.d.ASupport = SupA;
            outSimplex.d.BSupport = SupB;
            outSimplex.d.Point = extreme;

            const glm::vec3 d0 = -outSimplex.d.Point;

            const glm::vec3 da = outSimplex.a.Point - outSimplex.d.Point;
            const glm::vec3 db = outSimplex.b.Point - outSimplex.d.Point;
            const glm::vec3 dc = outSimplex.c.Point - outSimplex.d.Point;

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

    void EPA(const RigidBody &A, const RigidBody &B, const Simplex_t &Simplex,
             glm::vec3 &outPointA, glm::vec3 &outPointB, glm::vec3 &outDirection, float &outDepth) {

        Polytope_t polytope{};
        polytope.Vertices = {Simplex.a, Simplex.b, Simplex.c, Simplex.d};
        polytope.Indices = {
            0, 2, 1,
            0, 1, 3,
            1, 2, 3,
            0, 3, 2
        }; // Winding order : Counter clockwise.

        const float epsilon = 0.0001f;

        int i = 0;
        while(i++ < 100 /*safety*/) {
            float face_distance{};
            glm::vec3 face_normal{};
            size_t face_index = polytope.GetClosestFace(face_distance, face_normal);

            const glm::vec3 PointA = Support(face_normal, A);
            const glm::vec3 PointB = Support(-face_normal, B);
            MinkowskiVertex new_point{};
            new_point.Point = PointA - PointB;
            new_point.ASupport = PointA;
            new_point.BSupport = PointB;

            float new_point_distance = glm::dot(new_point.Point, face_normal);

            if(new_point_distance - face_distance < epsilon) {

                float u{};
                float v{};
                float w{};

                const MinkowskiVertex a = polytope.Vertices[polytope.Indices[face_index]];
                const MinkowskiVertex b = polytope.Vertices[polytope.Indices[face_index+1]];
                const MinkowskiVertex c = polytope.Vertices[polytope.Indices[face_index+2]];

                Barycentric(glm::vec3{0.0f}, 
                    a.Point,
                    b.Point,
                    c.Point,
                    u, v, w
                );

                outPointA = u * a.ASupport + v * b.ASupport + w * c.ASupport;
                outPointB = u * a.BSupport + v * b.BSupport + w * c.BSupport;
                outDirection = face_normal;
                outDepth = face_distance;
                return;
            }

            polytope.AddVertex(new_point);
        }

        ERR_MSG("Physics collision : EPA has too many loop iterations !!! is epsilon too small ?");
    }
    
    size_t Polytope_t::GetClosestFace(float &outFaceDistance, glm::vec3 &outFaceNormal) {
        
        size_t closest_index = -1;
        outFaceDistance = FLT_MAX;

        for (size_t i = 0; i < Indices.size(); i += 3) {
            glm::vec3 new_face_normal{};
            float new_dist = DistanceToOrigin(Vertices[Indices[i]].Point,
                                              Vertices[Indices[i + 1]].Point,
                                              Vertices[Indices[i + 2]].Point, new_face_normal);

            if (new_dist < outFaceDistance) {
                outFaceDistance = new_dist;
                closest_index = i;
                outFaceNormal = new_face_normal;
            }
        }
        return closest_index;
    }
    
    void Polytope_t::AddVertex(MinkowskiVertex vertex) {
        std::vector<std::pair<size_t, size_t>> edges{};
        std::vector<size_t> faces_to_remove_indices{};

        Vertices.emplace_back(vertex);
        size_t new_vert_index = Vertices.size() - 1;

        for(size_t i = 0; i < Indices.size(); i += 3) {
            //For every face in the shape
            const glm::vec3 face_normal = glm::cross(Vertices[Indices[i+1]].Point - Vertices[Indices[i]].Point, Vertices[Indices[i+2]].Point - Vertices[Indices[i]].Point);
            const glm::vec3 to_new_vert = vertex.Point - Vertices[Indices[i]].Point;
            const glm::vec3 face_center = (Vertices[Indices[i]].Point + Vertices[Indices[i+1]].Point + Vertices[Indices[i+2]].Point)/3.0f;

            if(glm::dot(face_normal, to_new_vert) > 0) {
                //This face can 'see' the new vertex.
                faces_to_remove_indices.emplace_back(i);

                for(int j = 0; j < 3; j++) {
                    //For each edge in the face
                    std::pair<size_t, size_t> edge{Indices[i+j], Indices[j == 2 ? i : i+j+1]};

                    bool exists = false;
                    for(int i = 0; i < edges.size(); i++) {
                        if(edges[i].first == edge.second && edges[i].second == edge.first) {
                            edges.erase(edges.begin() + i);
                            exists = true;
                            break;
                        } else if(edges[i] == edge) {
                            exists = true;
                            break;
                        }
                    }

                    if(!exists) {
                        edges.emplace_back(edge);
                    }
                }
            }
        }

        ASSERT(Indices.size() - faces_to_remove_indices.size() * 3 > 0)

        //Remove faces
        // We go in reverse since faces_to_remove_indices is sorted ascending.
        for (auto it = faces_to_remove_indices.rbegin(); it != faces_to_remove_indices.rend(); ++it) {
            Indices.erase(Indices.begin() + *it, Indices.begin() + *it + 3);
        }

        //Remains in edges only the edges that should build the new faces.
        Indices.reserve(edges.size() * 3);
        for(std::pair<size_t, size_t> &edge : edges) {
            Indices.emplace_back(edge.first);
            Indices.emplace_back(edge.second);
            Indices.emplace_back(new_vert_index);
        }
    }

    glm::vec3 Support(glm::vec3 direction, const RigidBody &rb)
    {
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
            glm::vec3 best_point = rb.Hull.RotatedVertices[0];
            float max_dot = glm::dot(best_point, direction);
            for(int i = 1; i < rb.Hull.Vertices.size(); i++) {
                const glm::vec3 new_point = rb.Hull.RotatedVertices[i];
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