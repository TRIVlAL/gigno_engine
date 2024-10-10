#include "rendered_entity.h"
#include "../application.h"

namespace gigno {

	RenderedEntity::RenderedEntity(ModelData_t modelData) : 
		Entity() {

		GetApp()->GetRenderer()->SubscribeRenderedEntity(this);
		GetApp()->GetRenderer()->CreateModel(pModel, modelData);
	}

	RenderedEntity::~RenderedEntity() {
		GetApp()->GetRenderer()->UnsubscribeRenderedEntity(this);
	}

}