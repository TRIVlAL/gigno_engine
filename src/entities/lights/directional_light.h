#ifndef DIRECTIONAL_LIGHT_H
#define DIRECTIONAL_LIGHT_H

#include "light.h"

namespace gigno {

    class DirectionalLight : public Light {
        ENABLE_SERIALIZATION(DireactionalLight);
    public:
        DirectionalLight() : Light() {};
        ~DirectionalLight() {}

        float Intensity;
        glm::vec3 Direction{0.0f, -1.0f, 0.0f};

        virtual uint32_t DataSlotsCount() const override { return 1; }
        virtual void FillDataSlots(glm::vec4 *data) const override;
    };

    DEFINE_SERIALIZATION(DirectionalLight) {
        
    }

}

#endif