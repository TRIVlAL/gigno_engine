#include "entity_server.h"
#include "entity.h"
#include "../rendering/gui.h"
#include "../error_macros.h"

namespace gigno {

	void EntityServer::Start() {
		Entity *curr = m_pFirstEntity;
		while(curr) {
			curr->Start();
			curr = curr->pNextEntity;
		}
	}

	void EntityServer::Tick(float dt) {
		Application::Singleton()->Debug()->Profiler()->Begin("Entity Update");

		Entity *curr = m_pFirstEntity;
		while(curr) {
			curr->Think(dt);
			curr = curr->pNextEntity;
		}

		Application::Singleton()->Debug()->Profiler()->End();
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
		ERR_MSG("Tried to remove entity '%s' but it was never added.", (entity->Name == "" ? "No name" : entity->Name.c_str()));
	}

#if USE_IMGUI
	void EntityServer::DrawEntityInspector(bool *open) {
		if(!*open) {
			return;
		}

		if(!ImGui::Begin("Entity Debug", open)) {
			ImGui::End();
			return;
		};

		int openAction = -1;
		if(ImGui::Button("Colapse All")) {
			openAction = 0;
		}
		ImGui::SameLine();
		if(ImGui::Button("Open All")) {
			openAction = 1;
		}

		Entity *curr = m_pFirstEntity;
		int i = 0;
		while(curr) {
			if(openAction >= 0) {
				ImGui::SetNextItemOpen(openAction);
			}

			std::string typeName = curr->GetTypeName();
			if(typeName == "#Noname") {
				continue;
			}
			if(ImGui::CollapsingHeader((std::to_string(i) + ". " + (curr->Name == "" ? "No name" : curr->Name) + " (" + typeName + ")").data())) {

				for(PropertySerializationData_t data : Serialization::GetProperties(curr)) {

					if(Serialization::IsSpecialToken(data)) {
						Serialization::HandleSpecialTokenForImGui(data);
						continue;
					}

					std::string valueString;
					if(data.ToString(valueString)) {
						ImGui::BulletText((data.Name + " : " + valueString).data());
					}
				}
			}
			curr = curr->pNextEntity;
			i++;
		}

		ImGui::End();
	}
#endif

}