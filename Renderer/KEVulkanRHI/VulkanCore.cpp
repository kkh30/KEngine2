#include "VulkanCore.h"


namespace VulkanCore {
	CDevice* device = nullptr;
	VkDevice vk_device = VK_NULL_HANDLE;
	VmaAllocator allocator = nullptr;
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	CInstance* instance = nullptr;
	CPhysicalDevices* gpus = nullptr;
	CPhysicalDevice *gpu = nullptr;
	CQueue* graphics_queue = nullptr;
	VkCommandPool cmd_pool = VK_NULL_HANDLE;





	VkDebugReportFlagsEXT debug_repost_flags = 
		VK_DEBUG_REPORT_WARNING_BIT_EXT |
		VK_DEBUG_REPORT_ERROR_BIT_EXT |
		VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
	VkFormat FindSupportedFormat(
		const std::vector<VkFormat>& candidates,
		VkImageTiling tiling,
		VkFormatFeatureFlags features)
	{
		for (VkFormat format : candidates)
		{
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(*gpu, format, &props);

			if ((tiling == VK_IMAGE_TILING_LINEAR) &&
				((props.linearTilingFeatures & features) == features))
			{
				return format;
			}
			else if ((tiling == VK_IMAGE_TILING_OPTIMAL) &&
				((props.optimalTilingFeatures & features) == features))
			{
				return format;
			}
		}
		return VK_FORMAT_UNDEFINED;
	}

	VkFormat FindDepthFormat()
	{
		std::vector<VkFormat> formats;
		formats.push_back(VK_FORMAT_D32_SFLOAT);
		formats.push_back(VK_FORMAT_D32_SFLOAT_S8_UINT);
		formats.push_back(VK_FORMAT_D24_UNORM_S8_UINT);

		return FindSupportedFormat(
			formats,
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	}
};//£¡namespace VulkanCore