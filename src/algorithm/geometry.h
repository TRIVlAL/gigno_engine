#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "glm/glm.hpp"

namespace gigno {

    struct Line {
        glm::vec3 Point{};
        glm::vec3 Dir{};
    };

    /*
    @brief Returns the Line of intersection of the two planes
    @param point A point through which the plan goes
    @param normal The NORMALIZED perpendicular vector of the plane
    @return If planes are paralel if equal, then The line's direction and point is (0, 0, 0)
    */
    Line PlaneIntersect(glm::vec3 point1, glm::vec3 normal1, glm::vec3 point2, glm::vec3 normal2);

    glm::vec3 ProjectToPlane(glm::vec3 vector, glm::vec3 normal);

    /*
    @param outAPoint the start of the shortest segment between a and b
    @param outBPoint the end of the shortes segment between a and b
    */
    void SegmentsClosestPoints(glm::vec3 a1, glm::vec3 a2, glm::vec3 b1, glm::vec3 b2, glm::vec3 &outAPoint, glm::vec3 &outBPoint);

    /*
    returns the shortest vector from the point to the segment.
    */
    glm::vec3 PointToSegment(glm::vec3 point, glm::vec3 seg1, glm::vec3 seg2);

    float LenSquared(glm::vec3 &vec);

    /*
    Return the result of the given vecctor after being rotated following the 
    Tait-Bryan YXZ (see @ https://en.wikipedia.org/wiki/Euler_angles (Rotation Matrix)) convention
    */
    glm::vec3 ApplyRotation(glm::vec3 rotation, glm::vec3 vector);
}

#endif