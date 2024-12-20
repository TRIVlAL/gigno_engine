#ifndef PHYSIC_SERVER_H
#define PHYSIC_SERVER_H

#include <thread>
#include <mutex>
#include <vector>
#include "collision.h"

namespace gigno {
    const size_t MAX_RIGIDBODY_COUNT = 50;

    class PhysicServer {
    public:

        /*
        On creation, the PhysicServer wil start his physic loop (Start()) with a fixed rate per second of convar_phys_loop_rate
        This loop is ran on a separate thread. 
        In this loop, the PhysicServer calls PhysicThink() to all the PhysicEntity in game (in the linked list of physics entities).
        */
        PhysicServer();
        ~PhysicServer();

        void SubscribeRigidBody(RigidBody *rb);
        void UnsubscribeRigidBody(RigidBody *rb);
        
    private:
        std::mutex m_WorldMutex{};

        std::thread m_LoopThread;
        volatile bool m_LoopContinue = true;

        RigidBody *RigidBodies{};

        void Loop();

        void ApplyGlobalForces();
        void DetectCollisions();
    };

}

#endif