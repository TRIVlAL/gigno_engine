#include "rendered_entity.h"
#include <chrono>

namespace gigno {


    class Spinner : public RenderedEntity {
        ENTITY_DECLARATIONS(Spinner, RenderedEntity)
    public:
        Spinner() : RenderedEntity() {}

        float Speed = 2.0f;

        std::chrono::_V2::system_clock::time_point last_rotation;

    private:
        virtual void Think(float dt) override {
            RenderedEntity::Think(dt);
            Rotation.y = glm::mod<float>(Rotation.y + (Speed * dt), glm::two_pi<float>());
        }
    };

    BEGIN_KEY_TABLE(Spinner)
        DEFINE_KEY_VALUE(float, Speed)
    END_KEY_TABLE

    ENTITY_DEFINITIONS(Spinner, RenderedEntity)

}