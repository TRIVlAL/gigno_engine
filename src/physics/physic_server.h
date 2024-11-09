#ifndef PHYSIC_SERVER_H
#define PHYSIC_SERVER_H

#include <thread>

namespace gigno {

    class PhysicServer {
    public:
        /*
        On creation, the PhysicServer wil start his physic loop (Start()) with a fixed rate per second of convar_phys_loop_rate.BaseConvar
        This loop is run on a separate thread. 
        In this loop, the PhysicServer calls PhysicThink() to all the PhysicEntity in game (in the linked list of physics entities).
        */
        PhysicServer();
        ~PhysicServer();

        void Stop() {
            m_LoopContinue = false;
            m_LoopThread.join();
        }


    private:
        std::thread m_LoopThread;
        volatile bool m_LoopContinue = true;

        void Loop();

    };

}

#endif