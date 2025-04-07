#ifndef RIGID_BODY_H
#define RIGID_BODY_H

#include "entities/rendered_entity.h"
#include "collision.h"
#include "collider_type.h"
#include "algorithm/geometry.h"


namespace gigno {
    struct CollisionModel_t;
    struct Sound_t;

    class RigidBody : public RenderedEntity {
        ENTITY_DECLARATIONS(RigidBody, RenderedEntity)
    public:
        
        void AddForce(const glm::vec3 &force, const glm::vec3 &application = glm::vec3{0.0f, 0.0f, 0.0f});
        void AddImpulse(const glm::vec3 &impulse, const glm::vec3 &application = glm::vec3{0.0f, 0.0f, 0.0f});
        void AddTorque(const glm::vec3 &torque);
        void AddRotationImpulse(const glm::vec3 &rotation);
        
        // Rigidbody will not recieve gravity force during this physics frame.
        void DisableGravity() { m_GravityDisabled = true; }
        
        virtual void Init() override;
        virtual void Think(float dt) override;
        virtual void LatePhysicThink(float dt) override;
        virtual void CleanUp() override;
        
        Collider_t AsCollider() const;
        const CollisionModel_t *GetModel() const;
        glm::mat3 GetInverseInertiaTensor() const;


        ColliderType_t ColliderType = COLLIDER_PLANE;

        /* -------------------------------------------
         Collider properties
        --------------------------------------------*/

        float Radius{};     // For sphere or capsule
        vec3 Normal{};      // normalized plane normal.
        float Length{};     // Full-length of the capsule
        const char *CollisionModelPath = nullptr; //  Path to the file loading the convex hull CollisionModel_t 
                                                  //  if ColliderType is set to COLLIDER_HULL
        std::vector<glm::vec3> TransformedModel{}; // Points of the CollisionModel_t transformed (rotated and scaled)
                                                     // in local space. Updated by UpdateTransformedModel every PhysicThink
                                                     // if CollioderType is COLLIDER_HULL
        /*------------------------------------------*/

        float Mass{1.0f};
        glm::mat3 InertiaTensor{1.0f};

        bool IsStatic{false}; //Static objects are considered like having "infinite mass".

        //Bounciness and friction are combined with the other object during collision
        //Following the Method in collision.cpp
        float Bounciness{0.5f};   //coefficient of return of enery in collision (1.0 means no enegry loss)
        float Friction{0.8f};     //coefficient of friction
        /*
        If vel smaller than friction that the friction that would be applied with
        Friction * StaticFrictionMultiplier as friction coefficient, that static friction
        is aplied, i.e. friction equals to the velocity to make the object stop.
        ! smaller than 1 !
        */
        float StaticFrictionMultiplier{0.3f};

        float Drag = 0.4f;
        float AngularDrag = 2.8f;

        RigidBody *pNextRigidBody{};

        /*--------------------------------------------
        HINGE : The object is locked on an axis positioned at HingePosition. 
                Set HingeDirection to (0,0,0) to disable (default)
        --------------------------------------------*/
        //Local Hinge Position
        glm::vec3 HingePosition{};
        //Axis on which the hinge rotates
        glm::vec3 HingeDirection{};
        // If 0.0, the object snaps to the hinge. 
        // Else, a force is applied to keep the object close to the hinge, proporional to power.
        float HingePower{10.0f};
        /*------------------------------------------*/

        bool LockRotation = false;

        glm::vec3 Force{};
        glm::vec3 Velocity{};

        glm::vec3 Torque{};
        glm::vec3 AngularVelocity{};
        
    private:
        void UpdateTransformedModel();
        void UpdateCollider();
        void UpdateInertiaTensor();
        void DrawCollider();

        glm::mat3 m_TransformedInverseInertiaTensor{};

        Collider_t m_Collider{};
        
        bool m_GravityDisabled = false; // Reset every physics frame
        
        glm::vec3 m_WorldTargetHinge{}; // The world position of the hinge. is set on Init
        
        bool hasCollider = false;
        
        bool m_WasRendered = true; // What was DoRender value before it got modified by convar_phys_draw_colliders
    };

    BEGIN_KEY_TABLE(RigidBody)
        DEFINE_KEY_VALUE(float, Mass)
        DEFINE_KEY_VALUE(mat3, InertiaTensor)
        DEFINE_KEY_VALUE(bool, IsStatic)
        DEFINE_KEY_VALUE(float, Bounciness)
        DEFINE_KEY_VALUE(float, Friction)
        DEFINE_KEY_VALUE(float, StaticFrictionMultiplier)
        DEFINE_KEY_VALUE(float, Drag)
        DEFINE_KEY_VALUE(ColliderType_t, ColliderType)
        DEFINE_KEY_VALUE(float, Radius)
        DEFINE_KEY_VALUE(vec3, Normal)
        DEFINE_KEY_VALUE(float, Length)
        DEFINE_KEY_VALUE(cstr, CollisionModelPath)

        DEFINE_KEY_VALUE(bool, LockRotation)

        DEFINE_KEY_VALUE(vec3, Force)
        DEFINE_KEY_VALUE(vec3, Velocity)
        DEFINE_KEY_VALUE(vec3, Torque)
        DEFINE_KEY_VALUE(vec3, AngularVelocity)
    END_KEY_TABLE


}

#endif