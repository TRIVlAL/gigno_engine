#include "geometry.h"
#include "../debug/console/console.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "../glm/glm/gtx/quaternion.hpp"

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
        const float seg_len = glm::length(seg2 - seg1);
        const glm::vec3 dir = (seg2 - seg1) / seg_len;

        const float t = glm::clamp(glm::dot(point - seg1, dir), 0.0f, seg_len);

        return seg1 + (dir * t);
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

        const glm::vec3 B_proj = a1b2_proj - a1b1_proj;
        const float B_proj_len = glm::length(B_proj);
        
        float t = glm::dot(-a1b1_proj, B_proj/B_proj_len);
        t /= B_proj_len;
        t = glm::clamp(t, 0.0f, 1.0f);

        outAPoint = PointToSegment(b1 * (1.0f - t) + (b2 * t), a1, a2);
        outBPoint = PointToSegment(outAPoint, b1, b2);
    }

    float LenSquared(const glm::vec3 &vec) {
        return vec.x * vec.x + vec.y * vec.y + vec.z * vec.z;
    }

    glm::vec3 ApplyRotation(glm::quat rotation, glm::vec3 vector) {
        glm::quat result{rotation * glm::quat{0.0f, vector.x, vector.y, vector.z} * InverseQuat(rotation)};
        return glm::vec3{result.x, result.y, result.z};
    }

    float DistanceToOrigin(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 &outNormal) {

        outNormal = glm::cross(b - a, c - a);
        outNormal = glm::normalize(outNormal);

        float ret = glm::dot(-a, glm::dot(a, outNormal) < 0 ? outNormal : -outNormal);
        if(ret != ret) {
            //Console::LogWarning("DistanceToOrigin NaN with parameters : a = (%f, %f, %f), b = (%f, %f, %f), c = (%f, %f, %f), outNormal = (%f, %f, %f)", a.x, a.y, a.z,
            //b.x, b.y, b.z, c.x, c.y, c.z, outNormal.x, outNormal.y, outNormal.z);
            ret = FLT_MAX;
        }
        return ret;
    }

    
    void Barycentric(glm::vec3 p, glm::vec3 a, glm::vec3 b, glm::vec3 c, float &u, float &v, float &w) {
        // From Chris Ericson's book - Real Time Collision Detection - p.47
        // @ https://www.r-5.org/files/books/computers/algo-list/realtime-3d/Christer_Ericson-Real-Time_Collision_Detection-EN.pdf

        glm::vec3 v0 = b - a, v1 = c - a, v2 = p - a;
        float d00 = glm::dot(v0, v0);
        float d01 = glm::dot(v0, v1);
        float d11 = glm::dot(v1, v1);
        float d20 = glm::dot(v2, v0);
        float d21 = glm::dot(v2, v1);
        float denom = d00 * d11 - d01 * d01;
        if(denom == 0.0f) {
            //Console::LogWarning("Barycentric should return NaN with arguments : p = (%f, %f, %f), a = (%f, %f, %f), b = (%f, %f, %f), c = (%f, %f, %f)!");
            denom = 1.0f;
        }
        v = (d11 * d20 - d01 * d21) / denom;
        w = (d00 * d21 - d01 * d20) / denom;
        u = 1.0f - v - w;
    }

    glm::quat FromEuler(glm::vec3 euler) {
        glm::quat qx = glm::angleAxis(euler.x, glm::vec3{1.0f, 0.0f, 0.0f});
        glm::quat qy = glm::angleAxis(euler.y, glm::vec3{0.0f, 1.0f, 0.0f});
        glm::quat qz = glm::angleAxis(euler.z, glm::vec3{0.0f, 0.0f, 1.0f});

        return qy * qz * qx;
    }

    glm::quat InverseQuat(glm::quat q) {
        return glm::conjugate(q) / glm::length2(q);
    }
}