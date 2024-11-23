#include "geometry.h"

namespace gigno {

    Line PlaneIntersect(glm::vec3 point1, glm::vec3 normal1, glm::vec3 point2, glm::vec3 normal2)  {
        if(normal1 == normal2) {
            return Line{};
        }

        const float a1 = normal1.x;
        const float a2 = normal1.y;
        const float a3 = normal1.z;
        const float a4 = -glm::dot(point1, normal1);

        const float b1 = normal2.x;
        const float b2 = normal2.y;
        const float b3 = normal2.z;
        const float b4 = -glm::dot(point2, normal2);

        Line line{};
        line.Point.x = (a2 * b4 - b2 * a4) / (a1 * b2 - a2 * b1);
        line.Point.y = (a1 * b4 - b1 * a4) / (b1 * a2 - a1 * b2);
        line.Point.z = 0.0f;

        line.Dir.x = (a2 * b3 - a3 * b2) / (a1 * b2 - a2 * b1);
        line.Dir.y = (a1 * b3 - b1 * a3) / (b1 * a2 - a1 * b2);
        line.Dir.z = 1.0f;

        return line;
    }

    bool LineIntersect(glm::vec3 point1, glm::vec3 dir1, glm::vec3 point2, glm::vec3 dir2, glm::vec3 &outResult) {
        // TODO : Implement
        return false;
    }
}