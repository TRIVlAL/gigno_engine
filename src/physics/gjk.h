#ifndef GJK_H
#define GJH_H

#include <vector>
#include <array>
#include "glm/glm.hpp"
#include <utility>

namespace gigno {

    struct Collider_t;

    struct MinkowskiVertex {
        glm::vec3 Point;
        glm::vec3 ASupport;
        glm::vec3 BSupport;
    };

    struct Simplex_t {
        MinkowskiVertex a;
        MinkowskiVertex b;
        MinkowskiVertex c;
        MinkowskiVertex d;
    };


    /*
    Gilbert–Johnson–Keerthi algorithm
    Given two colliders, returns wheter or not they collide
    if they collide, outSimplex is set to a fully-built simplex (i.e. all vetices are set) 
               enclosing the origin in the Minkowski difference of  collider A - collider B.
    */
    bool GJK(const Collider_t &A, const Collider_t &B, Simplex_t &outSimplex);

    /*
    Given two rbs and a Simplex on their Minkowski difference contining the origin (obtained with GJK), outputs
    their collision informations :
        outPointA/outPointB : Points of collision in world space.
        outDirection        : normalized colision normal from A to B
        outDepth            : POSITIVE number representing how deep inside themselves the objects are.
    */
    void EPA(const Collider_t &A, const Collider_t &B, const Simplex_t &Simplex,
             glm::vec3 &outPointA, glm::vec3 &outPointB, glm::vec3 &outDirection, float &outDepth);

    struct Polytope_t {
        std::vector<MinkowskiVertex>  Vertices;
        std::vector<size_t>           Indices;

        /*
        Return the index of the face closest to the origin.
        outputs the distance from the face to the origin, and the normalized normal vector of the face.
        */
        size_t GetClosestFace(float &outFaceDistance, glm::vec3 &outFaceNormal);

        /*
        Expands the Ppolytope and rebuilds its faces.
        */
        void AddVertex(MinkowskiVertex vertex);
    };

    /*
    Returns the point furthest on the collider (in world space) in the direction given.
    direction doest need to be normalized.
    */
    glm::vec3 Support(glm::vec3 direction, const Collider_t &col);
}

#endif