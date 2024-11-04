#include "physic_server.h"

#include "../debug/console/convar.h"

#include "../error_macros.h"
#include "../application.h"

#include "physic_entity.h"

#include <thread>
#include <chrono>

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

        while(m_LoopContinue) {
            frame_start = std::chrono::high_resolution_clock::now();
            
            PhysicEntity *curr = PhysicEntity::pFirstPhysicEntity;
            while(curr) {
                curr->PhysicThink(frame_time.count()/1e9);
                curr = curr->pNextPhysicEntity;
            }
            //TODO : Move PhysicThink to any entity instead of only PhysicEnity -> we can then completely remove PhysicENtity ???

            frame_end = std::chrono::high_resolution_clock::now();
            std::chrono::nanoseconds dur = frame_end - frame_start;

            frame_time = std::chrono::nanoseconds((int64_t)(1e9 / convar_phys_loop_rate));
            to_wait = frame_time - dur;

            if(to_wait < std::chrono::nanoseconds(0)) {
                Application::Singleton()->Debug()->GetConsole()->LogError("Physic Server : Late on process. Could not keep the cadence of %u frames per seconds. "
                                                                        "This frame took %d nanoseconds too long.", (uint32_t)convar_phys_loop_rate, -1 * to_wait.count());
                frame_time -= to_wait;
            }

            std::this_thread::sleep_for(to_wait);
        }
    }

}
