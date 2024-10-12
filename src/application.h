#ifndef APPLICATION_H
#define APPLICATION_H

#include "rendering/rendering_server.h"
#include "entities/entity_server.h"
#include "input/input_server.h"
#include "debug/debug_server.h"
#include "rendering/model.h"
#include "iostream"

namespace gigno {

	class Application {
	public:
		static Application *MakeApp();
		static void ShutdownApp();

		static Application *Singleton();

	public:
		int run();

		RenderingServer *GetRenderer() { return &m_RenderingServer; }
        EntityServer *GetEntityServer();
        InputServer *GetInputServer() { return &m_InputServer; }
		DebugServer *Debug() { return &m_DebugServer; }

	private:

		Application(int winw, int winh, const char *title, const std::string &vertShaderPath, const std::string &fragShaderPath);
		~Application();

		bool m_ShowMainUIWindow = true;
		void DrawMainUIWindow();

        const float MAX_FRAME_TIME = 1000.0f;

		static inline Application *s_Instance = nullptr;

		DebugServer m_DebugServer;
        InputServer m_InputServer; // Must be init before rendering server !
		RenderingServer m_RenderingServer;
        EntityServer m_EntityServer;
	};

}

#endif
