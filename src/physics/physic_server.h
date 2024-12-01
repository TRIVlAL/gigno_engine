#ifndef PHYSIC_SERVER_H
#define PHYSIC_SERVER_H

#include <thread>
#include <mutex>
#include <vector>
#include "collider.h"

namespace gigno {

    class PhysicServer {
    friend class RigidBody;
    public:
        /*
        On creation, the PhysicServer wil start his physic loop (Start()) with a fixed rate per second of convar_phys_loop_rate
        This loop is ran on a separate thread. 
        In this loop, the PhysicServer calls PhysicThink() to all the PhysicEntity in game (in the linked list of physics entities).
        */
        PhysicServer();
        ~PhysicServer();

        
    private:
        std::mutex m_WorldMutex{};

        std::thread m_LoopThread;
        volatile bool m_LoopContinue = true;

        void Loop();

        void DetectCollisions();

        void AddCollider(RigidBody *body, ColliderType_t type, Collider::ColliderParameter parameters);
        void RemoveCollider(RigidBody *body);

        std::vector<Collider> m_World{};
    };

}

#endif