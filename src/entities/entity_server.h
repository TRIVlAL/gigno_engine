#ifndef ENTITY_SERVER_H
#define ENTITY_SERVER_H

#include <vector>
#include <memory>
#include "../features_usage.h"

namespace gigno {

	class Entity;

	class EntityServer {
		friend class Entity;
	public:
		EntityServer() {};
		~EntityServer() {};

		void Start();
		void Tick(float dt);
		void PhysicTick(float dt);

	#if USE_IMGUI
		void DrawEntityInspectorTab();
	#endif
	private:
		// Called by the base entity constructor and destructor, limit visibility to Entity friend class only.
		void AddEntity(Entity *entity);
		void RemoveEntity(Entity *entity);

		// First entity in the chain of all entities (linked list). Use entity->pNextEntity for next element in the list.
		// If this is null, there are no entity. If next is null, it is the last entity.
		Entity *m_pFirstEntity{};
	};

}

#endif