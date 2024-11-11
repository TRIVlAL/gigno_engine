#include "entity_server.h"
#include "entity.h"
#include "../rendering/gui.h"
#include "../error_macros.h"
#include "../debug/profiler/profiler.h"

namespace gigno {

	void EntityServer::Start() {
		Entity *curr = m_pFirstEntity;
		while(curr) {
			curr->Start();
			curr = curr->pNextEntity;
		}
	}

	void EntityServer::Tick(float dt) {
		Profiler::Begin("Entity Update");

		Entity *curr = m_pFirstEntity;
		while(curr) {
			curr->Think(dt);
			curr = curr->pNextEntity;
		}

		Profiler::End();
	}

	void EntityServer::PhysicTick(float dt) {
		Entity *curr = m_pFirstEntity;
		while(curr) {
			curr->PhysicThink(dt);
			curr = curr->pNextEntity;
		}
		curr = m_pFirstEntity;
		while(curr) {
			curr->LatePhysicThink(dt);
			curr = curr->pNextEntity;
		}
	}

	void EntityServer::AddEntity(Entity *entity) {
		entity->pNextEntity = m_pFirstEntity;
		m_pFirstEntity = entity;
	}

	void EntityServer::RemoveEntity(Entity *entity) {
		Entity *curr = m_pFirstEntity;
		if(curr == entity) {
			m_pFirstEntity = entity->pNextEntity;
			return;
		}
		while(curr) {
			if(curr->pNextEntity == entity) {
				curr->pNextEntity = entity->pNextEntity;
				entity->pNextEntity = nullptr;
				return;
			}
			curr = curr->pNextEntity;
		}
		ERR_MSG("Tried to remove entity '%s' but it was never added.", (*entity->Name == '\0' ? "No name" : entity->Name));
	}

#if USE_IMGUI
	void EntityServer::DrawEntityInspectorTab() {

		int open_action = -1;
		if(ImGui::Button("Colapse All")) {
			open_action = 0;
		}
		ImGui::SameLine();
		if(ImGui::Button("Open All")) {
			open_action = 1;
		}

		Entity *curr = m_pFirstEntity;
		int i = 0;
		while(curr) {
			if (open_action >= 0) {
				ImGui::SetNextItemOpen(open_action);
			}

			const char *typeName = curr->GetTypeName();
			const char *name = curr->Name;
			if(!name || *name == '\0') {
				name = "No name";
			}
			size_t header_size = snprintf(nullptr, 0, "%d. %s (%s)", i, name, typeName) + 1;
			char header[header_size];
			snprintf(header, header_size, "%d. %s (%s)", i, name, typeName);
			if(ImGui::CollapsingHeader(header)) {
				bool did_one = false;
				for(BaseSerializedProperty *prop : Serialization::GetProperties(curr)) {

					if(Serialization::IsSpecialToken(prop)) {
						Serialization::HandleSpecialTokenForImGui(prop);
						continue;
					}

					size_t value_str_size = prop->ValueToString(nullptr);
					char value_str[value_str_size];
					prop->ValueToString(value_str);
					ImGui::BulletText("%s : %s", prop->GetName(), value_str);
					did_one = true;
				}
				if(!did_one) {
					ImGui::Text("* nothing to serialize *");
				}
			}
			curr = curr->pNextEntity;
			i++;
		}
	}
#endif

}