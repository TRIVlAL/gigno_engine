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
	glm::mat4 Entity::TransformationMatrix() const {
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

    glm::mat3 Entity::RotationMatrix() const {
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
			 (ca * cb) }
		};
	}

    glm::mat3 Entity::NormalMatrix() const {
		return glm::transpose(glm::inverse(glm::mat3(TransformationMatrix())));
	}

	glm::vec3 Entity::ApplyRotate(glm::vec3 v) const {
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

	std::vector<std::pair<const char *, Value_t>> Entity::KeyValues(){
		std::vector<std::pair<const char *, Value_t>> ret{Entity::KeyValueCount()};
		int i  = 0;
		for (auto& [key, owned_value] : KeyTableAccessor<Entity>::KeyValues) {
			ret[i++] = {key, FromOwnedValue(this, owned_value)};
		}
		return ret;
	}

    size_t Entity::KeyValueCount() const {
		auto &a = KeyTableAccessor<Entity>::KeyValues;
        return KeyTableAccessor<Entity>::KeyValues.size();
    }

    Value_t Entity::GetKeyValue_t(const char *key) {
        Value_t ret;
		if(GetKeyvalue<Entity>(ret, this, key)) {
			return ret;
		} else {
			Console::LogError("GetKeyValue_t : Entity has no key '%s'", key);
			return Value_t{};
		}
    }

    void Entity::SetKeyValue(const char *key, const char **args, int argC) {
		if(!SetKeyvalueFromString<Entity>(this, key, args, argC)) {
			Console::LogWarning("SetKeyValue : Entity has no key '%s'", key);
		}
    }

    Entity *Entity::CreateEntity(const char *entityType) {
		auto new_function = s_NewEntityMethodMap.find(entityType);
		if (new_function == s_NewEntityMethodMap.end())
		{
			return nullptr;
		}
		else
		{
			return (new_function->second)();
		}
	}

    Entity *Entity::CreateEntityAt(const char *entityType, void *position) {
		auto placement_new_function = s_PlacementNewEntityMethodMap.find(entityType);
		if(placement_new_function == s_PlacementNewEntityMethodMap.end()) {
			return nullptr;
		} else {
			return (placement_new_function->second)(position);
		}
    }

    size_t Entity::EntitySizeOf(const char *entityType) {
		auto size = s_SizeOfMap.find(entityType);
		if(size == s_SizeOfMap.end()) {
			Console::LogError("Querrying size of an entity type that does not exist : %s", entityType);
			return 0;
		} else {
			return size->second; 
		}
    }

    Application *Entity::GetApp() const {
        return Application::Singleton();
    }

    void Entity::Think(float dt)
    {
#if USE_DEBUG_DRAWING
		if((bool)convar_draw_transform_debug) {
			RenderingServer *renderer = GetApp()->GetRenderer();
			if(renderer->GetCameraHandle() == this) {
				return;
			}
			renderer->DrawLine(Position, Position + ApplyRotate(glm::vec3{1.0f, 0.0f, 0.0f} * 2.0f), 
							glm::vec3{1.0f, 0.0f, 0.0f});
			renderer->DrawLine(Position, Position + ApplyRotate(glm::vec3{0.0f, 1.0f, 0.0f} * 2.0f),
							   glm::vec3{0.0f, 1.0f, 0.0f});
			renderer->DrawLine(Position, Position + ApplyRotate(glm::vec3{0.0f, 0.0f, 1.0f} * 2.0f),
							   glm::vec3{0.0f, 0.0f, 1.0f});
		}
		#endif
    }
}