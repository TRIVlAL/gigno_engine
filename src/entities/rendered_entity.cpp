#include "rendered_entity.h"
#include "../application.h"

namespace gigno {

	ENTITY_DEFINITIONS(RenderedEntity, Entity)


    void RenderedEntity::Init() {
		Entity::Init();
		GetApp()->GetRenderer()->SubscribeRenderedEntity(this);
	}

    void RenderedEntity::CleanUp() {
		GetApp()->GetRenderer()->UnsubscribeRenderedEntity(this);
		if(m_pModel) {
			GetApp()->GetRenderer()->ClenUpModel(m_pModel);
		}
    }

    const std::shared_ptr<giModel> RenderedEntity::GetModel() {	
		if(DoRender && m_pModel == nullptr) {
			GetApp()->GetRenderer()->CreateModel(m_pModel, ModelData_t::FromObjFile(ModelPath));
		}
		return DoRender ? m_pModel : nullptr;
    }
}