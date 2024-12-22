#ifndef ENTITY_SERVER_H
#define ENTITY_SERVER_H

#include <vector>
#include <memory>
#include "../features_usage.h"

namespace gigno {

	class Entity;

	class EntityServer {
		friend class Application;
	public:
		EntityServer() {};
		~EntityServer() {};

		void Tick(float dt);
		void PhysicTick(float dt);

	#if USE_IMGUI
		void DrawEntityInspectorTab();
	#endif
	private:
		// Called by the base entity constructor and destructor, limit visibility to Entity friend class only.
		void AddEntity(Entity *entity);
		void RemoveEntity(Entity *entity);

		void UnloadMap();
		bool LoadFromFile(std::ifstream &source);

		std::vector<Entity*> m_Scene{};
	};

}

#endif