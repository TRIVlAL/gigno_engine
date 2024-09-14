#ifndef PIPELINE_H
#define PIPELINE_H

#include <vector>
#include <string>
#include "device.h"

namespace gigno {
	struct giPipelineConfigInfo {
		giPipelineConfigInfo(const giPipelineConfigInfo &) = delete;
		giPipelineConfigInfo &operator=(const giPipelineConfigInfo &) = delete;

		VkPipelineViewportStateCreateInfo viewportState;
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
		VkPipelineRasterizationStateCreateInfo rasterizationInfo;
		VkPipelineMultisampleStateCreateInfo multisampleInfo;
		VkPipelineColorBlendAttachmentState colorBlendAttachment;
		VkPipelineColorBlendStateCreateInfo colorBlendInfo;
		VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
		std::vector<VkDynamicState> dynamicStatesEnables;
		VkPipelineDynamicStateCreateInfo dynamicStatesCreateInfo;
		VkPipelineLayout pipelineLayout = nullptr;
		VkRenderPass renderPass = nullptr;
		uint32_t subpass = 0;
	};

	class giPipeline {
	public:
		giPipeline(VkDevice device, const std::string& vertFilepath, const std::string& fragFilepath, const giPipelineConfigInfo& createInfo);
		~giPipeline();

		void CleanUp(VkDevice device);

		VkPipeline GetVkPipeline() { return m_VkPipeline; }

		giPipeline(const giPipeline &) = delete;
		giPipeline(giPipeline &) = delete;

		static void DefaultConfig(giPipelineConfigInfo &configInfo);
	private:
		void CreateGraphicsPipeline(VkDevice device, const std::string &vertFilepath, const std::string &fragFilepath, const giPipelineConfigInfo& createInfo);
		void CreateShaderModule(VkDevice device, const std::vector<char> code, VkShaderModule *shaderModule);

		static std::vector<char> ReadFile(std::string filePath);

		VkPipeline m_VkPipeline;
		VkShaderModule m_VertShaderModule;
		VkShaderModule m_FragShaderModule;
	};

}


#endif
