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

    glm::vec3 ProjectToPlane(glm::vec3 vector, glm::vec3 normal) {
        return vector - normal * glm::dot(vector, normal);
    }

    glm::vec3 PointToSegment(glm::vec3 point, glm::vec3 seg1, glm::vec3 seg2) {
        const float seg_len = glm::length(seg2-seg1);
        const glm::vec3 dir = (seg2 - seg1) / seg_len;

        const float t = glm::clamp(glm::dot(point - seg1, dir), 0.0f, seg_len);

        return -(point - seg1 - dir * t);
    }

    void SegmentsClosestPoints(glm::vec3 a1, glm::vec3 a2, glm::vec3 b1, glm::vec3 b2, glm::vec3 &outAPoint, glm::vec3 &outBPoint)
    {
        /*
        Thanks to Johnathon Selstad ! @https://zalo.github.io/blog/closest-point-between-segments/
        */
       
        const glm::vec3 A = a2 - a1;
        const float A_len = glm::length(A);
        const glm::vec3 a1b1_proj = ProjectToPlane(b1 - a1, A / A_len);
        const glm::vec3 a1b2_proj = ProjectToPlane(b2 - a1, A / A_len);
        
        float t = glm::dot(-a1b1_proj, glm::normalize(a1b2_proj - a1b1_proj));
        t = glm::clamp(t, 0.0f, 1.0f);

        outBPoint = b1 * (1.0f - t) + (b2 * t);
        outAPoint = outBPoint + PointToSegment(outBPoint, a1, a2);
    }

    float LenSquared(glm::vec3 &vec) {
        return vec.x * vec.x + vec.y * vec.y + vec.z * vec.z;
    }

    glm::vec3 ApplyRotation(glm::vec3 rotation, glm::vec3 vector)
    {
        const float ca = glm::cos(rotation.y);
        const float sa = glm::sin(rotation.y);
        const float cb = glm::cos(rotation.x);
        const float sb = glm::sin(rotation.x);
        const float cc = glm::cos(rotation.z);
        const float sc = glm::sin(rotation.z);
        return glm::mat3{
                   {(ca * cc + sa * sb * sc),
                    (cb * sc),
                    (ca * sb * sc - cc * sa)},

                   {(cc * sa * sb - ca * sc),
                    (cb * cc),
                    (ca * cc * sb + sa * sc)},

                   {(cb * sa),
                    (-sb),
                    (ca * cb)}} *
               vector;
    }
}