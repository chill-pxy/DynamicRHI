#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include<volk.h>

#define NOMINMAX
#include<Windows.h>

#include<variant>

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

const int MAX_FRAMES_IN_FLIGHT = 3;

namespace DRHI
{
	typedef enum API
	{
		VULKAN,
		DIRECT3D12
	}API;

	typedef struct DynamicFormat
	{
		DynamicFormat(API api)
		{
			switch (api)
			{
			case DRHI::VULKAN:
				FORMAT_R32G32B32_SFLOAT = VK_FORMAT_R32G32B32_SFLOAT;
				FORMAT_R32G32_SFLOAT = VK_FORMAT_R32G32_SFLOAT;
				break;
			case DRHI::DIRECT3D12:
				break;
			}
		}

		uint32_t FORMAT_R32G32B32_SFLOAT{0};
		uint32_t FORMAT_R32G32_SFLOAT{0};

	}DynamicFormat;

	typedef struct DynamicPipelineBindPoint
	{
		DynamicPipelineBindPoint(API api)
		{
			switch (api)
			{
			case DRHI::VULKAN:
				PIPELINE_BIND_POINT_GRAPHICS = VK_PIPELINE_BIND_POINT_GRAPHICS;
				break;
			case DRHI::DIRECT3D12:
				break;
			}
		}

		uint32_t PIPELINE_BIND_POINT_GRAPHICS{ 0 };

	}DynamicPipelineBindPoint;

	typedef struct DynamicExtent2D
	{
		uint32_t width;
		uint32_t height;
	}DynamicExtent2D;

	typedef struct DynamicOffset2D
	{
		int32_t x;
		int32_t y;
	}DynamicOffset2D;

	typedef struct DynamicRect2D
	{
		DynamicExtent2D extent;
		DynamicOffset2D offset;
	}DynamicRect2D;

	class DynamicBuffer
	{
	public:
		std::variant<VkBuffer> internalID;

		inline VkBuffer getVulkanBuffer() { return std::get<VkBuffer>(internalID); }
	};

	class DynamicDeviceMemory
	{
	public:
		std::variant<VkDeviceMemory> internalID;

		inline VkDeviceMemory getVulkanDeviceMemory() { return std::get<VkDeviceMemory>(internalID); }
	};

	class DynamicImage
	{
	public:
		std::variant<VkImage> internalID;

		inline VkImage getVulkanImage() { return std::get<VkImage>(internalID); }
	};

	class DynamicImageView
	{
	public:
		std::variant<VkImageView> internalID;

		inline VkImageView getVulkanImageView() { return std::get<VkImageView>(internalID); }
	};

	class DynamicSampler
	{
	public:
		std::variant<VkSampler> internalID;

		inline VkSampler getVulkanSampler() { return std::get<VkSampler>(internalID); }
	};

	class DynamicPipeline
	{
	public:
		std::variant<VkPipeline> internalID;

		inline VkPipeline getVulkanPipeline() { return std::get<VkPipeline>(internalID); }
	};

	class DynamicPipelineLayout
	{
	public:
		std::variant<VkPipelineLayout> internalID;

		inline VkPipelineLayout getVulkanPipelineLayout() { return std::get<VkPipelineLayout>(internalID); }
	};

	class DynamicDescriptorSet
	{
	public:
		std::variant<VkDescriptorSet> internalID;

		inline VkDescriptorSet getVulkanDescriptorSet() { return std::get<VkDescriptorSet>(internalID); }
	};

	class DynamicDescriptorSetLayout
	{
	public:
		std::variant<VkDescriptorSetLayout> internalID;

		inline VkDescriptorSetLayout getVulkanDescriptorSetLayout() { return std::get<VkDescriptorSetLayout>(internalID); }
	};

	class DynamicDescriptorBufferInfo
	{
	public:
		std::variant<VkDescriptorBufferInfo> internalID;

		void set(API api, DynamicBuffer buffer, uint64_t bufferSize, uint32_t offset = 0)
		{
			switch (api)
			{
			case DRHI::VULKAN:
			{
				VkDescriptorBufferInfo vkinfo{};
				vkinfo.buffer = buffer.getVulkanBuffer();
				vkinfo.offset = offset;
				vkinfo.range = bufferSize;

				internalID = vkinfo;
			}break;
			case DRHI::DIRECT3D12:
				break;
			}
			
		}
		inline VkDescriptorBufferInfo getVulkanDescriptorBufferInfo() { return std::get<VkDescriptorBufferInfo>(internalID); }
	};

	class DynamicVertexInputBindingDescription
	{
	public:
		std::variant<VkVertexInputBindingDescription> internalID;

		void set(API api, uint32_t binding, uint32_t stride)
		{
			switch (api)
			{
			case DRHI::VULKAN:
			{
				VkVertexInputBindingDescription vkinfo{};
				vkinfo.binding = binding;
				vkinfo.stride = stride;
				vkinfo.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

				internalID = vkinfo;
					
			}break;
			case DRHI::DIRECT3D12:
				break;
			}
		}

		inline VkVertexInputBindingDescription getVulkanVertexInputBindingDescription(){ return std::get<VkVertexInputBindingDescription>(internalID); }
	};

	class DynamicVertexInputAttributeDescription
	{
	public:
		std::variant<VkVertexInputAttributeDescription> internalID;
		
		void set(API api, uint32_t location, uint32_t binding, uint32_t format, uint32_t offset)
		{
			switch (api)
			{
			case DRHI::VULKAN:
			{
				VkVertexInputAttributeDescription vkinfo{};
				vkinfo.binding = binding;
				vkinfo.location = location;
				vkinfo.offset = offset;
				vkinfo.format = (VkFormat)format;

				internalID = vkinfo;
			}break;
			case DRHI::DIRECT3D12:
				break;
			}
		}

		inline VkVertexInputAttributeDescription getVulkanVertexInputAttributeDescription() { return std::get<VkVertexInputAttributeDescription>(internalID); }
	};


}




