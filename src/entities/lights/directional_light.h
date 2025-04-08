#ifndef DIRECTIONAL_LIGHT_H
#define DIRECTIONAL_LIGHT_H

#include "light.h"
#include <utility>

namespace gigno {
    class Camera;

    class DirectionalLight : public Light {
        ENTITY_DECLARATIONS(DirectionalLight, Light);
    public:
        DirectionalLight() : Light() {};
        ~DirectionalLight() {}

        virtual void Init() override;

        float Intensity;
        glm::vec3 Direction{0.0f, -1.0f, 0.0f};

        float ShadowMapExtent = 300.0f;
        float ShadowMapFar = 200.0f;

        virtual uint32_t DataSlotsCount() const override { return 1; }
        virtual void FillDataSlots(glm::vec4 *data) const override;

        // Only Directional Light Supports Shadow Mapping
        std::pair<glm::mat4, glm::mat4> ShadowMapViewAndProjection(size_t cascadeIndex, size_t cascadeCount, const Camera *camera) const;
    private:

    };

    BEGIN_KEY_TABLE(DirectionalLight)
        DEFINE_KEY_VALUE(float, Intensity)
        DEFINE_KEY_VALUE(vec3, Direction)
        DEFINE_KEY_VALUE(float, ShadowMapExtent)
        DEFINE_KEY_VALUE(float, ShadowMapFar)
    END_KEY_TABLE


}

#endif