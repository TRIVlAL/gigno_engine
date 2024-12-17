#include "environment_light.h"

namespace gigno {

    ENTITY_DEFINITIONS(EnvironmentLight, Light)

    void EnvironmentLight::FillDataSlots(glm::vec4 *data) const {
        data[0] = glm::vec4{this->Intensity, 0.0f, 0.0f, LIGHT_DATA_ENVIRONMENT};
    }

}