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

	enum AppExitCode_t {
		EXIT_NONE = 0,
		CONTINUE = -1, //Do not exit
		SUCCESS = 0,
		EXIT_SIMPLE = SUCCESS,
		EXIT_FAILED_UNKNOWN = 1,
		EXIT_FAILED_GLFW = 2,
		EXIT_FAILED_RENDERER = 3,
		EXIT_FAILED_VULKAN_SUPPORT = 4,
	};

	static void status(const CommandToken_t &args); //command 'status'

	class Application {
		friend int ::main();
		#if USE_CONSOLE
		friend void status(const CommandToken_t &args); //command 'status'
		#endif
	public:
		static Application *Singleton();

	public:
		AppExitCode_t run();

		RenderingServer *GetRenderer() { return &m_RenderingServer; }
        EntityServer *GetEntityServer() { return &m_EntityServer; }
        InputServer *GetInputServer() { return &m_InputServer; }
		PhysicServer *GetPhysicServer() { return &m_PhysicServer; }
		AudioServer *GetAudioServer() { return &m_AudioServer; }
		DebugServer *Debug() { return &m_DebugServer; }

		std::string GetNextMap() const {return m_NextMap;};

		/*
		The app will continue until a call to SetExit, which gives the exit code.
		*/
		void SetExit(AppExitCode_t exit);
		AppExitCode_t GetExit() { return m_CurrentStatus; }
		
		void LoadMap(const char * filepath);

	private:
		static inline Application *s_Instance = nullptr;
		
		AppExitCode_t m_CurrentStatus = CONTINUE;

		Application();
		~Application();

		bool m_ShowMainUIWindow = true;
		void DrawMainUIWindow();

		std::string m_CurrentMap;
		std::string m_NextMap;
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
