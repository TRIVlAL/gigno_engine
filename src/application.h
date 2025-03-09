#ifndef APPLICATION_H
#define APPLICATION_H

#include "rendering/rendering_server.h"
#include "entities/entity_server.h"
#include "input/input_server.h"
#include "debug/debug_server.h"
#include "rendering/model.h"
#include "physics/physic_server.h"
#include "audio/audio_server.h"
#include <string>

int main();

namespace gigno {

	static void status(const CommandToken_t &args); //command 'status'

	class Application {
		friend int ::main();
		#if USE_CONSOLE
		friend void status(const CommandToken_t &args); //command 'status'
		#endif
	public:
		static Application *Singleton();

	public:
		int run();

		RenderingServer *GetRenderer() { return &m_RenderingServer; }
        EntityServer *GetEntityServer() { return &m_EntityServer; }
        InputServer *GetInputServer() { return &m_InputServer; }
		PhysicServer *GetPhysicServer() { return &m_PhysicServer; }
		AudioServer *GetAudioServer() { return &m_AudioServer; }
		DebugServer *Debug() { return &m_DebugServer; }

		bool Close = false; // When true, finish loop then close the app.

		void LoadMap(const char * filepath);
	private:
		static inline Application *s_Instance = nullptr;

		Application();
		~Application();

		bool m_ShowMainUIWindow = true;
		void DrawMainUIWindow();

		std::string m_CurrentMapFilepath;
		std::string m_NextMapFilepath;
		bool m_ShouldLoadMap = true;

        const float MAX_FRAME_TIME = 1000.0f;

		DebugServer m_DebugServer;
        InputServer m_InputServer; // Must be init before rendering server !
		RenderingServer m_RenderingServer;
		AudioServer m_AudioServer; // Must be init before physics server !
        EntityServer m_EntityServer;
		PhysicServer m_PhysicServer;
	};

}

#endif
