#include "rendered_entity.h"
#include <chrono>

namespace gigno {

    class Spinner : public RenderedEntity {
        BEGIN_SERIALIZE(Spinner, RenderedEntity)
        SERIALIZE(float, Speed);
        END_SERIALIZE
    public:
        Spinner(ModelData_t modelData) : RenderedEntity(modelData) {}

        float Speed = 2.0f;

        std::chrono::_V2::system_clock::time_point last_rotation;

    private:
        virtual void Think(float dt) override {
            GetApp()->GetProfiler()->Begin("Spinner think");

            RenderedEntity::Think(dt);
            Transform.rotation.y = glm::mod<float>(Transform.rotation.y + (Speed * dt), glm::two_pi<float>());
            
            GetApp()->GetProfiler()->End();
        }
    };

}