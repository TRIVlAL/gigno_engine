#include "rendered_entity.h"
#include "../application.h"

namespace gigno {

	RenderedEntity::RenderedEntity(const std::vector<Vertex> &vertices, const std::vector<giModel::indice_t> &indices, const glm::vec3 &color) : 
		Entity() ,
		Color{ color } {
		GetApp()->GetRenderer()->SubscribeRenderedEntity(this);
		GetApp()->GetRenderer()->CreateModel(pModel, vertices, indices);
	}

	RenderedEntity::~RenderedEntity() {
		giApplication::Singleton()->GetRenderer()->UnsubscribeRenderedEntity(this);

		Entity::~Entity();
	}

}