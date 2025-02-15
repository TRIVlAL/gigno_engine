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
    CONVAR(bool, phys_draw_hinges, false, "Red : Hinges targets, Green : Hinges current position.");
    CONVAR(bool, phys_draw_bounding_box, false, "Draws Wireframe of the bounding boxes of the objects (except planes).")


    RigidBody::RigidBody()
    : RenderedEntity() {
        if(GetApp() && GetApp()->GetPhysicServer()) {
            GetApp()->GetPhysicServer()->SubscribeRigidBody(this);
        }
    }

    RigidBody::~RigidBody() {
        if(GetApp() && GetApp()->GetPhysicServer()) {
            GetApp()->GetPhysicServer()->UnsubscribeRigidBody(this);
        }

        /*
        if(CollisionModelPath) {
            delete[] CollisionModelPath;
        }
        */ //cannot, fsr?
    }

    void RigidBody::AddForce(const glm::vec3 &force, const glm::vec3 &application) {
        Force += force;

        Torque += glm::cross(application, force);
    }

    void RigidBody::AddImpulse(const glm::vec3 &impulse, const glm::vec3 &application) {
        if(impulse != impulse) {
            Console::LogInfo("NaN IMpulse");
        }
        
        Velocity += impulse;


        AngularVelocity += glm::cross(application, impulse) / InertiaMoment;
    }

    void RigidBody::AddTorque(const glm::vec3 &torque) {
        Torque += torque;
    }

    void RigidBody::AddRotationImpulse(const glm::vec3 &rotation) {
        AngularVelocity += rotation;
    }

    void RigidBody::Init() {
        RenderedEntity::Init();

        if(ColliderType == COLLIDER_HULL) {
            if(CollisionModelPath) {
                GetApp()->GetPhysicServer()->AllocateCollisionModel(CollisionModelPath);
                UpdateTransformedModel();
            } else {
                Console::LogError("Rigidbody with collider type 'COLLIDER_HULL' requires CollisionModelPath to be set !");
                ColliderType = COLLIDER_SPHERE;
            }
        }

        m_WorldTargetHinge = ApplyRotation(Rotation, HingePosition) + Position;
    }

    void RigidBody::LatePhysicThink(float dt) {
        if(IsStatic) {
            UpdateBoundingBox();
            if(ColliderType == COLLIDER_HULL) {
                UpdateTransformedModel();
            }
            return;
        }

        if(!strcmp(Name, "b")) {
            int i = 0;
        }

        //Apply Hinge
        if(HingeDirection != glm::vec3{0.0f, 0.0f, 0.0f}) {
            const glm::vec3 current_hinge_pos = ApplyRotation(Rotation, HingePosition) + Position;
            const glm::vec3 diff = m_WorldTargetHinge - current_hinge_pos;
            const glm::vec3 diff_plane = ProjectToPlane(diff, HingeDirection);
            float dist = glm::length(diff_plane);

            if(HingePower == 0.0f) {
                Position += diff_plane;
                Rotation = glm::vec3{HingeDirection.x * Rotation.x, HingeDirection.y * Rotation.y, HingeDirection.z * Rotation.z};
            } else {
                const glm::vec3 target_rot = glm::vec3{HingeDirection.x * Rotation.x, HingeDirection.y * Rotation.y, HingeDirection.z * Rotation.z};
                const glm::vec3 rot_diff = target_rot - Rotation;

                AddForce(diff_plane * dist * dist * HingePower, HingePosition);
                // FIXME : Doesnt work, I can-t find a way to fix it! Just use the snap version for now i guess.
                // AddTorque(rot_diff * HingePower * 100.0f);
                Rotation = glm::vec3{HingeDirection.x * Rotation.x, HingeDirection.y * Rotation.y, HingeDirection.z * Rotation.z};
            }
        }

        //Gravity
        AddForce((glm::vec3)convar_phys_gravity * Mass);

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
        AngularVelocity+= dt * Torque / Mass / InertiaMoment;
        avrg_rot_vel += AngularVelocity;
        avrg_rot_vel *= 0.5f;
        Rotation += dt * avrg_rot_vel;

        Rotation.x = glm::mod<float>(Rotation.x, glm::pi<float>()*2);
        Rotation.y = glm::mod<float>(Rotation.y, glm::pi<float>()*2);
        Rotation.z = glm::mod<float>(Rotation.z, glm::pi<float>()*2);

        Force = glm::vec3{0.0f};
        Torque = glm::vec3{0.0f};

        if(ColliderType == COLLIDER_HULL && avrg_rot_vel != glm::vec3{0.0f}) {
            UpdateTransformedModel();
        }
        UpdateBoundingBox();
    }

    const CollisionModel_t *RigidBody::GetModel() {
        if(GetApp() && GetApp()->GetPhysicServer()) {
            return GetApp()->GetPhysicServer()->GetCollisionModel(ModelPath);
        } else {
            return nullptr;
        }
    }

    void RigidBody::UpdateTransformedModel()
    {
        if(!strcmp(Name, "wall")) {
            int i = 0;
        }

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

    void RigidBody::UpdateBoundingBox()
    {
        IsBBCollide = false;
        float epsilon = 0.0001f;
        switch(ColliderType) {
            case COLLIDER_SPHERE : {
                BBMin = Position + glm::vec3{-Radius};
                BBMax = Position + glm::vec3{Radius};
                break;
            }
            case COLLIDER_CAPSULE : {
                glm::vec3 up = ApplyRotate(glm::vec3{0.0f, 1.0f, 0.0f});
                glm::vec3 diag = ApplyRotate(glm::vec3{0.707f, 0.0f, .707f});
                BBMin = Position - (up * Length + Radius) - diag * Radius;
                BBMax = Position + (up * Length + Radius) + diag * Radius;
                break;
            }
            case COLLIDER_HULL : {
                BBMin = glm::vec3{FLT_MAX};
                BBMax = glm::vec3{-FLT_MAX};
                for(glm::vec3 vert : TransformedModel) {
                    const glm::vec3 world_vert = Position + vert;

                    if(world_vert.x < BBMin.x) { BBMin.x = world_vert.x; } 
                    if(world_vert.y < BBMin.y) { BBMin.y = world_vert.y; } 
                    if(world_vert.z < BBMin.z) { BBMin.z = world_vert.z; }

                    if(world_vert.x > BBMax.x) { BBMax.x = world_vert.x; } 
                    if(world_vert.y > BBMax.y) { BBMax.y = world_vert.y; } 
                    if(world_vert.z > BBMax.z) { BBMax.z = world_vert.z; } 
                }
                break;
            }
            case COLLIDER_PLANE : {
                if(Normal == glm::vec3{0.0f, 1.0f, 0.0f} || Normal == glm::vec3{0.0f, -1.0f, 0.0f}) {
                    BBMin = glm::vec3{-FLT_MAX, Position.y - epsilon, -FLT_MAX};
                    BBMax = glm::vec3{FLT_MAX, Position.y + epsilon, FLT_MAX};
                }
                else {
                    BBMin = glm::vec3{-FLT_MAX, -FLT_MAX, -FLT_MAX};
                    BBMax = glm::vec3{FLT_MAX, FLT_MAX, FLT_MAX};
                }
                break;
            }
        }
    } 

    void RigidBody::Think(float dt) {

        DoRender = ((int)convar_phys_draw_colliders != 2 || ColliderType == COLLIDER_PLANE);
        if((int)convar_phys_draw_colliders != 0) {
            DrawCollider();

            if(ColliderType == COLLIDER_PLANE) {
                return;
            }
        }

        if((bool)convar_phys_draw_hinges) {
            RenderingServer *r = GetApp()->GetRenderer();
            r->DrawLine(m_WorldTargetHinge - HingeDirection * 5.0f, m_WorldTargetHinge + HingeDirection * 5.0f, glm::vec3{1.0f, 0.0f, 0.0f});
            r->DrawPoint(m_WorldTargetHinge, glm::vec3{0.5f, 0.0f, 0.0f});

            const glm::vec3 curr_target = ApplyRotation(Rotation, HingePosition) + Position;

            const glm::vec3 current_hinge_pos = ApplyRotation(Rotation, HingePosition) + Position;
            const glm::vec3 diff = m_WorldTargetHinge - current_hinge_pos;
            const glm::vec3 diff_plane = ProjectToPlane(diff, HingeDirection);
            const float dist = glm::length(diff_plane);

            const glm::vec3 local_hinge_direction = ApplyRotation(Rotation, HingeDirection);
            const float dot = glm::dot(HingeDirection, local_hinge_direction);
            const float angle_diff = glm::acos(dot);
            const glm::vec3 rotation_axis = angle_diff > 0.0f ? glm::normalize(glm::cross(HingeDirection, local_hinge_direction)) : glm::vec3{};

            r->DrawLine(curr_target - local_hinge_direction * 5.0f, curr_target + local_hinge_direction * 5.0f, glm::vec3{0.0f, 1.0f, 0.0f});
            r->DrawLine(curr_target, curr_target + rotation_axis, glm::vec3{0.0f, 0.0f, 1.0f});
            r->DrawPoint(curr_target, glm::vec3{0.0f, 0.5f, 0.0f});
        }

        if((bool)convar_phys_draw_bounding_box) {
            RenderingServer *r = GetApp()->GetRenderer();
            
            const glm::vec3 a = BBMin;
            const glm::vec3 b = glm::vec3{BBMin.x, BBMin.y, BBMax.z};
            const glm::vec3 c = glm::vec3{BBMin.x, BBMax.y, BBMin.z};
            const glm::vec3 d = glm::vec3{BBMax.x, BBMin.y, BBMin.z};
            const glm::vec3 e = glm::vec3{BBMin.x, BBMax.y, BBMax.z};
            const glm::vec3 f = glm::vec3{BBMax.x, BBMin.y, BBMax.z};
            const glm::vec3 g = glm::vec3{BBMax.x, BBMax.y, BBMin.z};
            const glm::vec3 h = BBMax;

            const glm::vec3 col = IsBBCollide ? glm::vec3{1.0f, 0.0f, 0.0f} : glm::vec3{0.0f, 0.0f, 1.0f};

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

        //HACK HACK : For demo_3.map, no clean impl. for now :)
        if(strcmp(Name, "a") == 0) {
            const float pow = 1.0f * dt;
            if(GetApp()->GetInputServer()->GetKey(KEY_T)) {
                AddImpulse(glm::vec3{2.0f, 0.0f, 0.0f} * pow);
            }
            if(GetApp()->GetInputServer()->GetKey(KEY_G)) {
                AddImpulse(glm::vec3{-2.0f, 0.0f, 0.0f} * pow);
            }
            if(GetApp()->GetInputServer()->GetKey(KEY_F)) {
                AddImpulse(glm::vec3{0.0f, 0.0f, 2.0f} * pow);
            }
            if(GetApp()->GetInputServer()->GetKey(KEY_H)) {
                AddImpulse(glm::vec3{0.0f, 0.0f, -2.0f} * pow);
            }
            if(GetApp()->GetInputServer()->GetKey(KEY_SPACE)) {
                AddImpulse(glm::vec3{0.0f, 2.0f, 0.0f} * pow);
            }
            if(GetApp()->GetInputServer()->GetKey(KEY_LEFT_CONTROL)) {
                AddImpulse(glm::vec3{0.0f, -2.0f, 0.0f} * pow);
            }
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