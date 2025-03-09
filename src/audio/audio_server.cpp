#include "audio_server.h"
#include "../error_macros.h"
#include "../algorithm/geometry.h"

#define MINIAUDIO_IMPLEMENTATION
#include "../vendor/miniaudio/miniaudio.h"

namespace gigno {

    void AudioServer::Init() {
        ma_engine_config config = ma_engine_config_init();

        ma_result result = ma_engine_init(&config, &m_Engine);
        if(result != MA_SUCCESS) {
            Console::LogError("Failed to init Miniaudio Engine ! Miniaudio error code : %n", (int)result);
        }
    }

    AudioServer::~AudioServer() {
        ma_engine_uninit(&m_Engine);
    }

    Sound_t *AudioServer::NewSound(const char *path) {
        void *mem = m_SoundArena.Alloc(sizeof(Sound_t));
        Sound_t *sound = new(mem) Sound_t();
        sound->Init(path, &m_Engine);
        return sound;
    }

    void AudioServer::DeleteSound(Sound_t * sound) {
        sound->CleanUp();
        sound->~Sound_t();
        m_SoundArena.Free((void*)sound, sizeof(Sound_t));
    }

    void AudioServer::UpdateListener(glm::vec3 position, glm::vec3 rotation) {
        ma_engine_listener_set_position(&m_Engine, 0, position.x, position.y, position.z);
        glm::vec3 forward = ApplyRotation(rotation, glm::vec3{-1.0f, 0.0f, 0.0f});
        ma_engine_listener_set_direction(&m_Engine, 0, forward.x, forward.y, forward.z);
    }

    glm::vec3 AudioServer::GetListenerPosition() {
        ma_vec3f pos = ma_engine_listener_get_position(&m_Engine, 0);
        return glm::vec3{pos.x, pos.y, pos.z};
    }

    void Sound_t::Play(){
        ma_sound_start(&m_Sound);
    }

    void Sound_t::Stop() {
        ma_sound_stop(&m_Sound);
        ma_sound_seek_to_pcm_frame(&m_Sound, 0);
    }

    void Sound_t::Pause() {
        ma_sound_stop(&m_Sound);
    }

    void Sound_t::Restart() {
        ma_sound_seek_to_pcm_frame(&m_Sound, 0);
        Play();
    }

    float Sound_t::GetAdvencment() {
        uint64_t length{};
        ma_result result = ma_sound_get_length_in_pcm_frames(&m_Sound, &length);
        if(result != MA_SUCCESS) {
            Console::LogError("Failed to query sound length ! Miniaudio error code : %d", (int)result);
        }
        
        uint64_t position{};
        result = ma_sound_get_cursor_in_pcm_frames(&m_Sound, &position);
        if(result != MA_SUCCESS) {
            Console::LogError("Failed to query sound cursor ! Miniaudio error code : %d", (int)result);
        }

        return position / length;
    }

    void Sound_t::Init(const char *path, ma_engine *engine)
    {
        ma_result result = ma_sound_init_from_file(engine, path, 0, nullptr, nullptr, &m_Sound);
        if(result != MA_SUCCESS) {
            Console::LogError("Failed to initialize sound '%s' ! Miniaudio error code : %d", path, (int)result);
        }
    }

    void Sound_t::CleanUp() {
        ma_sound_uninit(&m_Sound);
    }
}