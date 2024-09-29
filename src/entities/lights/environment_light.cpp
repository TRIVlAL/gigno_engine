#include "environment_light.h"

namespace gigno {

    void EnvironmentLight::FillDataSlots(glm::vec4 *data) const {
        data[0] = glm::vec4{intensity, 0.0f, 0.0f, LIGHT_DATA_ENVIRONMENT};
    }

}