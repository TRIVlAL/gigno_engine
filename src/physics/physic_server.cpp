#include "physic_server.h"

#include "../debug/console/convar.h"

#include "../error_macros.h"
#include "../application.h"
#include "rigid_body.h"

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

    void PhysicServer::SubscribeRigidBody(RigidBody *rb) {
        rb->pNextRigidBody = RigidBodies;
        RigidBodies = rb;
    }

    void PhysicServer::UnsubscribeRigidBody(RigidBody *rb) {
        RigidBody *curr = RigidBodies;
        if(curr == rb) {
            RigidBodies = rb->pNextRigidBody;
        }
        while(curr) {
            if(curr->pNextRigidBody == rb) {
                curr->pNextEntity = rb->pNextRigidBody;
                return;
            }
            curr = curr->pNextRigidBody;
        }
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

            RigidBody *curr = RigidBodies;
            while(curr) {
                ApplyDrag(*curr);
                curr = curr->pNextRigidBody;
            }

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

        RigidBody *rb1 = RigidBodies;
        while(rb1) {
            RigidBody *rb2 = rb1->pNextRigidBody;
            while(rb2) {
                if(rb1 != rb2) {
                    ResolveCollision(*rb1, *rb2);
                }
                rb2 = rb2->pNextRigidBody;
            }
            rb1 = rb1->pNextRigidBody;
        }
    }

}
