#pragma once

#include<vector>

#include"../InterfaceType.h"

namespace DRHI
{
	namespace VulkanPipeline
	{
		struct VulkanPipelineCreateInfo
		{
			VkShaderModule vertexShader;
			VkShaderModule fragmentShader;
			uint32_t shaderCount;
			VkFormat colorImageFormat;
			VkFormat depthImageFormat;
			bool includeStencil;
			bool dynamicDepthBias;
			uint32_t cullMode;
			uint32_t sampleCounts;
			VkRenderPass renderPass;
		};

		void createPipelineLayout(DynamicPipelineLayout* pipelineLayout ,DynamicPipelineLayoutCreateInfo* createInfo, VkDevice* device);

		void createGraphicsPipelineKHR(VkPipeline* graphicsPipeline, VkPipelineLayout* pipelineLayout, VkPipelineCache* pipelineCache, VulkanPipelineCreateInfo createInfo, VkDevice* device, VkVertexInputBindingDescription bindingDescription, std::vector<VkVertexInputAttributeDescription> attributeDescriptions);

		void createGraphicsPipeline(VkPipeline* graphicsPipeline, VkPipelineLayout* pipelineLayout, VkPipelineCache* pipelineCache,
			VulkanPipelineCreateInfo createInfo, VkDevice* device,
			VkVertexInputBindingDescription bindingDescription, std::vector<VkVertexInputAttributeDescription> attributeDescriptions);

		void createPipelineCache(VkPipelineCache* pipelineCache, VkDevice* device);

		VkPipelineRenderingCreateInfoKHR getPipelineRenderingCreateInfo(VkFormat* swapChainImageFormat);
	}
}