#ifndef POINT_LIGHT_H
#define POINT_LIGHT_H

#include "light.h"

namespace gigno {

    class PointLight : public Light {
    ENTITY_DECLARATIONS(PointLight, Light)
    public:
        PointLight();
        ~PointLight();

        virtual uint32_t DataSlotsCount() const override { return 2; };
        virtual void FillDataSlots(glm::vec4 *data) const override;

        float Intensity = 1.0f;
    };

    BEGIN_KEY_TABLE(PointLight)
        DEFINE_KEY_VALUE(float, Intensity)
    END_KEY_TABLE

}

#endif