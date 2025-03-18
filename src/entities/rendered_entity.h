#ifndef RENDERED_ENTITY_H
#define RENDERED_ENTITY_H

#include "entity.h"
#include <memory>
#include "../rendering/model.h"

namespace gigno {

	class RenderedEntity : public Entity {
		ENTITY_DECLARATIONS(RenderedEntity, Entity)
	public:

		virtual void Init() override;
		virtual void CleanUp() override;

		const char *ModelPath = nullptr;

		// Next rendered entity in the RenderingServer's chain of all rendered entities (linked list). Set on construction.
		// 'nullptr' if is last element.
		RenderedEntity *pNextRenderedEntity{};

		const std::shared_ptr<giModel> GetModel();

		bool DoRender = true; //Visibility of this entity
	private:
		std::shared_ptr<giModel> m_pModel{};
	};

	BEGIN_KEY_TABLE(RenderedEntity)
		DEFINE_KEY_VALUE(cstr, ModelPath)
	END_KEY_TABLE

}

#endif
