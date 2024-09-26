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
            RenderedEntity::Think(dt);

            if(Transform.rotation.y + (Speed * dt) > glm::two_pi<float>()) {
                std::cout << "ROTATION AT TIME " << std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) << std::endl;
                std::cout << "TIME SINCE LAST ROTATION" << (std::chrono::system_clock::now() - last_rotation).count() << std::endl;
                last_rotation = std::chrono::system_clock::now();
            }

            Transform.rotation.y = glm::mod<float>(Transform.rotation.y + (Speed * dt), glm::two_pi<float>());

        }
    };

}