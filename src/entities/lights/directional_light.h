#ifndef DIRECTIONAL_LIGHT_H
#define DIRECTIONAL_LIGHT_H

#include "light.h"

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

        virtual uint32_t DataSlotsCount() const override { return 2; }
        virtual void FillDataSlots(glm::vec4 *data) const override;

        // Only Directional Light Supports Shadow Mapping
        glm::mat4 ShadowMapViewMatrix(const Camera *camera) const;
        glm::mat4 ShadowMapProjectionMatrix() const;
        
        glm::vec3 ShadowMapExtent{30.0f, 30.0f, 400.0f}; //left-right, up-down, far
    private:
        glm::mat4 m_ProjectionMatrix;
    };

    BEGIN_KEY_TABLE(DirectionalLight)
        DEFINE_KEY_VALUE(float, Intensity)
        DEFINE_KEY_VALUE(vec3, Direction)

        DEFINE_KEY_VALUE(vec3, ShadowMapExtent)
    END_KEY_TABLE


}

#endif