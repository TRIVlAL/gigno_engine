#ifndef COLLISION_SOUND_H
#define COLLISION_SOUND_H

#include <vector>
#include <string>
#include <set>
#include <functional>
#include "../vendor/glm/glm/glm.hpp"

namespace gigno {

    struct Sound_t;
    class AudioServer;

    class CollisionSoundManager {
    public:
        CollisionSoundManager() = default;

        void Init();

        enum CollisionSoundType_t {
            COLLISION_SOUND_TYPE_HIT = 0,
            COLLISION_SOUND_TYPE_FRICTION,
            COLLISION_SOUND_TYPE_MAX_ENUM
        };
        
        enum CollisionSoundPower_t {
            COLLISION_SOUND_POWER_1 = 0,
            COLLISION_SOUND_POWER_2,
            COLLISION_SOUND_POWER_3,
            COLLISION_SOUND_POWER_MAX_ENUM,
            COLLISION_SOUND_POWER_NONE
        };

        struct CollisionSoundQuery_t {
            CollisionSoundType_t Type;
            
            glm::vec3 ApplyPoint;
            glm::vec3 Velocity{}; //only set for friction
            float EnergyLoss;
        };
        
        void NewSoundQuery(CollisionSoundQuery_t query);
        void Update();
        
    private:
        glm::vec3 m_ListenerPosition;

        void ExecuteSound(Sound_t *sound, CollisionSoundQuery_t query);

        float Importance            (CollisionSoundQuery_t query);
        CollisionSoundPower_t Power (CollisionSoundQuery_t query);
        size_t SoundArrayIndex      (CollisionSoundQuery_t query);
        float Volume                (CollisionSoundQuery_t query);
        const float VOLUME_MULTIPLIER = 25.0f;

        std::vector<CollisionSoundQuery_t> m_CurrentQueries;

        struct CollisionSoundArray_t {
            CollisionSoundArray_t(size_t maxSoundCount, const char *name, CollisionSoundPower_t power);
            void Init();

            bool HasFreeSpaceLeft;
            // Returns a sound that is free to be used. Nullptr if they are all used.
            Sound_t *FreeSpace();

            const char *Name;
            CollisionSoundPower_t Power;
            const size_t MAX_SOUND_COUNT;
            std::vector<Sound_t *> Sounds{MAX_SOUND_COUNT};
        };

        std::function<bool(CollisionSoundQuery_t, CollisionSoundQuery_t)> m_FuncCompareQuery = 
                                                                        [this](CollisionSoundQuery_t a, CollisionSoundQuery_t b) 
                                                                        { return this->Importance(a) < this->Importance(b); };

        std::set<CollisionSoundQuery_t, decltype(m_FuncCompareQuery)> m_PendingQueries{m_FuncCompareQuery};

        typedef float PowerThreshold_t[COLLISION_SOUND_POWER_MAX_ENUM];

        PowerThreshold_t m_Thresholds[COLLISION_SOUND_TYPE_MAX_ENUM] {
            {10.0f, 1'500.0f, 30'000.0f},
            {10.0f, 1'500.0f, 25'000.0f}
        };

        CollisionSoundArray_t m_Sounds[COLLISION_SOUND_TYPE_MAX_ENUM * COLLISION_SOUND_POWER_MAX_ENUM] = {
            CollisionSoundArray_t{ 10, "hit", COLLISION_SOUND_POWER_1},
            CollisionSoundArray_t{ 10, "hit", COLLISION_SOUND_POWER_2},
            CollisionSoundArray_t{ 1, "hit", COLLISION_SOUND_POWER_3},
            CollisionSoundArray_t{ 5, "friction", COLLISION_SOUND_POWER_1},
            CollisionSoundArray_t{ 5, "friction", COLLISION_SOUND_POWER_2},
            CollisionSoundArray_t{ 1, "friction", COLLISION_SOUND_POWER_3},
        };

    };

}

#endif