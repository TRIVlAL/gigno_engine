#include "rendered_entity.h"

namespace gigno {

    class Spinner : public RenderedEntity {
        BEGIN_SERIALIZE(Spinner, RenderedEntity)
        SERIALIZE(float, Speed);
        END_SERIALIZE
    public:
        Spinner(ModelData_t modelData) : RenderedEntity(modelData) {}

        float Speed = 2.0f;

    private:
        virtual void Think(double dt) override {
            RenderedEntity::Think(dt);

            Transform.rotation.y = glm::mod<float>(Transform.rotation.y + (glm::radians(Speed) * static_cast<float>(dt)), glm::two_pi<float>());
        }
    };

}