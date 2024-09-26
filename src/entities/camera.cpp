#include "camera.h"
#include "../application.h"
#include "glm/geometric.hpp"

namespace gigno {

	Camera::Camera() : Entity() {
		if (!GetApp()->GetRenderer()->HasCamera()) {
			GetApp()->GetRenderer()->SetCurrentCamera(this);
		}
	}

	Camera::Camera(float left, float right, float top, float bottom, float near, float far) : Camera() {
		SetOrthographicProjection(left, right, top, bottom, near, far);
	}

	Camera::Camera(float fovy, float aspect, float near, float far) : Camera() {
		SetPerspectiveProjection(fovy, aspect, near, far);
	}

	Camera::~Camera() {
		Entity::~Entity();
	}

	void Camera::SetOrthographicProjection(float left, float right, float top, float bottom, float near, float far) {
		m_ProjMode = PROJECTION_MODE_ORTHOGRAPHIC;

		m_ProjectionMatrix = glm::mat4{ 1.0f };
		m_ProjectionMatrix[0][0] = 2.f / (right - left);
		m_ProjectionMatrix[1][1] = 2.f / (bottom - top);
		m_ProjectionMatrix[2][2] = 1.f / (far - near);
		m_ProjectionMatrix[3][0] = -(right + left) / (right - left);
		m_ProjectionMatrix[3][1] = -(bottom + top) / (bottom - top);
		m_ProjectionMatrix[3][2] = -near / (far - near);
	}

	void Camera::SetPerspectiveProjection(float fovy, float aspect, float near, float far) {
		m_ProjMode = PROJECTION_MODE_PERSPECTIVE;
		m_CurrentFovy = fovy;

		const float tanHalfFovy = tan(fovy / 2.f);
		m_ProjectionMatrix = glm::mat4{ 1.0f };
		m_ProjectionMatrix[0][0] = 1.f / (aspect * tanHalfFovy);
		m_ProjectionMatrix[1][1] = 1.f / (tanHalfFovy);
		m_ProjectionMatrix[2][2] = far / (far - near);
		m_ProjectionMatrix[2][3] = 1.f;
		m_ProjectionMatrix[3][2] = -(far * near) / (far - near);
	}

	void Camera::SetLookAtPoint(glm::vec3 point) {
		m_LookMode = LOOK_MODE_POINT;
		m_LookPoint = point;
	}

	void Camera::SetLookInTransformForward() {
		m_LookMode = LOOK_MODE_TRANSFORM_FORWARD;
	}

	glm::mat4 Camera::GetViewMatrix() const {
		
		if (m_LookMode == LOOK_MODE_POINT) {
			const glm::vec3 direction = m_LookPoint - Transform.translation;
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
			matrix[3][0] = -glm::dot(u, Transform.translation);
			matrix[3][1] = -glm::dot(v, Transform.translation);
			matrix[3][2] = -glm::dot(w, Transform.translation);
			return matrix;
		} else if (m_LookMode == LOOK_MODE_TRANSFORM_FORWARD || true) {
			const float c3 = glm::cos(Transform.rotation.z);
			const float s3 = glm::sin(Transform.rotation.z);
			const float c2 = glm::cos(Transform.rotation.x);
			const float s2 = glm::sin(Transform.rotation.x);
			const float c1 = glm::cos(Transform.rotation.y);
			const float s1 = glm::sin(Transform.rotation.y);
			const glm::vec3 u{ (c1 * c3 + s1 * s2 * s3), (c2 * s3), (c1 * s2 * s3 - c3 * s1) };
			const glm::vec3 v{ (c3 * s1 * s2 - c1 * s3), (c2 * c3), (c1 * c3 * s2 + s1 * s3) };
			const glm::vec3 w{ (c2 * s1), (-s2), (c1 * c2) };
			glm::mat4 matrix = glm::mat4{ 1.0f };
			matrix[0][0] = u.x;
			matrix[1][0] = u.y;
			matrix[2][0] = u.z;
			matrix[0][1] = v.x;
			matrix[1][1] = v.y;
			matrix[2][1] = v.z;
			matrix[0][2] = w.x;
			matrix[1][2] = w.y;
			matrix[2][2] = w.z;
			matrix[3][0] = -glm::dot(u, Transform.translation);
			matrix[3][1] = -glm::dot(v, Transform.translation);
			matrix[3][2] = -glm::dot(w, Transform.translation);
			return matrix;
		}
	}

	void Camera::Think(float dt) {
		Entity::Think(dt);

		// Partial rebuild of the Projection matrix to account for the change of aspect ratio

		if (m_ProjMode == PROJECTION_MODE_ORTHOGRAPHIC) {
			giApplication *app = GetApp();
			const float aspect = app->GetRenderer()->GetAspectRatio();
			m_ProjectionMatrix[0][0] = 2.f / (2 * aspect);
			m_ProjectionMatrix[3][0] = 0;
		}
		else if (m_ProjMode == PROJECTION_MODE_PERSPECTIVE && m_CurrentFovy != 0) {
			const float aspect = GetApp()->GetRenderer()->GetAspectRatio();
			const float tanHalfFovy = tan( m_CurrentFovy / 2.f);
			m_ProjectionMatrix[0][0] = 1.f / (aspect * tanHalfFovy);
			m_ProjectionMatrix[1][1] = 1.f / (tanHalfFovy);
		}
	}

	void Camera::SetAsCurrentCamera() {
		GetApp()->GetRenderer()->SetCurrentCamera(this);
	}

}