#pragma once
#include "VulkanResource.h"


//Dynamic buffer is used when data is coherent between cpu and gpu such as uniform buffers.
class KEVulkanDynamicBuffer :public KEVulkanResource
{
public:

	KEVulkanDynamicBuffer(float p_data[4][4]) :
		KEVulkanResource(sizeof(float)* 16, reinterpret_cast<uint8_t*>(p_data)),
		m_allocation_info({})
	{
		VkBufferCreateInfo l_buffer_create_info = {};
		l_buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		l_buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		l_buffer_create_info.size = m_size;
		l_buffer_create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

		VmaAllocationCreateInfo l_vma_create_info = {};
		l_vma_create_info.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
		l_vma_create_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

		vmaCreateBuffer(VulkanCore::allocator, &l_buffer_create_info, &l_vma_create_info, &m_buffer, &m_allocation, &m_allocation_info);

		if (p_data) UpdateData(p_data);
	};

	KEVulkanDynamicBuffer(const uint64_t& p_size,  uint8_t* p_data = nullptr):
		KEVulkanResource(p_size, p_data),
		m_allocation_info({})
	{
		VkBufferCreateInfo l_buffer_create_info = {};
		l_buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		l_buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		l_buffer_create_info.size = p_size;
		l_buffer_create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

		VmaAllocationCreateInfo l_vma_create_info = {};
		l_vma_create_info.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
		l_vma_create_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
	
		vmaCreateBuffer(VulkanCore::allocator, &l_buffer_create_info, &l_vma_create_info, &m_buffer, &m_allocation,&m_allocation_info );
	
		if (p_data) UpdateData(p_data);
	};
	~KEVulkanDynamicBuffer() {
		vmaDestroyBuffer(VulkanCore::allocator, m_buffer, m_allocation);
	};

	__forceinline void UpdateData(void* src) const{
		memcpy(m_allocation_info.pMappedData, src, m_size);
	}

	VkBuffer m_buffer = VK_NULL_HANDLE;
	VkDescriptorSet m_desc_set = VK_NULL_HANDLE;

	VkDescriptorBufferInfo GetBufferInfo() const {
		VkDescriptorBufferInfo l_info;
		l_info.buffer = m_buffer;
		l_info.offset = 0;
		l_info.range = m_size;
		return l_info;
	};

protected:
	VmaAllocation m_allocation = nullptr;
	VmaAllocationInfo m_allocation_info;

};

using KEVulkanUniformBuffer = KEVulkanDynamicBuffer;

