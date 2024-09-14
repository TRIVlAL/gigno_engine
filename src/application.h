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
        std::vector<Vertex> m_Vertices{
            // left face (white)
          {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
          {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
          {{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
          {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},

          // right face (yellow)
          {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
          {{.5f, .5f, .5f}, {.8f, .8f, .1f}},
          {{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
          {{.5f, .5f, -.5f}, {.8f, .8f, .1f}},

          // top face (orange, remember y axis points down)
          {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
          {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
          {{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
          {{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},

          // bottom face (red)
          {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
          {{.5f, .5f, .5f}, {.8f, .1f, .1f}},
          {{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
          {{.5f, .5f, -.5f}, {.8f, .1f, .1f}},

          // nose face (blue)
          {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
          {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
          {{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
          {{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},

          // tail face (green)
          {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
          {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
          {{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
          {{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},

        };

        std::vector<giModel::indice_t> m_Indices    { 0,  1,  2,  0,  3,  1,  4,  5,  6,  4,  7,  5,  8,  9,  10, 8,  11, 9,
                                                    12, 13, 14, 12, 15, 13, 16, 17, 18, 16, 19, 17, 20, 21, 22, 20, 23, 21 };

		giApplication(int winw, int winh, const char *title, const std::string &vertShaderPath, const std::string &fragShaderPath);
		~giApplication();

        const double MAX_FRAME_TIME = 1000.0;

		static inline giApplication *s_Instance = nullptr;

        giInputServer m_InputServer; // Must be init before rendering server !
		giRenderingServer m_RenderingServer;
        giEntityServer m_EntityServer;
	};

}

#endif
