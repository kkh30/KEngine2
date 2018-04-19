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
	VkCommandBuffer temp_command_buffer = VK_NULL_HANDLE;



	void BeginCommandBuffer(VkCommandBuffer p_cmd_buffer) {
		VkCommandBufferBeginInfo l_begin_info = {};
		l_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		vkBeginCommandBuffer(p_cmd_buffer, &l_begin_info);
	};
	void FlushCommandBuffer(VkCommandBuffer p_cmd_buffer) {
		vkEndCommandBuffer(p_cmd_buffer);
		VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &p_cmd_buffer;
		vkQueueSubmit(*graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(*graphics_queue);
		vkResetCommandBuffer(p_cmd_buffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
	};

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