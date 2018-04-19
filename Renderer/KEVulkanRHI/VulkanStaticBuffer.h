#pragma once
#include "VulkanResource.h"



//Static Buffer uses a staging buffer to upload data to vram


class KEVulkanStaticBuffer:public KEVulkanResource
{
public:

	KEVulkanStaticBuffer(const uint64_t& p_size,  uint8_t* p_data = nullptr):
	KEVulkanResource(p_size,p_data)
	{
		//Create Staging Buffer
		VmaAllocationCreateInfo l_static_buffer_alloc_info = {};
		l_static_buffer_alloc_info.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
		l_static_buffer_alloc_info.usage = VMA_MEMORY_USAGE_CPU_ONLY;

		VkBufferCreateInfo l_static_buffer_create_info = {};
		l_static_buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		l_static_buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		l_static_buffer_create_info.size = p_size;
		VmaAllocationInfo stagingVertexBufferAllocInfo = {};
		vmaCreateBuffer(VulkanCore::allocator, &l_static_buffer_create_info, &l_static_buffer_alloc_info, &m_staging_buffer, &m_staging_buffer_allocation, &stagingVertexBufferAllocInfo);

		//Create Vertex Buffer
		l_static_buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		l_static_buffer_alloc_info.flags = 0;
		l_static_buffer_alloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		vmaCreateBuffer(VulkanCore::allocator, &l_static_buffer_create_info, &l_static_buffer_alloc_info, &m_static_buffer, &m_static_buffer_allocation, nullptr);

		//Copy Data to Staging Buffer
		memcpy(stagingVertexBufferAllocInfo.pMappedData, p_data, p_size);

		//Copy Staging buffer to Static buffer
		VulkanCore::BeginCommandBuffer(VulkanCore::temp_command_buffer);
		VkBufferCopy vbCopyRegion = {};
		vbCopyRegion.srcOffset = 0;
		vbCopyRegion.dstOffset = 0;
		vbCopyRegion.size = p_size;
		vkCmdCopyBuffer(VulkanCore::temp_command_buffer, m_staging_buffer, m_static_buffer, 0, nullptr);
		VulkanCore::FlushCommandBuffer(VulkanCore::temp_command_buffer);

		vmaDestroyBuffer(VulkanCore::allocator, m_staging_buffer, m_staging_buffer_allocation);
	};

	
	~KEVulkanStaticBuffer() {
		vmaDestroyBuffer(VulkanCore::allocator, m_static_buffer, m_static_buffer_allocation);
	};

private:
	VkBuffer m_static_buffer = VK_NULL_HANDLE;
	VkBuffer m_staging_buffer = VK_NULL_HANDLE;
	VmaAllocation m_static_buffer_allocation = nullptr;
	VmaAllocation m_staging_buffer_allocation = nullptr;

};

