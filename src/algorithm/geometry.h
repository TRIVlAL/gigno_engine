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

    /*
    perpendicular projection of the plane given by the normal vector. normal MUST BE normalized.
    Returns vector 0 if the vectors are colinear.
    */
    glm::vec3 ProjectToPlane(glm::vec3 vector, glm::vec3 normal);

    /*
    @param outAPoint the start of the shortest segment between a and b
    @param outBPoint the end of the shortes segment between a and b
    */
    void SegmentsClosestPoints(glm::vec3 a1, glm::vec3 a2, glm::vec3 b1, glm::vec3 b2, glm::vec3 &outAPoint, glm::vec3 &outBPoint);

    /*
    returns the point on the segment that is closest to the point.
    */
    glm::vec3 PointToSegment(glm::vec3 point, glm::vec3 seg1, glm::vec3 seg2);

    float LenSquared(const glm::vec3 &vec);

    /*
    Return the result of the given vector after being rotated by the quaternion.
    */
    glm::vec3 ApplyRotation(glm::quat rotation, glm::vec3 vector);

    /*
    Returns the distance of a face from the origin.
    outputs the normalized normal vector of the face
    */
    float DistanceToOrigin(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 &outNormal);

    /*
    Outputs the barycentric coordinates (u, v, w) for
    point p on a triangle a b c
    */
    void Barycentric(glm::vec3 p, glm::vec3 a, glm::vec3 b, glm::vec3 c, float &u, float &v, float &w);

    glm::quat FromEuler(glm::vec3 euler);

    glm::quat InverseQuat(glm::quat q);
}

#endif