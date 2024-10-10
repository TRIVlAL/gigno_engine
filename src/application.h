#ifndef APPLICATION_H
#define APPLICATION_H

#include "rendering/rendering_server.h"
#include "entities/entity_server.h"
#include "input/input_server.h"
#include "profiling/profiling_server.h"
#include "vulkan/vulkan.h"
#include "rendering/model.h"
#include "iostream"

namespace gigno {

	class giApplication {
	public:
		static giApplication *MakeApp();
		static void ShutdownApp();

		static giApplication *Singleton();

	public:
		int run();

		RenderingServer *GetRenderer() { return &m_RenderingServer; }
        EntityServer *GetEntityServer();
        InputServer *GetInputServer() { return &m_InputServer; }
		ProfilingServer *GetProfiler() { return &m_ProfilingServer; }

	private:

		giApplication(int winw, int winh, const char *title, const std::string &vertShaderPath, const std::string &fragShaderPath);
		~giApplication();

		bool m_ShowMainUIWindow = true;
		void DrawMainUIWindow();

        const float MAX_FRAME_TIME = 1000.0f;

		static inline giApplication *s_Instance = nullptr;

		ProfilingServer m_ProfilingServer;
        InputServer m_InputServer; // Must be init before rendering server !
		RenderingServer m_RenderingServer;
        EntityServer m_EntityServer;
	};

}

#endif
