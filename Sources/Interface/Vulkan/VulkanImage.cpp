#include<stdexcept>

#include"../../Include/Vulkan/VulkanImage.h"
#include"../../Include/Vulkan/VulkanBuffer.h"
#include"../../Include/Vulkan/VulkanCommand.h"

#define STB_IMAGE_IMPLEMENTATION
#include<stb_image.h>

namespace DRHI
{
	namespace VulkanImage
	{
        void createTextureImage(VkImage* textureImage, VkDeviceMemory* textureMemory, int texWidth, int texHeight, int texChannels, stbi_uc* pixels, VkDevice* device, VkPhysicalDevice* physicalDevice, VkQueue* graphicsQueue, VkCommandPool* commandPool)
        {
            VkDeviceSize imageSize = texWidth * texHeight * 4;
            VkBuffer stagingBuffer;
            VkDeviceMemory stagingBufferMemory;
            VulkanBuffer::createBuffer(device, physicalDevice, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

            void* data;
            vkMapMemory(*device, stagingBufferMemory, 0, imageSize, 0, &data);
            memcpy(data, pixels, static_cast<size_t>(imageSize));
            vkUnmapMemory(*device, stagingBufferMemory);

            stbi_image_free(pixels);

            VulkanImage::createImage(textureImage,
                texWidth, texHeight,
                VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_1_BIT,
                VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, *textureMemory, device, physicalDevice);

            VulkanImage::transitionImageLayout(*textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, graphicsQueue, commandPool, device);
            VulkanImage::copyBufferToImage(&stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), graphicsQueue, commandPool, device);
            VulkanImage::transitionImageLayout(*textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, graphicsQueue, commandPool, device);

            vkDestroyBuffer(*device, stagingBuffer, nullptr);
            vkFreeMemory(*device, stagingBufferMemory, nullptr);
        }

        void createTextureImage(VkImage* textureImage, DynamicImageCreateInfo info, VkDeviceMemory* textureMemory, stbi_uc* pixels, VkDevice* device, VkPhysicalDevice* physicalDevice, VkQueue* graphicsQueue, VkCommandPool* commandPool)
        {
            VkDeviceSize imageSize = (float)info.extent.width * (float)info.extent.height * 4;
            VkBuffer stagingBuffer;
            VkDeviceMemory stagingBufferMemory;
            VkDeviceSize bufferSize = imageSize * info.arrayLayers;
            VulkanBuffer::createBuffer(device, physicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

            void* data;
            vkMapMemory(*device, stagingBufferMemory, 0, imageSize, 0, &data);
            memcpy(data, pixels, static_cast<size_t>(imageSize));
            vkUnmapMemory(*device, stagingBufferMemory);

            stbi_image_free(pixels);

            info.format = VK_FORMAT_R8G8B8A8_SRGB;
            info.tiling = VK_IMAGE_TILING_OPTIMAL;
            info.samples = VK_SAMPLE_COUNT_1_BIT;
            info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            VulkanImage::createImage(textureImage, info, *textureMemory, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, device, physicalDevice);

            VkImageSubresourceRange subr{};
            subr.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            subr.baseArrayLayer = 0;
            subr.baseMipLevel = 0;
            subr.layerCount = info.mipLevels;
            subr.levelCount = info.mipLevels;

            VkCommandBuffer commandBuffer = VulkanCommand::beginSingleTimeCommands(commandPool, device);

            VulkanImage::setImageLayout(commandBuffer, *textureImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subr, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
            VulkanImage::copyBufferToImage(&stagingBuffer, textureImage, static_cast<uint32_t>(info.extent.width), static_cast<uint32_t>(info.extent.height), subr, graphicsQueue, commandPool, device);
            VulkanImage::setImageLayout(commandBuffer, *textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, subr, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

            vkDestroyBuffer(*device, stagingBuffer, nullptr);
            vkFreeMemory(*device, stagingBufferMemory, nullptr);
        }

        void createImage(
            VkImage* image,
            uint32_t width, uint32_t height, 
            VkFormat format, VkImageTiling tiling, VkSampleCountFlagBits samples, 
            VkImageUsageFlags usage, VkMemoryPropertyFlags properties, 
            VkDeviceMemory& imageMemory, 
            VkDevice* device, VkPhysicalDevice* physicalDevice) 
        {
            VkImageCreateInfo imageInfo{};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.extent.width = width;
            imageInfo.extent.height = height;
            imageInfo.extent.depth = 1;
            imageInfo.mipLevels = 1;
            imageInfo.arrayLayers = 1;
            imageInfo.format = format;
            imageInfo.tiling = tiling;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageInfo.usage = usage;
            imageInfo.samples = samples;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            if (vkCreateImage(*device, &imageInfo, nullptr, image) != VK_SUCCESS) {
                throw std::runtime_error("failed to create image!");
            }

            VkMemoryRequirements memRequirements;
            vkGetImageMemoryRequirements(*device, *image, &memRequirements);

            VkMemoryAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = memRequirements.size;
            allocInfo.memoryTypeIndex = VulkanBuffer::findMemoryType(memRequirements.memoryTypeBits, properties, physicalDevice);

            if (vkAllocateMemory(*device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) 
            {
                throw std::runtime_error("failed to allocate image memory!");
            }

            vkBindImageMemory(*device, *image, imageMemory, 0);
        }

        void createImage(VkImage* image, DynamicImageCreateInfo info, VkDeviceMemory& imageMemory, VkMemoryPropertyFlags properties, VkDevice* device, VkPhysicalDevice* physicalDevice)
        {
            VkImageCreateInfo imageInfo{};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.extent.width = info.extent.width;
            imageInfo.extent.height = info.extent.height;
            imageInfo.extent.depth = info.extent.depth;
            imageInfo.mipLevels = info.mipLevels;
            imageInfo.arrayLayers = info.arrayLayers;
            imageInfo.format = (VkFormat)info.format;
            imageInfo.tiling = (VkImageTiling)info.tiling;
            imageInfo.initialLayout = (VkImageLayout)info.initialLayout;
            imageInfo.usage = (VkImageUsageFlags)info.usage;
            imageInfo.samples = (VkSampleCountFlagBits)info.samples;
            imageInfo.sharingMode = (VkSharingMode)info.sharingMode;
            imageInfo.flags = (VkImageCreateFlagBits)info.flags;

            if (vkCreateImage(*device, &imageInfo, nullptr, image) != VK_SUCCESS) {
                throw std::runtime_error("failed to create image!");
            }

            VkMemoryRequirements memRequirements;
            vkGetImageMemoryRequirements(*device, *image, &memRequirements);

            VkMemoryAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = memRequirements.size;
            allocInfo.memoryTypeIndex = VulkanBuffer::findMemoryType(memRequirements.memoryTypeBits, properties, physicalDevice);

            if (vkAllocateMemory(*device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to allocate image memory!");
            }

            vkBindImageMemory(*device, *image, imageMemory, 0);
        }

        void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VkQueue* graphicsQueue, VkCommandPool* commandPool, VkDevice* device) 
        {
            VkCommandBuffer commandBuffer = VulkanCommand::beginSingleTimeCommands(commandPool, device);

            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.oldLayout = oldLayout;
            barrier.newLayout = newLayout;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = image;
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;

            VkPipelineStageFlags sourceStage;
            VkPipelineStageFlags destinationStage;

           if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) 
           {
                barrier.srcAccessMask = 0;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

                sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
           }
           else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) 
           {
               barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
               barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
               sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
               destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
           }
           else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL) 
           {
                barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

                sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
           }
           else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) 
           {
                barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

                sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
           }
           else if (oldLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL) 
           {
               barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
               barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
               barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

               sourceStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
               destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
           }
           else 
           {
               throw std::invalid_argument("unsupported layout transition!");
           }

            vkCmdPipelineBarrier(
                commandBuffer,
                sourceStage, destinationStage,
                0,
                0, nullptr,
                0, nullptr,
                1, &barrier
            );

            VulkanCommand::endSingleTimeCommands(commandBuffer, graphicsQueue, commandPool, device);
        }

        void copyBufferToImage(VkBuffer* buffer, VkImage* image, uint32_t width, uint32_t height, VkQueue* graphicsQueue, VkCommandPool* commandPool, VkDevice* device)
        {
            VkCommandBuffer commandBuffer = VulkanCommand::beginSingleTimeCommands(commandPool, device);

            VkBufferImageCopy region{};
            region.bufferOffset = 0;
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;
            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.mipLevel = 0;
            region.imageSubresource.baseArrayLayer = 0;
            region.imageSubresource.layerCount = 1;
            region.imageOffset = { 0, 0, 0 };
            region.imageExtent = {
                width,
                height,
                1
            };

            vkCmdCopyBufferToImage(commandBuffer, *buffer, *image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

            VulkanCommand::endSingleTimeCommands(commandBuffer, graphicsQueue, commandPool, device);
        }

        void copyBufferToImage(VkBuffer* buffer, VkImage* image, uint32_t width, uint32_t height, VkImageSubresourceRange range,VkQueue* graphicsQueue, VkCommandPool* commandPool, VkDevice* device)
        {
            VkCommandBuffer commandBuffer = VulkanCommand::beginSingleTimeCommands(commandPool, device);

            std::vector<VkBufferImageCopy> region{};
            for (uint32_t i = 0; i < range.layerCount; ++i)
            {
                VkBufferImageCopy reg{};
                reg.bufferOffset = 0;
                reg.bufferRowLength = 0;
                reg.bufferImageHeight = 0;
                reg.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                reg.imageSubresource.mipLevel = i;
                reg.imageSubresource.baseArrayLayer = 0;
                reg.imageSubresource.layerCount = 1;
                reg.imageOffset = { 0, 0, 0 };
                reg.imageExtent = {
                    width,
                    height,
                    1
                };

                region.push_back(reg);
            }

            vkCmdCopyBufferToImage(commandBuffer, *buffer, *image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, static_cast<uint32_t>(region.size()), region.data());

            VulkanCommand::endSingleTimeCommands(commandBuffer, graphicsQueue, commandPool, device);
        }

        VkImageView createImageView(VkDevice* device, VkImage* image, VkFormat format, VkImageAspectFlags aspectFlags)
        {
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = *image;
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = format;
            viewInfo.subresourceRange.aspectMask = aspectFlags;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            VkImageView imageView;
            if (vkCreateImageView(*device, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create image view!");
            }

            return imageView;
        }

        VkImageView createImageView(VkDevice* device, VkImage* image, DynamicImageViewCreateInfo info)
        {
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = *image;
            viewInfo.viewType = (VkImageViewType)info.type;
            viewInfo.format = (VkFormat)info.format;
            viewInfo.subresourceRange.aspectMask = (VkImageAspectFlagBits)info.subresourceRange.aspectMask;
            viewInfo.subresourceRange.baseMipLevel = info.subresourceRange.baseMipLevel;
            viewInfo.subresourceRange.levelCount = info.subresourceRange.levelCount;
            viewInfo.subresourceRange.baseArrayLayer = info.subresourceRange.baseArrayLayer;
            viewInfo.subresourceRange.layerCount = info.subresourceRange.layerCount;

            VkImageView imageView;
            if (vkCreateImageView(*device, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create image view!");
            }

            return imageView;
        }

        void createTextureSampler(VkSampler* textureSampler, VkPhysicalDevice* physicalDevice, VkDevice* device)
        {
            VkPhysicalDeviceProperties properties{};
            vkGetPhysicalDeviceProperties(*physicalDevice, &properties);

            VkSamplerCreateInfo samplerInfo{};
            samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            samplerInfo.magFilter = VK_FILTER_LINEAR;
            samplerInfo.minFilter = VK_FILTER_LINEAR;
            samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            samplerInfo.anisotropyEnable = VK_TRUE;
            samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
            samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
            samplerInfo.unnormalizedCoordinates = VK_FALSE;
            samplerInfo.compareEnable = VK_FALSE;
            samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
            samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

            if (vkCreateSampler(*device, &samplerInfo, nullptr, textureSampler) != VK_SUCCESS) 
            {
                throw std::runtime_error("failed to create texture sampler!");
            }
        }

        void createSampler(VkSampler* sampler, DynamicSamplerCreateInfo createInfo, VkPhysicalDevice* physicalDevice, VkDevice* device)
        {
            VkPhysicalDeviceProperties properties{};
            vkGetPhysicalDeviceProperties(*physicalDevice, &properties);

            VkSamplerCreateInfo samplerInfo{};
            samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            samplerInfo.magFilter = VK_FILTER_LINEAR;
            samplerInfo.minFilter = VK_FILTER_LINEAR;
            samplerInfo.addressModeU = (VkSamplerAddressMode)(createInfo.sampleraAddressMode);
            samplerInfo.addressModeV = (VkSamplerAddressMode)(createInfo.sampleraAddressMode);
            samplerInfo.addressModeW = (VkSamplerAddressMode)(createInfo.sampleraAddressMode);
            samplerInfo.anisotropyEnable = VK_TRUE;
            samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
            samplerInfo.borderColor = (VkBorderColor)(createInfo.borderColor);
            samplerInfo.unnormalizedCoordinates = VK_FALSE;
            samplerInfo.compareEnable = VK_FALSE;
            samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
            samplerInfo.mipmapMode = (VkSamplerMipmapMode)createInfo.mipmapMode;
            samplerInfo.minLod = createInfo.minLod;
            samplerInfo.maxLod = createInfo.maxLod;

            if (vkCreateSampler(*device, &samplerInfo, nullptr, sampler) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create texture sampler!");
            }
        }

        // Create an image memory barrier for changing the layout of
        void setImageLayout(
            VkCommandBuffer cmdbuffer,
            VkImage& image,
            VkImageLayout oldImageLayout,
            VkImageLayout newImageLayout,
            VkImageSubresourceRange subresourceRange,
            VkPipelineStageFlags srcStageMask,
            VkPipelineStageFlags dstStageMask)
        {
            // Create an image barrier object
            VkImageMemoryBarrier imageMemoryBarrier{};
            imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            imageMemoryBarrier.oldLayout = oldImageLayout;
            imageMemoryBarrier.newLayout = newImageLayout;
            imageMemoryBarrier.image = image;
            imageMemoryBarrier.subresourceRange = subresourceRange;

            // Source layouts (old)
            // Source access mask controls actions that have to be finished on the old layout
            // before it will be transitioned to the new layout
            switch (oldImageLayout)
            {
            case VK_IMAGE_LAYOUT_UNDEFINED:
                // Image layout is undefined (or does not matter)
                // Only valid as initial layout
                // No flags required, listed only for completeness
                imageMemoryBarrier.srcAccessMask = 0;
                break;

            case VK_IMAGE_LAYOUT_PREINITIALIZED:
                // Image is preinitialized
                // Only valid as initial layout for linear images, preserves memory contents
                // Make sure host writes have been finished
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                // Image is a color attachment
                // Make sure any writes to the color buffer have been finished
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                // Image is a depth/stencil attachment
                // Make sure any writes to the depth/stencil buffer have been finished
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                // Image is a transfer source
                // Make sure any reads from the image have been finished
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                break;

            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                // Image is a transfer destination
                // Make sure any writes to the image have been finished
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                // Image is read by a shader
                // Make sure any shader reads from the image have been finished
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                break;
            default:
                // Other source layouts aren't handled (yet)
                break;
            }

            // Target layouts (new)
            // Destination access mask controls the dependency for the new image layout
            switch (newImageLayout)
            {
            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                // Image will be used as a transfer destination
                // Make sure any writes to the image have been finished
                imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                // Image will be used as a transfer source
                // Make sure any reads from the image have been finished
                imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                break;

            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                // Image will be used as a color attachment
                // Make sure any writes to the color buffer have been finished
                imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                // Image layout will be used as a depth/stencil attachment
                // Make sure any writes to depth/stencil buffer have been finished
                imageMemoryBarrier.dstAccessMask = imageMemoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                // Image will be read in a shader (sampler, input attachment)
                // Make sure any writes to the image have been finished
                if (imageMemoryBarrier.srcAccessMask == 0)
                {
                    imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
                }
                imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                break;
            default:
                // Other source layouts aren't handled (yet)
                break;
            }

            // Put barrier inside setup command buffer
            vkCmdPipelineBarrier(
                cmdbuffer,
                srcStageMask,
                dstStageMask,
                0,
                0, nullptr,
                0, nullptr,
                1, &imageMemoryBarrier);
        }
	}
}