#pragma once
#include "VulkanResource.h"
#include "KEWindow.h"

//RenderTargets is used in frequently update resource such as framebuffers or depth stencil buffer.
class KEVulkanRenderTarget :public KEVulkanResource
{
public:
	KEVulkanRenderTarget() {
	
	}

	KEVulkanRenderTarget
	(	
		uint32_t p_width,
		uint32_t p_height,
		VkFormat p_format,
		VmaMemoryUsage p_usage
	) :
		KEVulkanResource(),
		m_width(p_width),
		m_height(p_height),
		m_format(p_format),
		m_usage(p_usage)
	{
		VkImageCreateInfo imageInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = m_width;
		imageInfo.extent.height = m_height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = m_format;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.flags = 0;

		VmaAllocationCreateInfo depthImageAllocCreateInfo = {};
		depthImageAllocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		vmaCreateImage(VulkanCore::allocator, &imageInfo, &depthImageAllocCreateInfo, &m_image, &m_allocation, nullptr);

		VkImageViewCreateInfo depthImageViewInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		depthImageViewInfo.image = m_image;
		depthImageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		depthImageViewInfo.format = m_format;
		depthImageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		depthImageViewInfo.subresourceRange.baseMipLevel = 0;
		depthImageViewInfo.subresourceRange.levelCount = 1;
		depthImageViewInfo.subresourceRange.baseArrayLayer = 0;
		depthImageViewInfo.subresourceRange.layerCount = 1;

		vkCreateImageView(VulkanCore::vk_device, &depthImageViewInfo, nullptr, &m_image_view);
	};
	~KEVulkanRenderTarget() {
		vmaDestroyImage(VulkanCore::allocator, m_image, m_allocation);
		vkDestroyImageView(VulkanCore::vk_device, m_image_view, nullptr);
		m_allocation = nullptr;
		m_image = VK_NULL_HANDLE;
		m_image_view = VK_NULL_HANDLE;
	};
	__forceinline VkImageView GetImageView() const { return m_image_view; }

private:
	uint32_t m_width = 0 ;
	uint32_t m_height = 0 ;
	VkFormat m_format = VkFormat::VK_FORMAT_UNDEFINED;
	VmaMemoryUsage m_usage = VmaMemoryUsage::VMA_MEMORY_USAGE_UNKNOWN;
	VkImage m_image = VK_NULL_HANDLE;
	VkImageView m_image_view = VK_NULL_HANDLE;
	VmaAllocation m_allocation = nullptr;
};

