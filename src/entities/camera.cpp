#include "camera.h"
#include "../application.h"
#include "glm/geometric.hpp"
#include "../algorithm/geometry.h"
#include "../debug/console/convar.h"
#include "lights/directional_light.h"

namespace gigno {

	ENTITY_DEFINITIONS(Camera, Entity);

	CONVAR(int, r_shadowmap_debug_view, 0, "if not, the camerea will show the shadowmap's view (number gives what cascade)");

	Camera::Camera() : Entity() {
	}

	Camera::Camera(float left, float right, float top, float bottom, float _near, float _far) : Camera() {
		SetOrthographicProjection(left, right, top, bottom, _near, _far);
	}

	Camera::Camera(float fovy, float aspect, float _near, float _far) : Camera() {
		SetPerspectiveProjection(fovy, aspect, _near, _far);
	}

    void Camera::Init() {
		GetApp()->GetRenderer()->SetCurrentCamera(this);

		const float aspect = GetApp()->GetRenderer()->GetAspectRatio();
		
		if(ProjMode == PROJECTION_MODE_PERSPECTIVE) {
			SetPerspectiveProjection(glm::radians<float>(FovY), aspect, Near, Far);
		} else if(ProjMode == PROJECTION_MODE_ORTHOGRAPHIC) {
			SetOrthographicProjection(Left, Right, Top, Bottom, Near, Far);
		}
    }

    void Camera::CleanUp() {
		GetApp()->GetRenderer()->SetCurrentCamera(nullptr);
	}

    void Camera::SetOrthographicProjection(float left, float right, float top, float bottom, float _near, float _far) {
		ProjMode = PROJECTION_MODE_ORTHOGRAPHIC;

		m_ProjectionMatrix = glm::mat4{ 1.0f };
		m_ProjectionMatrix[0][0] = 2.f / (right - left);
		m_ProjectionMatrix[1][1] = 2.f / (bottom - top);
		m_ProjectionMatrix[2][2] = 1.f / (_far - _near);
		m_ProjectionMatrix[3][0] = -(right + left) / (right - left);
		m_ProjectionMatrix[3][1] = -(bottom + top) / (bottom - top);
		m_ProjectionMatrix[3][2] = -_near / (_far - _near);
	}

	void Camera::SetPerspectiveProjection(float fovy, float aspect, float _near, float _far) {
		ProjMode = PROJECTION_MODE_PERSPECTIVE;
		CurrentFovy = fovy;
		
		const float tan_half_fovy = tan(fovy / 2.f);
		m_ProjectionMatrix = glm::mat4{ 1.0f };
		m_ProjectionMatrix[0][0] = 1.f / (aspect * tan_half_fovy);
		m_ProjectionMatrix[1][1] = 1.f / (tan_half_fovy);
		m_ProjectionMatrix[2][2] = (_far + _near) / (_far - _near);
		m_ProjectionMatrix[2][3] = 1.f;
		m_ProjectionMatrix[3][2] = -(2* _far * _near) / (_far - _near);
		m_ProjectionMatrix[3][3] = 1.0f;
		
	}

	void Camera::SetLookAtPoint(glm::vec3 point) {
		LookMode = LOOK_MODE_POINT;
		LookPoint = point;
	}

	void Camera::SetLookInTransformForward() {
		LookMode = LOOK_MODE_TRANSFORM_FORWARD;
	}

    glm::mat4 Camera::GetProjection(bool bypass_convar) const {

		if(!bypass_convar && (int)convar_r_shadowmap_debug_view != 0) {
			RenderingServer *r = GetApp()->GetRenderer();
			auto view_and_proj = r->GetShadowmappedDirectionalLight()->ShadowMapViewAndProjection((int)convar_r_shadowmap_debug_view, r->GetCascadeCount(), this);
			return view_and_proj.second;
		}

        return m_ProjectionMatrix;
    }

    glm::mat4 Camera::GetViewMatrix(bool bypass_convar) const
    {
		if(!bypass_convar && (int)convar_r_shadowmap_debug_view != 0) {
			RenderingServer *r = GetApp()->GetRenderer();
			auto view_and_proj = r->GetShadowmappedDirectionalLight()->ShadowMapViewAndProjection((int)convar_r_shadowmap_debug_view + 1, r->GetCascadeCount(), this);
			return view_and_proj.first;
		}

        if (LookMode == LOOK_MODE_POINT) {
			const glm::vec3 direction = LookPoint - Position;
			const glm::vec3 up = { 0.0f, 1.0f, 0.0f };
			const glm::vec3 w{ glm::normalize(direction) };
			const glm::vec3 u{ glm::normalize(glm::cross(w, up)) };
			const glm::vec3 v{ glm::cross(w, u) };

			glm::mat4 matrix = glm::mat4{ 1.f };
			matrix[0][0] = u.x;
			matrix[1][0] = u.y;
			matrix[2][0] = u.z;
			matrix[0][1] = v.x;
			matrix[1][1] = v.y;
			matrix[2][1] = v.z;
			matrix[0][2] = w.x;
			matrix[1][2] = w.y;
			matrix[2][2] = w.z;
			matrix[3][0] = -glm::dot(u, Position);
			matrix[3][1] = -glm::dot(v, Position);
			matrix[3][2] = -glm::dot(w, Position);
			return matrix;
		} else if (LookMode == LOOK_MODE_TRANSFORM_FORWARD || true) {
			glm::vec3 forward = ApplyRotation(Rotation, glm::vec3{1.0f, 0.0f, 0.0f});
			return glm::lookAt(Position, Position + forward, glm::vec3{0.0f, -1.0f, 0.0f}); 
		}
    }

    void Camera::Think(float dt) {
		Entity::Think(dt);

		// Partial rebuild of the Projection matrix to account for the change of aspect ratio

		if (ProjMode == PROJECTION_MODE_ORTHOGRAPHIC) {
			Application *app = GetApp();
			const float aspect = app->GetRenderer()->GetAspectRatio();
			m_ProjectionMatrix[0][0] = 2.f / (2 * aspect);
			m_ProjectionMatrix[3][0] = 0;
		}
		else if (ProjMode == PROJECTION_MODE_PERSPECTIVE && CurrentFovy != 0) {
			const float aspect = GetApp()->GetRenderer()->GetAspectRatio();
			const float tanHalfFovy = tan( CurrentFovy / 2.f);
			m_ProjectionMatrix[0][0] = 1.f / (aspect * tanHalfFovy);
			m_ProjectionMatrix[1][1] = 1.f / (tanHalfFovy);
		}

		if(GetApp()->GetRenderer()->GetCameraHandle() == this) {
			GetApp()->GetAudioServer()->UpdateListener(Position, Rotation);
		}
	}

    glm::vec3 Camera::RayFromScreenPos(glm::vec2 screenPositionPixel) const {

		glm::vec2 screen_size = GetApp()->GetRenderer()->GetWindowSize();

		glm::vec4 ray_clip = glm::vec4{2.0f * (screenPositionPixel.x / screen_size.x) - 1.0f, 2.0f * (screenPositionPixel.y / screen_size.y) - 1.0f, 1.0f, 1.0f};

		glm::vec4 ray_view = glm::inverse(GetProjection()) * ray_clip;
		ray_view = glm::vec4{ray_view.x, ray_view.y, 1.0f, 0.0f};

		glm::vec3 ray_world = glm::vec3{glm::inverse(GetViewMatrix()) * ray_view};
		return glm::normalize(ray_world);
	}

    void Camera::SetAsCurrentCamera()
    {
        GetApp()->GetRenderer()->SetCurrentCamera(this);
    }
}