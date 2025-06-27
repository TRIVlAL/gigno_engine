#include "rigid_body.h"
#include "application.h"
#include "../debug/console/convar.h"

#include "../vendor/tiny_object_loader/tiny_obj_loader.h"
#include "error_macros.h"

#include <unordered_map>

#include "gjk.h"

#include "physic_server.h"

#include <exception>

namespace gigno {
    ENTITY_DEFINITIONS(RigidBody, RenderedEntity)

    #define DEFAULT_GRAVITY glm::vec3{0.0f, -9.81f, 0.0f}
    CONVAR(glm::vec3, phys_gravity, DEFAULT_GRAVITY, "");

    CONVAR(int, phys_draw_colliders, 0, "1 : Wireframe of the collider, models are drawn." 
                                        "2 : Wireframe of the collider, models are not drawn.")
    CONVAR(bool, phys_draw_bounding_box, false, "Draws Wireframe of the bounding boxes of the objects (except planes).")

    void RigidBody::AddForce(const glm::vec3 &force, const glm::vec3 &application) {
        Force += force;

        Torque += glm::cross(application, force);
    }

    void RigidBody::AddImpulse(const glm::vec3 &impulse, const glm::vec3 &application) {
        if(impulse != impulse) {
            Console::LogWarning("NaN IMpulse");
        }
        
        float mass_inv = IsStatic ? 0.0f : 1.0f / Mass;


        Velocity += impulse * mass_inv;

        AngularVelocity += glm::cross(application, impulse) * GetInverseInertiaTensor();
    }

    void RigidBody::AddTorque(const glm::vec3 &torque) {
        Torque += torque;
    }

    void RigidBody::AddRotationImpulse(const glm::vec3 &rotation) {
        AngularVelocity += rotation * GetInverseInertiaTensor();
    }

    void RigidBody::Init() {
        RenderedEntity::Init();

        GetApp()->GetPhysicServer()->SubscribeRigidBody(this);

        UpdateCollider();

        if(ColliderType == COLLIDER_HULL) {
            if(CollisionModelPath) {
                GetApp()->GetPhysicServer()->AllocateCollisionModel(CollisionModelPath);
                UpdateTransformedModel();
                UpdateCollider();
            } else {
                Console::LogError("Rigidbody with collider type 'COLLIDER_HULL' requires CollisionModelPath to be set !");
                ColliderType = COLLIDER_SPHERE;
            }
        }

        UpdateInertiaTensor();

        m_WasRendered = DoRender;
    }

    void RigidBody::LatePhysicThink(float dt) {
        
        if(ColliderType == COLLIDER_HULL) {
            UpdateTransformedModel();
        }
        UpdateCollider();
        UpdateInertiaTensor(); //?

        if(IsStatic) {
            return;
        }

        //Gravity
        if(!m_GravityDisabled) {
            AddForce((glm::vec3)convar_phys_gravity * Mass);
        }
        m_GravityDisabled = false;

        //Drag
        // Uses a linear approximation as can be seen in Unity or Godot.
        float ldrag = glm::clamp(1.0f - (Drag * dt), 0.0f, 1.0f);
        Velocity *= ldrag;
        float rdrag = glm::clamp(1.0f - (AngularDrag * dt), 0.0f, 1.0f);
        AngularVelocity *= rdrag;

        const float epsilon = 0.00000000001;
        if(LenSquared(Velocity) < epsilon) {
            Velocity  = glm::vec3{0.0f};
        }
        if(LenSquared(AngularVelocity) < epsilon) {
            AngularVelocity = glm::vec3{0.0f};
        }

        glm::vec3 avrg_vel = Velocity;
        Velocity += dt * Force / Mass;
        avrg_vel += Velocity;
        avrg_vel *= 0.5f;
        glm::vec3 applied_vel = dt * avrg_vel;
        Position += applied_vel;

        glm::vec3 avrg_rot_vel = AngularVelocity;
        AngularVelocity += dt * (Torque / Mass) * GetInverseInertiaTensor();
        avrg_rot_vel += AngularVelocity;
        avrg_rot_vel *= 0.5f;

        if(!LockRotation) {
            Rotation += glm::quat{0.0f, avrg_rot_vel * dt * 0.5f} * Rotation;
            Rotation = glm::normalize(Rotation);
        }

        Force = glm::vec3{0.0f};
        Torque = glm::vec3{0.0f};

        
        if(ColliderType == COLLIDER_HULL) {
            UpdateTransformedModel();
        }
        UpdateCollider();
        UpdateInertiaTensor();
    }

    void RigidBody::CleanUp() {

        GetApp()->GetPhysicServer()->UnsubscribeRigidBody(this);
        RenderedEntity::CleanUp();
    }

    void RigidBody::UpdateCollider() {
        m_Collider.ColliderType = ColliderType;
        m_Collider.Position = Position;
        m_Collider.Rotation = Rotation;
        m_Collider.Radius = Radius;
        m_Collider.Length = Length;
        m_Collider.Normal = Normal;
        if (ColliderType == COLLIDER_HULL) {
            m_Collider.Model = GetModel();

            m_Collider.IsTransformModelProxi = true;
            m_Collider.TransformedModelProxi = &TransformedModel;
        }
        m_Collider.SetBoundingBox();
    }

    void RigidBody::UpdateInertiaTensor() {
        m_TransformedInverseInertiaTensor = IsStatic ? glm::mat3{0.0f} : RotationMatrix() * glm::inverse(InertiaTensor * Mass) * glm::transpose(RotationMatrix());
    }

    Collider_t &RigidBody::AsCollider() {
        return m_Collider;
    }

    const CollisionModel_t *RigidBody::GetModel() const {
        ASSERT_V(ColliderType == COLLIDER_HULL, nullptr);
        if(GetApp() && GetApp()->GetPhysicServer()) {
            return GetApp()->GetPhysicServer()->GetCollisionModel(CollisionModelPath);
        } else {
            return nullptr;
        }
    }

    glm::mat3 RigidBody::GetInverseInertiaTensor() const {
        return m_TransformedInverseInertiaTensor;
    }

    void RigidBody::UpdateTransformedModel()
    {
        ASSERT(ColliderType == COLLIDER_HULL);

        const CollisionModel_t *model = GetModel();

        TransformedModel.resize(model->Vertices.size());

        for (size_t i = 0; i < model->Vertices.size(); i++) {
            glm::vec3 vert = model->Vertices[i];
            vert.x *= Scale.x;
            vert.y *= Scale.y;
            vert.z *= Scale.z;
            TransformedModel[i] = ApplyRotation(Rotation, vert);
        }
    }

    void RigidBody::Think(float dt) {

        if((int)convar_phys_draw_colliders == 2 && ColliderType != COLLIDER_PLANE) {
            DoRender = false;
        } else {
            DoRender = m_WasRendered;
        }

        if((int)convar_phys_draw_colliders != 0) {
            DrawCollider();

            if(ColliderType == COLLIDER_PLANE) {
                return;
            }
        }

        if((bool)convar_phys_draw_bounding_box) {
            RenderingServer *r = GetApp()->GetRenderer();
            
            BoundingBox_t aabb = AsCollider().AABB;
            const glm::vec3 a = aabb.Min;
            const glm::vec3 b = glm::vec3{aabb.Min.x, aabb.Min.y, aabb.Max.z};
            const glm::vec3 c = glm::vec3{aabb.Min.x, aabb.Max.y, aabb.Min.z};
            const glm::vec3 d = glm::vec3{aabb.Max.x, aabb.Min.y, aabb.Min.z};
            const glm::vec3 e = glm::vec3{aabb.Min.x, aabb.Max.y, aabb.Max.z};
            const glm::vec3 f = glm::vec3{aabb.Max.x, aabb.Min.y, aabb.Max.z};
            const glm::vec3 g = glm::vec3{aabb.Max.x, aabb.Max.y, aabb.Min.z};
            const glm::vec3 h = aabb.Max;
            
            glm::vec3 col{1.0f, 0.0f, 0.0f};

            r->DrawLine(a, b, col);
            r->DrawLine(a, c, col);
            r->DrawLine(a, d, col);

            r->DrawLine(b, e, col);
            r->DrawLine(b, f, col);

            r->DrawLine(c, e, col);
            r->DrawLine(c, g, col);

            r->DrawLine(d, f, col);
            r->DrawLine(d, g, col);

            r->DrawLine(h, e, col);
            r->DrawLine(h, f, col);
            r->DrawLine(h, g, col);
        }
    }
    
    void RigidBody::DrawCollider() {
        #if USE_DEBUG_DRAWING
        RenderingServer *r = Application::Singleton()->GetRenderer();

        const glm::vec3 up = ApplyRotation( Rotation, glm::vec3{0.0f, 1.0f, 0.0f});
        const glm::vec3 right = ApplyRotation( Rotation, glm::vec3{1.0f, 0.0f, 0.0f});
        const glm::vec3 forward = ApplyRotation( Rotation, glm::vec3{0.0f, 0.0f, 1.0f});

        const glm::vec3 color = glm::vec3{0.8f, 0.0f, 0.0f};
        const glm::vec3 color_faded = glm::vec3{0.6f, 0.0f, 0.0f};

        if( ColliderType == COLLIDER_SPHERE) {
            r->DrawPoint( Position, color_faded);

            const int res = 12;
            const int vertical_res = 3;
            const float angle_step = 2 * glm::pi<float>() / res;
            float curr_angle = 0;
            glm::vec3 a;
            glm::vec3 b = (right * glm::cos(curr_angle) + forward * glm::sin(curr_angle)) *  Radius;

            for(int i = 0; i < res; i++) {
                a = b;
                curr_angle += angle_step;
                b = (right * glm::cos(curr_angle) + forward * glm::sin(curr_angle)) *  Radius;
                for(int i = 0; i < vertical_res; i++) {
                    r->DrawLine( Position + a * glm::cos(glm::pi<float>() * 0.5f / vertical_res * i) + up * glm::sin(glm::pi<float>() * 0.5f / vertical_res * i) *  Radius,
                                 Position + b * glm::cos(glm::pi<float>() * 0.5f / vertical_res * i) + up * glm::sin(glm::pi<float>() * 0.5f / vertical_res * i) *  Radius,
                                 color);
                    r->DrawLine( Position + a * glm::cos(glm::pi<float>() * 0.5f / vertical_res * i) - up * glm::sin(glm::pi<float>() * 0.5f / vertical_res * i) *  Radius,
                                 Position + b * glm::cos(glm::pi<float>() * 0.5f / vertical_res * i) - up * glm::sin(glm::pi<float>() * 0.5f / vertical_res * i) *  Radius,
                                 color);
                }
            }

            r->DrawPoint( Position + up *  Radius, color);
            r->DrawPoint( Position - up *  Radius, color);
        } else if( ColliderType == COLLIDER_CAPSULE) {

            const int res = 8;
            const int vertical_res = 3;
            const float angle_step = 2 * glm::pi<float>() / res;
            float curr_angle = 0;
            glm::vec3 a;
            glm::vec3 b = (right * glm::cos(curr_angle) + forward * glm::sin(curr_angle)) *  Radius;

            for(int i = 0; i < res; i++) {
                a = b;
                curr_angle += angle_step;
                b = (right * glm::cos(curr_angle) + forward * glm::sin(curr_angle)) *  Radius;
                for(int i = 0; i < vertical_res; i++) {
                    r->DrawLine( Position + a * glm::cos(glm::pi<float>() * 0.5f / vertical_res * i) + up * glm::sin(glm::pi<float>() * 0.5f / vertical_res * i) *  Radius + up *  Length * 0.5f,
                                 Position + b * glm::cos(glm::pi<float>() * 0.5f / vertical_res * i) + up * glm::sin(glm::pi<float>() * 0.5f / vertical_res * i) *  Radius + up *  Length * 0.5f,
                                color);
                    r->DrawLine( Position + a * glm::cos(glm::pi<float>() * 0.5f / vertical_res * i) - up * glm::sin(glm::pi<float>() * 0.5f / vertical_res * i) *  Radius - up *  Length * 0.5f,
                                 Position + b * glm::cos(glm::pi<float>() * 0.5f / vertical_res * i) - up * glm::sin(glm::pi<float>() * 0.5f / vertical_res * i) *  Radius - up *  Length * 0.5f,
                                color);
                }
                r->DrawLine( Position + b - up *  Length * 0.5f,  Position + b + up *  Length * 0.5f, color);
            }

            r->DrawPoint( Position + up * ( Length * 0.5f +  Radius), color);
            r->DrawPoint( Position - up * ( Length * 0.5f +  Radius), color);
        } else if( ColliderType == COLLIDER_HULL) {
            const CollisionModel_t *model = GetModel();

            for(size_t i = 0; i <  model->Indices.size(); i += 3) {
                const glm::vec3 a =  Position + TransformedModel[model->Indices[i]];
                const glm::vec3 b =  Position + TransformedModel[model->Indices[i+1]];
                const glm::vec3 c =  Position + TransformedModel[model->Indices[i+2]];

                r->DrawLine(a, b, color);
                r->DrawLine(b, c, color);
                r->DrawLine(c, a, color);
            }
        }
        #endif
    }
}