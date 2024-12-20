#ifndef APPLICATION_H
#define APPLICATION_H

#include "rendering/rendering_server.h"
#include "entities/entity_server.h"
#include "input/input_server.h"
#include "debug/debug_server.h"
#include "rendering/model.h"
#include "physics/physic_server.h"

namespace gigno {

	class Application {
	public:
		static Application *MakeApp();
		static void ShutdownApp();

		static Application *Singleton();

	public:
		int run();

		RenderingServer *GetRenderer() { return &m_RenderingServer; }
        EntityServer *GetEntityServer() { return &m_EntityServer; }
        InputServer *GetInputServer() { return &m_InputServer; }
		PhysicServer *GetPhysicServer() { return &m_PhysicServer; }
		DebugServer *Debug() { return &m_DebugServer; }

		bool Close = false; // When true, finish loop then close the app.

	private:
		static inline Application *s_Instance = nullptr;

		Application(int winw, int winh, const char *title, const std::string &vertShaderPath, const std::string &fragShaderPath);
		~Application();

		bool m_ShowMainUIWindow = true;
		void DrawMainUIWindow();

		const char *m_CurrentMapFilepath;
		const char *m_NextMapFilepath;
		void LoadMap(const char * filepath);
		bool m_ShouldLoadMap = true;

        const float MAX_FRAME_TIME = 1000.0f;

		DebugServer m_DebugServer;
        InputServer m_InputServer; // Must be init before rendering server !
		RenderingServer m_RenderingServer;
        EntityServer m_EntityServer;
		PhysicServer m_PhysicServer;
	};

}

#endif
