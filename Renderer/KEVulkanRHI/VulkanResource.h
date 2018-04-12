#pragma once
#include <stdint.h>
#include <memory>
#include "vk_mem_alloc.h"
class KEVulkanResource
{
public:
	KEVulkanResource(VkDevice p_device,VmaAllocator* p_allocator, const uint64_t& p_size = 0, uint8_t* p_data=nullptr) :
		m_size(p_size),
		m_data(p_data),
		m_allocator(p_allocator),
		m_device(p_device)
	{

	};

	virtual ~KEVulkanResource() {
		if (m_data) {
			delete[] m_data;
			m_data = nullptr;
		}
	};

	virtual void Write(void* p_src, uint64_t p_size) {
		memmove(m_data, p_src, static_cast<uint64_t>(p_size));
	}

	virtual void Read() {
	
	}

protected:
	uint64_t m_size;
	uint8_t* m_data;
	VmaAllocator* m_allocator;
	VkDevice m_device;
};

