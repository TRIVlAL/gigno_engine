#include "rendered_entity.h"
#include "../application.h"

namespace gigno {

	ENTITY_DEFINITIONS(RenderedEntity, Entity)

	RenderedEntity::RenderedEntity() : 
		Entity() {
		if(GetApp() && GetApp()->GetRenderer()) {
			GetApp()->GetRenderer()->SubscribeRenderedEntity(this);
		}
	}

	RenderedEntity::~RenderedEntity() {
		if(GetApp() && GetApp()->GetRenderer()) {
			GetApp()->GetRenderer()->UnsubscribeRenderedEntity(this);
		}
	}

    const std::shared_ptr<giModel> &RenderedEntity::GetModel() 
    {	
		if(m_pModel == nullptr) {
			GetApp()->GetRenderer()->CreateModel(m_pModel, ModelData_t::FromObjFile(ModelPath));
		}
		return m_pModel;
    }
}