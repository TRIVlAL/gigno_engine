#ifndef ENTITY_SERVER_H
#define ENTITY_SERVER_H

#include <vector>
#include <memory>
#include <type_traits>
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

		template<typename TEntity>
		TEntity *CreateEntity();

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

    template <typename TEntity>
    inline TEntity *EntityServer::CreateEntity() {
		static_assert(std::is_convertible<TEntity*, Entity*>::value, "CreateEntity : The class must inherit from entity!");

		TEntity *entity = new TEntity();
		m_Scene.emplace_back(entity);
		return entity;
    }

}

#endif