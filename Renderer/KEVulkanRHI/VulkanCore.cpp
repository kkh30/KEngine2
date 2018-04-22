#include "VulkanCore.h"
#include <fstream>
#include <stdio.h>
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

	void DestroyShaderModule(VkShaderModule p_module){
		vkDestroyShaderModule(vk_device, p_module, nullptr);
	};


	VkShaderModule loadSPIRVShader(std::string filename)
	{
		size_t shaderSize;
		char* shaderCode;

#if defined(__ANDROID__)
		// Load shader from compressed asset
		AAsset* asset = AAssetManager_open(androidApp->activity->assetManager, filename.c_str(), AASSET_MODE_STREAMING);
		assert(asset);
		shaderSize = AAsset_getLength(asset);
		assert(shaderSize > 0);

		shaderCode = new char[shaderSize];
		AAsset_read(asset, shaderCode, shaderSize);
		AAsset_close(asset);
#else
		std::ifstream is(filename, std::ios::binary | std::ios::in | std::ios::ate);

		if (is.is_open())
		{
			shaderSize = is.tellg();
			is.seekg(0, std::ios::beg);
			// Copy file contents into a buffer
			shaderCode = new char[shaderSize];
			is.read(shaderCode, shaderSize);
			is.close();
			assert(shaderSize > 0);
		}
#endif
		if (shaderCode)
		{
			// Create a new shader module that will be used for pipeline creation
			VkShaderModuleCreateInfo moduleCreateInfo{};
			moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			moduleCreateInfo.codeSize = shaderSize;
			moduleCreateInfo.pCode = (uint32_t*)shaderCode;

			VkShaderModule shaderModule;
			vkCreateShaderModule(vk_device, &moduleCreateInfo, NULL, &shaderModule);

			delete[] shaderCode;

			return shaderModule;
		}
		else
		{
			assert("Error: Could not open shader file \n");
			return VK_NULL_HANDLE;
		}
	}

};//£¡namespace VulkanCore