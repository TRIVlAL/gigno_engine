#include "entity.h"
#include "../application.h"
#include "entity_server.h"
#include <string.h>

namespace  gigno {

	// translation -> rotation y -> rotation x -> rotation z -> scale
	glm::mat4 Transform_t::TransformationMatrix() const {
		float ca = glm::cos(rotation.y);
		float sa = glm::sin(rotation.y);
		float cb = glm::cos(rotation.x);
		float sb = glm::sin(rotation.x);
		float cc = glm::cos(rotation.z);
		float sc = glm::sin(rotation.z);
		return glm::mat4{
			{scale.x * (ca * cc + sa * sb * sc),
			scale.x * (cb * sc),
			scale.x * (ca * sb * sc - cc * sa),
			0.0f },

			{scale.y * (cc * sa * sb - ca * sc),
			scale.y * (cb * cc),
			scale.y * (ca * cc * sb + sa * sc),
			0.0f},

			{scale.z * (cb * sa),
			scale.z * (-sb),
			scale.z * (ca * cb),
			0.0f},

			{translation.x,
			translation.y,
			translation.z,
			1.0f}
		};
	}

	glm::mat3 Transform_t::NormalMatrix() const {
		return glm::transpose(glm::inverse(glm::mat3(TransformationMatrix())));
	}

	Application *Entity::GetApp() const{
		return Application::Singleton();
	}

	Entity::Entity() {
		GetApp()->GetEntityServer()->AddEntity(this);
	}

	Entity::~Entity() {
		GetApp()->GetEntityServer()->RemoveEntity(this);
	}

	void Entity::AddSerializedProperties() {
		serializedProps.reserve(3);
		SERIALIZE(glm::vec3, Transform.translation);
		SERIALIZE(glm::vec3, Transform.scale);
		SERIALIZE(glm::vec3, Transform.rotation);
	}
}