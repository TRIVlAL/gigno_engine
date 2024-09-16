#ifndef ENTITY_H
#define ENTITY_H

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/constants.hpp"

namespace gigno {
	class giApplication;

	struct Transform_t {
		glm::vec3 translation{};
		glm::vec3 scale{1.0f, 1.0f, 1.0f};
		glm::vec3 rotation{};

		glm::mat4 TransformationMatrix() const;
		glm::mat3 NormalMatrix() const;
	};

	class Entity {
	public:
		Entity(const Entity &) = delete;
		Entity &operator=(const Entity &) = delete;
		Entity(Entity &&) = default;
		Entity &operator=(Entity &&) = default;

		Entity();
		~Entity();

		// Called Every Tick by the Entity Server
		virtual void Think(double dt) {};

		Transform_t Transform{};
	protected:
		giApplication *GetApp() const;
	};

}

#endif
