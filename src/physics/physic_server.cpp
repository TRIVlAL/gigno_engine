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
        m_BoundingBoxCorners.emplace_back(BoundingBoxCorner_t{rb, true});
        m_BoundingBoxCorners.emplace_back(BoundingBoxCorner_t{rb, false});
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
        std::vector<std::pair<RigidBody *, RigidBody *>> possible_pairs{};

        {
            auto point = [](BoundingBoxCorner_t &c) { return c.opens ? c.RB->BBMin : c.RB->BBMax; };

            // Sort the Bounding Box corners in ascending order of the x coordinate
            // (Insertion Sort)
            for(int i = 0; i < m_BoundingBoxCorners.size(); i++) {
                BoundingBoxCorner_t compare =m_BoundingBoxCorners[i];
                int j = i;
                
                while(j > 0 && point(m_BoundingBoxCorners[j - 1]).x > point(compare).x) {
                    m_BoundingBoxCorners[j] = m_BoundingBoxCorners[j-1];
                    j--;
                }
                m_BoundingBoxCorners[j] = compare;
            }

            std::vector<std::pair<RigidBody *, RigidBody *>> possible_x_pairs{};
            std::vector<RigidBody *> currently_open{};
            for(BoundingBoxCorner_t corner : m_BoundingBoxCorners) {
                if(corner.opens) {
                    currently_open.emplace_back(corner.RB);
                } else {
                    RigidBody *closed = corner.RB;
                    int closed_index = -1;

                    for(int i = 0; i < currently_open.size(); i++) {
                        if(currently_open[i] == closed) {
                            closed_index = i;
                        } else {
                            possible_x_pairs.emplace_back(currently_open[i], closed);
                        }
                    }

                    ASSERT(closed_index != -1);
                    currently_open.erase(currently_open.begin() + closed_index);
                }
            }
            // At this point, possible_x_pairs contains every pairs overlapping on the x axis.

            //Add every pairs to possible_pair if they overlap on y and z.
            possible_pairs.reserve(possible_x_pairs.size());
            for(std::pair<RigidBody *, RigidBody*> &pair : possible_x_pairs) {
                if(((pair.first->BBMin.y <= pair.second->BBMin.y && pair.first->BBMax.y >= pair.second->BBMin.y) || 
                    (pair.first->BBMin.y <= pair.second->BBMax.y && pair.first->BBMax.y >= pair.second->BBMax.y)) &&
                    (pair.first->BBMin.z <= pair.second->BBMin.z && pair.first->BBMax.z >= pair.second->BBMin.z) || 
                    (pair.first->BBMin.z <= pair.second->BBMax.z && pair.first->BBMax.z >= pair.second->BBMax.z)) {
                    
                    possible_pairs.emplace_back(pair);
                    pair.first->IsBBCollide = true;
                    pair.second->IsBBCollide = true;

                }
            }
        }


        /* -----------------------------------------------
        NARROW PHASE
        -------------------------------------------------*/

        for(std::pair<RigidBody *, RigidBody *> pair : possible_pairs) {
            ResolveCollision(*pair.first, *pair.second);
        }
    }

}
