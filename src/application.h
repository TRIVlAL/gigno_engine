#ifndef APPLICATION_H
#define APPLICATION_H

#include "rendering/rendering_server.h"
#include "entities/entity_server.h"
#include "input/input_server.h"
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

		giRenderingServer *GetRenderer() { return &m_RenderingServer; }
        giEntityServer *GetEntityServer();
        giInputServer *GetInputServer() { return &m_InputServer; }

	private:

		giApplication(int winw, int winh, const char *title, const std::string &vertShaderPath, const std::string &fragShaderPath);
		~giApplication();

        const float MAX_FRAME_TIME = 1000.0f;

		static inline giApplication *s_Instance = nullptr;

        giInputServer m_InputServer; // Must be init before rendering server !
		giRenderingServer m_RenderingServer;
        giEntityServer m_EntityServer;
	};

}

#endif
