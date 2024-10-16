#ifndef ENTITY_SERVER_H
#define ENTITY_SERVER_H

#include <vector>
#include <memory>
#include "../core_macros.h"

namespace gigno {

	class Entity;

	class EntityServer {
		friend class Entity;
	public:
		EntityServer() {};
		~EntityServer() {};

		void Start();
		void Tick(float dt);

	#if USE_IMGUI
		void DrawEntityInspector(bool *open);
	#endif
	private:
		// Called by the base entity constructor and destructor, limit visibility to Entity friend class only.
		void AddEntity(Entity *entity);
		void RemoveEntity(Entity *entity);


		std::vector<Entity *> m_Entities;
	};

}

#endif