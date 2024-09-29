#ifndef ENVIRONMENT_LIGHT_H
#define ENVIRONMENT_LIGHT_H

#include "light.h"

namespace gigno {

    class EnvironmentLight : Light {
        ENABLE_SERIALIZE(EnvironmentLight, Light);
    public:
        EnvironmentLight() : Light() {}

        float intensity = 0.1f;

        virtual uint32_t DataSlotsCount() const override { return 1; }
        virtual void FillDataSlots(glm::vec4 *data) const override;
    };

}

#endif