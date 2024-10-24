#include "entity.h"
#include "../application.h"
#include "entity_server.h"
#include <string.h>

namespace  gigno {

	// translation -> rotation y -> rotation x -> rotation z -> scale
	glm::mat4 Transform_t::TransformationMatrix() const {
		const float ca = glm::cos(Rotation.y);
		const float sa = glm::sin(Rotation.y);
		const float cb = glm::cos(Rotation.x);
		const float sb = glm::sin(Rotation.x);
		const float cc = glm::cos(Rotation.z);
		const float sc = glm::sin(Rotation.z);
		return glm::mat4{
			{Scale.x * (ca * cc + sa * sb * sc),
			Scale.x * (cb * sc),
			Scale.x * (ca * sb * sc - cc * sa),
			0.0f },

			{Scale.y * (cc * sa * sb - ca * sc),
			Scale.y * (cb * cc),
			Scale.y * (ca * cc * sb + sa * sc),
			0.0f},

			{Scale.z * (cb * sa),
			Scale.z * (-sb),
			Scale.z * (ca * cb),
			0.0f},

			{Position.x,
			Position.y,
			Position.z,
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
		SERIALIZE(glm::vec3, Transform.Position);
		SERIALIZE(glm::vec3, Transform.Scale);
		SERIALIZE(glm::vec3, Transform.Rotation);
	}
}