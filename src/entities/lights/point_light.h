#ifndef POINT_LIGHT_H
#define POINT_LIGHT_H

#include "light.h"

namespace gigno {

    class PointLight : public Light {
        ENABLE_SERIALIZATION(PointLight);
    public:
        PointLight();
        ~PointLight();

        virtual uint32_t DataSlotsCount() const override { return 2; };
        virtual void FillDataSlots(glm::vec4 *data) const override;

        float Intensity = 1.0f;
    };

    DEFINE_SERIALIZATION(PointLight) {
        SERIALIZE_BASE_CLASS(Light);
    }

}

#endif