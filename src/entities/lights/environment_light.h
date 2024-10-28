#ifndef ENVIRONMENT_LIGHT_H
#define ENVIRONMENT_LIGHT_H

#include "light.h"

namespace gigno {

    class EnvironmentLight : Light {
        ENABLE_SERIALIZATION(EnvironmentLight);
    public:
        EnvironmentLight() : Light() {}
        ~EnvironmentLight() {}

        float intensity = 0.1f;

        virtual uint32_t DataSlotsCount() const override { return 1; }
        virtual void FillDataSlots(glm::vec4 *data) const override;
    };

    DEFINE_SERIALIZATION(EnvironmentLight) {
        SERIALIZE_BASE_CLASS(Light);
    }

}

#endif