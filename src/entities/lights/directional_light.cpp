#include "directional_light.h"
#include "../../application.h"
#include "../camera.h"
#include "../../algorithm/geometry.h"

namespace gigno {

    ENTITY_DEFINITIONS(DirectionalLight, Light)

    void DirectionalLight::Init() {
        m_ProjectionMatrix = glm::ortho(-ShadowMapExtent.x, ShadowMapExtent.x, -ShadowMapExtent.y, ShadowMapExtent.y, 0.001f, ShadowMapExtent.z);
        Direction = glm::normalize(Direction);

        Light::Init();
    }

    void DirectionalLight::FillDataSlots(glm::vec4 *data) const
    {
        data[0] = glm::vec4{-Direction * Intensity, LIGHT_DATA_DIRECTIONAL};
    }

    glm::mat4 DirectionalLight::ShadowMapViewMatrix(const Camera *camera) const {
        glm::vec3 camera_forward = ApplyRotation(camera->Rotation, glm::vec3{-1.0f, 0.0f, 0.0f});
        camera_forward.y = 0.0f;
        camera_forward = glm::normalize(camera_forward);
        glm::vec3 pov_position = camera->Position + (camera_forward * ShadowMapExtent.x * 0.5f) + (-Direction * ShadowMapExtent.z * 0.6f);
        return glm::lookAt(pov_position, pov_position + Direction, glm::vec3{0.0f, -1.0f, 0.0f});
    }

    glm::mat4 DirectionalLight::ShadowMapProjectionMatrix() const {
        return m_ProjectionMatrix;
    }
}