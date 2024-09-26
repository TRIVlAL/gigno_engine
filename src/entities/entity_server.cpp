#include "entity_server.h"
#include "entity.h"
#include "../rendering/gui.h"

namespace gigno {

	void giEntityServer::Start() {
		for(Entity *entity : m_Entities) {
			entity->Start();
		}
	}

	void giEntityServer::Tick(float dt) {
		for (Entity *entity : m_Entities) {
			entity->Think(dt);
		}

	#if USE_IMGUI
		if(EntityInspectorEnable) {
			DrawEntityInspector();
		}
	#endif
	}

	void giEntityServer::AddEntity(Entity *entity) {
		m_Entities.push_back(entity);
	}

	void giEntityServer::RemoveEntity(Entity *entity) {
		m_Entities.erase(std::remove(m_Entities.begin(), m_Entities.end(), entity), m_Entities.end());
	}

#if USE_IMGUI
	void giEntityServer::DrawEntityInspector() {
		if(!m_EntityInspectorOpen) {
			return;
		}

		if(!ImGui::Begin("Entity Debug", &m_EntityInspectorOpen)) {
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

		int i = 0;
		for(Entity *entity : m_Entities) {
			if(openAction >= 0) {
				ImGui::SetNextItemOpen(openAction);
			}

			std::string typeName = entity->GetTypeName();
			if(typeName == "##Noname") {
				continue;
			}
			if(ImGui::CollapsingHeader((std::to_string(i) + ". " + (entity->Name == "" ? "No name" : entity->Name) + " (" + typeName + ")").data())) {

				for(PropertySerializationData_t data : Serialization::GetProperties(entity)) {

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
			i++;
		}

		ImGui::End();
	}
#endif

}