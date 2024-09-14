#include "entity_server.h"
#include "entity.h"

namespace gigno {

	void giEntityServer::Tick(double dt) {
		for (Entity *entity : m_Entities) {
			entity->Think(dt);
		}
	}

	void giEntityServer::AddEntity(Entity *entity) {
		m_Entities.push_back(entity);
	}

	void giEntityServer::RemoveEntity(Entity *entity) {
		m_Entities.erase(std::remove(m_Entities.begin(), m_Entities.end(), entity), m_Entities.end());
	}

}