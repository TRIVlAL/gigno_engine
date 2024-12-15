#include "point_light.h"

namespace gigno {

    PointLight::PointLight() : Light() {

    }

    PointLight::~PointLight() { }

    void PointLight::FillDataSlots(glm::vec4 *data) const {
        data[0] = {Position, LIGHT_DATA_POINT};
        data[1] = {glm::vec3{Intensity}, LIGHT_DATA_POINT};
    }

}