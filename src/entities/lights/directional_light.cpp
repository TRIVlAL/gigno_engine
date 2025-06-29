#include "directional_light.h"
#include "../../application.h"
#include "../camera.h"
#include "../../utils/geometry.h"

namespace gigno {

    ENTITY_DEFINITIONS(DirectionalLight, Light)

    void DirectionalLight::Init() {
        Direction = glm::normalize(Direction);

        Light::Init();
    }

    void DirectionalLight::FillDataSlots(glm::vec4 *data) const
    {
        data[0] = glm::vec4{-Direction * Intensity, LIGHT_DATA_DIRECTIONAL};
    }

    std::pair<glm::mat4, glm::mat4> DirectionalLight::ShadowMapViewAndProjection(size_t cascadeIndex, size_t cascadeCount, const Camera *camera) const {
        
        std::array<glm::vec3, 4> far_face{};
        std::array<glm::vec3, 4> near_face{};

        glm::vec3 cam_forward = ApplyRotation(camera->Rotation, glm::vec3{-1.0f, 0.0f, 0.0f});
        glm::vec3 cam_up = ApplyRotation(camera->Rotation, glm::vec3{0.0f, 1.0f, 0.0f});
        glm::vec3 cam_right = ApplyRotation(camera->Rotation, glm::vec3{0.0f, 0.0f, 1.0f});
        
        float ar = GetApp()->GetRenderer()->GetAspectRatio();
        float angle_x = glm::min<float>(camera->FovY * 1.3f * ar, 89.9f);

        float corner_to_forward_cosine = glm::dot(glm::cos(angle_x) * cam_forward + glm::sin(angle_x) * cam_up + glm::sin(angle_x) * cam_right, cam_forward);
        float far_side_length = ShadowMapExtent/corner_to_forward_cosine;

        near_face[0] = camera->Position + (glm::cos(angle_x) * cam_forward + glm::sin(angle_x) * cam_up + glm::sin(angle_x) * cam_right) * -camera->Near;
        near_face[1] = camera->Position + (glm::cos(angle_x) * cam_forward - glm::sin(angle_x) * cam_up + glm::sin(angle_x) * cam_right) * -camera->Near;
        near_face[2] = camera->Position + (glm::cos(angle_x) * cam_forward + glm::sin(angle_x) * cam_up - glm::sin(angle_x) * cam_right) * -camera->Near;
        near_face[3] = camera->Position + (glm::cos(angle_x) * cam_forward - glm::sin(angle_x) * cam_up - glm::sin(angle_x) * cam_right) * -camera->Near;

        far_face[0] = camera->Position + (glm::cos(angle_x) * cam_forward + glm::sin(angle_x) * cam_up + glm::sin(angle_x) * cam_right) * far_side_length;
        far_face[1] = camera->Position + (glm::cos(angle_x) * cam_forward - glm::sin(angle_x) * cam_up + glm::sin(angle_x) * cam_right) * far_side_length;
        far_face[2] = camera->Position + (glm::cos(angle_x) * cam_forward + glm::sin(angle_x) * cam_up - glm::sin(angle_x) * cam_right) * far_side_length;
        far_face[3] = camera->Position + (glm::cos(angle_x) * cam_forward - glm::sin(angle_x) * cam_up - glm::sin(angle_x) * cam_right) * far_side_length;

        //Each cascade is trice the size of the last.
        float p1{};
        float p2{};{
            float base_percent = (1.0f)/glm::pow(3, cascadeCount);
            p1 = cascadeIndex == 0 ? 0.0f : base_percent * glm::pow(3, cascadeIndex - 1);
            p2 = base_percent * glm::pow(3, cascadeIndex);
        }

        std::array<glm::vec3, 4> sm_near{};
        std::array<glm::vec3, 4> sm_far{};
        for(size_t i = 0; i < 4; i++) {
            sm_near[i] = p1 * (far_face[i] - near_face[i]);
            sm_far[i] = p2 * (far_face[i] - near_face[i]);
        }

        glm::vec3 sm_center{};
        for(size_t i = 0; i < 4; i++) {
            sm_center += sm_near[i];
            sm_center += sm_far[i];
        }
        sm_center = sm_center / 8.0f;

        glm::vec3 pov = sm_center - Direction * ShadowMapFar * 0.6f;

        glm::mat4 light_view = glm::lookAt(camera->Position + pov, camera->Position + pov + Direction, glm::vec3{0.0f, -1.0f, 0.0f});

        float xmin = FLT_MAX;
        float xmax = -FLT_MAX;
        float ymin = FLT_MAX;
        float ymax = -FLT_MAX;

        glm::vec3 direction_floor = glm::vec3{Direction.x, 0.0f, Direction.z};
        glm::vec3 direction_floor_perpendicular = glm::vec3{-Direction.z, 0.0f, Direction.x};

        for(size_t i = 0; i < 4; i++) {
            xmin = glm::min<float>(xmin, glm::dot(sm_near[i], direction_floor_perpendicular));
            xmax = glm::max<float>(xmax, glm::dot(sm_near[i], direction_floor_perpendicular));
            xmin = glm::min<float>(xmin, glm::dot(sm_far[i], direction_floor_perpendicular));
            xmax = glm::max<float>(xmax, glm::dot(sm_far[i], direction_floor_perpendicular));

            ymin = glm::min<float>(ymin, glm::dot(sm_near[i], direction_floor));
            ymax = glm::max<float>(ymax, glm::dot(sm_near[i], direction_floor));
            ymin = glm::min<float>(ymin, glm::dot(sm_far[i], direction_floor));
            ymax = glm::max<float>(ymax, glm::dot(sm_far[i], direction_floor));
        }

        float xwidth = xmax - xmin;
        float ywidth = ymax - ymin;

        glm::mat4 light_projection = glm::ortho(xwidth, -xwidth, ywidth, -ywidth, 1.0f, ShadowMapFar);

        return std::pair<glm::mat4, glm::mat4>{
            glm::mat4{light_view},
            light_projection};
    }
}