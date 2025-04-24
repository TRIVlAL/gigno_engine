#ifndef CONSTRAINTS_H
#define CONSTRAINTS_H

#include <glm/glm.hpp>
#include "../entities/entity.h"

namespace gigno {
    class RigidBody;

    /*
    Constraints allow to quickly simulate a simplification of complex systems.
    The two currently implemented constraints are :
        - RopeConstraint
        - HingeConstraint

    The PHysicsConstraint Base Class Has no functionnalities and shall not be spawned in a map. 
    */
    class PhysicsConstraint : public Entity {
        ENTITY_DECLARATIONS(PhysicsConstraint, Entity)
    public:
        virtual void Init() override;
        virtual void CleanUp() override;
        virtual void Solve(float dt) {/*ERR*/};

        PhysicsConstraint *pNextConstraint{};
    };

    BEGIN_KEY_TABLE(PhysicsConstraint)
    END_KEY_TABLE

    /*
    Object is limited to move at a max distance 'Distance; to it's  point 'Point'

    TargetName is the name of the rigidbody that should be constrained. 
        That Rigidbody must exist on Init (i.e. spawned throught the .map file f.e.)for the constraint to take effect.
    Distance shall be positive.
    Point is relative to the Target's initial position.
    */
    class RopeConstraint : public PhysicsConstraint {
        ENTITY_DECLARATIONS(RopeConstraint, PhysicsConstraint)
    public:

        virtual void Init() override;
        virtual void Think(float dt) override;
        virtual void Solve(float dt) override;

        const char *TargetName{""};
        RigidBody *Target{nullptr};
        glm::vec3 Point{};
        float Distance{};
    };

    BEGIN_KEY_TABLE(RopeConstraint)
        DEFINE_KEY_VALUE(cstr, TargetName)
        DEFINE_KEY_VALUE(vec3, Point)
        DEFINE_KEY_VALUE(float, Distance)
    END_KEY_TABLE

    /*
    Object position is Locked at 'Point', and it can rotate only around the 'Axis' 

    TargetName is the name of the rigidbody that should be constrained.
        That Rigidbody must exist on Init (i.e. spawned throught the .map file f.e.)for the constraint to take effect.
    Point is relative to the Target's initial position.
    */
    class HingeConstraint : public PhysicsConstraint {
        ENTITY_DECLARATIONS(HingeConstraint, PhysicsConstraint)
    public:
        virtual void Init() override;
        virtual void Think(float dt) override;
        virtual void Solve(float dt) override;

        const char *TargetName{""};
        RigidBody *Target{nullptr};
        glm::vec3 Point{}; //Relative to body initial position
        glm::vec3 Axis{};

    };

    BEGIN_KEY_TABLE(HingeConstraint)
        DEFINE_KEY_VALUE(cstr, TargetName)
        DEFINE_KEY_VALUE(vec3, Point)
        DEFINE_KEY_VALUE(vec3, Axis)
    END_KEY_TABLE

}

#endif