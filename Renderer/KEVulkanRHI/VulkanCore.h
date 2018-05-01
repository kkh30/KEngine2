#pragma once
#define VMA_USE_STL_CONTAINERS
#include "vk_mem_alloc.h"
#include <vulkan\vulkan.h>
#include "KEWindow.h"
#include "VulkanSwapChain.h"
#include "WSIWindow\CDevices.h"

namespace VulkanCore {
	extern CDevice* device;
	extern VkDevice vk_device;
	extern VmaAllocator allocator;
	extern VkSurfaceKHR surface;
	extern CInstance* instance;
	extern CPhysicalDevices* gpus;
	extern CPhysicalDevice *gpu;
	extern CQueue* graphics_queue;
	extern VkCommandPool cmd_pool;
	extern VkCommandBuffer temp_command_buffer;
	extern VkDescriptorPool desc_pool;
	extern void BeginCommandBuffer(VkCommandBuffer p_cmd_buffer);
	extern void FlushCommandBuffer(VkCommandBuffer p_cmd_buffer);
	extern VkDebugReportFlagsEXT debug_repost_flags;
	VkFormat FindSupportedFormat(
		const std::vector<VkFormat>& candidates,
		VkImageTiling tiling,
		VkFormatFeatureFlags features);
	VkFormat FindDepthFormat();
	VkShaderModule loadSPIRVShader(std::string filename);
	void DestroyShaderModule(VkShaderModule);

};//£¡namespace VulkanCore