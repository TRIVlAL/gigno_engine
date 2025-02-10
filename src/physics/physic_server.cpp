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

using namespace std::chrono_literals;

namespace gigno {

    extern std::mutex s_EntityUnloadMutex;

    CONVAR(uint32_t, phys_loop_rate, 120, "How many times per second is the physics called.");
    CONVAR(float, phys_timescale, 1.0f, "physics is slowed down by that amount.");

    PhysicServer::PhysicServer() 
        : m_LoopThread{&PhysicServer::Loop, this} {
    }

    PhysicServer::~PhysicServer() {
        m_LoopContinue = false;
        m_Pause = false;
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
                curr->pNextRigidBody = rb->pNextRigidBody;
                return;
            }
            curr = curr->pNextRigidBody;
        }
    }

    void PhysicServer::Loop() {

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
            entity_serv->PhysicTick((target_dur.count() + time_overflow.count()) / 1e9);

            ResolveCollisions();
            s_EntityUnloadMutex.unlock();

            Profiler::End();

            frame_end = std::chrono::high_resolution_clock::now();
            std::chrono::nanoseconds dur = frame_end - frame_start;

            target_dur = std::chrono::nanoseconds((int64_t)(1e9 / convar_phys_loop_rate));

            if(dur > target_dur) {
                time_overflow = dur - target_dur;
                Console::LogError ("Physic Server : Late on process. Could not keep the cadence of %d frames per seconds. "
                                                                        "This frame took %f ms too long. (timescale : %f)", (uint32_t)convar_phys_loop_rate, 
                                                                        (float)time_overflow.count()/1e6f, (float)convar_phys_timescale);
            } else {
                time_overflow = 0ns;

                while ((std::chrono::high_resolution_clock::now() - frame_start) < target_dur / ((float)convar_phys_timescale != 0.0f ? (float)convar_phys_timescale : 0.001f))
                {
                    /* --- SPIN ---*/
                }
            }
        }
    }

    void PhysicServer::ResolveCollisions()
    {
        std::lock_guard<std::mutex>{m_WorldMutex};

        /* -----------------------------------------------
        BROAD PHASE
        -------------------------------------------------*/

        Profiler::Begin("Collision - Broad Phase");
        
        //Checking Axis Aligned Bounding Boxes and adding each overlapping pairs
        // to m_PossiblePairs 
        RigidBody *rb1 = RigidBodies;
        while(rb1) {
            RigidBody *rb2 = rb1->pNextRigidBody;
            while(rb2) {
                if (rb1->BBMin.x <= rb2->BBMax.x && rb1->BBMax.x >= rb2->BBMin.x &&
                    rb1->BBMin.y <= rb2->BBMax.y && rb1->BBMax.y >= rb2->BBMin.y &&
                    rb1->BBMin.z <= rb2->BBMax.z && rb1->BBMax.z >= rb2->BBMin.z)
                {
                        
                    m_PossiblePairs.emplace_back(rb1, rb2);
                    rb1->IsBBCollide = true;
                    rb2->IsBBCollide = true;
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
            ResolveCollision(*pair.first, *pair.second);
        }

        Profiler::End();
        
        m_PossiblePairs.clear();

    }

}
