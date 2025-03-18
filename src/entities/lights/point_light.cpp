#include "point_light.h"

namespace gigno {

    ENTITY_DEFINITIONS(PointLight, Light)

    PointLight::PointLight() : Light() {

    }

    void PointLight::FillDataSlots(glm::vec4 *data) const {
        data[0] = {Position, LIGHT_DATA_POINT};
        data[1] = {glm::vec3{Intensity}, LIGHT_DATA_POINT};
    }

}