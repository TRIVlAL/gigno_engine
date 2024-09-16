#include "pipeline.h"
#include "model.h"

#include <fstream>
#include "../error_macros.h"
#include "iostream"

namespace gigno {

	giPipeline::giPipeline(VkDevice device, const std::string& vertFilepath, const std::string& fragFilepath, const giPipelineConfigInfo& createInfo) {
		std::vector<char> vertCode = ReadFile(vertFilepath);
		std::vector<char> fragCode = ReadFile(fragFilepath);

		CreateShaderModule(device, vertCode, &m_VertShaderModule);
		CreateShaderModule(device, fragCode, &m_FragShaderModule);

		CreateGraphicsPipeline(device, vertFilepath, fragFilepath, createInfo);
	}

	giPipeline::~giPipeline() {
		
	}

	void giPipeline::CleanUp(VkDevice device) {
		vkDestroyShaderModule(device, m_VertShaderModule, nullptr);
		vkDestroyShaderModule(device, m_FragShaderModule, nullptr);
		vkDestroyPipeline(device, m_VkPipeline, nullptr);
	}

	void giPipeline::CreateGraphicsPipeline(VkDevice device, const std::string &vertFilepath, const std::string &fragFilepath, const giPipelineConfigInfo& createInfo) {
		ASSERT((createInfo.pipelineLayout != VK_NULL_HANDLE));
		ASSERT((createInfo.renderPass != VK_NULL_HANDLE));

		VkPipelineShaderStageCreateInfo shaderStageCreateInfos[2];
		shaderStageCreateInfos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStageCreateInfos[0].pNext = nullptr;
		shaderStageCreateInfos[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		shaderStageCreateInfos[0].module = m_VertShaderModule;
		shaderStageCreateInfos[0].pName = "main";
		shaderStageCreateInfos[0].pSpecializationInfo = nullptr;

		shaderStageCreateInfos[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStageCreateInfos[1].pNext = nullptr;
		shaderStageCreateInfos[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shaderStageCreateInfos[1].module = m_FragShaderModule;
		shaderStageCreateInfos[1].pName = "main";
		shaderStageCreateInfos[1].pSpecializationInfo = nullptr;

		VkVertexInputBindingDescription bindingDescription = Vertex::GetBindingDescription();
		std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions = Vertex::GetAttributeDescriptions();

		VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo;
		vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputCreateInfo.pNext = nullptr;
		vertexInputCreateInfo.vertexBindingDescriptionCount = 1;
		vertexInputCreateInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputCreateInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo{};
		graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		graphicsPipelineCreateInfo.pNext = nullptr;
		graphicsPipelineCreateInfo.stageCount = 2;
		graphicsPipelineCreateInfo.pStages = shaderStageCreateInfos;
		graphicsPipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;
		graphicsPipelineCreateInfo.pInputAssemblyState = &createInfo.inputAssemblyInfo;
		graphicsPipelineCreateInfo.pViewportState = &createInfo.viewportState;
		graphicsPipelineCreateInfo.pRasterizationState = &createInfo.rasterizationInfo;
		graphicsPipelineCreateInfo.pMultisampleState = &createInfo.multisampleInfo;
		graphicsPipelineCreateInfo.pDepthStencilState = &createInfo.depthStencilInfo;
		graphicsPipelineCreateInfo.pColorBlendState = &createInfo.colorBlendInfo;
		graphicsPipelineCreateInfo.pDynamicState = &createInfo.dynamicStatesCreateInfo;

		graphicsPipelineCreateInfo.layout = createInfo.pipelineLayout;
		graphicsPipelineCreateInfo.renderPass = createInfo.renderPass;
		graphicsPipelineCreateInfo.subpass = createInfo.subpass;

		graphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
		graphicsPipelineCreateInfo.basePipelineIndex = -1;

		VkResult result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, &m_VkPipeline);

		if (result != VK_SUCCESS) {
			ERR_MSG("Failed to create Graphics Pipeline ! Vulkand error code : " << (int)result);
		}
	}

	void giPipeline::CreateShaderModule(VkDevice device, std::vector<char> code, VkShaderModule *shaderModule) {
		VkShaderModuleCreateInfo createInfo;
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

		VkResult result = vkCreateShaderModule(device, &createInfo, nullptr, shaderModule);
		if (result != VK_SUCCESS) {
			ERR_MSG("Failed to create ShaderModule ! Vulkan error code : " << (int)result <<"." << std::endl << 
			"If the error code is -13, make sure the shader files are present in the running directory (from which the executable is ran) in the folder 'shader/'.");
		}
	}

	std::vector<char> giPipeline::ReadFile(std::string path) {
		std::ifstream infile{ path, std::ios::ate | std::ios::binary };

		if (!infile.is_open()) {
			ERR_MSG_V("failed to open file:" + path, std::vector<char>{});
		}

		size_t filesize = static_cast<size_t>(infile.tellg());
		std::vector<char> buffer (filesize);
		infile.seekg(0);
		infile.read(buffer.data(), filesize);

		infile.close();
		return buffer;
	}

	void giPipeline::DefaultConfig(giPipelineConfigInfo &configInfo) {
		configInfo.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		configInfo.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		configInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

		configInfo.viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		configInfo.viewportState.pNext = nullptr;
		configInfo.viewportState.viewportCount = 1;
		configInfo.viewportState.pViewports = nullptr;
		configInfo.viewportState.scissorCount = 1;
		configInfo.viewportState.pScissors = nullptr;

		configInfo.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		configInfo.rasterizationInfo.depthClampEnable = VK_FALSE;
		configInfo.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
		configInfo.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
		configInfo.rasterizationInfo.lineWidth = 1.0f;
		configInfo.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
		configInfo.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
		configInfo.rasterizationInfo.depthBiasEnable = VK_FALSE;
		configInfo.rasterizationInfo.depthBiasConstantFactor = 0.0f;
		configInfo.rasterizationInfo.depthBiasClamp = 0.0f;
		configInfo.rasterizationInfo.depthBiasSlopeFactor = 0.0f;

		configInfo.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		configInfo.multisampleInfo.sampleShadingEnable = VK_FALSE;
		configInfo.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		configInfo.multisampleInfo.minSampleShading = 1.0f;
		configInfo.multisampleInfo.pSampleMask = nullptr;
		configInfo.multisampleInfo.alphaToCoverageEnable = VK_FALSE;
		configInfo.multisampleInfo.alphaToOneEnable = VK_FALSE;

		configInfo.colorBlendAttachment.colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
			VK_COLOR_COMPONENT_A_BIT;
		configInfo.colorBlendAttachment.blendEnable = VK_FALSE;
		configInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		configInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		configInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		configInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		configInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		configInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		configInfo.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		configInfo.colorBlendInfo.logicOpEnable = VK_FALSE;
		configInfo.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;
		configInfo.colorBlendInfo.attachmentCount = 1;
		configInfo.colorBlendInfo.pAttachments = &configInfo.colorBlendAttachment;
		configInfo.colorBlendInfo.blendConstants[0] = 0.0f;
		configInfo.colorBlendInfo.blendConstants[1] = 0.0f;
		configInfo.colorBlendInfo.blendConstants[2] = 0.0f;
		configInfo.colorBlendInfo.blendConstants[3] = 0.0f;

		configInfo.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		configInfo.depthStencilInfo.depthTestEnable = VK_TRUE;
		configInfo.depthStencilInfo.depthWriteEnable = VK_TRUE;
		configInfo.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
		configInfo.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
		configInfo.depthStencilInfo.minDepthBounds = 0.0f;
		configInfo.depthStencilInfo.maxDepthBounds = 1.0f;
		configInfo.depthStencilInfo.stencilTestEnable = VK_FALSE;
		configInfo.depthStencilInfo.front = {};
		configInfo.depthStencilInfo.back = {};

		configInfo.dynamicStatesEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

		configInfo.dynamicStatesCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		configInfo.dynamicStatesCreateInfo.flags = 0;
		configInfo.dynamicStatesCreateInfo.pNext = nullptr;
		configInfo.dynamicStatesCreateInfo.dynamicStateCount = static_cast<uint32_t>(configInfo.dynamicStatesEnables.size());
		configInfo.dynamicStatesCreateInfo.pDynamicStates = configInfo.dynamicStatesEnables.data();
	}
}