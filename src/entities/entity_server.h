#ifndef ENTITY_SERVER_H
#define ENTITY_SERVER_H

#include <vector>
#include <memory>
#include <type_traits>
#include "../features_usage.h"
#include "../utils/arena.h"

namespace gigno {

	class Entity;

	class EntityServer {
		friend class Application;
	public:
		EntityServer() = default;
		~EntityServer() = default;

		void Tick(float dt);
		void PhysicTick(float dt);

		/*
		Returns pointer to entitiy named 'name', or nullptr if it does not exist.
		*/
		Entity *GetEntityByName(const char *name);

		template<typename TEntity>
		TEntity *CreateEntity();

	#if USE_IMGUI
		void DrawEntityInspectorTab();
	#endif
	private:
		Arena m_EntityArena{1024 * 1024 * 1024};

		void UnloadMap();
		bool LoadMap(const char *mapName);
		bool LoadFromFile(const char *filepath);

		std::vector<Entity*> m_Scene{};
	};

    template <typename TEntity>
    inline TEntity *EntityServer::CreateEntity() {
		static_assert(std::is_convertible<TEntity*, Entity*>::value, "CreateEntity : The class must inherit from entity!");

		void *position = m_EntityArena.Alloc(sizeof(TEntity));
		TEntity *entity = new(position) TEntity();
		m_Scene.emplace_back(entity);
		return entity;
    }

}

#endif