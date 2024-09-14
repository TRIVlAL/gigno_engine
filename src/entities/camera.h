#ifndef CAMERA_H
#define CAMERA_H

#include "entity.h"
#include "glm/glm.hpp"

#define CAMERA_ORTHOGRAPHIC_ARGUMENTS_TYPED float left, float right, float top, float bottom, float near, float far
#define CAMERA_ORTOGRAPHIC_ARGUMENTS_VALUES left, right, top, bottom, near, far
#define CAMERA_PROJECTION_ARGUMENTS_TYPED float fovy, float aspect, float near, float far
#define CAMERA_PROJECTION_ARGUMENTS_VALUES fovy, aspect, near, far

namespace gigno {

	enum ProjectionMode_t {
		PROJECTION_MODE_NONE,
		PROJECTION_MODE_PERSPECTIVE,
		PROJECTION_MODE_ORTHOGRAPHIC,
		PROJECTION_MODE_MAX_ENUM
	};

	enum LookMode_t {
		LOOK_MODE_TRANSFORM_FORWARD,
		LOOK_MODE_POINT
	};

	class Camera : public Entity {
	public:
		Camera();
		Camera(float left, float right, float top, float bottom, float near, float far);
		Camera(float fovy, float aspect, float near, float far);
		~Camera();

		void SetOrthographicProjection(float left, float right, float top, float bottom, float near, float far );
		void SetPerspectiveProjection(float fovy, float aspect, float near, float far);

		void SetLookAtPoint(glm::vec3 point);
		void SetLookInTransformForward();

		glm::mat4 GetProjection() const { return m_ProjectionMatrix; }
		glm::mat4 GetViewMatrix() const;

		void SetAsCurrentCamera();

		virtual void Think(double dt) override;

	private:
		ProjectionMode_t m_ProjMode = PROJECTION_MODE_NONE;
		float m_CurrentFovy = 0.0f;

		LookMode_t m_LookMode = LOOK_MODE_TRANSFORM_FORWARD;
		glm::vec3 m_LookPoint = {};

		glm::mat4 m_ProjectionMatrix = { 1.0f };
	};

}

#endif