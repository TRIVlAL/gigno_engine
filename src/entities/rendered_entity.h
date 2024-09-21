#ifndef RENDERED_ENTITY_H
#define RENDERED_ENTITY_H

#include "entity.h"
#include <memory>
#include "../rendering/model.h"

namespace gigno {

	class RenderedEntity : public Entity {
		ENABLE_SERIALIZE(RenderedEntity, Entity);
	public:
		RenderedEntity(ModelData_t modelData);
		~RenderedEntity();

		std::shared_ptr<giModel> pModel;
	};

}

#endif
