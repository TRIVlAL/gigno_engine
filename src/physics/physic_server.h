#ifndef PHYSIC_SERVER_H
#define PHYSIC_SERVER_H

#include <thread>
#include <mutex>
#include <vector>
#include "collision.h"

namespace gigno {
    const size_t MAX_RIGIDBODY_COUNT = 50;

    static void phys_remote_update(float); //defined in phys_remote_command.cpp

    class PhysicServer {
        friend void phys_remote_update(float);
    public :

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

        // modified by friend command phys_remote
        bool m_Pause = false;
        bool m_Step = false;

        struct BoundingBoxCorner_t {
            RigidBody *RB; // This a corner of the RB of this RigidBody.
            bool opens; //This represents the smallest corner of the BB
        };

        /*
            Every RifidBody has two coresponding Corners in this vector.
            One is Open and the other is Closed.
            This vector is sorted in ascending order of the x coodinate of the position of the BB
            If it is open, we use the BBMin, else we use the BBMax. 
            RigidBodies add their corner to the vector when the call SubscribeRigidBody() (and remove when Unsubscribe)
        */
        std::vector<BoundingBoxCorner_t> m_BoundingBoxCorners{};

        void Loop();

        void ResolveCollisions();
    };

}

#endif