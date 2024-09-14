#ifndef ENTITY_SERVER_H
#define ENTITY_SERVER_H

#include <vector>
#include <memory>

namespace gigno {

	class Entity;

	class giEntityServer {
		friend class Entity;
	public:
		giEntityServer() {};
		~giEntityServer() {};

		void Tick(double dt);

	private:
		// Called by the base entity constructor and destructor, limit visibility to Entity friend class only.
		void AddEntity(Entity *entity);
		void RemoveEntity(Entity *entity);

		std::vector<Entity *> m_Entities;
	};

}

#endif