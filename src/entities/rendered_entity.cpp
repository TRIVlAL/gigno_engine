#include "rendered_entity.h"
#include "../application.h"

namespace gigno {

	RenderedEntity::RenderedEntity(ModelData_t modelData) : 
		Entity() {
		GetApp()->GetProfiler()->Begin("Create Rendered Entity ");

		GetApp()->GetRenderer()->SubscribeRenderedEntity(this);
		GetApp()->GetRenderer()->CreateModel(pModel, modelData);
		
		GetApp()->GetProfiler()->End();
	}

	RenderedEntity::~RenderedEntity() {
		GetApp()->GetRenderer()->UnsubscribeRenderedEntity(this);
	}

}