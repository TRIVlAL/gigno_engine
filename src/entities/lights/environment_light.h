#ifndef ENVIRONMENT_LIGHT_H
#define ENVIRONMENT_LIGHT_H

#include "light.h"

namespace gigno {

    class EnvironmentLight : public Light {
        ENTITY_DECLARATIONS(EnvironmentLight, Light)
    public:
        EnvironmentLight() : Light() {}
        ~EnvironmentLight() {}

        float Intensity = 0.1f;

        virtual uint32_t DataSlotsCount() const override { return 1; }
        virtual void FillDataSlots(glm::vec4 *data) const override;
    };

    BEGIN_KEY_TABLE(EnvironmentLight)
        DEFINE_KEY_VALUE(float, Intensity)
    END_KEY_TABLE


}

#endif