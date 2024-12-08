#include "physic_server.h"

#include "../debug/console/convar.h"

#include "../error_macros.h"
#include "../application.h"

#include <thread>
#include <chrono>
#include <time.h>

#include "entities/entity_server.h"

#include "../debug/profiler/profiler.h"

namespace gigno {

    CONVAR(uint32_t, phys_loop_rate, 120, "How many times per second is the physics called.");

    PhysicServer::PhysicServer() 
        : m_LoopThread{&PhysicServer::Loop, this} {
    }

    PhysicServer::~PhysicServer() {
        m_LoopContinue = false;
        m_LoopThread.join();
    }

    void PhysicServer::Loop() {

        std::chrono::time_point<std::chrono::high_resolution_clock> frame_start{};
        std::chrono::time_point<std::chrono::high_resolution_clock> frame_end{};
        std::chrono::nanoseconds to_wait{0};
        std::chrono::nanoseconds frame_time((int64_t)(1e9/convar_phys_loop_rate));
        EntityServer* entity_serv = Application::Singleton()->GetEntityServer();
    
        while(m_LoopContinue) {
            Profiler::Begin("Physics Loop");

            frame_start = std::chrono::high_resolution_clock::now();

            entity_serv->PhysicTick(frame_time.count() / 1e9);

            DetectCollisions();

            Profiler::End();

            frame_end = std::chrono::high_resolution_clock::now();
            std::chrono::nanoseconds dur = frame_end - frame_start;

            frame_time = std::chrono::nanoseconds((int64_t)(1e9 / convar_phys_loop_rate));

            if(dur > frame_time) {
                Console::LogError ("Physic Server : Late on process. Could not keep the cadence of %u frames per seconds. "
                                                                        "This frame took %d nanoseconds too long.", dur.count());
                frame_time += dur;
            } else {
                while((std::chrono::high_resolution_clock::now() - frame_start) < frame_time) {
                    // SPIN
                }
            }
        }
    }

    void PhysicServer::DetectCollisions()
    {
        std::lock_guard<std::mutex>{m_WorldMutex};

        for(Collider &col : m_World) {
            col.PollRigidBodyValues();
        }

        for(int i = 0; i < m_World.size(); i++) {
            for(int j = i+1; j < m_World.size(); j++) {
                if(i != j) {
                    ResolveCollision(m_World[i], m_World[j]);
                }
            }
        }

        for (Collider &col : m_World) {
            col.ApplyImpulse();
        }
    }

    void PhysicServer::AddCollider(RigidBody *body, ColliderType_t type, Collider::ColliderParameter parameters) {
        std::lock_guard<std::mutex>{m_WorldMutex};

        Collider &coll = m_World.emplace_back();
        coll.type = type;
        coll.parameters = parameters;
        coll.BoundRigidBody = body;
    }

    void PhysicServer::RemoveCollider(RigidBody *body) {
        std::lock_guard<std::mutex>{m_WorldMutex};

        for(int i = 0; i < m_World.size(); i++) {
            if(m_World[i].BoundRigidBody == body) {
                m_World.erase(m_World.begin() + i);
                return;
            }
        }
        ERR_MSG("Tried to remove collider of a body that never had a collider !");
    }
}
