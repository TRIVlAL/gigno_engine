#include "constraints.h"
#include "rigid_body.h"
#include "../debug/console/convar.h"
#include "../application.h"

namespace gigno {

    CONVAR(float, phys_constraint_bias_factor, 0.05f, "");
    CONVAR(bool, phys_draw_constraints, true, "toggles debug drawings of physics constraints");

    ENTITY_DEFINITIONS(PhysicsConstraint, Entity)

    void PhysicsConstraint::Init() {
        GetApp()->GetPhysicServer()->SubscribeConstraint(this);

        Entity::Init();
    }

    void PhysicsConstraint::CleanUp() {
        GetApp()->GetPhysicServer()->UnsubscribeConstraint(this);

        Entity::CleanUp();
    }

    ENTITY_DEFINITIONS(RopeConstraint, PhysicsConstraint)

    void RopeConstraint::Init() {
        if(strcmp(TargetName, "") != 0) {
            Target = dynamic_cast<RigidBody *>(GetApp()->GetEntityServer()->GetEntityByName(TargetName));
            if(!Target) {
                Console::LogInfo("RopeConstraint : Rigidbody named '%s' does not exist !", TargetName);
            }
        }

        if(Target) {
            Point = Target->Position + Point;
        }

        PhysicsConstraint::Init();
    }

    void RopeConstraint::Think(float dt) {
        if((bool)convar_phys_draw_constraints && Target) {
            GetApp()->GetRenderer()->DrawLine(Target->Position, Point, glm::vec3{1.0f, 0.0f, 0.0f});
        }

        PhysicsConstraint::Think(dt);
    }

    void RopeConstraint::Solve(float dt) {
        if(!Target) {
            return;
        }

        const glm::vec3 velocity = Target->Velocity;

        const glm::vec3 delta = Target->Position - Point;
        const float delta_mag = glm::length(delta);
        const glm::vec3 delta_dir = delta / delta_mag;

        const float offset = delta_mag - Distance;
        
        if(offset <= 0.0f) {
            return;
        }
        
        const glm::vec3 jacobian = -delta_dir;
        
        const float effective_mass = Target->Mass;

        const float bias =  -((float)convar_phys_constraint_bias_factor / dt) * offset;

        const float lambda = -effective_mass * (glm::dot(jacobian, velocity) + bias);

        Target->AddImpulse(jacobian * lambda);
    }

    ENTITY_DEFINITIONS(HingeConstraint, PhysicsConstraint)

    void HingeConstraint::Init() {
        if(strcmp(TargetName, "") != 0) {
            Target = dynamic_cast<RigidBody *>(GetApp()->GetEntityServer()->GetEntityByName(TargetName));
            if(!Target) {
                Console::LogInfo("HingeConstraint : Rigidbody named '%s' does not exist !", TargetName);
            }
        }

        Axis = glm::normalize(Axis);

        if(Target) {
            Point = Target->Position + Point;
        }

        PhysicsConstraint::Init();
    }

    void HingeConstraint::Think(float dt) {
        if((bool)convar_phys_draw_constraints && Target) {
            float mult = 2.0f * glm::dot(Target->Scale, Axis);
            GetApp()->GetRenderer()->DrawLine(Point + mult * Axis, Point - mult * Axis, glm::vec3{1.0f, 0.0f, 0.0f});
        }

        PhysicsConstraint::Think(dt);
    }

    void HingeConstraint::Solve(float dt) {
        if(!Target) {
            return;
        }

        //Constraint position
        {
            const glm::vec3 velocity = Target->Velocity;

            const glm::vec3 delta = Target->Position - Point;
            const float delta_mag = glm::length(delta);
            const glm::vec3 delta_dir = delta / delta_mag;

            const float offset = delta_mag;

            if (offset == 0.0f) {
                return;
            }

            const glm::vec3 jacobian = glm::normalize(-delta_dir);

            const float effective_mass = Target->Mass;

            const float bias = -((float)convar_phys_constraint_bias_factor / dt) * offset;

            const float lambda = -effective_mass * (bias);

            Target->AddImpulse(jacobian * lambda);
            Target->AddImpulse(-velocity * effective_mass);
        }

        //Constraint rotation
        {
            const glm::vec3 ang_vel = Target->AngularVelocity;
    
            const float total = glm::length(ang_vel);
            const float along_axis = glm::dot(ang_vel, Axis);

            const glm::vec3 dir = ApplyRotation(Target->Rotation, Axis);
            const float angle_offset = glm::acos(glm::clamp(glm::dot(dir, Axis), -1.0f, 1.0f));

            const float offset = angle_offset;
    
            if (offset == 0.0f) {
                return;
            }
    
            const glm::vec3 jacobian = -glm::normalize(glm::cross(dir, Axis));
    
            const mat3 effective_mass = Target->Mass * Target->InertiaTensor;
    
            const float bias = ((float)convar_phys_constraint_bias_factor / dt) * offset;
    
            const mat3 lambda = -effective_mass * (bias);
    
            Target->AddRotationImpulse(jacobian * lambda);

            Target->AddRotationImpulse(-(ang_vel - Axis * along_axis) * effective_mass);
        }
    }

}
