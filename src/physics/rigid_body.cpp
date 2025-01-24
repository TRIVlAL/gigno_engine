#include "rigid_body.h"
#include "application.h"
#include "../debug/console/convar.h"

#include "../vendor/tiny_object_loader/tiny_obj_loader.h"
#include "error_macros.h"

#include <unordered_map>

#include "gjk.h"

#include <exception>

namespace gigno {
    ENTITY_DEFINITIONS(RigidBody, RenderedEntity)

    #define DEFAULT_GRAVITY glm::vec3{0.0f, -9.81f, 0.0f}
    CONVAR(glm::vec3, phys_gravity, DEFAULT_GRAVITY, "");

    CONVAR(int, phys_draw_colliders, 0, "1 : Wireframe of the collider, models are drawn." 
                                        "2 : Wireframe of the collider, models are not drawn.")
    CONVAR(bool, phys_draw_hinges, false, "Red : Hinges targets, Green : Hinges current position.");


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
                LoadColliderModel(CollisionModelPath);
            } else {
                Console::LogError("Rigidbody with collider type 'COLLIDER_HULL' requires CollisionModelPath to be set !");
                ColliderType = COLLIDER_SPHERE;
            }
        }
        m_WorldTargetHinge = ApplyRotation(Rotation, HingePosition) + Position;
    }

    CONVAR(bool, phys_new_pos_offset, true, "Whether to use the new method for applying the position offset.");

    void RigidBody::LatePhysicThink(float dt) {
        if(IsStatic) {
            if(ColliderType == COLLIDER_HULL) {
                UpdateRotatedModel();
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
            const float dist = glm::length(diff_plane);

            if(HingePower == 0.0f) {
                PositionOffset += diff_plane;
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
        
        if((bool)convar_phys_new_pos_offset) {
            // New Pos Offset : Only apply the Offset that has't been applied by velocity.
            float pos_offset_len = glm::length(PositionOffset);
            if(pos_offset_len > 0.0f) {
                glm::vec3 pos_offset_norm = PositionOffset/pos_offset_len;
                glm::vec3 unapplied_pos_offset = (pos_offset_len - glm::max(glm::dot(pos_offset_norm, applied_vel), 0.0f)) * pos_offset_norm;
                Position += unapplied_pos_offset; 
            }
        } else {
            Position += PositionOffset;
        }
        PositionOffset = glm::vec3{0.0f};

        glm::vec3 avrg_rot_vel = AngularVelocity;
        AngularVelocity+= dt * Torque / Mass / InertiaMoment;
        avrg_rot_vel += AngularVelocity;
        avrg_rot_vel *= 0.5f;
        Rotation += dt * avrg_rot_vel;

        Rotation.x = glm::mod<float>(Rotation.x, glm::pi<float>()*2);
        Rotation.y = glm::mod<float>(Rotation.y, glm::pi<float>()*2);
        Rotation.z = glm::mod<float>(Rotation.z, glm::pi<float>()*2);

        if(ColliderType == COLLIDER_HULL && avrg_rot_vel != glm::vec3{0.0f}) {
            UpdateRotatedModel();
        }

        Force = glm::vec3{0.0f};
        Torque = glm::vec3{0.0f};
    }

    void RigidBody::Think(float dt) {
        DoRender = ((int)convar_phys_draw_colliders != 2 || ColliderType == COLLIDER_PLANE);
        if((int)convar_phys_draw_colliders != 0) {
            DrawRigidbodyCollider(*this);

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

    void RigidBody::LoadColliderModel(const char *path) {
        Hull.Indices.clear();
        Hull.Vertices.clear();
        Hull.RotatedVertices.clear();

        tinyobj::attrib_t attrib{};
        std::vector<tinyobj::shape_t> shapes{};
        std::vector<tinyobj::material_t> materials{};

        std::string warn{};
        std::string err{};

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path)) {
            ColliderType = COLLIDER_SPHERE;
            ERR_MSG("LoadColliderModel : Tiny Object Loader Error : %s", err.c_str());
        }

        std::unordered_map<glm::vec3, size_t> unique_vertices{}; //Vertex coordinate to vertex index.

        size_t indices_count = 0;
        for(tinyobj::shape_t &shape : shapes) {
            indices_count += shape.mesh.indices.size();
        }
        Hull.Indices.reserve(indices_count);

        size_t vertices_count = attrib.vertices.size() / 3;
        Hull.Vertices.reserve(vertices_count);

        size_t vertex_index = 0;
        for(tinyobj::shape_t &shape : shapes) {
            for(tinyobj::index_t &index : shape.mesh.indices) {
                glm::vec3 vertex = {attrib.vertices[index.vertex_index * 3 + 0],
                                    attrib.vertices[index.vertex_index * 3 + 1],
                                    attrib.vertices[index.vertex_index * 3 + 2]};

                if(unique_vertices.count(vertex) == 0) {
                    unique_vertices[vertex] = Hull.Vertices.size();
                    Hull.Vertices.emplace_back(vertex);
                }

                Hull.Indices.emplace_back(unique_vertices[vertex]);
            }
        }

        UpdateRotatedModel();

    }

    void RigidBody::UpdateRotatedModel() {
        ASSERT(ColliderType == COLLIDER_HULL);

        Hull.RotatedVertices.resize(Hull.Vertices.size());

        for(size_t i = 0; i < Hull.Vertices.size(); i++) {
            Hull.RotatedVertices[i] = ApplyRotation(Rotation, Hull.Vertices[i]);
        }
    }

    void DrawRigidbodyCollider(RigidBody &rb) {
        #if USE_DEBUG_DRAWING
        RenderingServer *r = Application::Singleton()->GetRenderer();

        const glm::vec3 up = ApplyRotation(rb.Rotation, glm::vec3{0.0f, 1.0f, 0.0f});
        const glm::vec3 right = ApplyRotation(rb.Rotation, glm::vec3{1.0f, 0.0f, 0.0f});
        const glm::vec3 forward = ApplyRotation(rb.Rotation, glm::vec3{0.0f, 0.0f, 1.0f});

        const glm::vec3 color = glm::vec3{0.8f, 0.0f, 0.0f};
        const glm::vec3 color_faded = glm::vec3{0.6f, 0.0f, 0.0f};

        if(rb.ColliderType == COLLIDER_SPHERE) {
            r->DrawPoint(rb.Position, color_faded);

            const int res = 12;
            const int vertical_res = 3;
            const float angle_step = 2 * glm::pi<float>() / res;
            float curr_angle = 0;
            glm::vec3 a;
            glm::vec3 b = (right * glm::cos(curr_angle) + forward * glm::sin(curr_angle)) * rb.Radius;

            for(int i = 0; i < res; i++) {
                a = b;
                curr_angle += angle_step;
                b = (right * glm::cos(curr_angle) + forward * glm::sin(curr_angle)) * rb.Radius;
                for(int i = 0; i < vertical_res; i++) {
                    r->DrawLine(rb.Position + a * glm::cos(glm::pi<float>() * 0.5f / vertical_res * i) + up * glm::sin(glm::pi<float>() * 0.5f / vertical_res * i) * rb.Radius,
                                rb.Position + b * glm::cos(glm::pi<float>() * 0.5f / vertical_res * i) + up * glm::sin(glm::pi<float>() * 0.5f / vertical_res * i) * rb.Radius,
                                 color);
                    r->DrawLine(rb.Position + a * glm::cos(glm::pi<float>() * 0.5f / vertical_res * i) - up * glm::sin(glm::pi<float>() * 0.5f / vertical_res * i) * rb.Radius,
                                rb.Position + b * glm::cos(glm::pi<float>() * 0.5f / vertical_res * i) - up * glm::sin(glm::pi<float>() * 0.5f / vertical_res * i) * rb.Radius,
                                 color);
                }
            }

            r->DrawPoint(rb.Position + up * rb.Radius, color);
            r->DrawPoint(rb.Position - up * rb.Radius, color);
        } else if(rb.ColliderType == COLLIDER_CAPSULE) {

            const int res = 8;
            const int vertical_res = 3;
            const float angle_step = 2 * glm::pi<float>() / res;
            float curr_angle = 0;
            glm::vec3 a;
            glm::vec3 b = (right * glm::cos(curr_angle) + forward * glm::sin(curr_angle)) * rb.Radius;

            for(int i = 0; i < res; i++) {
                a = b;
                curr_angle += angle_step;
                b = (right * glm::cos(curr_angle) + forward * glm::sin(curr_angle)) * rb.Radius;
                for(int i = 0; i < vertical_res; i++) {
                    r->DrawLine(rb.Position + a * glm::cos(glm::pi<float>() * 0.5f / vertical_res * i) + up * glm::sin(glm::pi<float>() * 0.5f / vertical_res * i) * rb.Radius + up * rb.Length * 0.5f,
                                rb.Position + b * glm::cos(glm::pi<float>() * 0.5f / vertical_res * i) + up * glm::sin(glm::pi<float>() * 0.5f / vertical_res * i) * rb.Radius + up * rb.Length * 0.5f,
                                color);
                    r->DrawLine(rb.Position + a * glm::cos(glm::pi<float>() * 0.5f / vertical_res * i) - up * glm::sin(glm::pi<float>() * 0.5f / vertical_res * i) * rb.Radius - up * rb.Length * 0.5f,
                                rb.Position + b * glm::cos(glm::pi<float>() * 0.5f / vertical_res * i) - up * glm::sin(glm::pi<float>() * 0.5f / vertical_res * i) * rb.Radius - up * rb.Length * 0.5f,
                                color);
                }
                r->DrawLine(rb.Position + b - up * rb.Length * 0.5f, rb.Position + b + up * rb.Length * 0.5f, color);
            }

            r->DrawPoint(rb.Position + up * (rb.Length * 0.5f + rb.Radius), color);
            r->DrawPoint(rb.Position - up * (rb.Length * 0.5f + rb.Radius), color);
        } else if(rb.ColliderType == COLLIDER_HULL) {
            
            for(size_t i = 0; i < rb.Hull.Indices.size(); i += 3) {
                const glm::vec3 a = rb.Position + ApplyRotation(rb.Rotation, rb.Hull.Vertices[rb.Hull.Indices[i]]);
                const glm::vec3 b = rb.Position + ApplyRotation(rb.Rotation, rb.Hull.Vertices[rb.Hull.Indices[i+1]]);
                const glm::vec3 c = rb.Position + ApplyRotation(rb.Rotation, rb.Hull.Vertices[rb.Hull.Indices[i+2]]);

                r->DrawLine(a, b, color);
                r->DrawLine(b, c, color);
                r->DrawLine(c, a, color);
            }
        }
        #endif
    }
}