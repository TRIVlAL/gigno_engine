#include "dome_camera.h"
#include "../application.h"

namespace gigno {

	DomeCamera::DomeCamera(float moveSpeed) : Camera(),
		Speed{ moveSpeed }
	{
	}

	DomeCamera::DomeCamera(float moveSpeed, CAMERA_ORTHOGRAPHIC_ARGUMENTS_TYPED ) : Camera( CAMERA_ORTOGRAPHIC_ARGUMENTS_VALUES ),
		Speed{ moveSpeed }
	{
	}

	DomeCamera::DomeCamera(float moveSpeed, CAMERA_PROJECTION_ARGUMENTS_TYPED) : Camera( CAMERA_PROJECTION_ARGUMENTS_VALUES ),
		Speed{ moveSpeed }
	{
	}

	void DomeCamera::SetTarget(glm::vec3 target) {
		m_Target = target;
		Transform.translation.y = target.y;
		glm::vec3 to = target - Transform.translation;
		m_DistanceToTarget = glm::sqrt(to.x * to.x + to.y * to.y + to.z * to.z);
		SetLookAtPoint(target);
	}

	void DomeCamera::Think(float dt) {
		Camera::Think(dt);

		float rightMove = 0;
		float upMove = 0;

		InputServer *input = GetApp()->GetInputServer();

		if (input->GetKey(KEY_D)) rightMove++;
		if (input->GetKey(KEY_A)) rightMove--;
		if (input->GetKey(KEY_W)) upMove++;
		if (input->GetKey(KEY_S)) upMove--;

		glm::vec3 parralel = glm::vec3{ m_Target.z-Transform.translation.z, 0, -(m_Target.x-Transform.translation.x) } / m_DistanceToTarget;

		if (glm::sqrt(parralel.x * parralel.x + parralel.z * parralel.z) < 0.15f) {
			upMove-= glm::sign(Transform.translation.y - m_Target.y);
		}

		Transform.translation += parralel * rightMove * Speed * dt;
		Transform.translation.y += upMove * Speed * dt;

		glm::vec3 targetToMe = Transform.translation - m_Target;
		Transform.translation = m_Target + ( glm::normalize(targetToMe) * m_DistanceToTarget);
	}

}