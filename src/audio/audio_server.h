#ifndef AUDIO_SERVER_H
#define AUDIO_SERVER_H

#include "../vendor/miniaudio/miniaudio.h"
#include "../algorithm/arena.h"
#include "../vendor/glm/glm/glm.hpp"

namespace gigno {

    
    struct Sound_t {
        void Play();
        void Stop();
        void Pause();
        void Restart();

        bool IsPlaying() { return ma_sound_is_playing(&m_Sound); }

        void SetLooping(bool looping) {ma_sound_set_looping(&m_Sound, looping); }
        void SetPosition(glm::vec3 position) {ma_sound_set_position(&m_Sound, position.x, position.y, position.z); }
        void SetVolume(float volume) {ma_sound_set_volume(&m_Sound, volume); }
        void SetPitch(float pitch) {ma_sound_set_pitch(&m_Sound, pitch); }
        void SetVelocity(glm::vec3 velocity) {ma_sound_set_velocity(&m_Sound, velocity.x, velocity.y, velocity.z); }

        // 0 - is at begining, 1 - is at end
        float GetAdvencment();
        
    friend class AudioServer;
    private:
        Sound_t() = default;
        void Init(const char *path, ma_engine *engine);
        void CleanUp();

        ma_sound m_Sound{};
    };

    class AudioServer {
    public:
        AudioServer() = default;
        ~AudioServer();

        void Init();

        Sound_t *NewSound(const char *path);
        void DeleteSound(Sound_t *sound);

        void UpdateListener(glm::vec3 position, glm::quat rotation);
        glm::vec3 GetListenerPosition();

        void SetGlobalVolume(float volume) { ma_engine_set_volume(&m_Engine, volume); }

    private:
        ma_engine m_Engine{};

        //Every sounds are allocated in there.
        const size_t SOUND_ALLOCATED_MEMORY = (sizeof(Sound_t) * 510);
        Arena m_SoundArena{SOUND_ALLOCATED_MEMORY};
    };

}

#endif