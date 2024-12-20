#ifndef FPS_CAMERA_H
#define FPS_CAMERA_H

#include "camera.h"

namespace gigno {

	class DomeCamera : public Camera {
		ENTITY_DECLARATIONS(DomeCamera, Camera)
	public:
		DomeCamera();
		DomeCamera(float moveSpeed);
		DomeCamera(float moveSpeed, CAMERA_ORTHOGRAPHIC_ARGUMENTS_TYPED);
		DomeCamera(float moveSpeed, CAMERA_PROJECTION_ARGUMENTS_TYPED);

		virtual void Init() override;

		void SetTarget(glm::vec3 target);

		float Speed;
		float MaxLower = -1.0f; //-1.0f means no max lower.

		virtual void Think(float dt) override;
		
		glm::vec3 Target{};

	private:
		float m_DistanceToTarget{};
	};

	BEGIN_KEY_TABLE(DomeCamera)
		DEFINE_KEY_VALUE(float, Speed)
		DEFINE_KEY_VALUE(float, MaxLower)
		DEFINE_KEY_VALUE(vec3, Target)
	END_KEY_TABLE

}

#endif