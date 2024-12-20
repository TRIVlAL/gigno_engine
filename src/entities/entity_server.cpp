#include "entity_server.h"
#include "entity.h"
#include "../rendering/gui.h"
#include "../error_macros.h"
#include "../debug/profiler/profiler.h"
#include <fstream>

#include "../physics/rigid_body.h"

namespace gigno {

	void EntityServer::Tick(float dt) {
		Profiler::Begin("Entity Update");

		for(Entity *ent : m_Scene) {
			ent->Think(dt);
		}

		Console::Singleton()->UpdateCommands(dt);

		Profiler::End();
	}

	void EntityServer::PhysicTick(float dt) {
		for(Entity *ent : m_Scene) {
			ent->PhysicThink(dt);
		}
		for(Entity *ent : m_Scene) {
			ent->LatePhysicThink(dt);
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

    void EntityServer::UnloadMap() {
		m_Scene.clear();
    }

    bool EntityServer::LoadFromFile(std::ifstream &source)
    {

        char curr;

		const size_t MAX_WORD_SIZE = 255;
		const size_t MAX_VALUE_ARGC = 5;

		enum {
			AWAIT_ENTITY,
			AWAIT_OPEN_BRACE,
			AWAIT_KEY, 
			AWAIT_COLON,
			READING_VALUE
		};

		auto curr_key = AWAIT_ENTITY;
		bool in_word = false;
		bool begin_word = false;
		bool finish_word = false;

		char entity_name_buffer[MAX_WORD_SIZE];
		size_t entity_name_index = 0;
		char key_buffer[MAX_WORD_SIZE];
		size_t key_index = 0;
		// Normally I wouldnt allocate on the heap but
		// For some reason, i cant pass stack allocated char[][]
		// to functions.
		char **value_buffers = new char*[MAX_VALUE_ARGC];
		for(int i = 0; i < MAX_VALUE_ARGC; i++) {
			value_buffers[i] = new char[MAX_WORD_SIZE];
		}
		size_t value_word_index = 0;
		size_t value_letter_index = 0;
		Entity *curr_ent = nullptr;

		while(source>>curr) {
			if(curr == '"') {
				if(in_word) {
					finish_word = true;
					in_word = false;
				} else {
					begin_word = true;
					in_word = true;
				}
			}

			if(curr_key == AWAIT_ENTITY) {
				if(in_word && curr != '"') {
					entity_name_buffer[entity_name_index++] = curr;
				} else {
					if(finish_word) {
						entity_name_buffer[entity_name_index++] = '\0';
						curr_ent = m_Scene.emplace_back(Entity::CreateEntity(entity_name_buffer));
						if(!curr_ent) {
							Console::LogWarning("Parsing : Entity type '%s' does not exist !", entity_name_buffer);
						}
						entity_name_index = 0;
						curr_key = AWAIT_OPEN_BRACE;
					}
				}
			} else if(curr_key == AWAIT_OPEN_BRACE) {
				if(curr == '{') {
					curr_key = AWAIT_KEY;
				}
				if(curr == '"') {
					Console::LogInfo("Parsing : Expected open brace '{' (Entity declaration %s)", entity_name_buffer);
				}
			} else if(curr_key == AWAIT_KEY) {
				if(in_word && curr != '"') {
					key_buffer[key_index++] = curr;
				} else {
					if(curr_key == '}') {
						curr_key = AWAIT_ENTITY;
					}
					if(finish_word) {
						key_buffer[key_index++] = '\0';
						key_index = 0;
						curr_key = AWAIT_COLON;
					}
				}
			} else if(curr_key == AWAIT_COLON) {
				if(curr == ':') {
					curr_key = READING_VALUE;
				} else if(curr != '\n' && curr != ' ' && curr != '\t' != curr != '\v' && curr != '\r') {
					Console::LogInfo("Parsing : Expected colon ':' but got character '%c' (entity declaration '%s')", curr, entity_name_buffer);
					return false;
				}
			} else if(curr_key == READING_VALUE) {
				if(curr == 'j') {
					int v = 0;
				}
				if(curr == ',' || curr == '}') {
					curr_key = curr == ',' ? AWAIT_KEY : AWAIT_ENTITY;
					curr_ent->SetKeyValue(key_buffer, (const char **)value_buffers, value_word_index);
					value_word_index = 0;
					value_letter_index = 0;
				}

				if(in_word && !finish_word && curr != '"') {
					value_buffers[value_word_index][value_letter_index++] = curr;
				} else if(finish_word) {
					value_buffers[value_word_index][value_letter_index] = '\0';
					value_letter_index = 0;
					value_word_index++;
				}
			}


			begin_word = finish_word = false;
		}

		if(curr_key != AWAIT_ENTITY) {
			Console::LogInfo("Parsing : Entity declaration '%s' not finished!", entity_name_buffer);
			return false;
		}

		for(int i = 0; i < MAX_VALUE_ARGC; i++) {
			delete[] value_buffers[i];
		}
		delete[] value_buffers;

		for(Entity *ent : m_Scene) {
			ent->Init();
		}

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