#include "entity_server.h"
#include "entity.h"
#include "../rendering/gui.h"
#include "../error_macros.h"
#include "../debug/profiler/profiler.h"
#include <fstream>

#include "../physics/rigid_body.h"

#include <mutex>

#include "map_parser.h"

namespace gigno {

	std::mutex s_EntityUnloadMutex;

	void EntityServer::Tick(float dt) {
		Profiler::Begin("Entity Update");

		for(size_t i = 0; i < m_Scene.size(); i++) {
			m_Scene[i]->Think(dt);
		}

		Console::Singleton()->UpdateCommands(dt);

		Profiler::End();
	}

	void EntityServer::PhysicTick(float dt) {
		for(size_t i = 0; i < m_Scene.size(); i++) {
			m_Scene[i]->PhysicThink(dt);
		}
		for(size_t i = 0; i < m_Scene.size(); i++) {
			m_Scene[i]->LatePhysicThink(dt);
		}
	}

    void EntityServer::UnloadMap() {
		for(int i = 0; i < m_Scene.size(); i++) {
			m_Scene[i]->CleanUp();
			m_Scene[i]->~Entity();
		}
		m_Scene.clear();
		m_EntityArena.FreeAll();
    }

    bool EntityServer::LoadFromFile(const char *filepath) {

		Console::LogInfo("Loading map file '%s'", filepath);

		// Check if file exists before unloading the map.
		MapParser mp{};
		std::vector<MapParser::MapCommand_t> command_list{mp(filepath)};
		if(command_list.size() == 0) {
			return false;
		}

		std::lock_guard<std::mutex> lock{s_EntityUnloadMutex};
		UnloadMap();

		// Some entities are in the scene but were spawned by code.
		// In that case, we won't call Init() from here ! Thus, the distinction is made
		// Between entities loaded from file and entities in m_Scene.
		std::vector<Entity *> loaded_from_file{};
		Entity *current_entity{};

		size_t i = 0;
		while(i < command_list.size()) {
			if(command_list[i].Type == MapParser::MAP_COMMAND_CREATE_ENTITY) {

				void *position = m_EntityArena.Alloc(Entity::EntitySizeOf(command_list[i].TypeName));
				current_entity = Entity::CreateEntityAt(command_list[i].TypeName, position);

				if (!current_entity) {
					Console::LogWarning("PARSER : Entity type '%s' does not exist ! Did you forget to add the ENTITY_DEFINITIONS macro in a .cpp file ?", command_list[i].TypeName);
					i++;
					while(i < command_list.size() && command_list[i].Type == MapParser::MAP_COMMAND_SET_KEY_VALUE) {
						// Wont set for an entity type that does not exist. keep moving !
						i++;
					}
					continue;
				}
				else {
					m_Scene.emplace_back(current_entity);
					loaded_from_file.emplace_back(current_entity);
					i++;
				}
			} else {
				ASSERT_V(command_list[i].Type == MapParser::MAP_COMMAND_SET_KEY_VALUE, false);

				if(current_entity && command_list[i].TypeName  && strcmp(command_list[i].TypeName, "") != 0) {
					current_entity->SetKeyValue(command_list[i].TypeName, (const char **)(command_list[i].Values), command_list[i].ValueCount);
				}

				i++;
			}
		}

		for(int i = 0; i < loaded_from_file.size(); i++) {
			loaded_from_file[i]->Init();
		}

		Console::LogInfo(ConsoleMessageFlags_t::MESSAGE_NO_TIME_CODE_BIT,
		"-------------------------------------------------------------");

		return true;
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

		int i = 0;
		for(Entity *curr : m_Scene) {
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
				for(std::pair<const char *, Value_t> keyvalue : curr->KeyValues()) {
					size_t value_str_size = ToString<Value_t>(nullptr, keyvalue.second);
					char value_str[value_str_size];
					ToString<Value_t>(value_str, keyvalue.second);
					ImGui::BulletText("%s : %s", keyvalue.first, value_str);
					did_one = true;
				}
				if(!did_one) {
					ImGui::Text("* nothing to serialize *");
				}
			}
			i++;
		}
	}
#endif

}