#include "directional_light.h"

namespace gigno {

    void DirectionalLight::FillDataSlots(glm::vec4 *data) const {
        data[0] = glm::vec4{-Direction * Intensity, LIGHT_DATA_DIRECTIONAL};
    }

}