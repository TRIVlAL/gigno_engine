#ifndef GJK_H
#define GJH_H

#include <vector>
#include <array>
#include "glm/glm.hpp"

namespace gigno {

    class RigidBody;

    struct Simplex_t {
        glm::vec3 a;
        glm::vec3 b;
        glm::vec3 c;
        glm::vec3 d;
    };

    /*
    Gilbert–Johnson–Keerthi algorithm
    Given two rbs, returns wheter or not they collide.
    outSimplex is set to a fully-built simplex (i.e. all vetices are set) 
               enclosing the origin in the Minkowski difference of both shapes.
    */
    bool GJK(const RigidBody &A, const RigidBody &B, Simplex_t &outSimplex);

    /*
    Returns the point furthest on the rb's collider (in world space) in the direction given.
    direction doest need to be normalized.
    */
    glm::vec3 Support(glm::vec3 direction, const RigidBody &rb);
}

#endif