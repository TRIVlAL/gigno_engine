#ifndef ENTITY_SERVER_H
#define ENTITY_SERVER_H

#include <vector>
#include <memory>

#include "../core_macros.h"

namespace gigno {

	class Entity;

	class giEntityServer {
		friend class Entity;
	public:
		giEntityServer() {};
		~giEntityServer() {};

		void Start();
		void Tick(float dt);

#if USE_IMGUI
		bool EntityInspectorEnable = false;
#endif

	private:
		// Called by the base entity constructor and destructor, limit visibility to Entity friend class only.
		void AddEntity(Entity *entity);
		void RemoveEntity(Entity *entity);

#if USE_IMGUI
		bool m_EntityInspectorOpen = true; 
		void DrawEntityInspector();
#endif

		std::vector<Entity *> m_Entities;
	};

}

#endif