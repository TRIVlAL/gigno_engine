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
    @param outAPoint the start of the shortest segment between a and b
    @param outBPoint the end of the shortes segment between a and b
    */
    void SegmentsClosestPoints(glm::vec3 a1, glm::vec3 a2, glm::vec3 b1, glm::vec3 b2, glm::vec3 &outAPoint, glm::vec3 &outBPoint);
}