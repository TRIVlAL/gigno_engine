#include "physic_server.h"

#include "../debug/console/convar.h"

#include "../error_macros.h"
#include "../application.h"
#include "rigid_body.h"

#include <thread>
#include <chrono>
#include <time.h>
#include <future>

#include "entities/entity_server.h"

#include "../debug/profiler/profiler.h"

#include "../vendor/tiny_object_loader/tiny_obj_loader.h"


using namespace std::chrono_literals;

namespace gigno {

    extern std::mutex s_EntityUnloadMutex;

    CONVAR(uint32_t, phys_loop_rate, 120, "How many times per second is the physics called.");
    CONVAR(float, phys_timescale, 1.0f, "physics is slowed down by that amount.");
    CONVAR(int, phys_constraints_iteration_count, 3, "must be positive, higher means better results but more cost.");

    void PhysicServer::Init() {
        m_CollisionSoundManager.Init();
        m_LoopThread = std::thread{&PhysicServer::Loop, this};
    }

    PhysicServer::~PhysicServer() {
        m_LoopContinue = false;
        m_Pause = false;

        std::future<void>* terminate = new std::future<void>{std::async(std::launch::async, &std::thread::join, &m_LoopThread)};
        if(terminate->wait_for(std::chrono::seconds(5)) == std::future_status::timeout) {
            // Server is probably stuck in an infinite loop.
            // We let it leak but who cares at this point
            Console::LogError("Physics Thread Stuck ! Aborting uninitialization.");
            return;
        } else {
            delete terminate;
        }
    }

    void PhysicServer::SubscribeRigidBody(RigidBody *rb) {
        rb->pNextRigidBody = s_RigidBodies;
        s_RigidBodies = rb;
    }

    void PhysicServer::UnsubscribeRigidBody(RigidBody *rb) {
        RigidBody *curr = s_RigidBodies;
        if(curr == rb) {
            s_RigidBodies = rb->pNextRigidBody;
        }
        while(curr) {
            if(curr->pNextRigidBody == rb) {
                curr->pNextRigidBody = rb->pNextRigidBody;
                return;
            }
            curr = curr->pNextRigidBody;
        }
    }

    void PhysicServer::SubscribeConstraint(PhysicsConstraint *cons) {
        cons->pNextConstraint = s_Constraints;
        s_Constraints = cons;
    }

    void PhysicServer::UnsubscribeConstraint(PhysicsConstraint *cons) {
        PhysicsConstraint *curr = s_Constraints;
        if(curr == cons) {
            s_Constraints = cons->pNextConstraint;
        }
        while(curr) {
            if(curr->pNextConstraint == cons) {
                curr->pNextConstraint = cons->pNextConstraint;
                return;
            }
            curr = curr->pNextConstraint;
        }
    }

    void PhysicServer::Loop() {
        while(!Application::Singleton() && m_LoopContinue) {
            ;
        }

        std::chrono::time_point<std::chrono::high_resolution_clock> frame_start{};
        std::chrono::time_point<std::chrono::high_resolution_clock> frame_end{};
        std::chrono::nanoseconds to_wait{0};
        std::chrono::nanoseconds target_dur((int64_t)(1e9/convar_phys_loop_rate));
        EntityServer* entity_serv = Application::Singleton()->GetEntityServer();
        std::chrono::nanoseconds time_overflow{0};
    
        while(m_LoopContinue) {
            while(m_Pause) {
                if(m_Step) {
                    m_Step = false;
                    break;
                }
                /* --- SPIN ---*/
            }

            Profiler::Begin("Physics Loop");

            frame_start = std::chrono::high_resolution_clock::now();

            s_EntityUnloadMutex.lock();

            float delta_time = (target_dur.count() + time_overflow.count()) / 1e9;

            entity_serv->PhysicTick(delta_time);

            SolveConstraints(delta_time);

            ResolveCollisions();

            m_CollisionSoundManager.Update();

            s_EntityUnloadMutex.unlock();

            Profiler::End();

            frame_end = std::chrono::high_resolution_clock::now();
            std::chrono::nanoseconds dur = frame_end - frame_start;

            target_dur = std::chrono::nanoseconds((int64_t)(1e9 / convar_phys_loop_rate));

            if(dur > target_dur) {
                time_overflow = dur - target_dur;
                /*Console::LogError ("Physic Server : Late on process. Could not keep the cadence of %d frames per seconds. "
                                                                        "This frame took %f ms too long", (uint32_t)convar_phys_loop_rate, 
                                                                        (float)time_overflow.count()/1e6f);*/
            } else {
                time_overflow = 0ns;

                while ((std::chrono::high_resolution_clock::now() - frame_start) < target_dur / ((float)convar_phys_timescale != 0.0f ? (float)convar_phys_timescale : 0.001f))
                {
                    /* --- SPIN ---*/
                }
            }
        }
    }

    void PhysicServer::ResolveCollisions() {
        std::lock_guard<std::mutex>{m_WorldMutex};

        /* -----------------------------------------------
        BROAD PHASE
        -------------------------------------------------*/

        Profiler::Begin("Collision - Broad Phase");
        
        //Checking Axis Aligned Bounding Boxes and adding each overlapping pairs
        // to m_PossiblePairs
        RigidBody *rb1 = s_RigidBodies;
        while(rb1) {
            RigidBody *rb2 = rb1->pNextRigidBody;
            while(rb2) {
                if (!(rb1->IsStatic && rb2->IsStatic)
                    && AABBCollision(rb1->AsCollider().AABB, rb2->AsCollider().AABB)) 
                {
                    m_PossiblePairs.emplace_back(rb1, rb2);
                }
                rb2 = rb2->pNextRigidBody;
            }
            rb1 = rb1->pNextRigidBody;
        }
        
        Profiler::End();
            
        /* -----------------------------------------------
        NARROW PHASE
        -------------------------------------------------*/

        Profiler::Begin("Collision - Narrow Phase");

        for(std::pair<RigidBody *, RigidBody *> pair : m_PossiblePairs) {
            CollisionData_t collision = DetectCollision(pair.first->AsCollider(), pair.second->AsCollider());
            if(collision.Collision) {
                RespondCollision(*pair.first, *pair.second, collision, &m_CollisionSoundManager);
            }
        }

        Profiler::End();
        
        m_PossiblePairs.clear();

    }

    void PhysicServer::SolveConstraints(float dt) {

        size_t iter_count = (size_t)(glm::min<int>((int)convar_phys_constraints_iteration_count, 1));

        for(size_t i = 0; i < iter_count; i++) {

            PhysicsConstraint* curr = s_Constraints;
            while(curr) {
                curr->Solve(dt / (float)iter_count);

                curr = curr->pNextConstraint;
            }
        }
    }

    bool PhysicServer::AllocateCollisionModel(const char *path) {
        if(m_Models.find(path) != m_Models.end()) {
            return true; //Model already allocated.
        }

        tinyobj::attrib_t attrib{};
        std::vector<tinyobj::shape_t> shapes{};
        std::vector<tinyobj::material_t> materials{};

        std::string warn{};
        std::string err{};

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path)) {
            Console::LogError("AllocateCollisionModel : Tiny Object Loader Error : %s", err.c_str());
            return false;
        }

        std::unordered_map<glm::vec3, size_t> unique_vertices{}; // Vertex coordinate to vertex index.
        
        size_t indices_count = 0;
        for (tinyobj::shape_t &shape : shapes) {
            indices_count += shape.mesh.indices.size();
        }

        size_t vertices_count = attrib.vertices.size() / 3;

        m_Models[path] = CollisionModel_t{};
        CollisionModel_t &model = m_Models[path];
        model.Indices.reserve(indices_count);
        model.Vertices.reserve(vertices_count);

        size_t vertex_index = 0;
        for (tinyobj::shape_t &shape : shapes)
        {
            for (tinyobj::index_t &index : shape.mesh.indices)
            {
                glm::vec3 vertex = {attrib.vertices[index.vertex_index * 3 + 0],
                                    attrib.vertices[index.vertex_index * 3 + 1],
                                    attrib.vertices[index.vertex_index * 3 + 2]};

                if (unique_vertices.count(vertex) == 0)
                {
                    unique_vertices[vertex] = model.Vertices.size();
                    model.Vertices.emplace_back(vertex);
                }

                model.Indices.emplace_back(unique_vertices[vertex]);
            }
        }

        return true;
    }

    const CollisionModel_t *PhysicServer::GetCollisionModel(const char *path) {
        if(m_Models.find(path) == m_Models.end()) {
            if(!AllocateCollisionModel(path)) {
                return nullptr;
            }
        }

        return &m_Models[path];
    }

    CollisionData_t PhysicServer::GetColliding(const Collider_t &collider, RigidBody **current) {
        if(!(*current)) {
            *current = s_RigidBodies;
        } else {
            *current = (*current)->pNextRigidBody;
        }

        while(*current) {
            Collider_t curr_col = (*current)->AsCollider();
            if(AABBCollision(curr_col.AABB, collider.AABB)) {
                CollisionData_t data = DetectCollision(collider, (*current)->AsCollider());
                if(data.Collision) {
                    return data;
                }
            }
            *current = (*current)->pNextRigidBody;
        }

        *current = nullptr;
        return CollisionData_t{};
    }

    bool PhysicServer::Raycast(Ray_t ray, RaycastCollisionType_t interaction, std::vector<RaycastHit_t> *outHits) {

        if(interaction == RAYCAST_COLLIDE_AABB || interaction == RAYCAST_COLLIDE_COLLIDER) {

            RigidBody *curr = s_RigidBodies;
            while(curr) {
                Collider_t coll = curr->AsCollider();
                if (interaction == RAYCAST_COLLIDE_AABB &&
                    coll.ColliderType != COLLIDER_PLANE)
                {
                    RaycastHit_t hit{};
                    if(Raycast_AABB(ray, coll.AABB, &hit)) {
                        hit.EntityHit = (Entity*)curr;
                        hit.EntityHitType = RAYCAST_HIT_ENTITY_RIGIDBODY;
                    }
                    outHits->emplace_back(hit);
                }
                else if (interaction == RAYCAST_COLLIDE_COLLIDER) {
                    RaycastHit_t hit{};
                    if (Raycast_Collider(ray, coll, &hit))
                    {
                        hit.EntityHit = (Entity *)curr;
                        hit.EntityHitType = RAYCAST_HIT_ENTITY_RIGIDBODY;
                    }
                }

                curr = curr->pNextRigidBody;
            }
        }

        return outHits->size() > 0;
    }

    bool PhysicServer::RaycastSingle(Ray_t ray, RaycastCollisionType_t interaction, RaycastHit_t *outHit){
        float distance = FLT_MAX;

        if(interaction == RAYCAST_COLLIDE_AABB || interaction == RAYCAST_COLLIDE_COLLIDER) {

            RigidBody *curr = s_RigidBodies;
            while(curr) {
                Collider_t coll = curr->AsCollider();
                if (interaction == RAYCAST_COLLIDE_AABB &&
                    coll.ColliderType != COLLIDER_PLANE)
                {
                    RaycastHit_t hit{};
                    if(Raycast_AABB(ray, coll.AABB, &hit)) {
                        hit.EntityHit = (Entity*)curr;
                        hit.EntityHitType = RAYCAST_HIT_ENTITY_RIGIDBODY;
                        if(hit.Distance < distance) {
                            distance = hit.Distance;
                            *outHit = hit;
                        }
                    }
                }
                else if (interaction == RAYCAST_COLLIDE_COLLIDER) {
                    RaycastHit_t hit{};
                    if (Raycast_Collider(ray, coll, &hit))
                    {
                        hit.EntityHit = (Entity *)curr;
                        hit.EntityHitType = RAYCAST_HIT_ENTITY_RIGIDBODY;
                        if (hit.Distance < distance) {
                            distance = hit.Distance;
                            *outHit = hit;
                        }
                    }
                }

                curr = curr->pNextRigidBody;
            }
        }

        return distance != FLT_MAX;
    }

    bool PhysicServer::RaycastHas(Ray_t ray, RaycastCollisionType_t interaction) {
        if(interaction == RAYCAST_COLLIDE_AABB || interaction == RAYCAST_COLLIDE_COLLIDER) {

            RigidBody *curr = s_RigidBodies;
            while(curr) {
                Collider_t coll = curr->AsCollider();
                if(interaction == RAYCAST_COLLIDE_AABB &&
                    coll.ColliderType != COLLIDER_PLANE)
                {
                    RaycastHit_t hit{};
                    if(Raycast_AABB(ray, coll.AABB, &hit)) {
                        return true;
                    }
                }
                else if (interaction == RAYCAST_COLLIDE_COLLIDER) {
                    RaycastHit_t hit{};
                    if (Raycast_Collider(ray, coll, &hit)) {
                        return true;
                    }
                }

                curr = curr->pNextRigidBody;
            }
        }

        return false;
    }
}
