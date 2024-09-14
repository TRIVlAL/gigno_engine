#ifndef RENDERED_ENTITY_H
#define RENDERED_ENTITY_H

#include "entity.h"
#include <memory>
#include "../rendering/model.h"

namespace gigno {

	class RenderedEntity : public Entity {
	public:
		RenderedEntity(const std::vector<Vertex> &vertices, const std::vector<giModel::indice_t> &indices, const glm::vec3 &color);
		~RenderedEntity();

		glm::vec3 Color;
		std::shared_ptr<giModel> pModel;
	};

}

#endif
