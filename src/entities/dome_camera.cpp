#include "dome_camera.h"
#include "../application.h"

namespace gigno {

	ENTITY_DEFINITIONS(DomeCamera, Camera)

	DomeCamera::DomeCamera() : Camera()
    {
    }

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

    void DomeCamera::Init() {
		Camera::Init();
		SetTarget(Target);
    }

    void DomeCamera::SetTarget(glm::vec3 target) {
		Target = target;
		Position.y = target.y;
		glm::vec3 to = target - Position;
		m_DistanceToTarget = glm::sqrt(to.x * to.x + to.y * to.y + to.z * to.z);
		SetLookAtPoint(target);
	}

	void DomeCamera::Think(float dt) {
		Camera::Think(dt);

		float right_move = 0;
		float up_move = 0;

		InputServer *input = GetApp()->GetInputServer();

		if (input->GetKey(KEY_D)) right_move++;
		if (input->GetKey(KEY_A)) right_move--;
		if (input->GetKey(KEY_W)) up_move++;
		if (input->GetKey(KEY_S)) up_move--;

		glm::vec3 parralel = glm::vec3{ Target.z-Position.z, 0, -(Target.x-Position.x) } / m_DistanceToTarget;

		if (glm::sqrt(parralel.x * parralel.x + parralel.z * parralel.z) < 0.15f) {
			up_move -= glm::sign(Position.y - Target.y);
		}

		Position += parralel * right_move * Speed * dt;

		if(MaxLower >= 0.0f && Position.y + up_move * Speed * dt > Target.y - MaxLower) {
			Position.y += up_move * Speed * dt;
		}


		glm::vec3 targetToMe = Position - Target;
		Position = Target + ( glm::normalize(targetToMe) * m_DistanceToTarget);
	}

}