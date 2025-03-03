#ifndef PHYSIC_SERVER_H
#define PHYSIC_SERVER_H

#include <thread>
#include <mutex>
#include <vector>
#include "../algorithm/cstr_map.h"
#include "collision.h"

namespace gigno {
    const size_t MAX_RIGIDBODY_COUNT = 50;

    static void phys_remote_update(float); //defined in phys_remote_command.cpp

    struct CollisionModel_t {
        std::vector<glm::vec3> Vertices{};
        std::vector<int> Indices{};
    };

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

        /*
        Allocates the collision Model using the .obj file at path.
        Returns false if the allocation wasn't successful. Can be because the path does not exist
        or if the .obj file cannot be parsed. 
        If this CollisionModel was already allocated, will return true and leave.
        */
        bool AllocateCollisionModel(const char *path);
        /*
        Returns the collision Model using the .obj file at path. 
        Allocates it if it wasn't already. 
        Returns nullptr if the allocation was not successfull. 

        Pointer can be invalidated by any call to AllocateCollisionModel, GetCollisionModel and, thus, 
        any construction of a RigidBody. For that reason, use it once in YOUR SCOPE and LEAVE IT.
        */
        const CollisionModel_t *GetCollisionModel(const char *path);

        /*
        Returns the collisiond data between collider and current->pNext in the rigidbody chain.
        sets *current to current->pNext
        if *current == nullptr, sets and tests the first RigidBody in the RigidBody chain.
        if *current output is nullptr, you have reached the end of the chain.
        */
        CollisionData_t GetColliding(const Collider_t &collider, RigidBody **current);

    private :
        /*
        Map matching the path to the .obj model file to the CollisionModel_t instance.
        */
        CstrUnorderedMap_t<CollisionModel_t> m_Models {};

        std::mutex m_WorldMutex{};

        std::thread m_LoopThread;
        volatile bool m_LoopContinue = true;

        RigidBody *s_RigidBodies{};

        // modified by friend command phys_remote
        bool m_Pause = false;
        bool m_Step = false;
        
        // Container of the pairs of RigidBodies having their AABB overlapping.
        // Kept as a member variable to not dealocate it's memory and keep it for the next iteration.
        std::vector<std::pair<RigidBody*, RigidBody*>> m_PossiblePairs;

        void Loop();

        void ResolveCollisions();
    };

}

#endif