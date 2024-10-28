#include "rendered_entity.h"
#include <chrono>

namespace gigno {

    class Spinner : public RenderedEntity {
        ENABLE_SERIALIZATION(Spinner);
    public:
        Spinner(ModelData_t modelData) : RenderedEntity(modelData) {}

        float Speed = 2.0f;

        std::chrono::_V2::system_clock::time_point last_rotation;

    private:
        virtual void Think(float dt) override {
            RenderedEntity::Think(dt);
            Transform.Rotation.y = glm::mod<float>(Transform.Rotation.y + (Speed * dt), glm::two_pi<float>());
        }
    };

    DEFINE_SERIALIZATION(Spinner) {
        SERIALIZE_BASE_CLASS(RenderedEntity);

        SERIALIZE(float, Speed);
    }

}