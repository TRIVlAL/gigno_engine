#ifndef CONSTRAINTS_H
#define CONSTRAINTS_H

#include <glm/glm.hpp>

namespace gigno {
    class RigidBody;

    class Constraint {
    public:
        
        virtual void Solve(float dt) = 0;

    private:

    };

    class RopeConstraint : public Constraint {
    public:
        RopeConstraint(RigidBody *rb, glm::vec3 point, float distance) : 
            m_Point{point}, m_Distance{distance}, m_Body{rb} {}

        virtual void Solve(float dt) override;

    private:
    RigidBody *m_Body;
        glm::vec3 m_Point{};
        float m_Distance{};

    };

    class HingeConstraint : public Constraint {
    public:
        HingeConstraint(RigidBody *rb, glm::vec3 point, glm::vec3 axis) : 
            m_Point{point}, m_Axis{axis}, m_Body{rb} {m_Axis = glm::normalize(m_Axis);}

        virtual void Solve(float dt) override;

    private:
        RigidBody *m_Body;
        glm::vec3 m_Point{};
        glm::vec3 m_Axis{};

    };

}

#endif