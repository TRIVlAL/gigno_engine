#include "entity.h"
#include "../application.h"
#include "entity_server.h"
#include <string.h>
#include "../debug/console/convar.h"
#include "../features_usage.h"

namespace  gigno {

	#if USE_DEBUG_DRAWING
	CONVAR(bool, draw_transform_debug, false, "Shows debug drawings that represents the transforms of every entity");
	#endif

	// translation -> rotation y -> rotation x -> rotation z -> scale
	// Rotation Tait-Bryan YXZ (see @ https://en.wikipedia.org/wiki/Euler_angles (Rotation Matrix))
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

	glm::vec3 Transform_t::ApplyRotate(glm::vec3 v) const {
		const float ca = glm::cos(Rotation.y);
		const float sa = glm::sin(Rotation.y);
		const float cb = glm::cos(Rotation.x);
		const float sb = glm::sin(Rotation.x);
		const float cc = glm::cos(Rotation.z);
		const float sc = glm::sin(Rotation.z);
		return glm::mat3{
			{(ca * cc + sa * sb * sc),
			(cb * sc),
			(ca * sb * sc - cc * sa)},

			{(cc * sa * sb - ca * sc),
			(cb * cc),
			(ca * cc * sb + sa * sc)},

			{(cb * sa),
			(-sb),
			(ca * cb)}} * v;
	}

	Application *Entity::GetApp() const{
		return Application::Singleton();
	}

	Entity::Entity() {
		GetApp()->GetEntityServer()->AddEntity(this);
	}

	Entity::~Entity() {
		GetApp()->GetEntityServer()->RemoveEntity(this);
		for(BaseSerializedProperty *prop : serializedProps) {
			delete prop;
		}
	}

	void Entity::Think(float dt) {
		#if USE_DEBUG_DRAWING
		if((bool)convar_draw_transform_debug) {
			RenderingServer *renderer = GetApp()->GetRenderer();
			if(renderer->GetCameraHandle() == this) {
				return;
			}
			renderer->DrawLine(Transform.Position, Transform.Position + Transform.ApplyRotate(glm::vec3{1.0f, 0.0f, 0.0f} * 2.0f), 
							glm::vec3{1.0f, 0.0f, 0.0f});
			renderer->DrawLine(Transform.Position, Transform.Position + Transform.ApplyRotate(glm::vec3{0.0f, 1.0f, 0.0f} * 2.0f),
							   glm::vec3{0.0f, 1.0f, 0.0f});
			renderer->DrawLine(Transform.Position, Transform.Position + Transform.ApplyRotate(glm::vec3{0.0f, 0.0f, 1.0f} * 2.0f),
							   glm::vec3{0.0f, 0.0f, 1.0f});
		}
		#endif
	}

	DEFINE_SERIALIZATION(Entity)
	{
		SERIALIZE(glm::vec3, Transform.Position);
		SERIALIZE(glm::vec3, Transform.Rotation);
		SERIALIZE(glm::vec3, Transform.Scale);
	}
}