#ifndef FPS_CAMERA_H
#define FPS_CAMERA_H

#include "camera.h"

namespace gigno {

	class DomeCamera : public Camera {
		ENABLE_SERIALIZATION(DomeCamera);
	public:
		DomeCamera(float moveSpeed);
		DomeCamera(float moveSpeed, CAMERA_ORTHOGRAPHIC_ARGUMENTS_TYPED);
		DomeCamera(float moveSpeed, CAMERA_PROJECTION_ARGUMENTS_TYPED);

		void SetTarget(glm::vec3 target);

		float Speed;

		virtual void Think(float dt) override;
	private:
		glm::vec3 m_Target{};
		float m_DistanceToTarget{};
	};

	DEFINE_SERIALIZATION(DomeCamera) {
		SERIALIZE_BASE_CLASS(Camera);
	}

}

#endif