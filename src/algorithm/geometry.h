#include "glm/glm.hpp"

namespace gigno {

    struct Line {
        Line() = default;
        Line(glm::vec3 point, glm::vec3 dir) : Point{point}, Dir{dir} {}

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
    @brief Outputs the position of the intersection of two lines
    @param point A point through which the line goes
    @param dir a colinear vector to the line
    @return false if both lines where paralel or equal or if they dont intersect, true else.
    */
    bool LineIntersect(glm::vec3 point1, glm::vec3 dir1, glm::vec3 point2, glm::vec3 dir2, glm::vec3 &outResult);

}