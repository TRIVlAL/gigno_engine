#ifndef CAMERA_H
#define CAMERA_H

#include "entity.h"
#include "glm/glm.hpp"

#define CAMERA_ORTHOGRAPHIC_ARGUMENTS_TYPED float left, float right, float top, float bottom, float _near, float _far
#define CAMERA_ORTOGRAPHIC_ARGUMENTS_VALUES left, right, top, bottom, _near, _far
#define CAMERA_PROJECTION_ARGUMENTS_TYPED float fovy, float aspect, float _near, float _far
#define CAMERA_PROJECTION_ARGUMENTS_VALUES fovy, aspect, _near, _far

namespace gigno {

	enum ProjectionMode_t {
		PROJECTION_MODE_PERSPECTIVE = 0,
		PROJECTION_MODE_ORTHOGRAPHIC = 1,
		PROJECTION_MODE_MAX_ENUM
	};

	enum LookMode_t {
		LOOK_MODE_TRANSFORM_FORWARD = 0,
		LOOK_MODE_POINT = 1
	};

	class Camera : public Entity {
		ENTITY_DECLARATIONS(Camera, Entity);
	public:
		Camera();
		Camera(float left, float right, float top, float bottom, float _near, float _far);
		Camera(float fovy, float aspect, float _near, float _far);

		virtual void Init() override;
		virtual void CleanUp() override;

		void SetOrthographicProjection(float left, float right, float top, float bottom, float _near, float _far );
		void SetPerspectiveProjection(float fovy, float aspect, float _near, float _far);

		void SetLookAtPoint(glm::vec3 point);
		void SetLookInTransformForward();

		glm::mat4 GetProjection() const { return m_ProjectionMatrix; }
		glm::mat4 GetViewMatrix() const;

		void SetAsCurrentCamera();

		virtual void Think(float dt) override;

		// Perspective Projection :
		float FovY = 50.0f;
		float Near = -0.05f;
		float Far = 1.0f;

		// Orthographic Porjection :
		float Left;
		float Right;
		float Top;
		float Bottom;

		int ProjMode = (int)PROJECTION_MODE_PERSPECTIVE;
		float CurrentFovy = 0.0f;
		int LookMode = (int)LOOK_MODE_TRANSFORM_FORWARD;
		glm::vec3 LookPoint = {};
	private:
		glm::mat4 m_ProjectionMatrix = { 1.0f };
	};

	BEGIN_KEY_TABLE(Camera)
		DEFINE_KEY_VALUE(int, ProjMode)
		DEFINE_KEY_VALUE(float, CurrentFovy)
		DEFINE_KEY_VALUE(int, LookMode)
		DEFINE_KEY_VALUE(vec3, LookPoint)
		DEFINE_KEY_VALUE(float, FovY)
		DEFINE_KEY_VALUE(float, Near)
		DEFINE_KEY_VALUE(float, Far)
		DEFINE_KEY_VALUE(float, Left)
		DEFINE_KEY_VALUE(float, Right)
		DEFINE_KEY_VALUE(float, Top)
		DEFINE_KEY_VALUE(float, Bottom)
	END_KEY_TABLE

}

#endif