#include "collision_sound.h"

#include "../application.h"
#include <cstring>
#include "../error_macros.h"
#include "../algorithm/geometry.h"
#include <random>

namespace gigno {

    CollisionSoundManager::CollisionSoundArray_t::CollisionSoundArray_t(AudioServer *audioServer, size_t maxSoundCount, const char *name, CollisionSoundPower_t power) :
        MAX_SOUND_COUNT{maxSoundCount} {

        char path[12 + strlen(name) + 1 + 2 + 4 + 1];
        memcpy(path, "sounds/phys_", 12);
        memcpy(path + 12, name, strlen(name));
        memcpy(path + 12 + strlen(name), "_", 1);
        snprintf(path + 12 + strlen(name) + 1, 2 + 5, "%02u.wav", (size_t)power + 1); //start at 01.wav -> 02.wav etc...
        
        for(size_t i = 0; i < Sounds.size(); i++) {
            Sounds[i] = audioServer->NewSound(path);
        }

    }

    Sound_t *CollisionSoundManager::CollisionSoundArray_t::FreeSpace() {
        for(Sound_t *sound : Sounds) {
            if(!sound->IsPlaying() || sound->GetAdvencment() > 0.75f) {
                return sound;
            }
        }
        return nullptr;
    }

    CollisionSoundManager::CollisionSoundManager(AudioServer *audioServer) : m_AudioServer{audioServer} {
    }

    void CollisionSoundManager::NewSoundQuery(CollisionSoundQuery_t query)
    {
        if ( Power(query) != COLLISION_SOUND_POWER_NONE &&
            Volume(query) / LenSquared(query.ApplyPoint - m_ListenerPosition) > 0.00007f &&
            m_Sounds[SoundArrayIndex(query)].HasFreeSpaceLeft) 
        {
            m_PendingQueries.emplace(query);
        }
    }
    
    void CollisionSoundManager::Update() {
        for(auto it = m_PendingQueries.begin(); it != m_PendingQueries.end(); ++it) {
            if(Power(*it) == COLLISION_SOUND_POWER_NONE) {
                continue;
            }
            Sound_t *sound = m_Sounds[SoundArrayIndex(*it)].FreeSpace();
            if(sound) {
                ExecuteSound(sound, *it);
            }
        }
        
        m_PendingQueries.clear();

        for(size_t i = 0; i < COLLISION_SOUND_TYPE_MAX_ENUM * COLLISION_SOUND_POWER_MAX_ENUM; i++) {
            m_Sounds[i].HasFreeSpaceLeft = m_Sounds->FreeSpace() != nullptr;
        }

        m_ListenerPosition = m_AudioServer->GetListenerPosition();
    }

    void CollisionSoundManager::ExecuteSound(Sound_t *sound, CollisionSoundQuery_t query) {
        sound->SetPosition(query.ApplyPoint);
        sound->SetVolume(Volume(query));
        float random = ((float) rand()) / (float) RAND_MAX;
        sound->SetPitch((1 - random) * 0.5f + random * 0.75f);
        sound->SetVelocity(query.Velocity);
        sound->Restart();
    }

    float CollisionSoundManager::Importance(CollisionSoundManager::CollisionSoundQuery_t query)
    {
        return Power(query) == COLLISION_SOUND_POWER_NONE ? -FLT_MAX : Volume(query) / LenSquared(query.ApplyPoint - m_ListenerPosition);
    }

    CollisionSoundManager::CollisionSoundPower_t CollisionSoundManager::Power(CollisionSoundManager::CollisionSoundQuery_t query) {
        PowerThreshold_t &threshold = m_Thresholds[query.Type];
        for(int i = COLLISION_SOUND_POWER_MAX_ENUM - 1; i >= 0; i--) {
            if(query.EnergyLoss > threshold[i]) {
                return (CollisionSoundPower_t)i;
            }
        }
        return COLLISION_SOUND_POWER_NONE;
    }

    size_t CollisionSoundManager::SoundArrayIndex(CollisionSoundManager::CollisionSoundQuery_t query) {
        ASSERT_V(Power(query) != COLLISION_SOUND_POWER_NONE, -1);
        return query.Type * COLLISION_SOUND_POWER_MAX_ENUM + Power(query);
    }

    float CollisionSoundManager::Volume(CollisionSoundQuery_t query) {
        PowerThreshold_t &threshold = m_Thresholds[query.Type];
        float min = threshold[0];
        float max = threshold[COLLISION_SOUND_POWER_MAX_ENUM - 1] + 1'000.0f;
        float t = (query.EnergyLoss - min) / (max - min);
        t = glm::clamp<float>(t, 0.001f, 0.8f);
        return VOLUME_MULTIPLIER * t;
    }
}